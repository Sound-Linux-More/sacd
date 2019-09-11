/*
    Copyright 2015-2019 Robert Tari <robert@tari.in>
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

#include <unistd.h>
#include <sys/stat.h>
#include "scarletbook.h"
#include "sacd_media.h"

#define MIN(a,b) (((a)<(b))?(a):(b))

sacd_media_t::sacd_media_t()
{
}

sacd_media_t::~sacd_media_t()
{
}

bool sacd_media_t::open(const char* path)
{
    try
    {
        media_file = fopen(path, "r");
        m_strFilePath = path;
        return true;
    }
    catch (...)
    {
    }

    return false;
}

bool sacd_media_t::close()
{
    fclose(media_file);

    return true;
}

bool sacd_media_t::seek(int64_t position, int mode)
{
    fseek(media_file, position, mode);
    return true;
}

int64_t sacd_media_t::get_position()
{
    return ftell(media_file);
}

size_t sacd_media_t::read(void* data, size_t size)
{
    return fread(data, 1, size, media_file);
}

int64_t sacd_media_t::skip(int64_t bytes)
{
    return fseek(media_file, bytes, SEEK_CUR);
}

string sacd_media_t::getFileName()
{
    m_strFilePath = m_strFilePath.substr(m_strFilePath.find_last_of("/") + 1, string::npos);
    return m_strFilePath.substr(0, m_strFilePath.find_last_of(".")) + ".wav";
}
