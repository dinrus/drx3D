// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Reflection/StdAfx.h>
#include <drx3D/Reflection/ReflectedFunctionDesc.h>

#include <drx3D/Reflection/ReflectedTypeDesc.h>

namespace Drx {
namespace Reflection {

const ITypeDesc* CReflectedFunctionDesc::GetObjectTypeDesc() const
{
	return static_cast<const ITypeDesc*>(m_pObjectType);
}

} // ~Reflection namespace
} // ~Drx namespace
