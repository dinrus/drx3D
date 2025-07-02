// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/CoreX/Math/Random.h>
#include <drx3D/Schema/CoreAPI.h>

namespace sxema
{
namespace Bool
{
bool Flip(bool x)
{
	return !x;
}

bool Or(bool a, bool b)
{
	return a || b;
}

bool And(bool a, bool b)
{
	return a && b;
}

bool XOR(bool a, bool b)
{
	return a != b;
}

bool Equal(bool a, bool b)
{
	return a == b;
}

bool NotEqual(bool a, bool b)
{
	return a != b;
}

i32 ToInt32(bool bValue)
{
	return bValue ? 1 : 0;
}

u32 ToUInt32(bool bValue)
{
	return bValue ? 1 : 0;
}

void ToString(bool bValue, CSharedString& result)
{
	ToString(result, bValue);
}

static void RegisterFunctions(IEnvRegistrar& registrar)
{
	CEnvRegistrationScope scope = registrar.Scope(GetTypeDesc<bool>().GetGUID());
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Flip, "6a1d0e15-c4ad-4c87-ab38-3739020dd708"_drx_guid, "Flip");
		pFunction->SetDescription("Flip boolean value");
		pFunction->BindInput(1, 'x', "Value");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Or, "ff6f2ed1-942b-4726-9f13-b9e711cd28df"_drx_guid, "Or");
		pFunction->SetDescription("Logical OR operation");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&And, "87d62e0c-d32f-4ea7-aa5c-22aeeb1c9cd5"_drx_guid, "And");
		pFunction->SetDescription("Logical AND operation");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&XOR, "14a312fc-0885-4ab7-8bef-f124588e4c45"_drx_guid, "XOR");
		pFunction->SetDescription("Logical XOR operation");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Equal, "{ACA31DD3-AFC3-4EB1-AF27-27C70D25B1A6}"_drx_guid, "Equal");
		pFunction->SetDescription("Checks whether A and B are equal");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&NotEqual, "{C40AFA65-409B-4C69-922E-88F119DE18F4}"_drx_guid, "NotEqual");
		pFunction->SetDescription("Checks whether A and B are not equal");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&ToInt32, "F93DB38C-E4A1-4553-84D2-255A1E47AC8A"_drx_guid, "ToInt32");
		pFunction->SetDescription("Convert boolean to i32");
		pFunction->BindInput(1, 'val', "Value");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&ToUInt32, "FE42F543-AA16-47E8-A1EA-D4374C01B4A3"_drx_guid, "ToUInt32");
		pFunction->SetDescription("Convert boolean to u32");
		pFunction->BindInput(1, 'val', "Value");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&ToString, "9a6f082e-ba3c-4510-9248-8bf8235d5e03"_drx_guid, "ToString");
		pFunction->SetDescription("Convert boolean to string");
		pFunction->BindInput(1, 'val', "Value");
		pFunction->BindOutput(2, 'res', "String"); // #SchematycTODO : Rename 'Result'!
		scope.Register(pFunction);
	}
}
} // Bool

namespace Generic
{
	template<typename T>
	T Add(T a, T b)
	{
		return a + b;
	}

	template<typename T>
	T Subtract(T a, T b)
	{
		return a - b;
	}

	template<typename T>
	T Multiply(T a, T b)
	{
		return a * b;
	}

	template<typename T>
	T Divide(T a, T b)
	{
		return b != 0 ? a / b : 0;
	}

	template<typename T>
	T Modulus(T a, T b)
	{
		return b != 0 ? a % b : 0;
	}

	template<typename T>
	T Random(T min, T max)
	{
		return min < max ? drx_random(min, max) : min;
	}

	template<typename T>
	T Clamp(T value, T min, T max)
	{
		return drxmath::clamp(value, min, max);
	}

	template<typename T>
	T Abs(T x)
	{
		return abs(x);
	}

	template<typename T>
	T Negate(T x)
	{
		return -x;
	}

	template<typename T>
	i32 Sign(T x)
	{
		return sgn(x);
	}

	template<typename T>
	bool Equal(T a, T b)
	{
		return a == b;
	}

	template<typename T>
	bool NotEqual(T a, T b)
	{
		return a != b;
	}

	template<typename T>
	bool LessThan(T a, T b)
	{
		return a < b;
	}

