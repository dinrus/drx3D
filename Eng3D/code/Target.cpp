// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Eng3D/StdAfx.h>
#include <drx3D/Eng3D/Target.h>
#include <drx3D/CoreX/Serialization/Math.h>

DRX_PFX2_DBG

namespace pfx2
{

CTargetSource::CTargetSource(ETargetSource source)
	: m_offset(ZERO)
	, m_source(source)
{
}

void CTargetSource::Serialize(Serialization::IArchive& ar)
{
	ar(m_source, "Source", "^");
	ar(m_offset, "Offset", "Offset");
}

}
