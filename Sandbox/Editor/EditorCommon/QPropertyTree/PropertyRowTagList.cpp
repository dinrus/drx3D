// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "PropertyRowTagList.h"
#include "Serialization/PropertyTree/IMenu.h"
#include "Serialization/PropertyTree/PropertyRowString.h"
#include "Serialization/QPropertyTree/QPropertyTree.h"
#include <drx3D/CoreX/Serialization/Decorators/TagList.h>
#include <drx3D/CoreX/Serialization/ClassFactory.h>
#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Serialization/Decorators/TagListImpl.h>
#include <QMenu>

PropertyRowTagList::PropertyRowTagList()
	: source_(0)
{
}

PropertyRowTagList::~PropertyRowTagList()
{
	if (source_)
		source_->Release();
}

void PropertyRowTagList::generateMenu(property_tree::IMenu& item, PropertyTree* tree, bool addActions)
{
	if (userReadOnly() || isFixedSize())
		return;

	if (!source_)
		return;

	u32 numGroups = source_->GroupCount();
	for (u32 group = 0; group < numGroups; ++group)
	{
		u32 tagCount = source_->TagCount(group);
		if (tagCount == 0)
			continue;
		tukk groupName = source_->GroupName(group);
		string title = string("From ") + groupName;
		IMenu* menu = item.addMenu(title);
		for (u32 tagIndex = 0; tagIndex < tagCount; ++tagIndex)
		{
			string str;
			str = source_->TagValue(group, tagIndex);
			tukk desc = source_->TagDescription(group, tagIndex);
			if (desc && desc[0] != '\0')
			{
				str += "\t";
				str += desc;
			}
			TagListMenuHandler* handler = new TagListMenuHandler();
			handler->tree = tree;
			handler->row = this;
			handler->tag = source_->TagValue(group, tagIndex);
			tree->addMenuHandler(handler);
			menu->addAction(str.c_str(), "", 0, handler, &TagListMenuHandler::onMenuAddTag);
		}
	}

	TagListMenuHandler* handler = new TagListMenuHandler();
	handler->tree = tree;
	handler->row = this;
	handler->tag = string();
	tree->addMenuHandler(handler);
	item.addAction("Add", "", 0, handler, &TagListMenuHandler::onMenuAddTag);

	PropertyRowContainer::generateMenu(item, tree, false);
}

void PropertyRowTagList::addTag(tukk tag, PropertyTree* tree)
{
	yasli::SharedPtr<PropertyRowTagList> ref(this);

	PropertyRow* child = addElement(tree, false);
	if (child && strcmp(child->typeName(), "string") == 0)
	{
		PropertyRowString* stringRow = static_cast<PropertyRowString*>(child);
		tree->model()->rowAboutToBeChanged(stringRow);
		stringRow->setVal(tag, stringRow->searchHandle(), stringRow->typeId());
		tree->model()->rowChanged(stringRow);
	}
}

void TagListMenuHandler::onMenuAddTag()
{
	row->addTag(tag.c_str(), tree);
}

void PropertyRowTagList::setValueAndContext(const Serialization::IContainer& value, Serialization::IArchive& ar)
{
	if (source_)
		source_->Release();
	source_ = ar.context<ITagSource>();
	if (source_)
		source_->AddRef();

	PropertyRowContainer::setValueAndContext(value, ar);
}

REGISTER_PROPERTY_ROW(TagList, PropertyRowTagList)
DECLARE_SEGMENT(PropertyRowTagList)

