/*
    Copyright (c) 2015-2016 Robert Tari <robert@tari.in>
    Copyright (c) 2011-2015 Maxim V.Anisiutkin <maxim.anisiutkin@gmail.com>

    This file is part of SACD.

    SACD is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SACD is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SACD.  If not, see <http://www.gnu.org/licenses/gpl-3.0.txt>.
*/

#include "dsd_pcm_converter_engine.h"

static void* ConverterThread(void* threadarg)
{
    DSDPCMConverterSlot* slot = reinterpret_cast<DSDPCMConverterSlot*>(threadarg);

    while (1)
    {
        pthread_mutex_lock(&slot->hMutex);

        while (slot->state != PCM_SLOT_LOADED && slot->state != PCM_SLOT_TERMINATING)
        {
            pthread_cond_wait(&slot->hEventPut, &slot->hMutex);
        }

        if (slot->state == PCM_SLOT_TERMINATING)
        {
            slot->pcm_samples = 0;
            pthread_mutex_unlock(&slot->hMutex);
            return 0;
        }

        slot->state = PCM_SLOT_RUNNING;
        pthread_mutex_unlock(&slot->hMutex);

        slot->pcm_samples = slot->converter->convert(slot->dsd_data, slot->pcm_data, slot->dsd_samples);

        pthread_mutex_lock(&slot->hMutex);
        slot->state = PCM_SLOT_READY;
        pthread_cond_signal(&slot->hEventGet);
        pthread_mutex_unlock(&slot->hMutex);
    }

    return 0;
}

DSDPCMConverterEngine::DSDPCMConverterEngine()
{
    channels = 0;
    framerate = 0;
    dsd_samplerate = 0;
    pcm_samplerate = 0;
    conv_delay = 0.0f;
    convSlots_fp64 = nullptr;
    conv_called = false;

    for (int i = 0; i < 256; i++)
    {
        swap_bits[i] = 0;

        for (int j = 0; j < 8; j++)
        {
            swap_bits[i] |= ((i >> j) & 1) << (7 - j);
        }
    }
}

DSDPCMConverterEngine::~DSDPCMConverterEngine()
{
    free();

    conv_delay = 0.0f;
}

float DSDPCMConverterEngine::get_delay()
{
    return conv_delay;
}

bool DSDPCMConverterEngine::is_convert_called()
{
    return conv_called;
}

int DSDPCMConverterEngine::init(int channels, int framerate, int dsd_samplerate, int pcm_samplerate)
{
    free();

    this->channels = channels;
    this->framerate = framerate;
    this->dsd_samplerate = dsd_samplerate;
    this->pcm_samplerate = pcm_samplerate;
    convSlots_fp64 = init_slots(fltSetup_fp64);
    conv_delay = convSlots_fp64[0].converter->get_delay();
    conv_called = false;

    return 0;
}

int DSDPCMConverterEngine::free()
{
    if (convSlots_fp64)
    {
        free_slots(convSlots_fp64);
        convSlots_fp64 = nullptr;
    }

    return 0;
}

int DSDPCMConverterEngine::convert(uint8_t* dsd_data, int dsd_samples, float* pcm_data)
{
    int pcm_samples = 0;

    if (!dsd_data)
    {
        if (convSlots_fp64)
        {
            pcm_samples = convertR(convSlots_fp64, pcm_data);
        }

        return pcm_samples;
    }

    if (!conv_called)
    {
        if (convSlots_fp64)
        {
            convertL(convSlots_fp64, dsd_data, dsd_samples);
        }

        conv_called = true;
    }

    if (convSlots_fp64)
    {
        pcm_samples = convert(convSlots_fp64, dsd_data, dsd_samples, pcm_data);
    }

    return pcm_samples;
}

DSDPCMConverterSlot* DSDPCMConverterEngine::init_slots(DSDPCMFilterSetup& fltSetup)
{
    DSDPCMConverterSlot* convSlots = new DSDPCMConverterSlot[channels];

    int dsd_samples = dsd_samplerate / 8 / framerate;
    int pcm_samples = pcm_samplerate / framerate;
    int decimation = dsd_samplerate / pcm_samplerate;

    for (int ch = 0; ch < channels; ch++)
    {
        DSDPCMConverterSlot* slot = &convSlots[ch];
        slot->dsd_data = (uint8_t*)DSDPCMUtil::mem_alloc(dsd_samples * sizeof(uint8_t));
        slot->dsd_samples = dsd_samples;
        slot->pcm_data = (double*)DSDPCMUtil::mem_alloc(pcm_samples * sizeof(double));
        slot->pcm_samples = 0;

        DSDPCMConverterMultistage* pConv = nullptr;

        switch (decimation)
        {
            case 512:
                pConv = new DSDPCMConverterMultistage_x512();
                break;
            case 256:
                pConv = new DSDPCMConverterMultistage_x256();
                break;
            case 128:
                pConv = new DSDPCMConverterMultistage_x128();
                break;
            case 64:
                pConv = new DSDPCMConverterMultistage_x64();
                break;
            case 32:
                pConv = new DSDPCMConverterMultistage_x32();
                break;
            case 16:
                pConv = new DSDPCMConverterMultistage_x16();
                break;
            case 8:
                pConv = new DSDPCMConverterMultistage_x8();
                break;
        }

        pConv->init(fltSetup, dsd_samples);
        slot->converter = pConv;

        pthread_mutex_init(&slot->hMutex, NULL);
        pthread_cond_init(&slot->hEventGet, NULL);
        pthread_cond_init(&slot->hEventPut, NULL);
        pthread_create(&slot->hThread, NULL, ConverterThread, slot);
    }

    return convSlots;
}

