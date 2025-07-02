#include <assert.h>

#include <drx3D/Serialize/Bullet2FileLoader/b3DNA.h>
#include <drx3D/Serialize/Bullet2FileLoader/b3Chunk.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//this define will force traversal of structures, to check backward (and forward) compatibility
//#define TEST_BACKWARD_FORWARD_COMPATIBILITY

using namespace bParse;

// ----------------------------------------------------- //
bDNA::bDNA()
	: mPtrLen(0)
{
	// --
}

// ----------------------------------------------------- //
bDNA::~bDNA()
{
	// --
}

// ----------------------------------------------------- //
bool bDNA::lessThan(bDNA *file)
{
	return (m_Names.size() < file->m_Names.size());
}

// ----------------------------------------------------- //
char *bDNA::getName(i32 ind)
{
	assert(ind <= (i32)m_Names.size());
	return m_Names[ind].m_name;
}

// ----------------------------------------------------- //
char *bDNA::getType(i32 ind)
{
	assert(ind <= (i32)mTypes.size());
	return mTypes[ind];
}

// ----------------------------------------------------- //
short *bDNA::getStruct(i32 ind)
{
	assert(ind <= (i32)mStructs.size());
	return mStructs[ind];
}

// ----------------------------------------------------- //
short bDNA::getLength(i32 ind)
{
	assert(ind <= (i32)mTlens.size());
	return mTlens[ind];
}

// ----------------------------------------------------- //
i32 bDNA::getReverseType(short type)
{
	i32 *intPtr = mStructReverse.find(type);
	if (intPtr)
		return *intPtr;

	return -1;
}

// ----------------------------------------------------- //
i32 bDNA::getReverseType(tukk type)
{
	b3HashString key(type);
	i32 *valuePtr = mTypeLookup.find(key);
	if (valuePtr)
		return *valuePtr;

	return -1;
}

// ----------------------------------------------------- //
i32 bDNA::getNumStructs()
{
	return (i32)mStructs.size();
}

// ----------------------------------------------------- //
bool bDNA::flagNotEqual(i32 dna_nr)
{
	assert(dna_nr <= (i32)mCMPFlags.size());
	return mCMPFlags[dna_nr] == FDF_STRUCT_NEQU;
}

// ----------------------------------------------------- //
bool bDNA::flagEqual(i32 dna_nr)
{
	assert(dna_nr <= (i32)mCMPFlags.size());
	i32 flag = mCMPFlags[dna_nr];
	return flag == FDF_STRUCT_EQU;
}

// ----------------------------------------------------- //
bool bDNA::flagNone(i32 dna_nr)
{
	assert(dna_nr <= (i32)mCMPFlags.size());
	return mCMPFlags[dna_nr] == FDF_NONE;
}

// ----------------------------------------------------- //
i32 bDNA::getPointerSize()
{
	return mPtrLen;
}

// ----------------------------------------------------- //
void bDNA::initRecurseCmpFlags(i32 iter)
{
	// iter is FDF_STRUCT_NEQU

	short *oldStrc = mStructs[iter];
	short type = oldStrc[0];

	for (i32 i = 0; i < (i32)mStructs.size(); i++)
	{
		if (i != iter && mCMPFlags[i] == FDF_STRUCT_EQU)
		{
			short *curStruct = mStructs[i];
			i32 eleLen = curStruct[1];
			curStruct += 2;

			for (i32 j = 0; j < eleLen; j++, curStruct += 2)
			{
				if (curStruct[0] == type)
				{
					//char *name = m_Names[curStruct[1]].m_name;
					//if (name[0] != '*')
					if (m_Names[curStruct[1]].m_isPointer)
					{
						mCMPFlags[i] = FDF_STRUCT_NEQU;
						initRecurseCmpFlags(i);
					}
				}
			}
		}
	}
}