	template<typename T>
	bool LessThanOrEqual(T a, T b)
	{
		return a <= b;
	}

	template<typename T>
	bool GreaterThan(T a, T b)
	{
		return a > b;
	}

	template<typename T>
	bool GreaterThanOrEqual(T a, T b)
	{
		return a >= b;
	}
}

namespace Int32
{

float ToFloat(i32 value)
{
	return static_cast<float>(value);
}

u32 ToUInt32(i32 value)
{
	return static_cast<u32>(value);
}

void ToString(i32 value, CSharedString& result)
{
	ToString(result, value);
}

static void RegisterFunctions(IEnvRegistrar& registrar)
{
	CEnvRegistrationScope scope = registrar.Scope(GetTypeDesc<i32>().GetGUID());
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::Add<i32>, "8aaf394a-5288-40df-b31e-47e0c9757a93"_drx_guid, "Add");
		pFunction->SetDescription("Add A to B");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::Subtract<i32>, "cf5e826b-f22d-4fde-8a0c-259ceea75416"_drx_guid, "Subtract");
		pFunction->SetDescription("Subtract B from A");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::Multiply<i32>, "d03e2bfe-79fc-4321-bf30-13be640443bc"_drx_guid, "Multiply");
		pFunction->SetDescription("Multiply A by B");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::Divide<i32>, "450af09f-e7ea-4c6b-8cda-a913e53f4460"_drx_guid, "Divide");
		pFunction->SetDescription("Divide A by B");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::Modulus<i32>, "37a5e2ba-caba-431f-882a-3048a6b18904"_drx_guid, "Modulus");
		pFunction->SetDescription("Calculate remainder when A is divided by B");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::Random<i32>, "67bc3c63-e6f2-42c2-85af-a30fc30d0b11"_drx_guid, "Random");
		pFunction->SetDescription("Generate random number");
		pFunction->BindInput(1, 'min', "Min", "Minimum value");
		pFunction->BindInput(2, 'max', "Max", "Maximum value", i32(100));
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::Abs<i32>, "f2f63e0a-3adc-4659-9444-32546d73c550"_drx_guid, "Absolute");
		pFunction->SetDescription("Returns the absolute value");
		pFunction->BindInput(1, 'x', "Value"); // #SchematycTODO : Rename 'X'!
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::Negate<i32>, "{C244F1BA-9B6C-46E2-BB5C-6AD1CD0CBCF5}"_drx_guid, "Negate");
		pFunction->SetDescription("Returns the negated value, for example -1 becomes 1, 1 becomes -1");
		pFunction->BindInput(1, 'x', "Value");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::Sign<i32>, "{37661484-156F-430F-AA8B-5F3CFBC87BCC}"_drx_guid, "Sign");
		pFunction->SetDescription("Gets the sign of the entity, for example 10 returns 1, -10 returns -1");
		pFunction->BindInput(1, 'x', "Value");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::Equal<i32>, "{8437F7B5-E324-446F-AEC7-D7C5DAFCDFFB}"_drx_guid, "Equal");
		pFunction->SetDescription("Checks if A and B are equal");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::NotEqual<i32>, "{57974989-AB5F-4290-8670-37C2B8D4B12E}"_drx_guid, "NotEqual");
		pFunction->SetDescription("Checks if A and B are not equal");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::LessThan<i32>, "{8104A615-0C6D-4236-A4C2-2F2D63F7CA70}"_drx_guid, "LessThan");
		pFunction->SetDescription("Checks if A is less than B");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::LessThanOrEqual<i32>, "{C202CA58-821A-4A89-B3B7-CC869F3E9996}"_drx_guid, "LessThanOrEqual");
		pFunction->SetDescription("Checks if A is less than or equals B");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::GreaterThan<i32>, "{ED112D0B-3FDC-48DE-8D2F-A2240F4B0541}"_drx_guid, "GreaterThan");
		pFunction->SetDescription("Checks if A is greater than B");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::GreaterThanOrEqual<i32>, "{76BAAADB-ECEC-4038-A2EF-AB9CA696FBA0}"_drx_guid, "GreaterThanOrEqual");
		pFunction->SetDescription("Checks if A is greater than or equals B");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&ToFloat, "dc514c68-89ea-4c14-8ce3-9b45f950409a"_drx_guid, "ToFloat");
		pFunction->SetDescription("Convert i32 to float");
		pFunction->BindInput(1, 'val', "Value");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&ToUInt32, "FE95372E-42BA-48C7-A051-7B91BDC85DE4"_drx_guid, "ToUInt32");
		pFunction->SetDescription("Convert i32 to u32");
		pFunction->BindInput(1, 'val', "Value");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&ToString, "71b9bfa9-8578-42e3-8ad3-18ff4bb74cca"_drx_guid, "ToString");
		pFunction->SetDescription("Convert i32 to string");
		pFunction->BindInput(1, 'val', "Value");
		pFunction->BindOutput(2, 'res', "String"); // #SchematycTODO : Rename 'Result'!
		scope.Register(pFunction);
	}
}
} // Int32

