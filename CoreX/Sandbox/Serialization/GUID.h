// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Serialization/STL.h>

// FIXME: this should be replaced by DrxGUID instead of using Windows-specific type
// from guiddef.h
#ifndef GUID_DEFINED
	#define GUID_DEFINED
typedef struct _GUID
{
	u64  Data1;
	unsigned short Data2;
	unsigned short Data3;
	u8  Data4[8];
} GUID;
#endif
// ^^^^

#include <drx3D/CoreX/ToolsHelpers/GuidUtil.h>

inline bool Serialize(Serialization::IArchive& ar, GUID& guid, tukk name, tukk label)
{
	string str = GuidUtil::ToString(guid);
	if (!ar(str, name, label))
		return false;
	if (ar.isInput())
		guid = GuidUtil::FromString(str.c_str());
	return true;
}

