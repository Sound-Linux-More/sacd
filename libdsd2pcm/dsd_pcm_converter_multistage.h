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

#include "dsd_pcm_converter.h"

class DSDPCMConverterMultistage : public DSDPCMConverter
{
};

class DSDPCMConverterMultistage_x512 : public DSDPCMConverterMultistage
{
    DSDPCMFir dsd_fir1;
    PCMPCMFir pcm_fir2a;
    PCMPCMFir pcm_fir2b;
    PCMPCMFir pcm_fir2c;
    PCMPCMFir pcm_fir2d;
    PCMPCMFir pcm_fir3;

public:

    void init(DSDPCMFilterSetup& flt_setup, int dsd_samples)
    {
        alloc_pcm_temp1(dsd_samples / 2);
        alloc_pcm_temp2(dsd_samples / 4);
        dsd_fir1.init(flt_setup.get_fir1_16_ctables(), flt_setup.get_fir1_16_length(), 16);
        pcm_fir2a.init(flt_setup.get_fir2_2_coefs(), flt_setup.get_fir2_2_length(), 2);
        pcm_fir2b.init(flt_setup.get_fir2_2_coefs(), flt_setup.get_fir2_2_length(), 2);
        pcm_fir2c.init(flt_setup.get_fir2_2_coefs(), flt_setup.get_fir2_2_length(), 2);
        pcm_fir2d.init(flt_setup.get_fir2_2_coefs(), flt_setup.get_fir2_2_length(), 2);
        pcm_fir3.init(flt_setup.get_fir3_2_coefs(), flt_setup.get_fir3_2_length(), 2);
        delay = (((dsd_fir1.get_delay() / pcm_fir2a.get_decimation() + pcm_fir2a.get_delay()) / pcm_fir2b.get_decimation() + pcm_fir2b.get_delay()) / pcm_fir2c.get_decimation() + pcm_fir2c.get_delay()) / pcm_fir3.get_decimation() + pcm_fir3.get_delay();
    }

    int convert(uint8_t* dsd_data, double* pcm_data, int dsd_samples)
    {
        int pcm_samples;
        pcm_samples = dsd_fir1.run(dsd_data, pcm_temp1, dsd_samples);
        pcm_samples = pcm_fir2a.run(pcm_temp1, pcm_temp2, pcm_samples);
        pcm_samples = pcm_fir2b.run(pcm_temp2, pcm_temp1, pcm_samples);
        pcm_samples = pcm_fir2c.run(pcm_temp1, pcm_temp2, pcm_samples);
        pcm_samples = pcm_fir2d.run(pcm_temp2, pcm_temp1, pcm_samples);
        pcm_samples = pcm_fir3.run(pcm_temp1, pcm_data, pcm_samples);

        return pcm_samples;
    }
};

class DSDPCMConverterMultistage_x256 : public DSDPCMConverterMultistage
{
    DSDPCMFir dsd_fir1;
    PCMPCMFir pcm_fir2a;
    PCMPCMFir pcm_fir2b;
    PCMPCMFir pcm_fir2c;
    PCMPCMFir pcm_fir3;

public:

    void init(DSDPCMFilterSetup& flt_setup, int dsd_samples)
    {
        alloc_pcm_temp1(dsd_samples / 2);
        alloc_pcm_temp2(dsd_samples / 4);
        dsd_fir1.init(flt_setup.get_fir1_16_ctables(), flt_setup.get_fir1_16_length(), 16);
        pcm_fir2a.init(flt_setup.get_fir2_2_coefs(), flt_setup.get_fir2_2_length(), 2);
        pcm_fir2b.init(flt_setup.get_fir2_2_coefs(), flt_setup.get_fir2_2_length(), 2);
        pcm_fir2c.init(flt_setup.get_fir2_2_coefs(), flt_setup.get_fir2_2_length(), 2);
        pcm_fir3.init(flt_setup.get_fir3_2_coefs(), flt_setup.get_fir3_2_length(), 2);
        delay = (((dsd_fir1.get_delay() / pcm_fir2a.get_decimation() + pcm_fir2a.get_delay()) / pcm_fir2b.get_decimation() + pcm_fir2b.get_delay()) / pcm_fir2c.get_decimation() + pcm_fir2c.get_delay()) / pcm_fir3.get_decimation() + pcm_fir3.get_delay();
    }

