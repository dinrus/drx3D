// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Reflection/IReflection.h>
#include <drx3D/Reflection/ITypeDesc.h>

namespace Drx {
namespace Reflection {

class CTypeRegistrar
{
public:
	CTypeRegistrar(const DrxTypeDesc& typeDesc, const DrxGUID& guid, ReflectTypeFunction pReflectFunc, const SSourceFileInfo& srcPos)
		: m_guid(guid)
		, m_pReflectFunc(pReflectFunc)
		, m_typeDesc(typeDesc)
		, m_srcPos(srcPos)
	{
		CTypeRegistrar*& pFirst = GetFirst();
		m_pNextRegistration = pFirst;
		pFirst = this;
	}

	~CTypeRegistrar()
	{
		// TODO: Proper unregister!
	}

	static bool RegisterTypes()
	{
		CTypeRegistrar* pRegistrar = GetFirst();
		while (pRegistrar)
		{
			pRegistrar->Register();
			pRegistrar = pRegistrar->GetNext();
		}
		return true;
	}

	bool      IsRegistered() const { return m_typeIndex.IsValid(); }
	TypeIndex GetTypeIndex() const { return m_typeIndex; }

	DrxTypeId GetTypeId() const
	{
		const ITypeDesc* pTypeDesc = GetTypeDesc();
		DRX_ASSERT_MESSAGE(pTypeDesc, "Object is not reflected.");
		return pTypeDesc ? pTypeDesc->GetTypeId() : DrxTypeId();
	}

	const ITypeDesc* GetTypeDesc() const
	{
		const ITypeDesc* pTypeDesc = GetReflectionRegistry().FindTypeByIndex(m_typeIndex);
		DRX_ASSERT_MESSAGE(pTypeDesc, "Object is not reflected.");
		return pTypeDesc;
	}

	CTypeRegistrar* GetNext() const { return m_pNextRegistration; }

private:
	void Register()
	{
		ITypeDesc* pTypeDesc = GetReflectionRegistry().Register(m_typeDesc, m_guid, m_pReflectFunc, m_srcPos);
		if (pTypeDesc)
		{
			m_typeIndex = pTypeDesc->GetIndex();
		}
	}

	static CTypeRegistrar*& GetFirst()
	{
		static CTypeRegistrar* pFirst = nullptr;
		return pFirst;
	}

private:
	CTypeRegistrar*     m_pNextRegistration;

	DrxGUID             m_guid;
	const DrxTypeDesc&  m_typeDesc;
	ReflectTypeFunction m_pReflectFunc;
	SSourceFileInfo     m_srcPos;
	TypeIndex           m_typeIndex;
};

} // ~Reflection namespace
} // ~Drx namespace