// ----------------------------------------------------- //
void bDNA::initCmpFlags(bDNA *memDNA)
{
	// compare the file to memory
	// this ptr should be the file data

	assert(!(m_Names.size() == 0));  // && "SDNA empty!");
	mCMPFlags.resize(mStructs.size(), FDF_NONE);

	i32 i;
	for (i = 0; i < (i32)mStructs.size(); i++)
	{
		short *oldStruct = mStructs[i];

		i32 oldLookup = getReverseType(oldStruct[0]);
		if (oldLookup == -1)
		{
			mCMPFlags[i] = FDF_NONE;
			continue;
		}
		//tuk typeName = mTypes[oldStruct[0]];

//#define SLOW_FORWARD_COMPATIBLE 1
#ifdef SLOW_FORWARD_COMPATIBLE
		char *typeName = mTypes[oldLookup];
		i32 newLookup = memDNA->getReverseType(typeName);
		if (newLookup == -1)
		{
			mCMPFlags[i] = FDF_NONE;
			continue;
		}
		short *curStruct = memDNA->mStructs[newLookup];
#else
		// memory for file

		if (oldLookup < memDNA->mStructs.size())
		{
			short *curStruct = memDNA->mStructs[oldLookup];
#endif

		// rebuild...
		mCMPFlags[i] = FDF_STRUCT_NEQU;

#ifndef TEST_BACKWARD_FORWARD_COMPATIBILITY

		if (curStruct[1] == oldStruct[1])
		{
			// type len same ...
			if (mTlens[oldStruct[0]] == memDNA->mTlens[curStruct[0]])
			{
				bool isSame = true;
				i32 elementLength = oldStruct[1];

				curStruct += 2;
				oldStruct += 2;

				for (i32 j = 0; j < elementLength; j++, curStruct += 2, oldStruct += 2)
				{
					// type the same
					//tukk typeFileDNA = mTypes[oldStruct[0]];
					//tukk typeMemDNA = mTypes[curStruct[0]];
					if (strcmp(mTypes[oldStruct[0]], memDNA->mTypes[curStruct[0]]) != 0)
					{
						isSame = false;
						break;
					}

					// name the same
					if (strcmp(m_Names[oldStruct[1]].m_name, memDNA->m_Names[curStruct[1]].m_name) != 0)
					{
						isSame = false;
						break;
					}
				}
				// flag valid ==
				if (isSame)
					mCMPFlags[i] = FDF_STRUCT_EQU;
			}
		}
#endif
	}
}

// recurse in
for (i = 0; i < (i32)mStructs.size(); i++)
{
	if (mCMPFlags[i] == FDF_STRUCT_NEQU)
		initRecurseCmpFlags(i);
}
}

static i32 name_is_array(char *name, i32 *dim1, i32 *dim2)
{
	i32 len = strlen(name);
	/*fprintf(stderr,"[%s]",name);*/
	/*if (len >= 1) {
	if (name[len-1] != ']')
	return 1;
	}
	return 0;*/
	char *bp;
	i32 num;
	if (dim1)
	{
		*dim1 = 1;
	}
	if (dim2)
	{
		*dim2 = 1;
	}
	bp = strchr(name, '[');
	if (!bp)
	{
		return 0;
	}
	num = 0;
	while (++bp < name + len - 1)
	{
		const char c = *bp;
		if (c == ']')
		{
			break;
		}
		if (c <= '9' && c >= '0')
		{
			num *= 10;
			num += (c - '0');
		}
		else
		{
			printf("array parse error.\n");
			return 0;
		}
	}
	if (dim2)
	{
		*dim2 = num;
	}

	/* find second dim, if any. */
	bp = strchr(bp, '[');
	if (!bp)
	{
		return 1; /* at least we got the first dim. */
	}
	num = 0;
	while (++bp < name + len - 1)
	{
		const char c = *bp;
		if (c == ']')
		{
			break;
		}
		if (c <= '9' && c >= '0')
		{
			num *= 10;
			num += (c - '0');
		}
		else
		{
			printf("array2 parse error.\n");
			return 1;
		}
	}
	if (dim1)
	{
		if (dim2)
		{
			*dim1 = *dim2;
			*dim2 = num;
		}
		else
		{
			*dim1 = num;
		}
	}

	return 1;
}

// ----------------------------------------------------- //
void bDNA::init(char *data, i32 len, bool swap)
{
	i32 *intPtr = 0;
	short *shtPtr = 0;
	char *cp = 0;
	i32 dataLen = 0;
	//long nr=0;
	intPtr = (i32 *)data;

	/*
		SDNA (4 bytes) (magic number)
		NAME (4 bytes)
		<nr> (4 bytes) amount of names (i32)
		<string>
		<string>
	*/

	if (strncmp(data, "SDNA", 4) == 0)
	{
		// skip ++ NAME
		intPtr++;
		intPtr++;
	}

	// Parse names
	if (swap)
	{
		*intPtr = ChunkUtils::swapInt(*intPtr);
	}
	dataLen = *intPtr;
	intPtr++;

	cp = (char *)intPtr;
	i32 i;
	for (i = 0; i < dataLen; i++)
	{
		bNameInfo info;
		info.m_name = cp;
		info.m_isPointer = (info.m_name[0] == '*') || (info.m_name[1] == '*');
		name_is_array(info.m_name, &info.m_dim0, &info.m_dim1);
		m_Names.push_back(info);
		while (*cp) cp++;
		cp++;
	}

	cp = b3AlignPointer(cp, 4);

	/*
		TYPE (4 bytes)
		<nr> amount of types (i32)
		<string>
		<string>
	*/

	intPtr = (i32 *)cp;
	assert(strncmp(cp, "TYPE", 4) == 0);
	intPtr++;

	if (swap)
	{
		*intPtr = ChunkUtils::swapInt(*intPtr);
	}
	dataLen = *intPtr;
	intPtr++;

	cp = (char *)intPtr;
	for (i = 0; i < dataLen; i++)
	{
		mTypes.push_back(cp);
		while (*cp) cp++;
		cp++;
	}

	cp = b3AlignPointer(cp, 4);

	/*
		TLEN (4 bytes)
		<len> (short) the lengths of types
		<len>
	*/

	// Parse type lens
	intPtr = (i32 *)cp;
	assert(strncmp(cp, "TLEN", 4) == 0);
	intPtr++;

	dataLen = (i32)mTypes.size();

	shtPtr = (short *)intPtr;
	for (i = 0; i < dataLen; i++, shtPtr++)
	{
		if (swap)
			shtPtr[0] = ChunkUtils::swapShort(shtPtr[0]);
		mTlens.push_back(shtPtr[0]);
	}

	if (dataLen & 1) shtPtr++;

	/*
		STRC (4 bytes)
		<nr> amount of structs (i32)
		<typenr>
		<nr_of_elems>
		<typenr>
		<namenr>
		<typenr>
		<namenr>
	*/

	intPtr = (i32 *)shtPtr;
	cp = (char *)intPtr;
	assert(strncmp(cp, "STRC", 4) == 0);
	intPtr++;

	if (swap)
	{
		*intPtr = ChunkUtils::swapInt(*intPtr);
	}
	dataLen = *intPtr;
	intPtr++;

	shtPtr = (short *)intPtr;
	for (i = 0; i < dataLen; i++)
	{
		mStructs.push_back(shtPtr);
		if (swap)
		{
			shtPtr[0] = ChunkUtils::swapShort(shtPtr[0]);
			shtPtr[1] = ChunkUtils::swapShort(shtPtr[1]);

			i32 len = shtPtr[1];
			shtPtr += 2;

			for (i32 a = 0; a < len; a++, shtPtr += 2)
			{
				shtPtr[0] = ChunkUtils::swapShort(shtPtr[0]);
				shtPtr[1] = ChunkUtils::swapShort(shtPtr[1]);
			}
		}
		else
			shtPtr += (2 * shtPtr[1]) + 2;
	}

	// build reverse lookups
	for (i = 0; i < (i32)mStructs.size(); i++)
	{
		short *strc = mStructs.at(i);
		if (!mPtrLen && strcmp(mTypes[strc[0]], "ListBase") == 0)
		{
			mPtrLen = mTlens[strc[0]] / 2;
		}

		mStructReverse.insert(strc[0], i);
		mTypeLookup.insert(b3HashString(mTypes[strc[0]]), i);
	}
}

