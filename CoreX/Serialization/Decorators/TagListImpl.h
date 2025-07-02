// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <vector>
#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Serialization/STL.h>

struct TagListContainer : Serialization::ContainerSTL<std::vector<string>, string>
{
	TagListContainer(TagList& tagList)
		: ContainerSTL(tagList.tags)
	{
	}

	Serialization::TypeID containerType() const override { return Serialization::TypeID::get<TagList>(); };
};

inline bool Serialize(Serialization::IArchive& ar, TagList& tagList, tukk name, tukk label)
{
	TagListContainer container(tagList);
	return ar(static_cast<Serialization::IContainer&>(container), name, label);
}
