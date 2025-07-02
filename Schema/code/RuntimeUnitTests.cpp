// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>

#include <drx3D/Schema/RuntimeParams.h>
#include <drx3D/Schema/Scratchpad.h>
#include <drx3D/Schema/SharedString.h>

#include <drx3D/Schema/UnitTestRegistrar.h>

namespace sxema
{
namespace RuntimeUnitTests
{

UnitTestResultFlags TestScratchpad()
{
	i32k valueA = 101;
	const float valueB = 202.0f;
	const CSharedString valueC = "303";

	StackScratchpad stackScratchpad;

	u32k posA = stackScratchpad.Add(CAnyConstRef(valueA));
	u32k posB = stackScratchpad.Add(CAnyConstRef(valueB));
	u32k posC = stackScratchpad.Add(CAnyConstRef(valueC));

	HeapScratchpad heapScratchpad(stackScratchpad);

	i32k* pA = DynamicCast<i32>(heapScratchpad.Get(posA));
	if (!pA || (*pA != valueA))
	{
		return EUnitTestResultFlags::FatalError;
	}

	const float* pB = DynamicCast<float>(heapScratchpad.Get(posB));
	if (!pB || (*pB != valueB))
	{
		return EUnitTestResultFlags::FatalError;
	}

	const CSharedString* pC = DynamicCast<CSharedString>(heapScratchpad.Get(posC));
	if (!pC || (*pC != valueC))
	{
		return EUnitTestResultFlags::FatalError;
	}

	return EUnitTestResultFlags::Success;
}

UnitTestResultFlags TestRuntimeParams()
{
	i32k valueA = 101;
	const float valueB = 202.0f;
	const CSharedString valueC = "303";

	StackRuntimeParams stackRuntimeParams;

	stackRuntimeParams.AddInput(CUniqueId::FromUInt32('a'), valueA);
	stackRuntimeParams.AddInput(CUniqueId::FromUInt32('b'), valueB);
	stackRuntimeParams.AddInput(CUniqueId::FromUInt32('c'), valueC);

	HeapRuntimeParams heapRuntimeParams(stackRuntimeParams);

	i32k* pA = DynamicCast<i32>(heapRuntimeParams.GetInput(CUniqueId::FromUInt32('a')));
	if (!pA || (*pA != valueA))
	{
		return EUnitTestResultFlags::FatalError;
	}

	const float* pB = DynamicCast<float>(heapRuntimeParams.GetInput(CUniqueId::FromUInt32('b')));
	if (!pB || (*pB != valueB))
	{
		return EUnitTestResultFlags::FatalError;
	}

	const CSharedString* pC = DynamicCast<CSharedString>(heapRuntimeParams.GetInput(CUniqueId::FromUInt32('c')));
	if (!pC || (*pC != valueC))
	{
		return EUnitTestResultFlags::FatalError;
	}

	return EUnitTestResultFlags::Success;
}

UnitTestResultFlags Run()
{
	UnitTestResultFlags resultFlags = EUnitTestResultFlags::Success;
	resultFlags.Add(TestScratchpad());
	resultFlags.Add(TestRuntimeParams());
	return resultFlags;
}

} // RuntimeUnitTests

SXEMA_REGISTER_UNIT_TEST(&RuntimeUnitTests::Run, "Runtime")

} // sxema
