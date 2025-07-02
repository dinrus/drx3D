// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/ByteStream.h>

u32 CByteStreamPacker::GetNextOfs(u32 sz)
{
	u32 out;
	switch (sz)
	{
	case 0:
		return m_data[eD_Size];
	case 1:
		out = GetNextOfs_Fixed(NTypelist::Int2Type<1>());
		break;
	case 2:
		out = GetNextOfs_Fixed(NTypelist::Int2Type<2>());
		break;
	case 3:
	case 4:
		out = GetNextOfs_Fixed(NTypelist::Int2Type<4>());
		break;
	default:
		out = m_data[eD_Size];
		m_data[eD_Size] += sz + (sz % 4 != 0) * (4 - sz % 4);
		break;
	}
	return out;
}

void CByteOutputStream::Grow(size_t sz)
{
	size_t oldCapacity = m_capacity;
	// exponentially grow, to get an amortized constant growth time
	do
		m_capacity *= 2;
	while (m_capacity < sz);
	m_buffer = (u8*)m_pSA->Realloc(m_buffer, m_capacity);
	memset(m_buffer + oldCapacity, 0, m_capacity - oldCapacity);
}
