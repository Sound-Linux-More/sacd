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

#include "dsd_pcm_constants.h"
#include "dsd_pcm_util.h"

class DSDPCMFilterSetup
{
    using ctable_t = double[256];
    ctable_t* dsd_fir1_8_ctables;
    ctable_t* dsd_fir1_16_ctables;
    ctable_t* dsd_fir1_64_ctables;
    double* pcm_fir2_2_coefs;
    double* pcm_fir3_2_coefs;

public:

    DSDPCMFilterSetup()
    {
        dsd_fir1_8_ctables = nullptr;
        dsd_fir1_16_ctables = nullptr;
        dsd_fir1_64_ctables = nullptr;
        pcm_fir2_2_coefs = nullptr;
        pcm_fir3_2_coefs = nullptr;
    }

    ~DSDPCMFilterSetup()
    {

        DSDPCMUtil::mem_free(pcm_fir2_2_coefs);
        DSDPCMUtil::mem_free(pcm_fir3_2_coefs);
    }

    static const double NORM_I(const int scale = 0)
    {
        return (double)1 / (double)((unsigned int)1 << (31 - scale));
    }

    ctable_t* get_fir1_8_ctables()
    {
        if (!dsd_fir1_8_ctables)
        {
            dsd_fir1_8_ctables = (ctable_t*)DSDPCMUtil::mem_alloc(CTABLES(DSDFIR1_8_LENGTH) * sizeof(ctable_t));
            set_ctables(DSDFIR1_8_COEFS, DSDFIR1_8_LENGTH, NORM_I(3), dsd_fir1_8_ctables);
        }

        return dsd_fir1_8_ctables;
    }

    int get_fir1_8_length()
    {
        return DSDFIR1_8_LENGTH;
    }

    ctable_t* get_fir1_16_ctables()
    {
        if (!dsd_fir1_16_ctables)
        {
            dsd_fir1_16_ctables = (ctable_t*)DSDPCMUtil::mem_alloc(CTABLES(DSDFIR1_16_LENGTH) * sizeof(ctable_t));
            set_ctables(DSDFIR1_16_COEFS, DSDFIR1_16_LENGTH, NORM_I(3), dsd_fir1_16_ctables);
        }

        return dsd_fir1_16_ctables;
    }

    int get_fir1_16_length()
    {
        return DSDFIR1_16_LENGTH;
    }

    ctable_t* get_fir1_64_ctables()
    {
        if (!dsd_fir1_64_ctables)
        {
            dsd_fir1_64_ctables = (ctable_t*)DSDPCMUtil::mem_alloc(CTABLES(DSDFIR1_64_LENGTH) * sizeof(ctable_t));
            set_ctables(DSDFIR1_64_COEFS, DSDFIR1_64_LENGTH, NORM_I(), dsd_fir1_64_ctables);
        }

        return dsd_fir1_64_ctables;
    }


    int get_fir1_64_length()
    {
        return DSDFIR1_64_LENGTH;
    }

    double* get_fir2_2_coefs()
    {
        if (!pcm_fir2_2_coefs)
        {
            pcm_fir2_2_coefs = (double*)DSDPCMUtil::mem_alloc(PCMFIR2_2_LENGTH * sizeof(double));
            set_coefs(PCMFIR2_2_COEFS, PCMFIR2_2_LENGTH, NORM_I(), pcm_fir2_2_coefs);
        }

        return pcm_fir2_2_coefs;
    }

    int get_fir2_2_length()
    {
        return PCMFIR2_2_LENGTH;
    }

    double* get_fir3_2_coefs()
    {
        if (!pcm_fir3_2_coefs)
        {
            pcm_fir3_2_coefs = (double*)DSDPCMUtil::mem_alloc(PCMFIR3_2_LENGTH * sizeof(double));
            set_coefs(PCMFIR3_2_COEFS, PCMFIR3_2_LENGTH, NORM_I(), pcm_fir3_2_coefs);
        }

        return pcm_fir3_2_coefs;
    }

    int get_fir3_2_length()
    {
        return PCMFIR3_2_LENGTH;
    }

private:

    int set_ctables(const double* fir_coefs, const int fir_length, const double fir_gain, ctable_t* out_ctables)
    {
        int ctables = CTABLES(fir_length);

        for (int ct = 0; ct < ctables; ct++)
        {
            int k = fir_length - ct * 8;

            if (k > 8)
            {
                k = 8;
            }

            if (k < 0)
            {
                k = 0;
            }

            for (int i = 0; i < 256; i++)
            {
                double cvalue = 0.0;

                for (int j = 0; j < k; j++)
                {
                    cvalue += (((i >> (7 - j)) & 1) * 2 - 1) * fir_coefs[fir_length - 1 - (ct * 8 + j)];
                }

                out_ctables[ct][i] = (double)(cvalue * fir_gain);
            }
        }

        return ctables;
    }

    void set_coefs(const double* fir_coefs, const int fir_length, const double fir_gain, double* out_coefs)
    {
        for (int i = 0; i < fir_length; i++)
        {
            out_coefs[i] = (double)(fir_coefs[fir_length - 1 - i] * fir_gain);
        }
    }
};
