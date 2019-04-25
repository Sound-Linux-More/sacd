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

class DSDPCMFir
{
    using ctable_t = double[256];
    ctable_t* fir_ctables;
    int fir_order;
    int fir_length;
    int decimation;
    uint8_t*  fir_buffer;
    int fir_index;

public:

    DSDPCMFir()
    {
        fir_ctables = nullptr;
        fir_order = 0;
        fir_length = 0;
        decimation = 0;
        fir_buffer = nullptr;
        fir_index = 0;
    }

    ~DSDPCMFir()
    {
        free();
    }

    void init(ctable_t* fir_ctables, int fir_length, int decimation)
    {
        this->fir_ctables = fir_ctables;
        this->fir_order = fir_length - 1;
        this->fir_length = CTABLES(fir_length);
        this->decimation = decimation / 8;
        int buf_size = 2 * this->fir_length * sizeof(uint8_t);
        this->fir_buffer = (uint8_t*)DSDPCMUtil::mem_alloc(buf_size);
        memset(this->fir_buffer, DSD_SILENCE_BYTE, buf_size);
        fir_index = 0;
    }

    void free()
    {
        if (fir_buffer)
        {
            DSDPCMUtil::mem_free(fir_buffer);
            fir_buffer = nullptr;
        }
    }

    int get_decimation() {
        return decimation;
    }

    float get_delay()
    {
        return (float)fir_order / 2 / 8 / decimation;
    }

    int run(uint8_t* dsd_data, double* pcm_data, int dsd_samples)
    {
        int pcm_samples = dsd_samples / decimation;

        for (int sample = 0; sample < pcm_samples; sample++)
        {
            for (int i = 0; i < decimation; i++)
            {
                fir_buffer[fir_index + fir_length] = fir_buffer[fir_index] = *(dsd_data++);
                fir_index = fir_index + 1;
                fir_index = fir_index % fir_length;
            }

            pcm_data[sample] = (double)0;

            for (int j = 0; j < fir_length; j++)
            {
                pcm_data[sample] += fir_ctables[j][fir_buffer[fir_index + j]];
            }
        }

        return pcm_samples;
    }
};
