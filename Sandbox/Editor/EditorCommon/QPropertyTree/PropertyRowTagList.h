// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>
#include "Serialization/PropertyTree/PropertyRowContainer.h"

struct ITagSource;

class PropertyRowTagList : public PropertyRowContainer
{
public:
	PropertyRowTagList();
	~PropertyRowTagList();
	void setValueAndContext(const yasli::ContainerInterface& value, Serialization::IArchive& ar) override;
	void generateMenu(property_tree::IMenu& item, PropertyTree* tree, bool addActions) override;
	void addTag(tukk tag, PropertyTree* tree);

private:
	using PropertyRow::setValueAndContext;
	ITagSource* source_;
};

struct TagListMenuHandler : public PropertyRowMenuHandler
{
	string              tag;
	PropertyRowTagList* row;
	PropertyTree*       tree;

	void onMenuAddTag();
};

