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

#ifndef _SACD_DISC_H_INCLUDED
#define _SACD_DISC_H_INCLUDED

#include <stdint.h>
#include "endianess.h"
#include "scarletbook.h"
#include "sacd_reader.h"

constexpr int SACD_PSN_SIZE = 2064;

using namespace std;

typedef struct
{
    uint8_t data[1024 * 64];
    int size;
    bool started;
    int channel_count;
    int dst_encoded;
} audio_frame_t;

class sacd_disc_t : public sacd_reader_t
{
private:
    sacd_media_t* m_file;
    scarletbook_handle_t m_sb;
    area_id_e m_track_area;
    uint32_t m_track_start_lsn;
    uint32_t m_track_length_lsn;
    uint32_t m_track_current_lsn;
    uint8_t m_channel_count;
    audio_sector_t m_audio_sector;
    audio_frame_t m_frame;
    int m_packet_info_idx;
    uint8_t m_sector_buffer[SACD_PSN_SIZE];
    uint32_t m_sector_size;
    int m_sector_bad_reads;
    uint8_t* m_buffer;
    int m_buffer_offset;
public:
    static bool g_is_sacd(const char* p_path);
    sacd_disc_t();
    ~sacd_disc_t();
    scarletbook_area_t* get_area(area_id_e area_id);
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
    bool read_blocks_raw(uint32_t lb_start, size_t block_count, uint8_t* data);
    void getTrackDetails(uint32_t track_number, area_id_e area_id, TrackDetails* cTrackDetails);
private:
    bool read_master_toc();
    bool read_area_toc(int area_idx);
    void free_area(scarletbook_area_t* area);
};

#endif
