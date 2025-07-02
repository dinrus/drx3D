// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Extension/DrxGUID.h>

namespace DrxGraph {

typedef struct SPinId
{
	enum : u16 { Invalid = 0xffff };

	SPinId()
		: m_id(Invalid)
	{}

	SPinId(u16 id)
		: m_id(id)
	{}

	SPinId operator=(u16 id)
	{
		m_id = id;
		return *this;
	}

	bool IsValid() const
	{
		return m_id != Invalid;
	}

	operator u16() const
	{
		return m_id;
	}

private:
	u16 m_id;
} PinId;

typedef struct SPinIndex
{
	enum : u16 { Invalid = 0xffff };

	SPinIndex()
		: m_index(Invalid)
	{}

	SPinIndex(u16 index)
		: m_index(index)
	{}

	SPinIndex operator=(u16 index)
	{
		m_index = index;
		return *this;
	}

	bool IsValid() const
	{
		return m_index != Invalid;
	}

	operator u16() const
	{
		return m_index;
	}

private:
	u16 m_index;
} PinIndex;

enum class EPinType
{
	Unset = 0,
	DataInput,
	DataOutput,
};

class CBaseNode
{
};

typedef DrxGUID DataType;

}

