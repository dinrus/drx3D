// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>

struct STypeInfoInstance
{
	template<class T>
	STypeInfoInstance(T& obj)
		: m_pTypeInfo(&TypeInfo(&obj))
		, m_pObject(&obj)
	{
	}

	STypeInfoInstance(const CTypeInfo* typeInfo, uk object)
		: m_pTypeInfo(typeInfo)
		, m_pObject(object)
	{
	}

	inline void Serialize(Serialization::IArchive& ar);

	const CTypeInfo* m_pTypeInfo;
	uk            m_pObject;
	std::set<string> m_persistentStrings;
};

#include "TypeInfoImpl.h"
