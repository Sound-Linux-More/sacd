/*
    Copyright (c) 2015-2016 Robert Tari <robert.tari@gmail.com>
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

#pragma once

#include <pthread.h>
#include "dsd_pcm_converter_multistage.h"

enum pcm_slot_state_t {PCM_SLOT_EMPTY, PCM_SLOT_LOADED, PCM_SLOT_RUNNING, PCM_SLOT_READY, PCM_SLOT_TERMINATING};

class DSDPCMConverterSlot
{
public:

    uint8_t* dsd_data;
    int dsd_samples;
    double* pcm_data;
    int pcm_samples;
    DSDPCMConverter* converter;
    pthread_t hThread;
    pthread_cond_t hEventGet;
    pthread_cond_t hEventPut;
    pthread_mutex_t hMutex;
    volatile int state;

    DSDPCMConverterSlot()
    {
        state = PCM_SLOT_EMPTY;
        dsd_data = nullptr;
        dsd_samples = 0;
        pcm_data = nullptr;
        pcm_samples = 0;
        converter = nullptr;
    }
};

class DSDPCMConverterEngine
{

public:

    DSDPCMConverterEngine();
    ~DSDPCMConverterEngine();
    float get_delay();
    bool is_convert_called();
    int init(int channels, int framerate, int dsd_samplerate, int pcm_samplerate);
    int free();
    int convert(uint8_t* dsd_data, int dsd_samples, float* pcm_data);

private:

    int channels;
    int framerate;
    int dsd_samplerate;
    int pcm_samplerate;
    float conv_delay;
    bool conv_fp64;
    bool conv_called;
    DSDPCMFilterSetup fltSetup_fp64;
    DSDPCMConverterSlot* convSlots_fp64;
    uint8_t swap_bits[256];

    DSDPCMConverterSlot* init_slots(DSDPCMFilterSetup& fltSetup);
    void free_slots(DSDPCMConverterSlot* convSlots);
    int convert(DSDPCMConverterSlot* convSlots, uint8_t* dsd_data, int dsd_samples, float* pcm_data);
    int convertL(DSDPCMConverterSlot* convSlots, uint8_t* dsd_data, int dsd_samples);
    int convertR(DSDPCMConverterSlot* convSlots, float* pcm_data);
};
