/*
    Copyright 2015-2018 Robert Tari <robert.tari@gmail.com>
    Copyright 2012 Vladislav Goncharov <vl-g@yandex.ru>
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

#include <vector>
#include <string>
#include <cstring>
#include <cmath>
#include <thread>
#include <stdio.h>
#include <locale>
#include <getopt.h>
#include <unistd.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include "libsacd/sacd_reader.h"
#include "libsacd/sacd_disc.h"
#include "libsacd/sacd_dsdiff.h"
#include "libsacd/sacd_dsf.h"
#include "libsacd/version.h"
#include "libdsd2pcm/dsd_pcm_converter_hq.h"
#include "libdsd2pcm/dsd_pcm_converter_engine.h"
#include "libdstdec/dst_decoder_mt.h"

struct TrackInfo
{
    int nTrack;
    area_id_e nArea;
};

int g_nCPUs = 2;
int g_nThreads = 2;
vector<TrackInfo> g_arrQueue;
pthread_mutex_t g_hMutex = PTHREAD_MUTEX_INITIALIZER;
string g_strOut = "";
int g_StdOut = 0;
int g_nSampleRate = 96000;
bool g_bProgressLine = false;
int g_nFinished = 0;
area_id_e g_nArea = AREA_MULCH;

void packageInt(unsigned char * buf, int offset, int num, int bytes)
{
    buf[offset + 0] = (unsigned char)(num & 0xff);
    buf[offset + 1] = (unsigned char)((num >> 8) & 0xff);

    if (bytes == 4)
    {
        buf[offset + 2] = (unsigned char) ((num >> 0x10) & 0xff);
        buf[offset + 3] = (unsigned char) ((num >> 0x18) & 0xff);
    }
}

string toLower(const string& s)
{
    string result;
    locale loc;

    for (unsigned int i = 0; i < s.length(); ++i)
    {
        result += tolower(s.at(i), loc);
    }

    return result;
}

class SACD
{
private:

    sacd_media_t* m_pSacdMedia;
    dst_decoder_t* m_pDstDecoder;
    vector<uint8_t> m_arrDstBuf;
    vector<uint8_t> m_arrDsdBuf;
    vector<float> m_arrPcmBuf;
    int m_nDsdBufSize;
    int m_nDstBufSize;
    dsdpcm_converter_hq* m_pDsdPcmConverter480;
    DSDPCMConverterEngine* m_pDsdPcmConverter441;
    int m_nDsdSamplerate;
    int m_nFramerate;
    int m_nPcmOutSamples;
    int m_nPcmOutDelta;

    void dsd2pcm(uint8_t* dsd_data, int dsd_samples, float* pcm_data)
    {

        if (m_pDsdPcmConverter480)
        {
            m_pDsdPcmConverter480->convert(dsd_data, dsd_samples, pcm_data);
        }
        else if (m_pDsdPcmConverter441)
        {
            m_pDsdPcmConverter441->convert(dsd_data, dsd_samples, pcm_data);
        }
    }

    void writeData(FILE * pFile, int nOffset, int nSamples)
    {
        int nFramesIn = nSamples * m_nPcmOutChannels;
        int nBytesOut = nFramesIn * 3;
        char * pSrc = (char*)(m_arrPcmBuf.data() + nOffset * m_nPcmOutChannels);
        char * pDst = new char[nBytesOut];
        float fSample;
        int32_t nVal;
        int nFrame;
        int nOut = 0;

        for (nFrame = 0; nFrame < nFramesIn; nFrame++, pSrc += 4)
        {
            fSample = *(float*)(pSrc);
            fSample = MIN(fSample, 1.0);
            fSample = MAX(fSample, -1.0);
            fSample *= 8388608.0;

            /*const float DITHER_NOISE = rand() / (float)RAND_MAX - 0.5f;
            const float SHAPED_BS[] = { 2.033f, -2.165f, 1.959f, -1.590f, 0.6149f };
            float mTriangleState = 0;
            int mPhase = 0;
            float mBuffer[8];
            memset(mBuffer, 0, sizeof(float) * 8);*/

            // 1 RectangleDither
            /*fSample = fSample - DITHER_NOISE;*/

            // 2 TriangleDither
            /*float r = DITHER_NOISE;
            float result = fSample + r - mTriangleState;
            mTriangleState = r;
            fSample = result;*/

            // 3 ShapedDither
            /*// Generate triangular dither, +-1 LSB, flat psd
            float r = DITHER_NOISE + DITHER_NOISE;

            // test for NaN and do the best we can with it
            if(fSample != fSample)
               fSample = 0;

            // Run FIR
            float xe = fSample + mBuffer[mPhase] * SHAPED_BS[0]
                + mBuffer[(mPhase - 1) & 7] * SHAPED_BS[1]
                + mBuffer[(mPhase - 2) & 7] * SHAPED_BS[2]
                + mBuffer[(mPhase - 3) & 7] * SHAPED_BS[3]
                + mBuffer[(mPhase - 4) & 7] * SHAPED_BS[4];

            // Accumulate FIR and triangular noise
            float result = xe + r;

            // Roll buffer and store last error
            mPhase = (mPhase + 1) & 7;
            mBuffer[mPhase] = xe - lrintf(result);

            fSample = result;*/

            nVal = lrintf(fSample);
            nVal = MIN(nVal, 8388607);
            nVal = MAX(nVal, -8388608);

            pDst[nOut++] = nVal;
            pDst[nOut++] = nVal >> 8;
            pDst[nOut++] = nVal >> 16;
        }

        fwrite(pDst, 1, nBytesOut, pFile);
        delete [] pDst;

        m_fProgress = m_pSacdReader->getProgress();
    }

