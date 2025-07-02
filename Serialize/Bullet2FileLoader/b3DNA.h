
#ifndef __BDNA_H__
#define __BDNA_H__

#include <drx3D/Serialize/Bullet2FileLoader/b3Common.h>

namespace bParse
{
struct bNameInfo
{
	char *m_name;
	bool m_isPointer;
	i32 m_dim0;
	i32 m_dim1;
};

class bDNA
{
public:
	bDNA();
	~bDNA();

	void init(char *data, i32 len, bool swap = false);

	i32 getArraySize(char *str);
	i32 getArraySizeNew(short name)
	{
		const bNameInfo &nameInfo = m_Names[name];
		return nameInfo.m_dim0 * nameInfo.m_dim1;
	}
	i32 getElementSize(short type, short name)
	{
		const bNameInfo &nameInfo = m_Names[name];
		i32 size = nameInfo.m_isPointer ? mPtrLen * nameInfo.m_dim0 * nameInfo.m_dim1 : mTlens[type] * nameInfo.m_dim0 * nameInfo.m_dim1;
		return size;
	}

	i32 getNumNames() const
	{
		return m_Names.size();
	}

	char *getName(i32 ind);
	char *getType(i32 ind);
	short *getStruct(i32 ind);
	short getLength(i32 ind);
	i32 getReverseType(short type);
	i32 getReverseType(tukk type);

	i32 getNumStructs();

	//
	bool lessThan(bDNA *other);

	void initCmpFlags(bDNA *memDNA);
	bool flagNotEqual(i32 dna_nr);
	bool flagEqual(i32 dna_nr);
	bool flagNone(i32 dna_nr);

	i32 getPointerSize();

	void dumpTypeDefinitions();

private:
	enum FileDNAFlags
	{
		FDF_NONE = 0,
		FDF_STRUCT_NEQU,
		FDF_STRUCT_EQU
	};

	void initRecurseCmpFlags(i32 i);

	b3AlignedObjectArray<i32> mCMPFlags;

	b3AlignedObjectArray<bNameInfo> m_Names;
	b3AlignedObjectArray<char *> mTypes;
	b3AlignedObjectArray<short *> mStructs;
	b3AlignedObjectArray<short> mTlens;
	b3HashMap<b3HashInt, i32> mStructReverse;
	b3HashMap<b3HashString, i32> mTypeLookup;

	i32 mPtrLen;
};
}  // namespace bParse

#endif  //__BDNA_H__
