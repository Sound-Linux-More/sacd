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

#pragma once

class PCMPCMFir
{
    double* fir_coefs;
    int fir_order;
    int fir_length;
    int decimation;
    double* fir_buffer;
    int fir_index;

public:

    PCMPCMFir()
    {
        fir_coefs = nullptr;
        fir_order = 0;
        fir_length = 0;
        decimation = 0;
        fir_buffer = nullptr;
        fir_index = 0;
    }

    ~PCMPCMFir()
    {
        free();
    }

    void init(double* fir_coefs, int fir_length, int decimation)
    {
        this->fir_coefs = fir_coefs;
        this->fir_order = fir_length - 1;
        this->fir_length = fir_length;
        this->decimation = decimation;
        int buf_size = 2 * this->fir_length * sizeof(double);
        this->fir_buffer = (double*)DSDPCMUtil::mem_alloc(buf_size);
        memset(this->fir_buffer, 0, buf_size);
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

    int get_decimation()
    {
        return decimation;
    }

    float get_delay()
    {
        return (float)fir_order / 2 / decimation;
    }

    int run(double* pcm_data, double* out_data, int pcm_samples)
    {
        int out_samples = pcm_samples / decimation;

        for (int sample = 0; sample < out_samples; sample++)
        {
            for (int i = 0; i < decimation; i++)
            {
                fir_buffer[fir_index + fir_length] = fir_buffer[fir_index] = *(pcm_data++);
                fir_index = fir_index + 1;
                fir_index = fir_index % fir_length;
            }

            out_data[sample] = (double)0;

            for (int j = 0; j < fir_length; j++)
            {
                out_data[sample] += fir_coefs[j] * fir_buffer[fir_index + j];
            }
        }

        return out_samples;
    }
};
