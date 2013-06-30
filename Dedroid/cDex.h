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
#include "cFile.h"

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

struct DEX_OPT_HEADER 
{
    UCHAR	magic[8];           /* includes version number */

    UINT	dexOffset;          /* file offset of DEX header */
    UINT	dexLength;
    UINT	depsOffset;         /* offset of optimized DEX dependency table */
    UINT	depsLength;
    UINT	optOffset;          /* file offset of optimized data tables */
    UINT	optLength;

    UINT	flags;              /* some info flags */
    UINT	checksum;           /* adler32 checksum covering deps/opt */

    /* pad for 64-bit alignment if necessary */
};

struct DEX_FIELD_ID
{
    USHORT	classIdx;           /* index into typeIds list for defining class */
    USHORT	typeIdx;            /* index into typeIds for field type */
    UINT	nameIdx;            /* index into stringIds for field name */
};

struct DEX_METHOD_ID
{
    USHORT  classIdx;           /* index into typeIds list for defining class */
    USHORT  protoIdx;           /* index into protoIds for method prototype */
    UINT	nameIdx;            /* index into stringIds for method name */
};

struct DEX_PROTO_ID
{
    UINT	shortyIdx;          /* index into stringIds for shorty descriptor */
    UINT	returnTypeIdx;      /* index into typeIds list for return type */
    UINT	parametersOff;      /* file offset to type_list for parameter types */
};

struct DEX_CLASS_DEF 
{
    UINT	classIdx;           /* index into typeIds for this class */
    UINT	accessFlags;
    UINT	superclassIdx;      /* index into typeIds for superclass */
    UINT	interfacesOff;      /* file offset to DexTypeList */
    UINT	sourceFileIdx;      /* index into stringIds for source file name */
    UINT	annotationsOff;     /* file offset to annotations_directory_item */
    UINT	classDataOff;       /* file offset to class_data_item */
    UINT	staticValuesOff;    /* file offset to DexEncodedArray */
};

struct DEX_TYPE_ITEM	{ USHORT	typeIdx; };
struct DEX_LINK			{ USHORT	bleargh; };
struct DEX_STRING_ID	{ UINT		stringDataOff; };
struct DEX_TYPE_ID		{ UINT		descriptorIdx; };

struct DEX_CLASS_LOOKUP
{
    INT     size;                       // total size, including "size"
    INT     numEntries;                 // size of table[]; always power of 2
    struct {
        UINT	classDescriptorHash;    // class descriptor hash code
        INT     classDescriptorOffset;  // in bytes, from start of DEX
        INT     classDefOffset;         // in bytes, from start of DEX
    } table[1];
};

struct DEX_STRING_ITEM
{
	UINT	stringSize;
	UCHAR*	data;
};

struct DEX_FILE {
	//const DEX_OPT_HEADER* DexOptHeader;

	const DEX_HEADER*		DexHeader;
	const DEX_STRING_ID*	DexStringIds;
    const DEX_TYPE_ID*		DexTypeIds;
	const DEX_FIELD_ID*		DexFieldIds;
	const DEX_METHOD_ID*	DexMethodIds;
	const DEX_PROTO_ID*		DexProtoIds;
	const DEX_CLASS_DEF*	DexClassDefs;
	const DEX_LINK*			DexLinkData;

    const DEX_CLASS_LOOKUP* DexClassLookup;
    const void*				RegisterMapPool;
    const UCHAR*			baseAddr;
    INT						overhead;
};

#define DEX_MAGIC				"dex\n"
#define DEX_MAGIC_VERS			"036\0"
#define DEX_MAGIC_VERS_API_13	"035\0"
#define DEX_OPT_MAGIC			"dey\n"
#define DEX_OPT_MAGIC_VERS		"036\0"
#define DEX_DEP_MAGIC			"deps"

class DLLEXPORT cDex
{
	cFile*		DexFile;
	BOOL		ParseDex();
	BOOL		ValidChecksum();
public:
	cDex(CHAR* Filename);
	~cDex();

	CHAR*		DexFilename;
	BOOL		isLoaded;

	UCHAR		DexVersion[4];
	UINT		nStringIDs,
				nFieldIDs,
				nTypeIDs,
				nMethodIDs,
				nPrototypeIDs,
				nClassDefinitions,
				
				nStringItems;

	const DEX_HEADER*		DexHeader;
	const DEX_STRING_ID*	DexStringIds;
    const DEX_TYPE_ID*		DexTypeIds;
	const DEX_FIELD_ID*		DexFieldIds;
	const DEX_METHOD_ID*	DexMethodIds;
	const DEX_PROTO_ID*		DexProtoIds;
	const DEX_CLASS_DEF*	DexClassDefs;
	const DEX_LINK*			DexLinkData;

	DEX_STRING_ITEM*		StringItems;

	UCHAR*		BaseAddress;
};

