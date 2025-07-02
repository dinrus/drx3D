// Copyright 2001-2018 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include  <drx3D/CoreX/StaticInstanceList.h>
#include  <drx3D/Schema2/TemplateUtils_PreprocessorUtils.h>

#define SXEMA2_REGISTER_UNIT_TEST(function, name) static sxema2::CUnitTestRegistrar PP_JOIN_XY(schematycUnitTestRegistrar, __COUNTER__)(function, name);

namespace sxema2
{
	enum class EUnitTestResultFlags
	{
		Success       = 0,
		CriticalError = BIT(0),
		FatalError    = BIT(1)
	};

	DECLARE_ENUM_CLASS_FLAGS(EUnitTestResultFlags)

	typedef EUnitTestResultFlags (*UnitTestFunctionPtr)();

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