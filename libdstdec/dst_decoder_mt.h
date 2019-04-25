/*
    Copyright 2015 Robert Tari <robert@tari.in>
    Copyright 2011-2014 Maxim V.Anisiutkin <maxim.anisiutkin@gmail.com>

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

#ifndef _DST_DECODER_H_INCLUDED
#define _DST_DECODER_H_INCLUDED

#include <pthread.h>
#include "dst_decoder.h"

enum slot_state_t {SLOT_EMPTY, SLOT_LOADED, SLOT_RUNNING, SLOT_READY, SLOT_READY_WITH_ERROR, SLOT_TERMINATING};

class frame_slot_t
{
    public:

        volatile int state;
        int frame_nr;
        uint8_t* dsd_data;
        int dsd_size;
        uint8_t* dst_data;
        int dst_size;
        int channel_count;
        int samplerate;
        int framerate;
        pthread_t hThread;
        pthread_cond_t hEventGet;
        pthread_cond_t hEventPut;
        pthread_mutex_t hMutex;
        CDSTDecoder D;

        frame_slot_t()
        {
            state = SLOT_EMPTY;
            dsd_data = nullptr;
            dsd_size = 0;
            dst_data = nullptr;
            dst_size = 0;
            channel_count = 0;
            samplerate = 0;
            framerate = 0;
            frame_nr = 0;
        }
};

class dst_decoder_t
{
    frame_slot_t* frame_slots;
    int thread_count;
    int channel_count;
    int samplerate;
    int framerate;
    uint32_t frame_nr;

public:

    int slot_nr;

    dst_decoder_t(int threads);
    ~dst_decoder_t();
    int init(int channel_count, int samplerate, int framerate);
    int decode(uint8_t* dst_data, size_t dst_size, uint8_t** dsd_data, size_t* dsd_size);
};

#endif
