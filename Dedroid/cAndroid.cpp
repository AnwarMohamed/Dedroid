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

#include "cAndroid.h"

cAndroid::cAndroid(CHAR* ApkFilename) : cFile(ApkFilename)
{
	this->ApkFilename = ApkFilename;
	DexBuffer = NULL;
	isReady = ProcessApk();
}

BOOL cAndroid::ProcessApk()
{
	if (Decompress() < 0) return FALSE;

	if (DexBuffer == NULL ||
	DexBufferSize < sizeof(DEX_HEADER)) 
	return FALSE;

	return ParseDex();
}

BOOL cAndroid::ParseDex()
{
	DexHeader = (DEX_HEADER*)DexBuffer;

	//check for magic code for opt header
	if (memcmp(DexHeader->magic, DEX_MAGIC, 4) != 0)
		return FALSE;

	memcpy_s((UCHAR*)DexVersion, 4, (UCHAR*)DexHeader->magic + 4, 4);

	if (DexBufferSize != DexHeader->fileSize)
		return FALSE;

	nStringIDs = DexHeader->stringIdsSize;
	nStringItems = nStringIDs;
	DexStringIds = (DEX_STRING_ID*)(DexBuffer + DexHeader->stringIdsOff);
	StringItems = (DEX_STRING_ITEM*)malloc(nStringItems * sizeof(DEX_STRING_ITEM));

	UINT StringSize;
	for (UINT i=0; i<nStringIDs; i++)
	{
		StringItems[i].data = ULEB128_to_UCHAR((UCHAR*)DexBuffer + DexStringIds[i].stringDataOff, &StringSize);
		StringItems[i].stringSize = StringSize;
		//printf("%s\n", StringItems[i].data);
	}

	nFieldIDs = DexHeader->fieldIdsSize;
	DexFieldIds = (DEX_FIELD_ID*)(DexBuffer + DexHeader->fieldIdsOff);

	nTypeIDs = DexHeader->typeIdsSize;
	DexTypeIds = (DEX_TYPE_ID*)(DexBuffer + DexHeader->typeIdsOff);

	for (UINT i=0; i<nTypeIDs; i++)
	{
		//printf("%s\n", StringItems[DexTypeIds[i].descriptorIdx].data);
	}

	nMethodIDs = DexHeader->methodIdsSize;
	DexMethodIds = (DEX_METHOD_ID*)(DexBuffer + DexHeader->methodIdsOff);

	nPrototypeIDs = DexHeader->protoIdsSize;
	DexProtoIds = (DEX_PROTO_ID*)(DexBuffer + DexHeader->protoIdsOff);

	for (UINT i=0; i<nPrototypeIDs; i++)
	{
		//printf("%s\n", StringItems[DexProtoIds[i].shortyIdx].data); 
		//printf("%s\n", StringItems[DexTypeIds[DexProtoIds[i].returnTypeIdx].descriptorIdx].data);
	}

	nClassDefinitions = DexHeader->classDefsSize;
	DexClassDefs = (DEX_CLASS_DEF*)(DexBuffer + DexHeader->classDefsOff);

	return TRUE;
}

long cAndroid::Decompress()
{
	ZipHandler = OpenZip((PVOID)BaseAddress, FileLength, 0);

	ZIPENTRY ArchieveEntry;
	INT ZipItemIndex;

	FindZipItem(ZipHandler, "classes.dex", true, &ZipItemIndex, &ArchieveEntry);

	DexBufferSize = ArchieveEntry.unc_size;
	DexBuffer = new char[DexBufferSize];
	UnzipItem(ZipHandler, ZipItemIndex, DexBuffer, DexBufferSize);

	CloseZip(ZipHandler);
	return DexBufferSize;
}

UCHAR *cAndroid::ULEB128_to_UCHAR(UCHAR *data, UINT *v) 
{
	UCHAR c;
	UINT s, sum;
	for (s = sum = 0; ; s+= 7) {
		c = *(data++) & 0xff;
		sum |= ((UINT) (c&0x7f)<<s);
		if (!(c&0x80)) break;
	}
	if (v) *v = sum;
	return data;
}

cAndroid::~cAndroid()
{
	free(StringItems);
}
