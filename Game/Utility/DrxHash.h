// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __DRX_HASH_UTIL_H__
#define __DRX_HASH_UTIL_H__

#include <drx3D/CoreX/String/StringUtils.h>

//-----------------------------------------------------------------------------------
// HASH Tools.
// from Frd's code-base courtesy of AW. /FH
typedef u32 DrxHash;

struct DrxHashStringId
{
	DrxHashStringId()
		: id (0)
	{

	}

	DrxHashStringId(DrxHash _id)
		: id (_id)
	{

	}

	DrxHashStringId( tukk _name )
	{
		Set(_name);
	}

	DrxHashStringId(const DrxHashStringId& _otherId)
	{
#if defined(_DEBUG)
		debugName = _otherId.debugName;
#endif
		id = _otherId.id;
	}

	void Set( tukk _name )
	{
		DRX_ASSERT(_name);
#if defined(_DEBUG)
		debugName = _name;
#endif
		id = DrxStringUtils::HashString(_name);
	}

	ILINE bool operator == (const DrxHashStringId& rhs) const
	{
		return (id == rhs.id);
	}

	ILINE bool operator != (const DrxHashStringId& rhs) const
	{
		return (id != rhs.id);
	}

	ILINE bool operator <= (const DrxHashStringId& rhs) const
	{
		return (id <= rhs.id);
	}

	ILINE bool operator < (const DrxHashStringId& rhs) const
	{
		return (id < rhs.id);
	}

	ILINE bool operator >= (const DrxHashStringId& rhs) const
	{
		return (id >= rhs.id);
	}

	ILINE bool operator > (const DrxHashStringId& rhs) const
	{
		return (id > rhs.id);
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
#if defined(_DEBUG)
		pSizer->AddObject(debugName);
#endif
	}

	tukk GetDebugName() const
	{
#if defined(_DEBUG)
		return debugName.c_str();
#else
		return "";
#endif
	}

	//This is useful for look ups in debug, so we don't allocate strings
	static DrxHashStringId GetIdForName(tukk _name);

#if defined(_DEBUG)
	string debugName;
#endif
	DrxHash id;
};

#endif // __DRX_HASH_UTIL_H__
