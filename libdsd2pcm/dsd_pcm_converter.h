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

#include "dsd_pcm_filter_setup.h"
#include "dsd_pcm_fir.h"
#include "pcm_pcm_fir.h"

enum conv_type_e
{
    DSDPCM_CONV_UNKNOWN    = -1,
    DSDPCM_CONV_MULTISTAGE =  0,
    DSDPCM_CONV_DIRECT     =  1,
    DSDPCM_CONV_USER       =  2
};

enum declick_side_e
{
    DECLICK_NONE = 0,
    DECLICK_LEFT = 1,
    DECLICK_RIGHT = 2
};

class DSDPCMConverter
{
protected:

    int framerate;
    int dsd_samplerate;
    int pcm_samplerate;
    float delay;
    double* pcm_temp1;
    double* pcm_temp2;

public:

    DSDPCMConverter()
    {
        pcm_temp1 = nullptr;
        pcm_temp2 = nullptr;
    }

    virtual ~DSDPCMConverter()
    {
        free_pcm_temp1();
        free_pcm_temp2();
    }

    float get_delay()
    {
        return delay;
    }

    virtual void init(DSDPCMFilterSetup& flt_setup, int dsd_samples) = 0;
    virtual int convert(uint8_t* dsd_data, double* pcm_data, int dsd_samples) = 0;

protected:

    void alloc_pcm_temp1(int pcm_samples)
    {
        free_pcm_temp1();
        pcm_temp1 = (double*)DSDPCMUtil::mem_alloc(pcm_samples * sizeof(double));
    }

    void alloc_pcm_temp2(int pcm_samples)
    {
        free_pcm_temp2();
        pcm_temp2 = (double*)DSDPCMUtil::mem_alloc(pcm_samples * sizeof(double));
    }

    void free_pcm_temp1()
    {
        DSDPCMUtil::mem_free(pcm_temp1);
        pcm_temp1 = nullptr;
    }

    void free_pcm_temp2()
    {
        DSDPCMUtil::mem_free(pcm_temp2);
        pcm_temp2 = nullptr;
    }
};
