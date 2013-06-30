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

#include "cDex.h"
#include <stdio.h>

UCHAR *ULEB128_to_UCHAR (UCHAR *data, UINT *v) {
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

USHORT Uleb128Align(UINT Decimal)
{
	if (Decimal < 0xff) return 1;
	else if (Decimal < 0xffff) return 2;
	else if (Decimal < 0xffffff) return 3;
	else return 4;
};

cDex::cDex(CHAR* Filename)
{
	DexFilename = Filename;
	isLoaded = FALSE;

	DexFile = new cFile(Filename);

	if (DexFile->BaseAddress == NULL ||
		DexFile->FileLength < sizeof(DEX_FILE)) 
		return;

	BaseAddress = (UCHAR*)DexFile->BaseAddress;
	isLoaded = ParseDex();
}

cDex::~cDex()
{
	delete DexFile;
}

BOOL cDex::ParseDex()
{
	DexHeader = (DEX_HEADER*)BaseAddress;

	//check for magic code for opt header
	if (memcmp(DexHeader->magic, DEX_MAGIC, 4) != 0)
		return FALSE;

	memcpy_s((UCHAR*)DexVersion, 4, (UCHAR*)DexHeader->magic + 4, 4);

	if (DexFile->FileLength != DexHeader->fileSize)
		return FALSE;

	nStringIDs = DexHeader->stringIdsSize;
	nStringItems = nStringIDs;
	DexStringIds = (DEX_STRING_ID*)(BaseAddress + DexHeader->stringIdsOff);
	StringItems = (DEX_STRING_ITEM*)malloc(nStringItems * sizeof(DEX_STRING_ITEM));

	UINT StringSize;
	for (UINT i=0; i<nStringIDs; i++)
	{
		StringItems[i].data = ULEB128_to_UCHAR(BaseAddress + DexStringIds[i].stringDataOff, &StringSize);
		StringItems[i].stringSize = StringSize;
		//printf("%s\n", StringItems[i].data);
	}

	nFieldIDs = DexHeader->fieldIdsSize;
	DexFieldIds = (DEX_FIELD_ID*)(BaseAddress + DexHeader->fieldIdsOff);

	nTypeIDs = DexHeader->typeIdsSize;
	DexTypeIds = (DEX_TYPE_ID*)(BaseAddress + DexHeader->typeIdsOff);

	for (UINT i=0; i<nTypeIDs; i++)
	{
		//printf("%s\n", StringItems[DexTypeIds[i].descriptorIdx].data);
	}

	nMethodIDs = DexHeader->methodIdsSize;
	DexMethodIds = (DEX_METHOD_ID*)(BaseAddress + DexHeader->methodIdsOff);

	nPrototypeIDs = DexHeader->protoIdsSize;
	DexProtoIds = (DEX_PROTO_ID*)(BaseAddress + DexHeader->protoIdsOff);

	for (UINT i=0; i<nPrototypeIDs; i++)
	{
		//printf("%s\n", StringItems[DexProtoIds[i].shortyIdx].data); 
		//printf("%s\n", StringItems[DexTypeIds[DexProtoIds[i].returnTypeIdx].descriptorIdx].data);
	}

	nClassDefinitions = DexHeader->classDefsSize;
	DexClassDefs = (DEX_CLASS_DEF*)(BaseAddress + DexHeader->classDefsOff);

	return TRUE;
}

BOOL cDex::ValidChecksum()
{
	return TRUE;
}

