// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Math/Drx_Math.h>
#include <drx3D/CoreX/Math/Drx_Color.h>

#include <drx3D/CoreX/Serialization/Forward.h>

#include <drx3D/CoreX/Serialization/STL.h>
#include <drx3D/CoreX/Serialization/Math.h>
#include <drx3D/CoreX/Serialization/Color.h>
#include <drx3D/CoreX/Serialization/Decorators/BitFlags.h>
#include <drx3D/CoreX/Serialization/Decorators/Range.h>
#include <Serialization/Decorators/ToggleButton.h>
#include <Serialization/Qt.h>
#include <drx3D/CoreX/Serialization/ClassFactory.h>
#include <drx3D/CoreX/Serialization/Enum.h>
#include <drx3D/CoreX/Serialization/SmartPtr.h>
#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Serialization/IArchiveHost.h>

using Serialization::IArchive;
using Serialization::BitFlags;

#if 0 // useful for debugging
	#include <drx3D/CoreX/Serialization/yasli/JSONArchive.h>
typedef yasli::JSONOArchive MemoryOArchive;
typedef yasli::JSONIArchive MemoryIArchive;
#else
	#include <drx3D/CoreX/Serialization/yasli/BinArchive.h>
typedef yasli::BinOArchive MemoryOArchive;
typedef yasli::BinIArchive MemoryIArchive;
#endif

namespace Serialization {

void EDITOR_COMMON_API SerializeToMemory(std::vector<char>* buffer, const SStruct& obj);
void EDITOR_COMMON_API SerializeToMemory(DynArray<char>* buffer, const SStruct& obj);
void EDITOR_COMMON_API SerializeFromMemory(const SStruct& outObj, const std::vector<char>& buffer);
void EDITOR_COMMON_API SerializeFromMemory(const SStruct& outObj, const DynArray<char>& buffer);

}

