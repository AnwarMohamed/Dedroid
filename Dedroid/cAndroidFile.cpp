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
 *  Note that some structs were coded by Google
 */

#include "cAndroidFile.h"
#include <stdio.h>
#include <string>
#include <cstdio>
#include <regex>
#include <iostream>
#include "hOpcodes.h"

#if _MSC_VER
#define snprintf _snprintf
#endif

using namespace std;

cAndroidFile::cAndroidFile(CHAR* ApkFilename) : cFile(ApkFilename)
{
	this->ApkFilename = ApkFilename;
	DexBuffer = NULL;
	isReady = ProcessApk();
}

BOOL cAndroidFile::ProcessApk()
{
	if (Decompress() < 0) return FALSE;

	if (DexBuffer == NULL ||
	DexBufferSize < sizeof(DEX_HEADER)) 
	return FALSE;

	return ParseDex();
}

BOOL cAndroidFile::ParseDex()
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

			UINT CurIndex = 0;

			for (UINT j=0; j<DexClasses[i].ClassData->StaticFieldsSize; j++)
			{	
				CurIndex += ReadUnsignedLeb128((const UCHAR**)&BufPtr);
				DexClasses[i].ClassData->StaticFields[j].Type = StringItems[DexTypeIds[ DexFieldIds[CurIndex].TypeIdex ].StringIndex].Data;
				DexClasses[i].ClassData->StaticFields[j].Name = StringItems[DexFieldIds[CurIndex].StringIndex].Data;
				DexClasses[i].ClassData->StaticFields[j].AccessFlags = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
			}

			CurIndex = 0;
			for (UINT j=0; j<DexClasses[i].ClassData->InstanceFieldsSize; j++)
			{
				CurIndex += ReadUnsignedLeb128((const UCHAR**)&BufPtr);
				DexClasses[i].ClassData->InstanceFields[j].Type = StringItems[DexTypeIds[DexFieldIds[CurIndex].TypeIdex].StringIndex].Data;
				DexClasses[i].ClassData->InstanceFields[j].Name = StringItems[DexFieldIds[CurIndex].StringIndex].Data;
				DexClasses[i].ClassData->InstanceFields[j].AccessFlags = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
			}

			CurIndex = 0;
			for (UINT j=0; j<DexClasses[i].ClassData->DirectMethodsSize; j++)
			{
				CurIndex += ReadUnsignedLeb128((const UCHAR**)&BufPtr);
				DexClasses[i].ClassData->DirectMethods[j].ProtoType = 
					StringItems[DexTypeIds[DexProtoIds[DexMethodIds[CurIndex].PrototypeIndex].returnTypeIdx].StringIndex].Data;

				//DexClasses[i].ClassData->DirectMethods[j].Type = StringItems[DexTypeIds[DexMethodIds[CurIndex].ClassIndex].StringIndex].Data;
				DexClasses[i].ClassData->DirectMethods[j].Name = StringItems[DexMethodIds[CurIndex].StringIndex].Data;
				DexClasses[i].ClassData->DirectMethods[j].AccessFlags = ReadUnsignedLeb128((const UCHAR**)&BufPtr);

				UINT code_offset = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
				if (code_offset == NULL)
					DexClasses[i].ClassData->DirectMethods[j].CodeDisassemblyArea = NULL;
				else
					GetCodeArea(DexClasses[i].ClassData->DirectMethods[j].CodeDisassemblyArea, code_offset);

			}

			CurIndex = 0;
			for (UINT j=0; j<DexClasses[i].ClassData->VirtualMethodsSize; j++)
			{
				CurIndex += ReadUnsignedLeb128((const UCHAR**)&BufPtr);
				DexClasses[i].ClassData->VirtualMethods[j].ProtoType = 
					StringItems[DexTypeIds[DexProtoIds[DexMethodIds[CurIndex].PrototypeIndex].returnTypeIdx].StringIndex].Data;

				//DexClasses[i].ClassData->VirtualMethods[j].Type = 
				//	StringItems[DexProtoIds[DexMethodIds[CurIndex].PrototypeIndex].StringIndex].Data;

				DexClasses[i].ClassData->VirtualMethods[j].Name = StringItems[DexMethodIds[CurIndex].StringIndex].Data;
				DexClasses[i].ClassData->VirtualMethods[j].AccessFlags = ReadUnsignedLeb128((const UCHAR**)&BufPtr);

				UINT code_offset = ReadUnsignedLeb128((const UCHAR**)&BufPtr);
				if (code_offset == NULL)
					DexClasses[i].ClassData->VirtualMethods[j].CodeDisassemblyArea = NULL;
				else
					GetCodeArea(DexClasses[i].ClassData->VirtualMethods[j].CodeDisassemblyArea, code_offset);
			}
		}
	}

	/* End Class Definitions */

	return TRUE;
}