// ----------------------------------------------------- //
i32 bDNA::getArraySize(char *string)
{
	i32 ret = 1;
	i32 len = strlen(string);

	char *next = 0;
	for (i32 i = 0; i < len; i++)
	{
		char c = string[i];

		if (c == '[')
			next = &string[i + 1];
		else if (c == ']')
			if (next)
				ret *= atoi(next);
	}

	//	print (string << ' ' << ret);
	return ret;
}

void bDNA::dumpTypeDefinitions()
{
	i32 i;

	i32 numTypes = mTypes.size();

	for (i = 0; i < numTypes; i++)
	{
	}

	for (i = 0; i < (i32)mStructs.size(); i++)
	{
		i32 totalBytes = 0;
		short *oldStruct = mStructs[i];

		i32 oldLookup = getReverseType(oldStruct[0]);
		if (oldLookup == -1)
		{
			mCMPFlags[i] = FDF_NONE;
			continue;
		}

		short *newStruct = mStructs[oldLookup];
		char *typeName = mTypes[newStruct[0]];
		printf("%3d: %s ", i, typeName);

		//char *name = mNames[oldStruct[1]];
		i32 len = oldStruct[1];
		printf(" (%d fields) ", len);
		oldStruct += 2;

		printf("{");
		i32 j;
		for (j = 0; j < len; ++j, oldStruct += 2)
		{
			tukk name = m_Names[oldStruct[1]].m_name;
			printf("%s %s", mTypes[oldStruct[0]], name);
			i32 elemNumBytes = 0;
			i32 arrayDimensions = getArraySizeNew(oldStruct[1]);

			if (m_Names[oldStruct[1]].m_isPointer)
			{
				elemNumBytes = VOID_IS_8 ? 8 : 4;
			}
			else
			{
				elemNumBytes = getLength(oldStruct[0]);
			}
			printf(" /* %d bytes */", elemNumBytes * arrayDimensions);

			if (j == len - 1)
			{
				printf(";}");
			}
			else
			{
				printf("; ");
			}
			totalBytes += elemNumBytes * arrayDimensions;
		}
		printf("\ntotalBytes=%d\n\n", totalBytes);
	}

#if 0
	/* dump out display of types and their sizes */
	for (i=0; i<bf->types_count; ++i) {
		/* if (!bf->types[i].is_struct)*/
		{
			printf("%3d: sizeof(%s%s)=%d",
				i,
				bf->types[i].is_struct ? "struct " : "atomic ",
				bf->types[i].name, bf->types[i].size);
			if (bf->types[i].is_struct) {
				i32 j;
				printf(", %d fields: { ", bf->types[i].fieldtypes_count);
				for (j=0; j<bf->types[i].fieldtypes_count; ++j) {
					printf("%s %s",
						bf->types[bf->types[i].fieldtypes[j]].name,
						bf->names[bf->types[i].fieldnames[j]]);
					if (j == bf->types[i].fieldtypes_count-1) {
						printf(";}");
					} else {
						printf("; ");
					}
				}
			}
			printf("\n\n");

		}
	}
#endif
}

//eof
