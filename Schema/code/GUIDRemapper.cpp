// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/GUIDRemapper.h>

namespace sxema
{
bool CGUIDRemapper::Empty() const
{
	return m_guids.empty();
}

void CGUIDRemapper::Bind(const DrxGUID& from, const DrxGUID& to)
{
	m_guids.insert(GUIDs::value_type(from, to));
}

DrxGUID CGUIDRemapper::Remap(const DrxGUID& from) const
{
	GUIDs::const_iterator itGUID = m_guids.find(from);
	return itGUID != m_guids.end() ? itGUID->second : from;
}
} // sxema