namespace UInt32
{
float ToFloat(u32 value)
{
	return static_cast<float>(value);
}

i32 ToInt32(u32 value)
{
	return static_cast<i32>(value);
}

void ToString(u32 value, CSharedString& result)
{
	ToString(result, value);
}

static void RegisterFunctions(IEnvRegistrar& registrar)
{
	CEnvRegistrationScope scope = registrar.Scope(GetTypeDesc<u32>().GetGUID());
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::Add<u32>, "5dcc5f2e-573e-41e8-8b38-4aaf5a9c0854"_drx_guid, "Add");
		pFunction->SetDescription("Add A to B");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::Subtract<u32>, "bd78264c-f111-4760-b9bc-7a65fe2b722e"_drx_guid, "Subtract");
		pFunction->SetDescription("Subtract B from A");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::Multiply<u32>, "53a75fa7-e113-4f24-99c0-a88a54ea1bf7"_drx_guid, "Multiply");
		pFunction->SetDescription("Multiply A by B");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::Divide<u32>, "2258b2d2-387d-41f3-bf98-63f2458fc1d9"_drx_guid, "Divide");
		pFunction->SetDescription("Divide A by B");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::Modulus<u32>, "770748ab-83c5-4da5-8bf8-c9006b75d8d7"_drx_guid, "Modulus");
		pFunction->SetDescription("Calculate remainder when A is divided by B");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::Random<u32>, "d06c1c65-a8be-4473-aae0-23e7f8e1ff98"_drx_guid, "Random");
		pFunction->SetDescription("Generate random number");
		pFunction->BindInput(1, 'min', "Min", "Minimum value");
		pFunction->BindInput(2, 'max', "Max", "Maximum value", i32(100));
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::Equal<u32>, "{A53F1140-6DE8-4D75-A803-40085CCD8622}"_drx_guid, "Equal");
		pFunction->SetDescription("Checks if A and B are equal");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::NotEqual<u32>, "{66B34C59-8E3A-4698-AFBD-2051D88D644A}"_drx_guid, "NotEqual");
		pFunction->SetDescription("Checks if A and B are not equal");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::LessThan<u32>, "{B7757CAD-8FDE-4FF6-AD07-8D9E66F9B34E}"_drx_guid, "LessThan");
		pFunction->SetDescription("Checks if A is less than B");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::LessThanOrEqual<u32>, "{FC372AA0-1FDF-4932-A170-CE6101F7A464}"_drx_guid, "LessThanOrEqual");
		pFunction->SetDescription("Checks if A is less than or equals B");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::GreaterThan<u32>, "{A4EFE085-DD9F-4BF7-92E4-35E32359B509}"_drx_guid, "GreaterThan");
		pFunction->SetDescription("Checks if A is greater than B");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::GreaterThanOrEqual<u32>, "{86C707B1-0ABD-48B8-88D0-CF768EBA2277}"_drx_guid, "GreaterThanOrEqual");
		pFunction->SetDescription("Checks if A is greater than or equals B");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&ToFloat, "89fc18a2-0fde-4d07-ada6-7d95ef613132"_drx_guid, "ToFloat");
		pFunction->SetDescription("Convert u32 to float");
		pFunction->BindInput(1, 'val', "Value");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&ToInt32, "ee7dbbc9-d950-44a4-9fd4-0a3891414147"_drx_guid, "ToInt32");
		pFunction->SetDescription("Convert u32 to i32");
		pFunction->BindInput(1, 'val', "Value");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&ToString, "38a094d6-a8b2-4a87-baee-8d456faf3739"_drx_guid, "ToString");
		pFunction->SetDescription("Convert u32 to string");
		pFunction->BindInput(1, 'val', "Value");
		pFunction->BindOutput(2, 'res', "String"); // #SchematycTODO : Rename 'Result'!
		scope.Register(pFunction);
	}
}
} // UInt32