void DSDPCMConverterEngine::free_slots(DSDPCMConverterSlot* convSlots)
{
    for (int ch = 0; ch < channels; ch++)
    {
        DSDPCMConverterSlot* slot = &convSlots[ch];

        // Release worker (decoding) thread for exit
        pthread_mutex_lock(&slot->hMutex);
        slot->state = PCM_SLOT_TERMINATING;
        pthread_cond_signal(&slot->hEventPut);
        pthread_mutex_unlock(&slot->hMutex);

        // Wait until worker (decoding) thread exit
        pthread_join(slot->hThread, NULL);
        pthread_cond_destroy(&slot->hEventGet);
        pthread_cond_destroy(&slot->hEventPut);
        pthread_mutex_destroy(&slot->hMutex);

        delete slot->converter;
        slot->converter = nullptr;
        DSDPCMUtil::mem_free(slot->dsd_data);
        slot->dsd_data = nullptr;
        slot->dsd_samples = 0;
        DSDPCMUtil::mem_free(slot->pcm_data);
        slot->pcm_data = nullptr;
        slot->pcm_samples = 0;
    }

    delete[] convSlots;
    convSlots = nullptr;
}

int DSDPCMConverterEngine::convert(DSDPCMConverterSlot* convSlots, uint8_t* dsd_data, int dsd_samples, float* pcm_data)
{
    int pcm_samples = 0;

    for (int ch = 0; ch < channels; ch++)
    {
        DSDPCMConverterSlot* slot = &convSlots[ch];
        slot->dsd_samples = dsd_samples / channels;

        for (int sample = 0; sample < slot->dsd_samples; sample++)
        {
            slot->dsd_data[sample] = dsd_data[sample * channels + ch];
        }

        // Release worker (decoding) thread on the loaded slot
        pthread_mutex_lock(&slot->hMutex);
        slot->state = PCM_SLOT_LOADED;
        pthread_cond_signal(&slot->hEventPut);
        pthread_mutex_unlock(&slot->hMutex);
    }

    for (int ch = 0; ch < channels; ch++)
    {
        DSDPCMConverterSlot* slot = &convSlots[ch];

        // Wait until worker (decoding) thread is complete
        pthread_mutex_lock(&slot->hMutex);

        while (slot->state != PCM_SLOT_READY)
        {
            pthread_cond_wait(&slot->hEventGet, &slot->hMutex);
        }

        pthread_mutex_unlock(&slot->hMutex);

        for (int sample = 0; sample < slot->pcm_samples; sample++)
        {
            pcm_data[sample * channels + ch] = (float)slot->pcm_data[sample];
        }

        pcm_samples += slot->pcm_samples;
    }

    return pcm_samples;
}

int DSDPCMConverterEngine::convertL(DSDPCMConverterSlot* convSlots, uint8_t* dsd_data, int dsd_samples)
{
    for (int ch = 0; ch < channels; ch++)
    {
        DSDPCMConverterSlot* slot = &convSlots[ch];

        slot->dsd_samples = dsd_samples / channels;

        for (int sample = 0; sample < slot->dsd_samples; sample++)
        {
            slot->dsd_data[sample] = swap_bits[dsd_data[(slot->dsd_samples - 1 - sample) * channels + ch]];
        }

        // Release worker (decoding) thread on the loaded slot
        pthread_mutex_lock(&convSlots[ch].hMutex);
        convSlots[ch].state = PCM_SLOT_LOADED;
        pthread_cond_signal(&convSlots[ch].hEventPut);
        pthread_mutex_unlock(&convSlots[ch].hMutex);
    }

    for (int ch = 0; ch < channels; ch++)
    {
        DSDPCMConverterSlot* slot = &convSlots[ch];

        // Wait until worker (decoding) thread is complete
        pthread_mutex_lock(&slot->hMutex);

        while (slot->state != PCM_SLOT_READY)
        {
            pthread_cond_wait(&slot->hEventGet, &slot->hMutex);
        }

        pthread_mutex_unlock(&slot->hMutex);
    }

    return 0;
}

int DSDPCMConverterEngine::convertR(DSDPCMConverterSlot* convSlots, float* pcm_data)
{
    int pcm_samples = 0;

    for (int ch = 0; ch < channels; ch++)
    {
        DSDPCMConverterSlot* slot = &convSlots[ch];

        for (int sample = 0; sample < slot->dsd_samples / 2; sample++)
        {
            uint8_t temp = slot->dsd_data[slot->dsd_samples - 1 - sample];
            slot->dsd_data[slot->dsd_samples - 1 - sample] = swap_bits[slot->dsd_data[sample]];
            slot->dsd_data[sample] = swap_bits[temp];
        }

        // Release worker (decoding) thread on the loaded slot
        pthread_mutex_lock(&convSlots[ch].hMutex);
        convSlots[ch].state = PCM_SLOT_LOADED;
        pthread_cond_signal(&convSlots[ch].hEventPut);
        pthread_mutex_unlock(&convSlots[ch].hMutex);
    }

    for (int ch = 0; ch < channels; ch++)
    {
        DSDPCMConverterSlot* slot = &convSlots[ch];

        // Wait until worker (decoding) thread is complete
        pthread_mutex_lock(&slot->hMutex);

        while (slot->state != PCM_SLOT_READY)
        {
            pthread_cond_wait(&slot->hEventGet, &slot->hMutex);
        }

        pthread_mutex_unlock(&slot->hMutex);

        for (int sample = 0; sample < slot->pcm_samples; sample++)
        {
            pcm_data[sample * channels + ch] = (float)slot->pcm_data[sample];
        }

        pcm_samples += slot->pcm_samples;
    }

    return pcm_samples;
}
