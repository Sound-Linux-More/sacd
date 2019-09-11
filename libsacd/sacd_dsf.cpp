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

#include "sacd_dsf.h"

#define MIN(a,b) (((a)<(b))?(a):(b))

sacd_dsf_t::sacd_dsf_t()
{
    for (int i = 0; i < 256; i++)
    {
        swap_bits[i] = 0;

        for (int j = 0; j < 8; j++)
        {
            swap_bits[i] |= ((i >> j) & 1) << (7 - j);
        }
    }
}

sacd_dsf_t::~sacd_dsf_t()
{
    close();
}

uint32_t sacd_dsf_t::get_track_count(area_id_e area_id)
{
    if ((area_id == AREA_TWOCH && m_channel_count <= 2) || (area_id == AREA_MULCH && m_channel_count > 2) || area_id == AREA_BOTH)
    {
        return 1;
    }

    return 0;
}

int sacd_dsf_t::get_channels()
{
    return m_channel_count;
}

int sacd_dsf_t::get_samplerate()
{
    return m_samplerate;
}

int sacd_dsf_t::get_framerate()
{
    return 75;
}

float sacd_dsf_t::getProgress()
{
    return ((float)(m_file->get_position() - m_read_offset) * 100.0) / (float)m_data_size;
}

bool sacd_dsf_t::is_dst()
{
    return false;
}

int sacd_dsf_t::open(sacd_media_t* p_file)
{
    m_file = p_file;
    Chunk ck;
    FmtDSFChunk fmt;
    uint64_t pos;

    if (!(m_file->read(&ck, sizeof(ck)) == sizeof(ck) && ck == "DSD "))
    {
        return false;
    }

    if (ck.get_size() != hton64((uint64_t)28))
    {
        return false;
    }

    if (m_file->read(&m_file_size, sizeof(m_file_size)) != sizeof(m_file_size))
    {
        return false;
    }

    if (m_file->read(&m_id3_offset, sizeof(m_id3_offset)) != sizeof(m_id3_offset))
    {
        return false;
    }

    pos = m_file->get_position();

    if (!(m_file->read(&fmt, sizeof(fmt)) == sizeof(fmt) && fmt == "fmt "))
    {
        return false;
    }

    if (fmt.format_id != 0) {
        return false;
    }

    switch (fmt.channel_type)
    {
        case 1:
            m_loudspeaker_config = 5;
            break;
        case 2:
            m_loudspeaker_config = 0;
            break;
        case 3:
            m_loudspeaker_config = 6;
            break;
        case 4:
            m_loudspeaker_config = 1;
            break;
        case 5:
            m_loudspeaker_config = 2;
            break;
        case 6:
            m_loudspeaker_config = 3;
            break;
        case 7:
            m_loudspeaker_config = 4;
            break;
        default:
            m_loudspeaker_config = 65535;
            break;
    }

    if (fmt.channel_count < 1 || fmt.channel_count > 6)
    {
        return false;
    }

    m_channel_count = fmt.channel_count;
    m_samplerate = fmt.samplerate;

    switch (fmt.bits_per_sample)
    {
        case 1:
            m_is_lsb = true;
            break;
        case 8:
            m_is_lsb = false;
            break;
        default:
            return false;
            break;
    }

    m_sample_count = fmt.sample_count;
    m_block_size = fmt.block_size;
    m_block_offset = m_block_size;
    m_block_data_end = 0;
    m_file->seek(pos + hton64(fmt.get_size()));

    if (!(m_file->read(&ck, sizeof(ck)) == sizeof(ck) && ck == "data"))
    {
        return false;
    }

    m_block_data.resize(m_channel_count * m_block_size);
    m_data_offset = m_file->get_position();
    m_data_end_offset = m_data_offset + ((m_sample_count / 8) * m_channel_count);
    m_data_size = hton64(ck.get_size()) - sizeof(ck);
    m_read_offset = m_data_offset;

    return 1;
}

bool sacd_dsf_t::close()
{
    return true;
}

void sacd_dsf_t::getTrackDetails(uint32_t track_number, area_id_e area_id, TrackDetails* cTrackDetails)
{
    cTrackDetails->strArtist = "Unknown Artist";
    cTrackDetails->strTitle = "Unknown Title";
    cTrackDetails->nChannels = m_channel_count;
}

string sacd_dsf_t::set_track(uint32_t track_number, area_id_e area_id, uint32_t offset)
{
    if (track_number)
    {
        return "";
    }

    m_file->seek(m_data_offset);

    return m_file->getFileName();
}

bool sacd_dsf_t::read_frame(uint8_t* frame_data, size_t* frame_size, frame_type_e* frame_type)
{
    int samples_read = 0;

    for (int i = 0; i < (int)*frame_size / m_channel_count; i++)
    {
        if (m_block_offset * m_channel_count >= m_block_data_end)
        {
                m_block_data_end = (int)MIN(m_data_end_offset - m_file->get_position(), m_block_data.size());

            if (m_block_data_end > 0)
            {
                m_block_data_end = m_file->read(m_block_data.data(), m_block_data_end);
            }

            if (m_block_data_end > 0)
            {
                m_block_offset = 0;
            }
            else
            {
                break;
            }
        }

        for (int ch = 0; ch < m_channel_count; ch++)
        {
            uint8_t b = m_block_data.data()[ch * m_block_size + m_block_offset];
            frame_data[i * m_channel_count + ch] = m_is_lsb ? swap_bits[b] : b;
        }

        m_block_offset++;
        samples_read++;
    }

    *frame_size = samples_read * m_channel_count;
    *frame_type = samples_read > 0 ? FRAME_DSD : FRAME_INVALID;

    return samples_read > 0;
}