    int convert(uint8_t* dsd_data, double* pcm_data, int dsd_samples)
    {
        int pcm_samples;
        pcm_samples = dsd_fir1.run(dsd_data, pcm_temp1, dsd_samples);
        pcm_samples = pcm_fir2a.run(pcm_temp1, pcm_temp2, pcm_samples);
        pcm_samples = pcm_fir2b.run(pcm_temp2, pcm_temp1, pcm_samples);
        pcm_samples = pcm_fir2c.run(pcm_temp1, pcm_temp2, pcm_samples);
        pcm_samples = pcm_fir3.run(pcm_temp2, pcm_data, pcm_samples);

        return pcm_samples;
    }
};

class DSDPCMConverterMultistage_x128 : public DSDPCMConverterMultistage
{
    DSDPCMFir dsd_fir1;
    PCMPCMFir pcm_fir2a;
    PCMPCMFir pcm_fir2b;
    PCMPCMFir pcm_fir3;

public:

    void init(DSDPCMFilterSetup& flt_setup, int dsd_samples)
    {
        alloc_pcm_temp1(dsd_samples / 2);
        alloc_pcm_temp2(dsd_samples / 4);
        dsd_fir1.init(flt_setup.get_fir1_16_ctables(), flt_setup.get_fir1_16_length(), 16);
        pcm_fir2a.init(flt_setup.get_fir2_2_coefs(), flt_setup.get_fir2_2_length(), 2);
        pcm_fir2b.init(flt_setup.get_fir2_2_coefs(), flt_setup.get_fir2_2_length(), 2);
        pcm_fir3.init(flt_setup.get_fir3_2_coefs(), flt_setup.get_fir3_2_length(), 2);
        delay = ((dsd_fir1.get_delay() / pcm_fir2a.get_decimation() + pcm_fir2a.get_delay()) / pcm_fir2b.get_decimation() + pcm_fir2b.get_delay()) / pcm_fir3.get_decimation() + pcm_fir3.get_delay();
    }

    int convert(uint8_t* dsd_data, double* pcm_data, int dsd_samples)
    {
        int pcm_samples;
        pcm_samples = dsd_fir1.run(dsd_data, pcm_temp1, dsd_samples);
        pcm_samples = pcm_fir2a.run(pcm_temp1, pcm_temp2, pcm_samples);
        pcm_samples = pcm_fir2b.run(pcm_temp2, pcm_temp1, pcm_samples);
        pcm_samples = pcm_fir3.run(pcm_temp1, pcm_data, pcm_samples);

        return pcm_samples;
    }
};

class DSDPCMConverterMultistage_x64 : public DSDPCMConverterMultistage
{
    DSDPCMFir dsd_fir1;
    PCMPCMFir pcm_fir2a;
    PCMPCMFir pcm_fir3;

public:

    void init(DSDPCMFilterSetup& flt_setup, int dsd_samples)
    {
        alloc_pcm_temp1(dsd_samples / 2);
        alloc_pcm_temp2(dsd_samples / 4);
        dsd_fir1.init(flt_setup.get_fir1_16_ctables(), flt_setup.get_fir1_16_length(), 16);
        pcm_fir2a.init(flt_setup.get_fir2_2_coefs(), flt_setup.get_fir2_2_length(), 2);
        pcm_fir3.init(flt_setup.get_fir3_2_coefs(), flt_setup.get_fir3_2_length(), 2);
        delay = (dsd_fir1.get_delay() / pcm_fir2a.get_decimation() + pcm_fir2a.get_delay()) / pcm_fir3.get_decimation() + pcm_fir3.get_delay();
    }

