// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/StaticInstanceList.h>

#include <drx3D/Schema/EnumFlags.h>
#include <drx3D/Schema/PreprocessorUtils.h>
#include <drx3D/Schema/TypeUtils.h>

#define SXEMA_REGISTER_UNIT_TEST(function, name) static sxema::CUnitTestRegistrar SXEMA_PP_JOIN_XY(schematycUnitTestRegistrar, __COUNTER__)(function, name);

namespace sxema
{
enum class EUnitTestResultFlags
{
	Success       = 0,
	CriticalError = BIT(0),
	FatalError    = BIT(1)
};

typedef CEnumFlags<EUnitTestResultFlags> UnitTestResultFlags;

typedef UnitTestResultFlags (*           UnitTestFunctionPtr)();

class CUnitTestRegistrar : public CStaticInstanceList<CUnitTestRegistrar>
{
public:

	CUnitTestRegistrar(UnitTestFunctionPtr pFuntion, tukk szName);

	static void RunUnitTests();

private:

	UnitTestFunctionPtr m_pFuntion;
	tukk         m_szName;
};
}