public:

    int m_nTracks;
    float m_fProgress;
    int m_nPcmOutChannels;
    unsigned int m_nPcmOutChannelMap;
    sacd_reader_t* m_pSacdReader;
    bool m_bTrackCompleted;

    SACD()
    {
        m_pSacdMedia = nullptr;
        m_pSacdReader = nullptr;
        m_pDsdPcmConverter441 = nullptr;
        m_pDsdPcmConverter480 = nullptr;
        m_pDstDecoder = nullptr;
        m_fProgress = 0;
        m_nTracks = 0;
        m_nPcmOutSamples = 0;
        m_nPcmOutDelta = 0;
    }

    ~SACD()
    {
        if (m_pSacdReader)
        {
            delete m_pSacdReader;
        }

        if (m_pSacdMedia)
        {
            delete m_pSacdMedia;
        }

        if (m_pDsdPcmConverter441)
        {
            delete m_pDsdPcmConverter441;
        }
        else if (m_pDsdPcmConverter480)
        {
            delete m_pDsdPcmConverter480;
        }

        if (m_pDstDecoder)
        {
            delete m_pDstDecoder;
        }
    }

    int open(string p_path)
    {
        string ext = toLower(p_path.substr(p_path.length()-3, 3));
        media_type_t tMediaType = UNK_TYPE;

        if (ext == "iso")
        {
            tMediaType = ISO_TYPE;
        }
        else if (ext == "dat")
        {
            tMediaType = ISO_TYPE;
        }
        else if (ext == "dff")
        {
            tMediaType = DSDIFF_TYPE;
        }
        else if (ext == "dsf")
        {
            tMediaType = DSF_TYPE;
        }

        if (tMediaType == UNK_TYPE)
        {
            fprintf(stderr, "PANIC: exception_io_unsupported_format\n");
            return 0;
        }

        m_pSacdMedia = new sacd_media_t();

        if (!m_pSacdMedia)
        {
            fprintf(stderr, "PANIC: exception_overflow\n");
            return 0;
        }

        switch (tMediaType)
        {
            case ISO_TYPE:
                m_pSacdReader = new sacd_disc_t;
                if (!m_pSacdReader)
                {
                    fprintf(stderr, "PANIC: exception_overflow\n");
                    return 0;
                }
                break;
            case DSDIFF_TYPE:
                m_pSacdReader = new sacd_dsdiff_t;
                if (!m_pSacdReader)
                {
                    fprintf(stderr, "PANIC: exception_overflow\n");
                    return 0;
                }
                break;
            case DSF_TYPE:
                m_pSacdReader = new sacd_dsf_t;
                if (!m_pSacdReader)
                {
                    fprintf(stderr, "PANIC: exception_overflow\n");
                    return 0;
                }
                break;
            default:
                fprintf(stderr, "PANIC: exception_io_data\n");
                return 0;
                break;
        }

        if (!m_pSacdMedia->open(p_path.c_str()))
        {
            fprintf(stderr, "PANIC: exception_io_data\n");
            return 0;
        }

        if ((m_nTracks = m_pSacdReader->open(m_pSacdMedia)) == 0)
        {
            fprintf(stderr, "PANIC: Failed to parse SACD media\n");
            return 0;
        }

        return m_nTracks;
    }

    string init(uint32_t nSubsong, int g_nSampleRate, area_id_e nArea)
    {
        if (m_pDsdPcmConverter441)
        {
            delete m_pDsdPcmConverter441;
            m_pDsdPcmConverter441 = nullptr;
        }
        else if (m_pDsdPcmConverter480)
        {
            delete m_pDsdPcmConverter480;
            m_pDsdPcmConverter480 = nullptr;
        }

        if (m_pDstDecoder)
        {
            delete m_pDstDecoder;
            m_pDstDecoder = nullptr;
        }

        string strFileName = m_pSacdReader->set_track(nSubsong, nArea, 0);
        m_nDsdSamplerate = m_pSacdReader->get_samplerate();
        m_nFramerate = m_pSacdReader->get_framerate();
        m_nPcmOutSamples = g_nSampleRate / m_nFramerate;
        m_nPcmOutChannels = m_pSacdReader->get_channels();

        switch (m_nPcmOutChannels)
        {
            case 1:
                m_nPcmOutChannelMap = 1<<2;
                break;
            case 2:
                m_nPcmOutChannelMap = 1<<0 | 1<<1;
                break;
            case 3:
                m_nPcmOutChannelMap = 1<<0 | 1<<1 | 1<<2;
                break;
            case 4:
                m_nPcmOutChannelMap = 1<<0 | 1<<1 | 1<<4 | 1<<5;
                break;
            case 5:
                m_nPcmOutChannelMap = 1<<0 | 1<<1 | 1<<2 | 1<<4 | 1<<5;
                break;
            case 6:
                m_nPcmOutChannelMap = 1<<0 | 1<<1 | 1<<2 | 1<<3 | 1<<4 | 1<<5;
                break;
            default:
                m_nPcmOutChannelMap = 0;
                break;
        }

        m_nDstBufSize = m_nDsdBufSize = m_nDsdSamplerate / 8 / m_nFramerate * m_nPcmOutChannels;
        m_arrDsdBuf.resize(m_nDsdBufSize * g_nCPUs);
        m_arrDstBuf.resize(m_nDstBufSize * g_nCPUs);
        m_arrPcmBuf.resize(m_nPcmOutChannels * m_nPcmOutSamples);

        if (g_nSampleRate == 96000 or g_nSampleRate == 192000)
        {
            m_pDsdPcmConverter480 = new dsdpcm_converter_hq();
            m_pDsdPcmConverter480->init(m_nPcmOutChannels, m_nDsdSamplerate, g_nSampleRate);
        }
        else
        {
            m_pDsdPcmConverter441 = new DSDPCMConverterEngine();
            m_pDsdPcmConverter441->init(m_nPcmOutChannels, m_nFramerate, m_nDsdSamplerate, g_nSampleRate);
        }

        float fPcmOutDelay = 0.0f;

        if (m_pDsdPcmConverter480)
        {
            fPcmOutDelay = m_pDsdPcmConverter480->get_delay();
        }
        else
        {
            fPcmOutDelay = m_pDsdPcmConverter441->get_delay();
        }

        m_nPcmOutDelta = (int)(fPcmOutDelay - 0.5f);//  + 0.5f originally

        if (m_nPcmOutDelta > m_nPcmOutSamples - 1)
        {
            m_nPcmOutDelta = m_nPcmOutSamples - 1;
        }

        m_bTrackCompleted = false;

        return strFileName;
    }

    void fixPcmStream(bool bIsEnd, float* pPcmData, int nPcmSamples)
    {
        if (!bIsEnd)
        {
            if (nPcmSamples > 1)
            {
                for (int ch = 0; ch < m_nPcmOutChannels; ch++)
                {
                    pPcmData[0 * m_nPcmOutChannels + ch] = pPcmData[1 * m_nPcmOutChannels + ch];
                }
            }
        }
        else
        {
            if (nPcmSamples > 1)
            {
                for (int ch = 0; ch < m_nPcmOutChannels; ch++)
                {
                    pPcmData[(nPcmSamples - 1) * m_nPcmOutChannels + ch] = pPcmData[(nPcmSamples - 2) * m_nPcmOutChannels + ch];
                }
            }
        }
    }

    bool decode(FILE* pFile)
    {
        if (m_bTrackCompleted)
        {
            return true;
        }

        uint8_t* pDsdData;
        uint8_t* pDstData;
        size_t nDsdSize = 0;
        size_t nDstSize = 0;
        int nThread = 0;

        while (1)
        {
            nThread = m_pDstDecoder ? m_pDstDecoder->slot_nr : 0;
            pDsdData = m_arrDsdBuf.data() + m_nDsdBufSize * nThread;
            pDstData = m_arrDstBuf.data() + m_nDstBufSize * nThread;
            nDstSize = m_nDstBufSize;
            frame_type_e nFrameType;

            if (m_pSacdReader->read_frame(pDstData, &nDstSize, &nFrameType))
            {
                if (nDstSize > 0)
                {
                    if (nFrameType == FRAME_INVALID)
                    {
                        nDstSize = m_nDstBufSize;
                        memset(pDstData, DSD_SILENCE_BYTE, nDstSize);
                    }

                    if (nFrameType == FRAME_DST)
                    {
                        if (!m_pDstDecoder)
                        {
                            m_pDstDecoder = new dst_decoder_t(g_nCPUs);

                            if (!m_pDstDecoder || m_pDstDecoder->init(m_nPcmOutChannels, m_nDsdSamplerate, m_nFramerate) != 0)
                            {
                                return true;
                            }
                        }

                        m_pDstDecoder->decode(pDstData, nDstSize, &pDsdData, &nDsdSize);
                    }
                    else
                    {
                        pDsdData = pDstData;
                        nDsdSize = nDstSize;
                    }

                    if (nDsdSize > 0)
                    {
                        int nRemoveSamples = 0;

                        if ((m_pDsdPcmConverter480 && !m_pDsdPcmConverter480->is_convert_called()) || (m_pDsdPcmConverter441 && !m_pDsdPcmConverter441->is_convert_called()))
                        {
                            nRemoveSamples = m_nPcmOutDelta;
                        }

                        dsd2pcm(pDsdData, nDsdSize, m_arrPcmBuf.data());

                        if (nRemoveSamples > 0)
                        {
                            fixPcmStream(false, m_arrPcmBuf.data() + m_nPcmOutChannels * nRemoveSamples, m_nPcmOutSamples - nRemoveSamples);
                        }

                        writeData(pFile, nRemoveSamples, m_nPcmOutSamples - nRemoveSamples);

                        return false;
                    }
                }
            }
            else
            {
                break;
            }
        }

        pDsdData = nullptr;
        pDstData = nullptr;
        nDstSize = 0;

        if (m_pDstDecoder)
        {
            m_pDstDecoder->decode(pDstData, nDstSize, &pDsdData, &nDsdSize);
        }

        if (nDsdSize > 0)
        {
            dsd2pcm(pDsdData, nDsdSize, m_arrPcmBuf.data());
            writeData(pFile, 0, m_nPcmOutSamples);

            return false;
        }

        if (m_nPcmOutDelta > 0)
        {
            dsd2pcm(nullptr, 0, m_arrPcmBuf.data());
            fixPcmStream(true, m_arrPcmBuf.data(), m_nPcmOutDelta);
            writeData(pFile, 0, m_nPcmOutDelta);
        }

        m_bTrackCompleted = true;

        return true;
    }
};