long cAndroidFile::Decompress()
{
	string str;
	tr1::regex rx("res/");
	nResourceFiles = 0;
	ResourceFiles = (UCHAR**)malloc( nResourceFiles * sizeof(UCHAR*));

	ZipHandler = OpenZip((PVOID)BaseAddress, FileLength, 0);

	ZIPENTRY ArchieveEntry;
	INT ZipItemIndex;

	FindZipItem(ZipHandler, "classes.dex", true, &ZipItemIndex, &ArchieveEntry);

	DexBufferSize = ArchieveEntry.unc_size;
	DexBuffer = new char[DexBufferSize];
	UnzipItem(ZipHandler, ZipItemIndex, DexBuffer, DexBufferSize);

	GetZipItem(ZipHandler, -1, &ArchieveEntry); 
	UINT numitems=ArchieveEntry.index;

	for (UINT i=0; i<numitems; i++)
	{ 
		GetZipItem(ZipHandler, i, &ArchieveEntry);
		str = ArchieveEntry.name;
		if (regex_match(str.begin(), str.begin() + 4, rx))
		{
			nResourceFiles++;
			ResourceFiles = (UCHAR**)realloc(ResourceFiles, nResourceFiles * sizeof(UCHAR*));
			ResourceFiles[nResourceFiles-1] = new UCHAR[str.length()];
			memset(ResourceFiles[nResourceFiles-1], 0, str.length());
			int len = str.length();
			memcpy_s(ResourceFiles[nResourceFiles-1], str.length(), &ArchieveEntry.name, str.length());
		}
		
	}
	
	return DexBufferSize;
}

cAndroidFile::~cAndroidFile()
{
	CloseZip(ZipHandler);

	if (isReady)
	{
		free(StringItems);
		delete DexBuffer;

		for (UINT i=0; i<nResourceFiles; i++)
			delete ResourceFiles[i];

		free(ResourceFiles);
	}

}

INT cAndroidFile::ReadUnsignedLeb128(const UCHAR** pStream) 
{
    const UCHAR* ptr = *pStream;
    int result = *(ptr++);

    if (result > 0x7f) 
	{
        int cur = *(ptr++);
        result = (result & 0x7f) | ((cur & 0x7f) << 7);
        if (cur > 0x7f) 
		{
            cur = *(ptr++);
            result |= (cur & 0x7f) << 14;
            if (cur > 0x7f) 
			{
                cur = *(ptr++);
                result |= (cur & 0x7f) << 21;
                if (cur > 0x7f) 
				{
                    cur = *(ptr++);
                    result |= cur << 28;
                }
            }
        }
    }

    *pStream = ptr;
    return result;
};

typedef DEX_CLASS_STRUCTURE::CLASS_DATA::CLASS_METHOD::CLASS_CODE ClassCodeStruct;

