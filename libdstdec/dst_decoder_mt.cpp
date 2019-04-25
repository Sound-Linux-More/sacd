/*
    Copyright 2015-2016 Robert Tari <robert@tari.in>
    Copyright 2011-2015 Maxim V.Anisiutkin <maxim.anisiutkin@gmail.com>

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

#include "dst_decoder_mt.h"

#define DSD_SILENCE_BYTE 0x69

void* DSTDecoderThread(void* threadarg)
{
    frame_slot_t* frame_slot = (frame_slot_t*)threadarg;

    while (1)
    {
        pthread_mutex_lock(&frame_slot->hMutex);

        while (frame_slot->state != SLOT_LOADED && frame_slot->state != SLOT_TERMINATING)
        {
            pthread_cond_wait(&frame_slot->hEventPut, &frame_slot->hMutex);
        }

        if (frame_slot->state == SLOT_TERMINATING)
        {
            frame_slot->dsd_data = nullptr;
            frame_slot->dst_size = 0;
            pthread_mutex_unlock(&frame_slot->hMutex);
            return 0;
        }

        frame_slot->state = SLOT_RUNNING;
        pthread_mutex_unlock(&frame_slot->hMutex);

        bool bError = false;

        try
        {
            frame_slot->D.decode(frame_slot->dst_data, frame_slot->dst_size * 8, frame_slot->dsd_data);
        }
        catch (...)
        {
            bError = true;
            frame_slot->D.close();
            frame_slot->D.init(frame_slot->channel_count, frame_slot->samplerate / 44100);
        }

        pthread_mutex_lock(&frame_slot->hMutex);
        frame_slot->state = bError ? SLOT_READY_WITH_ERROR : SLOT_READY;
        pthread_cond_signal(&frame_slot->hEventGet);
        pthread_mutex_unlock(&frame_slot->hMutex);
    }

    return 0;
}

dst_decoder_t::dst_decoder_t(int threads)
{
    thread_count = threads;

    frame_slots = new frame_slot_t[thread_count];

    if (!frame_slots)
    {
        thread_count = 0;
    }

    channel_count = 0;
    samplerate = 0;
    framerate = 0;
    slot_nr = 0;
}

dst_decoder_t::~dst_decoder_t()
{
    for (int i = 0; i < thread_count; i++)
    {
        frame_slot_t* frame_slot = &frame_slots[i];

        // Release worker (decoding) thread for exit
        pthread_mutex_lock(&frame_slot->hMutex);
        frame_slot->state = SLOT_TERMINATING;
        frame_slot->D.close();
        pthread_cond_signal(&frame_slot->hEventPut);
        pthread_mutex_unlock(&frame_slot->hMutex);

        // Wait until worker (decoding) thread exit
        pthread_join(frame_slot->hThread, NULL);
        pthread_cond_destroy(&frame_slot->hEventGet);
        pthread_cond_destroy(&frame_slot->hEventPut);
        pthread_mutex_destroy(&frame_slot->hMutex);
    }

    delete[] frame_slots;
}

int dst_decoder_t::init(int channel_count, int samplerate, int framerate)
{
    for (int i = 0; i < thread_count; i++)
    {
        frame_slot_t* frame_slot = &frame_slots[i];

        if (frame_slot->D.init(channel_count, (samplerate / 44100) / (framerate / 75)) == 0)
        {
            frame_slot->channel_count = channel_count;
            frame_slot->samplerate = samplerate;
            frame_slot->framerate = framerate;
            frame_slot->dsd_size = (size_t)(samplerate / 8 / framerate * channel_count);
            pthread_mutex_init(&frame_slot->hMutex, NULL);
            pthread_cond_init(&frame_slot->hEventGet, NULL);
            pthread_cond_init(&frame_slot->hEventPut, NULL);
        }
        else
        {
            return -1;
        }

        pthread_create(&frame_slot->hThread, NULL, DSTDecoderThread, frame_slot);
    }

    this->channel_count = channel_count;
    this->samplerate = samplerate;
    this->framerate = framerate;
    this->frame_nr = 0;

    return 0;
}

int dst_decoder_t::decode(uint8_t* dst_data, size_t dst_size, uint8_t** dsd_data, size_t* dsd_size)
{
    // Get current slot
    frame_slot_t* frame_slot = &frame_slots[slot_nr];

    // Allocate encoded frame into the slot
    frame_slot->dsd_data = *dsd_data;
    frame_slot->dst_data = dst_data;
    frame_slot->dst_size = dst_size;
    frame_slot->frame_nr = frame_nr;

    // Release worker (decoding) thread on the loaded slot
    if (dst_size > 0)
    {
        pthread_mutex_lock(&frame_slot->hMutex);
        frame_slot->state = SLOT_LOADED;
        pthread_cond_signal(&frame_slot->hEventPut);
        pthread_mutex_unlock(&frame_slot->hMutex);
    }
    else
    {
        frame_slot->state = SLOT_EMPTY;
    }

    // Advance to the next slot
    slot_nr = (slot_nr + 1) % thread_count;
    frame_slot = &frame_slots[slot_nr];

    // Dump decoded frame
    if (frame_slot->state != SLOT_EMPTY)
    {
        pthread_mutex_lock(&frame_slot->hMutex);

        while (frame_slot->state != SLOT_READY)
        {
            pthread_cond_wait(&frame_slot->hEventGet, &frame_slot->hMutex);
        }

        pthread_mutex_unlock(&frame_slot->hMutex);
    }

    switch (frame_slot->state)
    {
        case SLOT_READY:
            *dsd_data = frame_slot->dsd_data;
            *dsd_size = (size_t)(samplerate / 8 / framerate * channel_count);
            break;
        case SLOT_READY_WITH_ERROR:
            *dsd_data = frame_slot->dsd_data;
            *dsd_size = (size_t)(samplerate / 8 / framerate * channel_count);
            memset(*dsd_data, DSD_SILENCE_BYTE, *dsd_size);
            break;
        default:
            *dsd_data = nullptr;
            *dsd_size = 0;
            break;
    }

    frame_nr++;

    return 0;
}
