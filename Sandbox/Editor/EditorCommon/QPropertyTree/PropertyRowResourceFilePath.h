// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <Serialization/PropertyTree/IDrawContext.h>
#include <Serialization/PropertyTree/PropertyRowField.h>
#include <Serialization/PropertyTree/PropertyRowString.h>
#include <Serialization/PropertyTree/PropertyTree.h>
#include <Serialization/PropertyTree/PropertyTreeModel.h>
#include "Serialization.h"
#include <drx3D/CoreX/Serialization/Decorators/ResourceFilePath.h>
#include <drx3D/CoreX/Serialization/Decorators/ResourceFilePathImpl.h>
#include <Serialization/Decorators/IconXPM.h>

using Serialization::ResourceFilePath;

class PropertyRowResourceFilePath : public PropertyRowString
{
public:
	void                  clear();

	bool                  isLeaf() const override   { return true; }
	bool                  isStatic() const override { return false; }

	void                  setValueAndContext(const Serialization::SStruct& ser, Serialization::IArchive& ar) override;
	bool                  assignTo(const Serialization::SStruct& ser) const override;
	bool                  onActivateButton(i32 buttonIndex, const PropertyActivationEvent& e) override;

	i32                   buttonCount() const override     { return 1; }
	property_tree::Icon   buttonIcon(const PropertyTree* tree, i32 index) const override;
	virtual bool          usePathEllipsis() const override { return true; }
	void                  serializeValue(Serialization::IArchive& ar);
	ukk           searchHandle() const override { return handle_; }
	Serialization::TypeID typeId() const override       { return Serialization::TypeID::get<string>(); }

	bool                  onContextMenu(property_tree::IMenu& menu, PropertyTree* tree) override;
private:
	string      filter_;
	string      startFolder_;
	i32         flags_;
	ukk handle_;
};

struct ResourceFilePathMenuHandler : PropertyRowMenuHandler
{
	PropertyTree*                tree;
	PropertyRowResourceFilePath* self;

	ResourceFilePathMenuHandler(PropertyTree* tree, PropertyRowResourceFilePath* container);

	void onMenuClear();
};