void * fnProgress (void* threadargs)
{
    vector<SACD*>* arrSACD = (vector<SACD*>*)threadargs;

    while(1)
    {
        float fProgress = 0;
        int nTracks = (*arrSACD).at(0)->m_nTracks;

        for (int i = 0; i < g_nThreads; i++)
        {
            fProgress += (*arrSACD).at(i)->m_fProgress;
        }

        fProgress = MAX(((((float)nTracks - (float)MIN(g_nThreads, nTracks) - (float)g_arrQueue.size()) * 100.0) + fProgress) / (float)nTracks, 0);

        if (g_bProgressLine)
        {
            fprintf(stderr, "PROGRESS\t%.2f\n", fProgress);
        }
        else
        {
            fprintf(stderr, "\r%.2f%%", fProgress);
        }

        fflush(stderr);

        if (g_nFinished == nTracks)
        {
            break;
        }

        sleep(1);
    }

    return 0;
}

void * fnDecoder (void* threadargs)
{
    SACD* pSACD = (SACD*)threadargs;

    while(!g_arrQueue.empty())
    {
        pthread_mutex_lock(&g_hMutex);

        TrackInfo cTrackInfo = g_arrQueue.front();
        g_arrQueue.erase(g_arrQueue.begin());

        pthread_mutex_unlock(&g_hMutex);

        string strOutFile = g_strOut + pSACD->init(cTrackInfo.nTrack, g_nSampleRate, cTrackInfo.nArea);
        unsigned int nSize = 0x7fffffff;
        unsigned char arrHeader[68];
        unsigned char arrFormat[2] = {0xFE, 0xFF};
        unsigned char arrSubtype[16] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71};

        memcpy (arrHeader, "RIFF", 4);
        packageInt (arrHeader, 4, nSize - 8, 4);
        memcpy (arrHeader + 8, "WAVE", 4);
        memcpy (arrHeader + 12, "fmt ", 4);
        packageInt (arrHeader, 16, 40, 4);
        memcpy (arrHeader + 20, arrFormat, 2);
        packageInt (arrHeader, 22, pSACD->m_nPcmOutChannels, 2);
        packageInt (arrHeader, 24, g_nSampleRate, 4);
        packageInt (arrHeader, 28, (g_nSampleRate * 24 * pSACD->m_nPcmOutChannels) / 8, 4);
        packageInt (arrHeader, 32, pSACD->m_nPcmOutChannels * 3, 2);
        packageInt (arrHeader, 34, 24, 2);
        packageInt (arrHeader, 36, 22, 2);
        packageInt (arrHeader, 38, 24, 2);
        packageInt (arrHeader, 40, pSACD->m_nPcmOutChannelMap, 4);
        memcpy (arrHeader + 44, arrSubtype, 16);
        memcpy (arrHeader + 60, "data", 4);
        packageInt (arrHeader, 64, nSize - 68, 4);
        FILE * pFile = (g_StdOut == 0) ? fopen(strOutFile.data(), "wb") : stdout;
        fwrite(arrHeader, 1, 68, pFile);

        bool bDone = false;

        while (!bDone || !pSACD->m_bTrackCompleted)
        {
            bDone = pSACD->decode(pFile);
        }

        nSize = ftell (pFile);
        packageInt (arrHeader, 4, nSize - 8, 4);
        packageInt (arrHeader, 64, nSize - 68, 4);
        fseek (pFile, 0, SEEK_SET);
        fwrite (arrHeader, 1, 68, pFile);
        fclose(pFile);

        if (g_bProgressLine)
        {
            fprintf(stderr, "FILE\t%s\t%.2i\t%.2i\n", strOutFile.data(), cTrackInfo.nTrack + 1, pSACD->m_nTracks);
        }

        g_nFinished++;
    }

    return 0;
}