namespace Float
{

	float Sin(float x)
	{
		return sin_tpl(x);
	}

	float ArcSin(float x)
	{
		return asin_tpl(x);
	}

	float Cos(float x)
	{
		return cos_tpl(x);
	}

	float ArcCos(float x)
	{
		return acos_tpl(x);
	}

	float Tan(float x)
	{
		return tan_tpl(x);
	}

	float ArcTan(float x)
	{
		return atan_tpl(x);
	}

	float Modulus(float a, float b)
	{
		return b != 0.0f ? fmodf(a, b) : 0.0f;
	}

	float Abs(float x)
	{
		return fabs_tpl(x);
	}

	i32 ToInt32(float input)
	{
		return static_cast<i32>(input);
	}

	u32 ToUInt32(float input)
	{
		return static_cast<u32>(input);
	}

	void ToString(float input, CSharedString& output)
	{
		sxema::ToString(output, input);
	}

	static void RegisterFunctions(IEnvRegistrar& registrar)
	{
		CEnvRegistrationScope scope = registrar.Scope(GetTypeDesc<float>().GetGUID());
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::Add<float>, "d62aede8-ed2c-47d9-b7f3-6de5ed0ef51f"_drx_guid, "Add");
			pFunction->SetDescription("Add A to B");
			pFunction->BindInput(1, 'a', "A");
			pFunction->BindInput(2, 'b', "B");
			pFunction->BindOutput(0, 'res', "Output"); // #SchematycTODO : Rename 'Result'!
			scope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::Subtract<float>, "3a7c8a1d-2938-40f8-b5ac-b6becf32baf2"_drx_guid, "Subtract");
			pFunction->SetDescription("Subtract B from A");
			pFunction->BindInput(1, 'a', "A");
			pFunction->BindInput(2, 'b', "B");
			pFunction->BindOutput(0, 'res', "Output"); // #SchematycTODO : Rename 'Result'!
			scope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::Multiply<float>, "b04f428a-4408-4c3d-b73b-173a9738c857"_drx_guid, "Multiply");
			pFunction->SetDescription("Multiply A by B");
			pFunction->BindInput(1, 'a', "A");
			pFunction->BindInput(2, 'b', "B");
			pFunction->BindOutput(0, 'res', "Output"); // #SchematycTODO : Rename 'Result'!
			scope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::Divide<float>, "fd26901b-dbbe-4f4d-b8c1-6547f55339ae"_drx_guid, "Divide");
			pFunction->SetDescription("Divide A by B");
			pFunction->BindInput(1, 'a', "A");
			pFunction->BindInput(2, 'b', "B");
			pFunction->BindOutput(0, 'res', "Output"); // #SchematycTODO : Rename 'Result'!
			scope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::Clamp<float>, "350ad86a-a61a-40c4-9f2e-30b6fdf2ae06"_drx_guid, "Clamp");
			pFunction->SetDescription("Clamp the value between min and max");
			pFunction->BindInput(1, 'val', "Value");
			pFunction->BindInput(2, 'min', "Min");
			pFunction->BindInput(3, 'max', "Max");
			pFunction->BindOutput(0, 'res', "Output"); // #SchematycTODO : Rename 'Result'!
			scope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Modulus, "fc35a48f-7926-4417-b940-08162bc4a9a5"_drx_guid, "Modulus");
			pFunction->SetDescription("Calculate remainder when A is divided by B");
			pFunction->BindInput(1, 'a', "A");
			pFunction->BindInput(2, 'b', "B");
			pFunction->BindOutput(0, 'res', "Output"); // #SchematycTODO : Rename 'Result'!
			scope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Sin, "{7F014586-066F-467B-B8C5-F2862BCF193A}"_drx_guid, "Sin");
			pFunction->SetDescription("Calculate sine of X");
			pFunction->BindInput(1, 'x', "X");
			pFunction->BindOutput(0, 'res', "Result");
			scope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&ArcSin, "{EF239779-D4A9-406B-82F6-8688E27A3962}"_drx_guid, "ArcSin");
			pFunction->SetDescription("Calculate arc-sine of X");
			pFunction->BindInput(1, 'x', "X");
			pFunction->BindOutput(0, 'res', "Result");
			scope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Cos, "{EFC13075-2624-42D8-8D15-F5E24E68A4E8}"_drx_guid, "Cos");
			pFunction->SetDescription("Calculate cosine of X");
			pFunction->BindInput(1, 'x', "X");
			pFunction->BindOutput(0, 'res', "Result");
			scope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&ArcCos, "{3852F6E4-6D24-4278-AA84-4888C04C1B12}"_drx_guid, "ArcCos");
			pFunction->SetDescription("Calculate arc-cosine of X");
			pFunction->BindInput(1, 'x', "X");
			pFunction->BindOutput(0, 'res', "Result");
			scope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Tan, "{746FCEB3-B64F-4384-A988-C2593D70A874}"_drx_guid, "Tan");
			pFunction->SetDescription("Calculate tangent of X");
			pFunction->BindInput(1, 'x', "X");
			pFunction->BindOutput(0, 'res', "Result");
			scope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&ArcTan, "{E7934899-D17F-45EF-BFB4-D37B0D831E13}"_drx_guid, "ArcTan");
			pFunction->SetDescription("Calculate arc-tangent of X");
			pFunction->BindInput(1, 'x', "X");
			pFunction->BindOutput(0, 'res', "Result");
			scope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::Random<float>, "a17b82b7-3b4a-440d-a4f1-082f3d9cf0f5"_drx_guid, "Random");
			pFunction->SetDescription("Generate random number within range");
			pFunction->BindInput(1, 'min', "Min", "Minimum value", 0.0f);
			pFunction->BindInput(2, 'max', "Max", "Minimum value", 100.0f);
			pFunction->BindOutput(0, 'res', "Output"); // #SchematycTODO : Rename 'Result'!
			scope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Abs, "08e66eec-9e40-4fe9-b4dd-0df8073c681d"_drx_guid, "Abs");
			pFunction->SetDescription("Calculate absolute (non-negative) value");
			pFunction->BindInput(1, 'x', "Input"); // #SchematycTODO : Rename 'X'!
			pFunction->BindOutput(0, 'res', "Output"); // #SchematycTODO : Rename 'Result'!
			scope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::Equal<float>, "{DBF69A60-658D-48A8-802E-2EB6220ED60B}"_drx_guid, "Equal");
			pFunction->SetDescription("Checks if A and B are equal");
			pFunction->BindInput(1, 'a', "A");
			pFunction->BindInput(2, 'b', "B");
			pFunction->BindOutput(0, 'res', "Result");
			scope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::NotEqual<float>, "{94BD55E0-0E9B-4740-9951-00D494B56A81}"_drx_guid, "NotEqual");
			pFunction->SetDescription("Checks if A and B are not equal");
			pFunction->BindInput(1, 'a', "A");
			pFunction->BindInput(2, 'b', "B");
			pFunction->BindOutput(0, 'res', "Result");
			scope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::LessThan<float>, "{C75AEA3B-E845-40A2-887C-861B2704DDFD}"_drx_guid, "LessThan");
			pFunction->SetDescription("Checks if A is less than B");
			pFunction->BindInput(1, 'a', "A");
			pFunction->BindInput(2, 'b', "B");
			pFunction->BindOutput(0, 'res', "Result");
			scope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::LessThanOrEqual<float>, "{5EF1C108-DE37-4076-BF40-E8652F9DF67D}"_drx_guid, "LessThanOrEqual");
			pFunction->SetDescription("Checks if A is less than or equals B");
			pFunction->BindInput(1, 'a', "A");
			pFunction->BindInput(2, 'b', "B");
			pFunction->BindOutput(0, 'res', "Result");
			scope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::GreaterThan<float>, "{E51E4BD3-EC49-48FB-9C00-D354D137857F}"_drx_guid, "GreaterThan");
			pFunction->SetDescription("Checks if A is greater than B");
			pFunction->BindInput(1, 'a', "A");
			pFunction->BindInput(2, 'b', "B");
			pFunction->BindOutput(0, 'res', "Result");
			scope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::GreaterThanOrEqual<float>, "{C6F34366-D6E8-460B-AA96-CEC175E0959D}"_drx_guid, "GreaterThanOrEqual");
			pFunction->SetDescription("Checks if A is greater than or equals B");
			pFunction->BindInput(1, 'a', "A");
			pFunction->BindInput(2, 'b', "B");
			pFunction->BindOutput(0, 'res', "Result");
			scope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::Negate<float>, "{7FA0213D-9354-4E2A-A47F-971323EAC9F6}"_drx_guid, "Negate");
			pFunction->SetDescription("Returns the negated value, for example -1 becomes 1, 1 becomes -1");
			pFunction->BindInput(1, 'x', "Value");
			pFunction->BindOutput(0, 'res', "Result");
			scope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Generic::Sign<float>, "{00670DC4-663A-424B-ADC3-052B5CF5CEF0}"_drx_guid, "Sign");
			pFunction->SetDescription("Gets the sign of the entity, for example 10 returns 1, -10 returns -1");
			pFunction->BindInput(1, 'x', "Value");
			pFunction->BindOutput(0, 'res', "Result");
			scope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&ToInt32, "6e6356f3-7cf6-4746-9ef7-e8031c2a4117"_drx_guid, "ToInt32");  //#SchematycTODO: parameter to define how to round
			pFunction->SetDescription("Convert to Int32");
			pFunction->BindInput(1, 'val', "Input"); // #SchematycTODO : Rename 'Value'!
			pFunction->BindOutput(0, 'res', "Output"); // #SchematycTODO : Rename 'Result'!
			scope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&ToUInt32, "FAA240D7-7187-4A9F-8345-3D77A095EDE2"_drx_guid, "ToUInt32");
			pFunction->SetDescription("Convert to UInt32");
			pFunction->BindInput(1, 'val', "Input"); // #SchematycTODO : Rename 'Value'!
			pFunction->BindOutput(0, 'res', "Output"); // #SchematycTODO : Rename 'Result'!
			scope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&ToString, "171188d6-b66a-4d4b-8508-b6c003c9da78"_drx_guid, "ToString");
			pFunction->SetDescription("Convert to String");
			pFunction->BindInput(1, 'val', "Input"); // #SchematycTODO : Rename 'Value'!
			pFunction->BindOutput(2, 'res', "Output"); // #SchematycTODO : Rename 'Result'!
			scope.Register(pFunction);
		}
	}

} // Float

