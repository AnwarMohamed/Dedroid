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
#include <stdio.h>

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

	/* Start String Items */
	nStringIDs = DexHeader->stringIdsSize;
	nStringItems = nStringIDs;
	DexStringIds = (DEX_STRING_ID*)(DexBuffer + DexHeader->stringIdsOff);
	StringItems = (DEX_STRING_ITEM*)malloc(nStringItems * sizeof(DEX_STRING_ITEM));

	UINT StringSize;
	for (UINT i=0; i<nStringIDs; i++)
	{
		StringItems[i].Data = ULEB128_to_UCHAR((UCHAR*)DexBuffer + DexStringIds[i].stringDataOff, &StringSize);
		StringItems[i].StringSize = StringSize;
	}
	/* End String Items */

	/* Start Field IDs */
	nFieldIDs = DexHeader->fieldIdsSize;
	DexFieldIds = (DEX_FIELD_ID*)(DexBuffer + DexHeader->fieldIdsOff);
	/* End Field IDs */

	/* Start Type IDs */
	nTypeIDs = DexHeader->typeIdsSize;
	DexTypeIds = (DEX_TYPE_ID*)(DexBuffer + DexHeader->typeIdsOff);
	/* End Type IDs */

	/* Start Method IDs */
	nMethodIDs = DexHeader->methodIdsSize;
	DexMethodIds = (DEX_METHOD_ID*)(DexBuffer + DexHeader->methodIdsOff);
	/* End Method IDs */

	/* Start Prototype IDs */
	nPrototypeIDs = DexHeader->protoIdsSize;
	DexProtoIds = (DEX_PROTO_ID*)(DexBuffer + DexHeader->protoIdsOff);
	/* End Prototype IDs */

	/* Start Class Definitions */
	nClassDefinitions = DexHeader->classDefsSize;
	nClasses = nClassDefinitions;

	DexClasses = (DEX_CLASS_STRUCTURE*)malloc(nClasses * sizeof(DEX_CLASS_STRUCTURE));
	DexClassDefs = (DEX_CLASS_DEF*)(DexBuffer + DexHeader->classDefsOff);

	for (UINT i=0; i<nClasses; i++)
	{
		DexClasses[i].Index = i;
		DexClasses[i].Descriptor = StringItems[DexTypeIds[DexClassDefs[i].classIdx].StringIndex].Data;
		DexClasses[i].AccessFlags = DexClassDefs[i].accessFlags;
		DexClasses[i].SuperClass = StringItems[DexTypeIds[DexClassDefs[i].superclassIdx].StringIndex].Data;

		if (DexClassDefs[i].sourceFileIdx != NO_INDEX)
			DexClasses[i].SourceFile = StringItems[DexClassDefs[i].sourceFileIdx].Data;
		else
			DexClasses[i].SourceFile = (UCHAR*)"NULL";
	}
	/* End Class Definitions */

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
	delete DexBuffer;
}
