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

#include <memory.h>
#include <stdlib.h>
#ifndef _ISOC11_SOURCE
#include <malloc.h>
#endif

class DSDPCMUtil
{

public:

    static void* mem_alloc(size_t size)
    {
#ifdef _ISOC11_SOURCE
        void* memory = aligned_alloc(MEM_ALIGN, size);
#else
        void* memory = memalign(MEM_ALIGN, size);
#endif

        if (memory)
        {
            memset(memory, 0, size);
        }

        return memory;
    }

    static void mem_free(void* memory)
    {
        if (memory)
        {
            free(memory);
        }
    }
};
