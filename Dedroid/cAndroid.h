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
#include "unzip.h"

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
    USHORT	ClassIndex;           /* index into typeIds list for defining class */
    USHORT	TypeIdex;            /* index into typeIds for field type */
    UINT	StringIndex;            /* index into stringIds for field name */
};

struct DEX_METHOD_ID
{
    USHORT  ClassIndex;           /* index into typeIds list for defining class */
    USHORT  PrototypeIndex;           /* index into protoIds for method prototype */
    UINT	StringIndex;            /* index into stringIds for method name */
};

struct DEX_PROTO_ID
{
    UINT	StringIndex;          /* index into stringIds for shorty descriptor */
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
struct DEX_TYPE_ID		{ UINT		StringIndex; };

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
	UINT	StringSize;
	UCHAR*	Data;
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

enum {
    ACC_PUBLIC       = 0x00000001,       // class, field, method, ic
    ACC_PRIVATE      = 0x00000002,       // field, method, ic
    ACC_PROTECTED    = 0x00000004,       // field, method, ic
    ACC_STATIC       = 0x00000008,       // field, method, ic
    ACC_FINAL        = 0x00000010,       // class, field, method, ic
    ACC_SYNCHRONIZED = 0x00000020,       // method (only allowed on natives)
    ACC_SUPER        = 0x00000020,       // class (not used in Dalvik)
    ACC_VOLATILE     = 0x00000040,       // field
    ACC_BRIDGE       = 0x00000040,       // method (1.5)
    ACC_TRANSIENT    = 0x00000080,       // field
    ACC_VARARGS      = 0x00000080,       // method (1.5)
    ACC_NATIVE       = 0x00000100,       // method
    ACC_INTERFACE    = 0x00000200,       // class, ic
    ACC_ABSTRACT     = 0x00000400,       // class, method, ic
    ACC_STRICT       = 0x00000800,       // method
    ACC_SYNTHETIC    = 0x00001000,       // field, method, ic
    ACC_ANNOTATION   = 0x00002000,       // class, ic (1.5)
    ACC_ENUM         = 0x00004000,       // class, field, ic (1.5)
    ACC_CONSTRUCTOR  = 0x00010000,       // method (Dalvik only)
    ACC_DECLARED_SYNCHRONIZED =
                       0x00020000,       // method (Dalvik only)
    ACC_CLASS_MASK =
        (ACC_PUBLIC | ACC_FINAL | ACC_INTERFACE | ACC_ABSTRACT
                | ACC_SYNTHETIC | ACC_ANNOTATION | ACC_ENUM),
    ACC_INNER_CLASS_MASK =
        (ACC_CLASS_MASK | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC),
    ACC_FIELD_MASK =
        (ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC | ACC_FINAL
                | ACC_VOLATILE | ACC_TRANSIENT | ACC_SYNTHETIC | ACC_ENUM),
    ACC_METHOD_MASK =
        (ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC | ACC_FINAL
                | ACC_SYNCHRONIZED | ACC_BRIDGE | ACC_VARARGS | ACC_NATIVE
                | ACC_ABSTRACT | ACC_STRICT | ACC_SYNTHETIC | ACC_CONSTRUCTOR
                | ACC_DECLARED_SYNCHRONIZED),
};

struct DEX_CLASS_STRUCTURE
{
	UINT	Index;
	UCHAR*	Descriptor;
	UINT	AccessFlags;
	UCHAR*	SuperClass;
	UCHAR*	SourceFile;
};

UINT NO_INDEX = 0xffffffff; 

class DLLEXPORT cAndroid : public cFile
{
	HZIP	ZipHandler;
	UCHAR*	ULEB128_to_UCHAR(UCHAR *data, UINT *v);
	long	Decompress();
	BOOL	ProcessApk();
	BOOL	ParseDex();
	BOOL	ValidChecksum();

public:
	cAndroid(CHAR* ApkFilename);
	~cAndroid();

	CHAR*	ApkFilename;
	BOOL	isReady;

	CHAR*	DexBuffer;
	long	DexBufferSize;

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

	UINT nClasses;
	DEX_CLASS_STRUCTURE* DexClasses;
};