int main(int argc, char* argv[])
{
    int nCPUs = sysconf(_SC_NPROCESSORS_ONLN);

    if (nCPUs < 2)
    {
        nCPUs = thread::hardware_concurrency();
    }

    if (nCPUs > 2)
    {
        g_nCPUs = nCPUs;
    }

    string strIn = "";
    char strPath[PATH_MAX];
    int nOpt;
    bool bPrintHelp = false;

    const char strHelpText[] =
    "\n"
    "Usage: sacd -i infile [-o outdir] [options]\n\n"
    "  -i, --infile         : Specify the input file (*.iso, *.dsf, *.dff)\n"
    "  -o, --outdir         : The folder to write the WAVE files to. If you omit\n"
    "                         this, the files will be placed in the input file's\n"
    "                         directory\n"
    "  -c, --stdout         : Stdout output (for pipe), sample:\n"
    "                         sacd -i file.dsf -c | play -\n"
    "  -r, --rate           : The output samplerate.\n"
    "                         Valid rates are: 88200, 96000, 176400 and 192000.\n"
    "                         If you omit this, 96KHz will be used.\n"
    "  -s, --stereo         : Only extract the 2-channel area if it exists.\n"
    "                         If you omit this, the multichannel area will have priority.\n"
    "  -p, --progress       : Display progress to new lines. Use this if you intend\n"
    "                         to parse the output through a script. This option only\n"
    "                         lists either one progress percentage per line, or one\n"
    "                         status/error message.\n"
    "  -h, --help           : Show this help message\n\n";

    const struct option tOptionsTable[] =
    {
        {"infile", required_argument, NULL, 'i' },
        {"outdir", required_argument, NULL, 'o' },
        {"stdout", no_argument, NULL, 'c'},
        {"rate", required_argument, NULL, 'r' },
        {"stereo", no_argument, NULL, 's'},
        {"progress", no_argument, NULL, 'p'},
        {"help", no_argument, NULL, 'h' },
        { NULL, 0, NULL, 0 }
    };

    while ((nOpt = getopt_long(argc, argv, "i:o:cr:sph", tOptionsTable, NULL)) >= 0)
    {
        switch (nOpt)
        {
            case 'i':
                strIn = optarg;
                break;
            case 'o':
                g_strOut = optarg;
                break;
            case 'c':
                g_StdOut = 1;
                break;
            case 'r':
            {
                string s = optarg;

                if (s == "88200" || s == "96000" || s == "176400" || s == "192000")
                {
                    g_nSampleRate = stoi(optarg);
                }
                else
                {
                    fprintf(stderr, "PANIC: Invalid samplerate\n");
                    return 0;
                }
                break;
            }
            case 's':
                g_nArea = AREA_TWOCH;
                break;
            case 'p':
                g_bProgressLine = true;
                break;
            default:
                bPrintHelp = true;
                break;
        }
    }

    if (bPrintHelp || argc == 1 || strIn.empty())
    {
        fprintf(stderr, g_bProgressLine ? "PANIC: Invalid command-line syntax\n" : strHelpText);
        return 0;
    }

    struct stat tStat;

    if (stat(strIn.c_str(), &tStat) == -1 || !S_ISREG(tStat.st_mode))
    {
        fprintf(stderr, "PANIC: Input file does not exist\n");
        return 0;
    }

    strIn = realpath(strIn.data(), strPath);

    if (g_strOut.empty())
    {
        g_strOut = strIn.substr(0, strIn.find_last_of("/") + 1);
    }

    if (g_strOut.empty() || stat(g_strOut.c_str(), &tStat) == -1 || !S_ISDIR(tStat.st_mode))
    {
        fprintf(stderr, "PANIC: Output directory does not exist\n");
        return 0;
    }

    g_strOut = realpath(g_strOut.data(), strPath);

    if (g_strOut.compare(g_strOut.size() - 1, 1, "/") != 0)
    {
        g_strOut += "/";
    }

    SACD * pSacd = new SACD();

    if (!pSacd->open(strIn))
    {
        exit(1);
    }

    if (!g_bProgressLine)
    {
        fprintf(stderr, "\n\nsacd - Command-line SACD decoder version %s\n\n", APPVERSION);
    }

    int nTwoch = pSacd->m_pSacdReader->get_track_count(AREA_TWOCH);
    int nMulch = pSacd->m_pSacdReader->get_track_count(AREA_MULCH);
    bool bWarn = false;
    area_id_e nArea;

    if (nMulch > 0 && nTwoch > nMulch && g_nArea != AREA_TWOCH)
    {
        nArea = AREA_BOTH;
        bWarn = true;
    }
    else if (nTwoch > 0 && nMulch > nTwoch && g_nArea != AREA_TWOCH)
    {
        nArea = AREA_BOTH;
        bWarn = true;
    }
    else if (nMulch > 0 && g_nArea != AREA_TWOCH)
    {
        nArea = AREA_MULCH;
    }
    else if (nTwoch > 0)
    {
        nArea = AREA_TWOCH;
    }
    else
    {
        nArea = AREA_MULCH;
    }

    if(nArea == AREA_MULCH || nArea == AREA_BOTH)
    {
        for (int i = 0; i < nMulch; i++)
        {
            TrackInfo cTrackInfo = {i, AREA_MULCH};
            g_arrQueue.push_back(cTrackInfo);
        }
    }

    if(nArea == AREA_TWOCH || nArea == AREA_BOTH)
    {
        for (int i = 0; i < nTwoch; i++)
        {
            TrackInfo cTrackInfo = {i, AREA_TWOCH};
            g_arrQueue.push_back(cTrackInfo);
        }
    }

    if(bWarn)
    {
        if (g_bProgressLine)
        {
            fprintf(stderr, "WARNINGThe multichannel and stereo areas have a different track count: extracting both.\n");
        }
        else
        {
            fprintf(stderr, "WARNING: The multichannel and stereo areas have a different track count: extracting both.\n\n");
        }
    }

    delete pSacd;

    g_nThreads = MIN(g_nCPUs, (int)g_arrQueue.size());

    time_t nNow = time(0);
    pthread_t hThreadProgress;
    vector<SACD*> arrSACD(g_nThreads);
    vector<pthread_t> arrThreads(g_nThreads);

    for (int i = 0; i < g_nThreads; i++)
    {
        arrSACD[i] = new SACD();
        arrSACD[i]->open(strIn);
        pthread_create(&arrThreads[i], NULL, fnDecoder, arrSACD[i]);
        pthread_detach(arrThreads[i]);
    }

    pthread_create(&hThreadProgress, NULL, fnProgress, &arrSACD);
    pthread_join(hThreadProgress, NULL);
    pthread_mutex_destroy(&g_hMutex);

    for (int i = 0; i < g_nThreads; i++)
    {
        delete arrSACD[i];
    }

    int nSeconds = time(0) - nNow;

    if (g_bProgressLine)
    {
        fprintf(stderr, "FINISHED\t%d\n", nSeconds);
    }
    else
    {
        fprintf(stderr, "\nFinished in %d seconds.\n\n", nSeconds);
    }

    return 0;
}
