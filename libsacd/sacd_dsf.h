/*
    Copyright 2015-2019 Robert Tari <robert@tari.in>
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

#ifndef _SACD_DSF_H_INCLUDED
#define _SACD_DSF_H_INCLUDED

#include <stdint.h>
#include <vector>
#include "endianess.h"
#include "scarletbook.h"
#include "sacd_reader.h"
#include "sacd_dsd.h"

#pragma pack(1)

class FmtDSFChunk : public Chunk
{
public:
    uint32_t format_version;
    uint32_t format_id;
    uint32_t channel_type;
    uint32_t channel_count;
    uint32_t samplerate;
    uint32_t bits_per_sample;
    uint64_t sample_count;
    uint32_t block_size;
    uint32_t reserved;
};

#pragma pack()

class sacd_dsf_t : public sacd_reader_t
{
    sacd_media_t* m_file;
    int m_samplerate;
    int m_channel_count;
    int m_loudspeaker_config;
    uint64_t m_file_size;
    vector<uint8_t> m_block_data;
    int m_block_size;
    int m_block_offset;
    int m_block_data_end;
    uint64_t m_sample_count;
    uint64_t m_data_offset;
    uint64_t m_data_size;
    uint64_t m_data_end_offset;
    uint64_t m_read_offset;
    bool m_is_lsb;
    uint64_t m_id3_offset;
    vector<uint8_t> m_id3_data;
    uint8_t swap_bits[256];
public:
    sacd_dsf_t();
    ~sacd_dsf_t();
    uint32_t get_track_count(area_id_e area_id = AREA_BOTH);
    int get_channels();
    int get_samplerate();
    int get_framerate();
    float getProgress();
    bool is_dst();
    int open(sacd_media_t* p_file);
    bool close();
    string set_track(uint32_t track_number, area_id_e area_id = AREA_BOTH, uint32_t offset = 0);
    bool read_frame(uint8_t* frame_data, size_t* frame_size, frame_type_e* frame_type);
    void getTrackDetails(uint32_t track_number, area_id_e area_id, TrackDetails* cTrackDetails);
};

#endif
