/*
    Copyright 2015-2016 Robert Tari <robert.tari@gmail.com>
    Copyright 2011-2016 Maxim V.Anisiutkin <maxim.anisiutkin@gmail.com>

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

#ifndef _SACD_READER_H_INCLUDED
#define _SACD_READER_H_INCLUDED

#include <stdint.h>
#include <string>
#include "sacd_media.h"

using namespace std;

enum area_id_e {AREA_BOTH = 0, AREA_TWOCH = 1, AREA_MULCH = 2};
enum frame_type_e {FRAME_DSD = 0, FRAME_DST = 1, FRAME_INVALID = -1};
enum media_type_t {UNK_TYPE = 0, ISO_TYPE = 1, DSDIFF_TYPE = 2, DSF_TYPE = 3};

class sacd_reader_t {
public:
    sacd_reader_t() {}
    virtual ~sacd_reader_t() {}
    virtual int open(sacd_media_t* p_file) = 0;
    virtual bool close() = 0;
    virtual uint32_t get_track_count(area_id_e area_id = AREA_BOTH) = 0;
    virtual int get_channels() = 0;
    virtual int get_samplerate() = 0;
    virtual int get_framerate() = 0;
    virtual float getProgress() = 0;
    virtual bool is_dst() = 0;
    virtual string set_track(uint32_t track_number, area_id_e area_id = AREA_BOTH, uint32_t offset = 0) = 0;
    virtual bool read_frame(uint8_t* frame_data, size_t* frame_size, frame_type_e* frame_type) = 0;
};

#endif