namespace String
{
void Append(const CSharedString& a, const CSharedString& b, CSharedString& result)
{
	result = a;
	result.append(b.c_str());
}

bool Equal(const CSharedString& a, const CSharedString& b)
{
	return strcmp(a.c_str(), b.c_str()) == 0;
}

bool NotEqual(const CSharedString& a, const CSharedString& b)
{
	return strcmp(a.c_str(), b.c_str()) != 0;
}

static void RegisterFunctions(IEnvRegistrar& registrar)
{
	CEnvRegistrationScope scope = registrar.Scope(GetTypeDesc<CSharedString>().GetGUID());
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Append, "f7984665-576c-44cb-8bbb-7401365faa7a"_drx_guid, "Append");
		pFunction->SetDescription("Combine A and B");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(3, 'res', "String"); // #SchematycTODO : Rename 'Result'!
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&Equal, "{86F6ED6B-44E3-4534-B600-37BA398760D5}"_drx_guid, "Equal");
		pFunction->SetDescription("Checks whether A and B are equal");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&NotEqual, "{A891817C-422C-4121-9A47-5D81742A06AF}"_drx_guid, "NotEqual");
		pFunction->SetDescription("Checks whether A and B are not equal");
		pFunction->BindInput(1, 'a', "A");
		pFunction->BindInput(2, 'b', "B");
		pFunction->BindOutput(0, 'res', "Result");
		scope.Register(pFunction);
	}
}
} // String

static void RegisterBasicFunctions(IEnvRegistrar& registrar)
{
	Bool::RegisterFunctions(registrar);
	Int32::RegisterFunctions(registrar);
	UInt32::RegisterFunctions(registrar);
	Float::RegisterFunctions(registrar);
	String::RegisterFunctions(registrar);
}
} // sxema

DRX_STATIC_AUTO_REGISTER_FUNCTION(&sxema::RegisterBasicFunctions)
