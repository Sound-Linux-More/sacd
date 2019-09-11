/*
    Copyright 2015-2016 Robert Tari <robert@tari.in>
    Copyright 2011-2019 Maxim V.Anisiutkin <maxim.anisiutkin@gmail.com>

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

#ifndef _SACD_DSD_H_INCLUDED
#define _SACD_DSD_H_INCLUDED

#include <stdint.h>
#include "endianess.h"

#pragma pack(1)

class ID
{
    char ckID[4];

public:

    bool operator == (const char* id)
    {
        return ckID[0] == id[0] && ckID[1] == id[1] && ckID[2] == id[2] && ckID[3] == id[3];
    }

    bool operator != (const char* id)
    {
        return !(*this == id);
    }

    void set_id(const char* id)
    {
        ckID[0] = id[0];
        ckID[1] = id[1];
        ckID[2] = id[2];
        ckID[3] = id[3];
    }
};

class Chunk : public ID
{

public:

    uint64_t ckDataSize;

    uint64_t get_size()
    {
        return hton64(ckDataSize);
    }

    void set_size(uint64_t size)
    {
        ckDataSize = hton64(size);
    }
};

#pragma pack()

#endif
