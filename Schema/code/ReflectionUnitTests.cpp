// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>

#include <drx3D/Schema/TypeDesc.h>

#include <drx3D/Schema/UnitTestRegistrar.h>
#include <drx3D/Schema/ReflectionUtils.h>

namespace sxema
{
namespace ReflectionUnitTests
{

enum class EType
{
	BaseA,
	BaseB,
	BaseC
};

struct SBaseA
{
	inline SBaseA()
		: type(EType::BaseA)
	{}

	static void ReflectType(CTypeDesc<SBaseA>& desc)
	{
		desc.SetGUID("f4c516fa-05be-4fdd-86f6-9bef1deeec8d"_drx_guid);
	}

	EType type;
};

struct SBaseB
{
	inline SBaseB()
		: type(EType::BaseB)
	{}

	static void ReflectType(CTypeDesc<SBaseB>& desc)
	{
		desc.SetGUID("c8b8de47-1fff-4ec9-8a45-462086611dcb"_drx_guid);
	}

	EType type;
};

struct SBaseC : public SBaseA, public SBaseB
{
	inline SBaseC()
		: type(EType::BaseC)
	{}

	static void ReflectType(CTypeDesc<SBaseC>& desc)
	{
		desc.SetGUID("618307d6-a7c5-4fd4-8859-db67ee998778"_drx_guid);
		desc.AddBase<SBaseA>();
		desc.AddBase<SBaseB>();
	}

	EType type;
};

struct SDerived : public SBaseC
{
	static void ReflectType(CTypeDesc<SDerived>& desc)
	{
		desc.SetGUID("660a3811-7c0c-450e-bd41-5f375cd11771"_drx_guid);
		desc.AddBase<SBaseC>();
	}
};

UnitTestResultFlags Run()
{
	SDerived derived;

	SBaseA* pBaseA = DynamicCast<SBaseA>(&derived);
	if(!pBaseA || (pBaseA->type != EType::BaseA))
	{
		return EUnitTestResultFlags::FatalError;
	}
	
	SBaseB* pBaseB = DynamicCast<SBaseB>(&derived);
	if (!pBaseB || (pBaseB->type != EType::BaseB))
	{
		return EUnitTestResultFlags::FatalError;
	}

	SBaseC* pBaseC = DynamicCast<SBaseC>(&derived);
	if (!pBaseC || (pBaseC->type != EType::BaseC))
	{
		return EUnitTestResultFlags::FatalError;
	}

	return EUnitTestResultFlags::Success;
}

} // ReflectionUnitTests

SXEMA_REGISTER_UNIT_TEST(&ReflectionUnitTests::Run, "Reflection")

} // sxema
