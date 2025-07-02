// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/IDrxMannequin.h>
#include <drx3D/Act/Serialization.h>
#include <drx3D/Sys/DrxUnitTest.h>

namespace mannequin
{
namespace test
{
struct STestParams
	: public IProceduralParams
{
	STestParams(const string& paramName, i32k param)
		: m_paramName(paramName)
		, m_param(param)
	{
	}

	virtual void Serialize(Serialization::IArchive& ar)
	{
		ar(m_param, m_paramName.c_str(), m_paramName.c_str());
	}

	const string m_paramName;
	i32          m_param;
};
}

//////////////////////////////////////////////////////////////////////////
DRX_UNIT_TEST(IProceduralParams_OperatorEquals_Self)
{
	test::STestParams param("Param", 3);
	DRX_UNIT_TEST_ASSERT(param == param);
}

//////////////////////////////////////////////////////////////////////////
DRX_UNIT_TEST(IProceduralParams_OperatorEquals_DifferentInstancesEqual)
{
	test::STestParams param1("Param", 3);
	test::STestParams param2("Param", 3);
	DRX_UNIT_TEST_ASSERT(param1 == param2);
}

//////////////////////////////////////////////////////////////////////////
DRX_UNIT_TEST(IProceduralParams_OperatorEquals_DifferentParamNames)
{
	test::STestParams param1("Param1", 3);
	test::STestParams param2("Param2", 3);
	DRX_UNIT_TEST_ASSERT(!(param1 == param2));
}

//////////////////////////////////////////////////////////////////////////
DRX_UNIT_TEST(IProceduralParams_OperatorEquals_DifferentParamValues)
{
	test::STestParams param1("Param", 3);
	test::STestParams param2("Param", 2);
	DRX_UNIT_TEST_ASSERT(!(param1 == param2));
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
DRX_UNIT_TEST(IProceduralParams_StringWrapper_EmptyConstructor)
{
	const IProceduralParams::StringWrapper s;
	DRX_UNIT_TEST_ASSERT(string("") == s.c_str());
}

DRX_UNIT_TEST(IProceduralParams_StringWrapper_Constructor)
{
	const IProceduralParams::StringWrapper s("test");
	DRX_UNIT_TEST_ASSERT(string("test") == s.c_str());
}

DRX_UNIT_TEST(IProceduralParams_StringWrapper_OperatorEqual)
{
	IProceduralParams::StringWrapper s("test");
	DRX_UNIT_TEST_ASSERT(string("test") == s.c_str());

	s = "t";
	DRX_UNIT_TEST_ASSERT(string("t") == s.c_str());

	s = "test";
	DRX_UNIT_TEST_ASSERT(string("test") == s.c_str());

	IProceduralParams::StringWrapper s2;
	DRX_UNIT_TEST_ASSERT(string("") == s2.c_str());

	s2 = s = IProceduralParams::StringWrapper("test2");
	DRX_UNIT_TEST_ASSERT(string("test2") == s.c_str());
	DRX_UNIT_TEST_ASSERT(string("test2") == s2.c_str());
}

DRX_UNIT_TEST(IProceduralParams_StringWrapper_GetLength)
{
	IProceduralParams::StringWrapper s("test");
	DRX_UNIT_TEST_ASSERT(4 == s.GetLength());

	s = "t";
	DRX_UNIT_TEST_ASSERT(1 == s.GetLength());

	s = "test";
	DRX_UNIT_TEST_ASSERT(4 == s.GetLength());

	const IProceduralParams::StringWrapper s2("test2");
	DRX_UNIT_TEST_ASSERT(5 == s2.GetLength());
}

DRX_UNIT_TEST(IProceduralParams_StringWrapper_IsEmpty)
{
	{
		IProceduralParams::StringWrapper s("test");
		DRX_UNIT_TEST_ASSERT(!s.IsEmpty());

		s = "";
		DRX_UNIT_TEST_ASSERT(s.IsEmpty());

		s = "test";
		DRX_UNIT_TEST_ASSERT(!s.IsEmpty());
	}

	{
		const IProceduralParams::StringWrapper s("test");
		DRX_UNIT_TEST_ASSERT(!s.IsEmpty());
	}

	{
		const IProceduralParams::StringWrapper s("");
		DRX_UNIT_TEST_ASSERT(s.IsEmpty());
	}

	{
		const IProceduralParams::StringWrapper s;
		DRX_UNIT_TEST_ASSERT(s.IsEmpty());
	}
}
}
