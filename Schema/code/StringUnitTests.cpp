// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>

#include <drx3D/Schema/StackString.h>
#include <drx3D/Schema/SharedString.h>

#include <drx3D/Schema/UnitTestRegistrar.h>

namespace sxema
{
	namespace StringUnitTests
	{
		bool TestString(IString& value)
		{
			{
				value.assign("abcdef");
				if(strcmp(value.c_str(), "abcdef") != 0)
				{
					return false;
				}
				value.clear();
			}

			{
				tukk szValue = "_abcghi_";
				value.assign(szValue, 1, 6);
				value.insert(3, "def");
				if(strcmp(value.c_str(), "abcdefghi") != 0)
				{
					return false;
				}
				value.clear();
			}
			
			return true;
		}

		UnitTestResultFlags TestStrings()
		{
			{
				CStackString value;
				if (!TestString(value))
				{
					return EUnitTestResultFlags::FatalError;
				}
			}

			{
				CSharedString value;
				if (!TestString(value))
				{
					return EUnitTestResultFlags::FatalError;
				}
			}

			return EUnitTestResultFlags::Success;
		}

		UnitTestResultFlags Run()
		{
			UnitTestResultFlags resultFlags = EUnitTestResultFlags::Success;

			resultFlags.Add(TestStrings());

			return resultFlags;
		}
	}

	SXEMA_REGISTER_UNIT_TEST(&StringUnitTests::Run, "String")
}
