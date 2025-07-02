// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <vector>
#include <drx3D/CoreX/Serialization/yasli/Config.h>
#include <drx3D/CoreX/Serialization/Forward.h>

struct ITagSource
{
	virtual void         AddRef() = 0;
	virtual void         Release() = 0;
	virtual u32 TagCount(u32 group) const = 0;
	virtual tukk  TagValue(u32 group, u32 index) const = 0;
	virtual tukk  TagDescription(u32 group, u32 index) const = 0;
	virtual tukk  GroupName(u32 group) const = 0;
	virtual u32 GroupCount() const = 0;
};

struct TagList
{
	std::vector<Serialization::string>* tags;

	TagList(std::vector<Serialization::string>& tags)
		: tags(&tags)
	{
	}
};

bool Serialize(Serialization::IArchive& ar, TagList& tagList, tukk name, tukk label);

#include "TagListImpl.h"
