// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
// FBX SDK declarations
#pragma once
#if !defined(BUILDING_FBX_TOOL)
	#error This header must not be included directly, #include FbxTool.h instead
#endif

#pragma warning(push)
#pragma warning(disable:4266) // no override available for virtual member function from base; function is hidden
#include <fbxsdk.h>
#pragma warning(pop)

// Use the nice DrxAssert dialog for now
// TODO: replace this with #include <assert.h> later
#include <intrin.h>
#include <drx3D/CoreX/Assert/DrxAssert.h>