void cAndroidFile::GetCodeArea(ClassCodeStruct *CodeDisassemblyArea, UINT Offset)
{
	CodeDisassemblyArea =  new ClassCodeStruct;
	DexCode = (DEX_CODE*)(DexBuffer + Offset);
	
	CodeDisassemblyArea->nRegisters = DexCode->registersSize; 
	CodeDisassemblyArea->nInArgvs = DexCode->insSize;
	CodeDisassemblyArea->nOutArgvs = DexCode->outsSize;

	CodeDisassemblyArea->nInstructions = 1;
	CodeDisassemblyArea->Instructions = (ClassCodeStruct::CLASS_CODE_INSTRUCTIONS*)
		malloc( sizeof(ClassCodeStruct::CLASS_CODE_INSTRUCTIONS) * (CodeDisassemblyArea->nInstructions - 1));

	UINT InstIndex = 0;
	for (UINT i=0; i< CodeDisassemblyArea->nInstructions; i++)
	{
		CodeDisassemblyArea->Instructions = (ClassCodeStruct::CLASS_CODE_INSTRUCTIONS*)
			realloc(CodeDisassemblyArea->Instructions, 
			sizeof(ClassCodeStruct::CLASS_CODE_INSTRUCTIONS) * CodeDisassemblyArea->nInstructions);

		CodeDisassemblyArea->Instructions[i].Opcode = DexCode->insns[InstIndex];
		CodeDisassemblyArea->Instructions[i].OpcodeMnemonic = GetOpcodeName(OpcodeFromUshort(DexCode->insns[InstIndex]));
		CodeDisassemblyArea->Instructions[i].nArgvs = GetWidthFromInstruction(&DexCode->insns[InstIndex]) - 1;

		if (CodeDisassemblyArea->Instructions[i].nArgvs < 1)
			CodeDisassemblyArea->Instructions[i].Argvs = NULL;
		else
		{
			CodeDisassemblyArea->Instructions[i].Argvs = new 
			ClassCodeStruct::CLASS_CODE_INSTRUCTIONS::INSTRUCTION_ARGVS;

			/* start get args for instruction */
		
			//get format
			DEX_OPCODES op = OpcodeFromUshort(DexCode->insns[InstIndex]);
			INSTRUCTIONS_FORMAT format = GetFormatFromOpcode(op);

			switch (format) 
			{
			case kFmt10x:       // op
				/* nothing to do; copy the AA bits out for the verifier */
				CodeDisassemblyArea->Instructions[i].Argvs->vA = INST_AA(op);
				break;

			case kFmt12x:       // op vA, vB
				CodeDisassemblyArea->Instructions[i].Argvs->vA = INST_A(op);
				CodeDisassemblyArea->Instructions[i].Argvs->vB = INST_B(op);
				break;

			case kFmt11n:       // op vA, #+B
				CodeDisassemblyArea->Instructions[i].Argvs->vA = INST_A(op);
				CodeDisassemblyArea->Instructions[i].Argvs->vB = (INT) (INST_B(op) << 28) >> 28; // sign extend 4-bit value
				break;

			case kFmt11x:       // op vAA
				CodeDisassemblyArea->Instructions[i].Argvs->vA = INST_AA(op);
				break;

			case kFmt10t:       // op +AA
				CodeDisassemblyArea->Instructions[i].Argvs->vA = (CHAR) INST_AA(op);              // sign-extend 8-bit value
				break;

			case kFmt20t:       // op +AAAA
				CodeDisassemblyArea->Instructions[i].Argvs->vA = (SHORT) FETCH(1, ((PUSHORT)DexCode->insns + InstIndex));                   // sign-extend 16-bit value
				break;

			case kFmt20bc:      // [opt] op AA, thing@BBBB
			case kFmt21c:       // op vAA, thing@BBBB
			case kFmt22x:       // op vAA, vBBBB
				CodeDisassemblyArea->Instructions[i].Argvs->vA = INST_AA(op);
				CodeDisassemblyArea->Instructions[i].Argvs->vB = FETCH(1, ((PUSHORT)DexCode->insns + InstIndex));
				break;

			case kFmt21s:       // op vAA, #+BBBB
			case kFmt21t:       // op vAA, +BBBB
				CodeDisassemblyArea->Instructions[i].Argvs->vA = INST_AA(op);
				CodeDisassemblyArea->Instructions[i].Argvs->vB = (SHORT) FETCH(1, ((PUSHORT)DexCode->insns + InstIndex));                   // sign-extend 16-bit value
				break;

			case kFmt21h:       // op vAA, #+BBBB0000[00000000]
				CodeDisassemblyArea->Instructions[i].Argvs->vA = INST_AA(op);
				CodeDisassemblyArea->Instructions[i].Argvs->vB = FETCH(1, ((PUSHORT)DexCode->insns + InstIndex));
				break;

			case kFmt23x:       // op vAA, vBB, vCC
				CodeDisassemblyArea->Instructions[i].Argvs->vA = INST_AA(op);
				CodeDisassemblyArea->Instructions[i].Argvs->vB = FETCH(1, ((PUSHORT)DexCode->insns + InstIndex)) & 0xff;
				CodeDisassemblyArea->Instructions[i].Argvs->vC = FETCH(1, ((PUSHORT)DexCode->insns + InstIndex)) >> 8;
				break;

			case kFmt22b:       // op vAA, vBB, #+CC
				CodeDisassemblyArea->Instructions[i].Argvs->vA = INST_AA(op);
				CodeDisassemblyArea->Instructions[i].Argvs->vB = FETCH(1, ((PUSHORT)DexCode->insns + InstIndex)) & 0xff;
				CodeDisassemblyArea->Instructions[i].Argvs->vC = (CHAR) (FETCH(1, DexCode->insns) >> 8);            // sign-extend 8-bit value
				break;

			case kFmt22s:       // op vA, vB, #+CCCC
			case kFmt22t:       // op vA, vB, +CCCC
				CodeDisassemblyArea->Instructions[i].Argvs->vA = INST_A(op);
				CodeDisassemblyArea->Instructions[i].Argvs->vB = INST_B(op);
				CodeDisassemblyArea->Instructions[i].Argvs->vC = (SHORT) FETCH(1, ((PUSHORT)DexCode->insns + InstIndex));                   // sign-extend 16-bit value
				break;

			case kFmt22c:       // op vA, vB, thing@CCCC
			case kFmt22cs:      // [opt] op vA, vB, field offset CCCC
				CodeDisassemblyArea->Instructions[i].Argvs->vA = INST_A(op);
				CodeDisassemblyArea->Instructions[i].Argvs->vB = INST_B(op);
				CodeDisassemblyArea->Instructions[i].Argvs->vC = FETCH(1, ((PUSHORT)DexCode->insns + InstIndex));
				break;

			case kFmt30t:       // op +AAAAAAAA
				CodeDisassemblyArea->Instructions[i].Argvs->vA = FETCH_UINT(1, ((PUSHORT)DexCode->insns + InstIndex));                     // signed 32-bit value
				break;

			case kFmt31t:       // op vAA, +BBBBBBBB
			case kFmt31c:       // op vAA, string@BBBBBBBB
				CodeDisassemblyArea->Instructions[i].Argvs->vA = INST_AA(op);
				CodeDisassemblyArea->Instructions[i].Argvs->vB = FETCH_UINT(1, ((PUSHORT)DexCode->insns + InstIndex));                     // 32-bit value
				break;

			case kFmt32x:       // op vAAAA, vBBBB
				CodeDisassemblyArea->Instructions[i].Argvs->vA = FETCH(1, ((PUSHORT)DexCode->insns + InstIndex));
				CodeDisassemblyArea->Instructions[i].Argvs->vB = FETCH(2, ((PUSHORT)DexCode->insns + InstIndex));
				break;

			case kFmt31i:       // op vAA, #+BBBBBBBB
				CodeDisassemblyArea->Instructions[i].Argvs->vA = INST_AA(op);
				CodeDisassemblyArea->Instructions[i].Argvs->vB = FETCH_UINT(1, ((PUSHORT)DexCode->insns + InstIndex));                     // signed 32-bit value
				break;

			case kFmt35c:       // op {vC, vD, vE, vF, vG}, thing@BBBB
			case kFmt35ms:      // [opt] invoke-virtual+super
			case kFmt35mi:      // [opt] inline invoke
			{

				USHORT regList;
				INT count;

				CodeDisassemblyArea->Instructions[i].Argvs->vA = INST_B(op); // This is labeled A in the spec.
				CodeDisassemblyArea->Instructions[i].Argvs->vB = FETCH(1, ((PUSHORT)DexCode->insns + InstIndex));
				regList = FETCH(2, ((PUSHORT)DexCode->insns + InstIndex));

				count = CodeDisassemblyArea->Instructions[i].Argvs->vA;

				switch (count) 
				{
				case 5:
					{
						if (format == kFmt35mi)
							break;

						CodeDisassemblyArea->Instructions[i].Argvs->arg[4] = INST_A(op);
					}
				case 4: CodeDisassemblyArea->Instructions[i].Argvs->arg[3] = (regList >> 12) & 0x0f;
				case 3: CodeDisassemblyArea->Instructions[i].Argvs->arg[2] = (regList >> 8) & 0x0f;
				case 2: CodeDisassemblyArea->Instructions[i].Argvs->arg[1] = (regList >> 4) & 0x0f;
				case 1: CodeDisassemblyArea->Instructions[i].Argvs->vC = CodeDisassemblyArea->Instructions[i].Argvs->arg[0] = regList & 0x0f; break;
				case 0: break; // Valid, but no need to do anything.
				default: break;
				
			
				}
				break;
			}

			case kFmt3rc:       // op {vCCCC .. v(CCCC+AA-1)}, meth@BBBB
			case kFmt3rms:      // [opt] invoke-virtual+super/range
			case kFmt3rmi:      // [opt] execute-inline/range
				CodeDisassemblyArea->Instructions[i].Argvs->vA = INST_AA(op);
				CodeDisassemblyArea->Instructions[i].Argvs->vB = FETCH(1, ((PUSHORT)DexCode->insns + InstIndex));
				CodeDisassemblyArea->Instructions[i].Argvs->vC = FETCH(2, ((PUSHORT)DexCode->insns + InstIndex));
				break;

			case kFmt51l:       // op vAA, #+BBBBBBBBBBBBBBBB
				CodeDisassemblyArea->Instructions[i].Argvs->vA = INST_AA(op);
				CodeDisassemblyArea->Instructions[i].Argvs->vB_wide = FETCH_UINT(1, ((PUSHORT)DexCode->insns + InstIndex)) | 
					((UINT64) FETCH_UINT(3, ((PUSHORT)DexCode->insns + InstIndex)) << 32);
				break;

			default:
				assert(false);
				break;
			}

			UINT index, width;

			switch (GetFormatFromOpcode(op)) {
			case kFmt20bc:
			case kFmt21c:
			case kFmt35c:
			case kFmt35ms:
			case kFmt3rc:
			case kFmt3rms:
			case kFmt35mi:
			case kFmt3rmi:
				index = CodeDisassemblyArea->Instructions[i].Argvs->vB;
				width = 4;
				break;
			case kFmt31c:
				index = CodeDisassemblyArea->Instructions[i].Argvs->vB;
				width = 8;
				break;
			case kFmt22c:
			case kFmt22cs:
				index = CodeDisassemblyArea->Instructions[i].Argvs->vC;
				width = 4;
				break;
			default:
				index = 0;
				width = 4;
				break;
			}

			INSTRUCTION_INDEX_TYPE IndexType = GetIndexTypeFromOpcode(op);
			CodeDisassemblyArea->Instructions[i].Argvs->IndexType = new CHAR;
			
			INT BufSize;
			
			switch (IndexType)
			{

			case kIndexUnknown:
				CodeDisassemblyArea->Instructions[i].Argvs->IndexType = "<unknown-index>";
				break;

			case kIndexNone:
				CodeDisassemblyArea->Instructions[i].Argvs->IndexType = "<no-index>";
				break;

			case kIndexVaries:
			/*
				* This one should never show up in a dexdump, so no need to try
				* to get fancy here.
				*/
			BufSize = snprintf(CodeDisassemblyArea->Instructions[i].Argvs->IndexType, 1, "<index-varies> // thing@%0*x", width, index);
			CodeDisassemblyArea->Instructions[i].Argvs->IndexType = new CHAR[BufSize + 1];
			snprintf(CodeDisassemblyArea->Instructions[i].Argvs->IndexType, BufSize + 1, "<index-varies> // thing@%0*x", width, index);
			break;
		case kIndexTypeRef:
			if (index < DexHeader->typeIdsSize) {
				BufSize = snprintf(CodeDisassemblyArea->Instructions[i].Argvs->IndexType, 1, "%s // type@%0*x", StringItems[DexTypeIds[index].StringIndex].Data, width, index);
				CodeDisassemblyArea->Instructions[i].Argvs->IndexType = new CHAR[BufSize + 1];
				snprintf(CodeDisassemblyArea->Instructions[i].Argvs->IndexType, BufSize +1, "%s // type@%0*x", StringItems[DexTypeIds[index].StringIndex].Data, width, index);

			} else {
				BufSize = snprintf(CodeDisassemblyArea->Instructions[i].Argvs->IndexType, 1, "<type?> // type@%0*x", width, index);
				CodeDisassemblyArea->Instructions[i].Argvs->IndexType = new CHAR[BufSize + 1];
				snprintf(CodeDisassemblyArea->Instructions[i].Argvs->IndexType, BufSize + 1, "<type?> // type@%0*x", width, index);
			}
			break;
		case kIndexStringRef:
			if (index < DexHeader->stringIdsSize) {
				//BufSize = snprintf(CodeDisassemblyArea->Instructions[i].Argvs->IndexType, 0, "'%s' // string@%0*x", StringItems[index].Data, width, index);
				CodeDisassemblyArea->Instructions[i].Argvs->IndexType = new CHAR[/*BufSize + 1*/100];
				snprintf(CodeDisassemblyArea->Instructions[i].Argvs->IndexType, /*BufSize +1*/100, "\"%s\" // string@%0*x", StringItems[index].Data, width, index);
			
			} else {
				BufSize = snprintf(CodeDisassemblyArea->Instructions[i].Argvs->IndexType, 1, "<string?> // string@%0*x", width, index);
				CodeDisassemblyArea->Instructions[i].Argvs->IndexType = new CHAR[BufSize + 1];
				snprintf(CodeDisassemblyArea->Instructions[i].Argvs->IndexType, BufSize +1, "<string?> // string@%0*x", width, index);
			}
			break;
		
		default:
			CodeDisassemblyArea->Instructions[i].Argvs->IndexType = "<?>";
			break;
		}

			/* finish get args for instruction */
		}

		InstIndex += CodeDisassemblyArea->Instructions[i].nArgvs + 1;
		if (InstIndex < DexCode->insnsSize)
			CodeDisassemblyArea->nInstructions++;
	}
	cout << endl;
};

cFile**	cAndroidFile::DecompressResourceFiles(/*INT Index*/)
{
	ZIPENTRY ArchieveEntry;
	INT ZipItemIndex;
	UINT BufferSize;
	CHAR* Buffer;
	cFile**	ResFiles;

	ResFiles = new cFile*[nResourceFiles];
	for (UINT i=0; i<nResourceFiles; i++)
	{
		memset(&ArchieveEntry, 0, sizeof(ZIPENTRY));

		FindZipItem(ZipHandler, (const TCHAR*)ResourceFiles[i], TRUE, &ZipItemIndex, &ArchieveEntry);
		BufferSize = ArchieveEntry.unc_size;
		Buffer = new CHAR[DexBufferSize];
		UnzipItem(ZipHandler, ZipItemIndex, Buffer, BufferSize);
		ResFiles[i] = new cFile(Buffer, BufferSize);
	}

	return ResFiles;
}