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

#include "cApk.h"

cApk::cApk(CHAR* Filename, CHAR* BaseDir)
{
	ApkFilename = Filename;
	ApkBaseDir = BaseDir;
}

cApk::~cApk()
{
}

void cApk::Decompress()
{
	ZipHandler = OpenZip(ApkFilename,0);
	SetUnzipBaseDir(ZipHandler,ApkBaseDir);

	ZIPENTRY ArchieveEntry; 
	GetZipItem(ZipHandler,-1,&ArchieveEntry); 

	UINT nFiles = ArchieveEntry.index;
	for (UINT i=0; i<nFiles; i++)
	{ 
		GetZipItem(ZipHandler, i, &ArchieveEntry);
		UnzipItem(ZipHandler, i, ArchieveEntry.name);
	}

	CloseZip(ZipHandler);
}