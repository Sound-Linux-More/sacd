/*
    Copyright 2015-2016 Robert Tari <robert@tari.in>
    Copyright 2011-2012 Maxim V.Anisiutkin <maxim.anisiutkin@gmail.com>

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

#ifndef _SACD_MEDIA_H_INCLUDED
#define _SACD_MEDIA_H_INCLUDED

#include <stdint.h>
#include <cstring>
#include <string>
#include <stdio.h>

using namespace std;

class sacd_media_t
{
    FILE * media_file;
    string m_strFilePath;
public:
    sacd_media_t();
    virtual ~sacd_media_t();
    virtual bool open(const char* path);
    virtual bool close();
    virtual bool seek(int64_t position, int mode = SEEK_SET);
    virtual int64_t get_position();
    virtual size_t read(void* data, size_t size);
    virtual int64_t skip(int64_t bytes);
    virtual string getFileName();
};

#endif
