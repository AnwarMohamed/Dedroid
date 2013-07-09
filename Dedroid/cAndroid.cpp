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
	UCHAR* BufPtr;
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

	for (UINT i=0; i<nStringIDs; i++)
	{
		BufPtr = (UCHAR*)DexBuffer + DexStringIds[i].stringDataOff;
		StringItems[i].StringSize = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
		StringItems[i].Data = BufPtr;
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
		DexClasses[i].Descriptor = StringItems[DexTypeIds[DexClassDefs[i].classIdx].StringIndex].Data;
		DexClasses[i].AccessFlags = DexClassDefs[i].accessFlags;
		DexClasses[i].SuperClass = StringItems[DexTypeIds[DexClassDefs[i].superclassIdx].StringIndex].Data;

		if (DexClassDefs[i].sourceFileIdx != NO_INDEX)
			DexClasses[i].SourceFile = StringItems[DexClassDefs[i].sourceFileIdx].Data;
		else
			DexClasses[i].SourceFile = (UCHAR*)"No Information Found";

		if (DexClassDefs[i].classDataOff != NULL)
		{
			DexClasses[i].ClassData = new DEX_CLASS_STRUCTURE::CLASS_DATA;
			DexClassData = (DEX_CLASS_DATA*)(DexBuffer + DexClassDefs[i].classDataOff);

			BufPtr = (UCHAR*)DexClassData;

			DexClasses[i].ClassData->StaticFieldsSize = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
			DexClasses[i].ClassData->InstanceFieldsSize = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
			DexClasses[i].ClassData->DirectMethodsSize = ReadUnsignedLeb128((const UCHAR**)&BufPtr);	
			DexClasses[i].ClassData->VirtualMethodsSize = ReadUnsignedLeb128((const UCHAR**)&BufPtr);

			DexClasses[i].ClassData->StaticFields = 
				new DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_FIELD[DexClasses[i].ClassData->StaticFieldsSize];
			DexClasses[i].ClassData->InstanceFields = 
				new DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_FIELD[DexClasses[i].ClassData->InstanceFieldsSize];
			DexClasses[i].ClassData->DirectMethods = 
				new DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD[DexClasses[i].ClassData->DirectMethodsSize];
			DexClasses[i].ClassData->VirtualMethods = 
				new DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD[DexClasses[i].ClassData->VirtualMethodsSize];

			UINT tmp, array_size = 0;

			//if (DexClasses[i].ClassData->StaticFieldsSize > 0)
				//array_size = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
			for (UINT j=0; j<DexClasses[i].ClassData->StaticFieldsSize; j++)
			{	
				tmp = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
				DexClasses[i].ClassData->StaticFields[j].Type = StringItems[DexTypeIds[ DexFieldIds[tmp].TypeIdex ].StringIndex].Data;
				DexClasses[i].ClassData->StaticFields[j].Name = StringItems[DexFieldIds[tmp].StringIndex].Data;
				DexClasses[i].ClassData->StaticFields[j].AccessFlags = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
			}

			//if (DexClasses[i].ClassData->InstanceFieldsSize > 0)
				//array_size = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
			for (UINT j=0; j<DexClasses[i].ClassData->InstanceFieldsSize; j++)
			{
				tmp = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
				DexClasses[i].ClassData->InstanceFields[j].Type = StringItems[DexTypeIds[DexFieldIds[tmp].TypeIdex].StringIndex].Data;
				DexClasses[i].ClassData->InstanceFields[j].Name = StringItems[DexFieldIds[tmp].StringIndex].Data;
				DexClasses[i].ClassData->InstanceFields[j].AccessFlags = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
				//ReadUnsignedLeb128((const UCHAR**)&BufPtr);
			}

			//if (DexClasses[i].ClassData->DirectMethodsSize > 0)
				//array_size = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
			for (UINT j=0; j<DexClasses[i].ClassData->DirectMethodsSize; j++)
			{
				tmp = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
				DexClasses[i].ClassData->DirectMethods[j].ProtoType = 0;//StringItems[DexTypeIds[DexProtoIds[DexMethodIds[tmp].PrototypeIndex].returnTypeIdx].StringIndex].Data;
				DexClasses[i].ClassData->DirectMethods[j].Type = StringItems[DexTypeIds[DexMethodIds[tmp].ClassIndex].StringIndex].Data;
				DexClasses[i].ClassData->DirectMethods[j].Name = StringItems[DexMethodIds[tmp].StringIndex].Data;
				DexClasses[i].ClassData->DirectMethods[j].AccessFlags = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
				tmp = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
			}

			//if (DexClasses[i].ClassData->VirtualMethodsSize > 0)
				//array_size = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
			for (UINT j=0; j<DexClasses[i].ClassData->VirtualMethodsSize; j++)
			{
				tmp = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
				DexClasses[i].ClassData->VirtualMethods[j].ProtoType = 0;//StringItems[DexProtoIds[DexMethodIds[tmp].PrototypeIndex].StringIndex].Data;
				DexClasses[i].ClassData->VirtualMethods[j].Type = StringItems[DexTypeIds[DexMethodIds[tmp].ClassIndex].StringIndex].Data;
				DexClasses[i].ClassData->VirtualMethods[j].Name = StringItems[DexMethodIds[tmp].StringIndex].Data;
				DexClasses[i].ClassData->VirtualMethods[j].AccessFlags = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
				tmp = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
			}
		}
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

cAndroid::~cAndroid()
{
	free(StringItems);
	delete DexBuffer;
}

INT cAndroid::ReadUnsignedLeb128(const UCHAR** pStream) 
{
    const UCHAR* ptr = *pStream;
    int result = *(ptr++);

    if (result > 0x7f) {
        int cur = *(ptr++);
        result = (result & 0x7f) | ((cur & 0x7f) << 7);
        if (cur > 0x7f) {
            cur = *(ptr++);
            result |= (cur & 0x7f) << 14;
            if (cur > 0x7f) {
                cur = *(ptr++);
                result |= (cur & 0x7f) << 21;
                if (cur > 0x7f) {
                    /*
                     * Note: We don't check to see if cur is out of
                     * range here, meaning we tolerate garbage in the
                     * high four-order bits.
                     */
                    cur = *(ptr++);
                    result |= cur << 28;
                }
            }
        }
    }

    *pStream = ptr;
    return result;
}
