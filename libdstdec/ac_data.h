/*
    MPEG-4 Audio RM Module
    Lossless coding of 1-bit oversampled audio - DST (Direct Stream Transfer)

    This software was originally developed by:

    * Aad Rijnberg
    Philips Digital Systems Laboratories Eindhoven
    <aad.rijnberg@philips.com>

    * Fons Bruekers
    Philips Research Laboratories Eindhoven
    <fons.bruekers@philips.com>

    * Eric Knapen
    Philips Digital Systems Laboratories Eindhoven
    <h.w.m.knapen@philips.com>

    And edited by:

    * Richard Theelen
    Philips Digital Systems Laboratories Eindhoven
    <r.h.m.theelen@philips.com>

    * Maxim V.Anisiutkin
    <maxim.anisiutkin@gmail.com>

    * Robert Tari
    <robert@tari.in>

    in the course of development of the MPEG-4 Audio standard ISO-14496-1, 2 and 3.
    This software module is an implementation of a part of one or more MPEG-4 Audio
    tools as specified by the MPEG-4 Audio standard. ISO/IEC gives users of the
    MPEG-4 Audio standards free licence to this software module or modifications
    thereof for use in hardware or software products claiming conformance to the
    MPEG-4 Audio standards. Those intending to use this software module in hardware
    or software products are advised that this use may infringe existing patents.
    The original developers of this software of this module and their company,
    the subsequent editors and their companies, and ISO/EIC have no liability for
    use of this software module or modifications thereof in an implementation.
    Copyright is not released for non MPEG-4 Audio conforming products. The
    original developer retains full right to use this code for his/her own purpose,
    assign or donate the code to a third party and to inhibit third party from
    using the code for non MPEG-4 Audio conforming products. This copyright notice
    must be included in all copies of derivative works.

    Copyright � 2004-2016.

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

#ifndef ACDATA_H
#define ACDATA_H

#include "dst_defs.h"

class CACData
{
    unsigned int Init;
    unsigned int C;
    unsigned int A;
    int cbptr;

public:

    int getPtableIndex(long PredicVal, int PtableLen);
    void decodeBit(uint8_t& b, int p, uint8_t* cb, int fs, int flush);
    void decodeBit_Init(ADataByte* cb, int fs);
    void decodeBit_Decode(uint8_t* b, int p, ADataByte* cb, int fs);
    void decodeBit_Flush(uint8_t* b, int p, ADataByte* cb, int fs);
};

#endif