    int convert(uint8_t* dsd_data, double* pcm_data, int dsd_samples)
    {
        int pcm_samples;
        pcm_samples = dsd_fir1.run(dsd_data, pcm_temp1, dsd_samples);
        pcm_samples = pcm_fir2a.run(pcm_temp1, pcm_temp2, pcm_samples);
        pcm_samples = pcm_fir3.run(pcm_temp2, pcm_data, pcm_samples);

        return pcm_samples;
    }
};

class DSDPCMConverterMultistage_x32 : public DSDPCMConverterMultistage
{
    DSDPCMFir dsd_fir1;
    PCMPCMFir pcm_fir2a;
    PCMPCMFir pcm_fir3;

public:

    void init(DSDPCMFilterSetup& flt_setup, int dsd_samples)
    {
        alloc_pcm_temp1(dsd_samples);
        alloc_pcm_temp2(dsd_samples / 2);
        dsd_fir1.init(flt_setup.get_fir1_8_ctables(), flt_setup.get_fir1_8_length(), 8);
        pcm_fir2a.init(flt_setup.get_fir2_2_coefs(), flt_setup.get_fir2_2_length(), 2);
        pcm_fir3.init(flt_setup.get_fir3_2_coefs(), flt_setup.get_fir3_2_length(), 2);
        delay = (dsd_fir1.get_delay() / pcm_fir2a.get_decimation() + pcm_fir2a.get_delay()) / pcm_fir3.get_decimation() + pcm_fir3.get_delay();
    }

    int convert(uint8_t* dsd_data, double* pcm_data, int dsd_samples)
    {
        int pcm_samples;
        pcm_samples = dsd_fir1.run(dsd_data, pcm_temp1, dsd_samples);
        pcm_samples = pcm_fir2a.run(pcm_temp1, pcm_temp2, pcm_samples);
        pcm_samples = pcm_fir3.run(pcm_temp2, pcm_data, pcm_samples);

        return pcm_samples;
    }
};

class DSDPCMConverterMultistage_x16 : public DSDPCMConverterMultistage
{
    DSDPCMFir dsd_fir1;
    PCMPCMFir pcm_fir3;

public:

    void init(DSDPCMFilterSetup& flt_setup, int dsd_samples)
    {
        alloc_pcm_temp1(dsd_samples);
        dsd_fir1.init(flt_setup.get_fir1_8_ctables(), flt_setup.get_fir1_8_length(), 8);
        pcm_fir3.init(flt_setup.get_fir3_2_coefs(), flt_setup.get_fir3_2_length(), 2);
        delay = dsd_fir1.get_delay() / pcm_fir3.get_decimation() + pcm_fir3.get_delay();
    }

    int convert(uint8_t* dsd_data, double* pcm_data, int dsd_samples)
    {
        int pcm_samples;
        pcm_samples = dsd_fir1.run(dsd_data, pcm_temp1, dsd_samples);
        pcm_samples = pcm_fir3.run(pcm_temp1, pcm_data, pcm_samples);

        return pcm_samples;
    }
};

class DSDPCMConverterMultistage_x8 : public DSDPCMConverterMultistage
{
    DSDPCMFir dsd_fir1;

public:

    void init(DSDPCMFilterSetup& flt_setup, int dsd_samples)
    {
        dsd_fir1.init(flt_setup.get_fir1_8_ctables(), flt_setup.get_fir1_8_length(), 8);
        delay = dsd_fir1.get_delay();
    }

    int convert(uint8_t* dsd_data, double* pcm_data, int dsd_samples)
    {
        int pcm_samples;
        pcm_samples = dsd_fir1.run(dsd_data, pcm_data, dsd_samples);

        return pcm_samples;
    }
};
