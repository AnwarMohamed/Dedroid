/*
 *
 *  Copyright (C) 2013  Anwar Mohamed <anwarelmakrahy[at]gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to Anwar Mohamed
 *  anwarelmakrahy[at]gmail.com
 *
 */

#pragma once
#include "Dedroid.h"

struct DEX_HEADER 
{
    UCHAR	magic[8];       /* includes version number */
    UINT	checksum;		/* adler32 checksum */
    UCHAR	signature[20];	/* SHA-1 hash */
    UINT	fileSize;       /* length of entire file */
    UINT	headerSize;     /* offset to start of next section */
    UINT	endianTag;
    UINT	linkSize;
    UINT	linkOff;
    UINT	mapOff;
    UINT	stringIdsSize;
    UINT	stringIdsOff;
    UINT	typeIdsSize;
    UINT	typeIdsOff;
    UINT	protoIdsSize;
    UINT	protoIdsOff;
    UINT	fieldIdsSize;
    UINT	fieldIdsOff;
    UINT	methodIdsSize;
    UINT	methodIdsOff;
    UINT	classDefsSize;
    UINT	classDefsOff;
    UINT	dataSize;
    UINT	dataOff;
};

class cDex
{
public:
	cDex(CHAR* Filename);
	~cDex();

	CHAR* DexFilename;
};

