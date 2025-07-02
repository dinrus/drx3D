// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <Serialization/PropertyTree/IDrawContext.h>
#include <Serialization/PropertyTree/PropertyRowField.h>
#include "Serialization/QPropertyTree/QPropertyTree.h"
#include "Serialization/PropertyTree/PropertyTreeModel.h"
#include "Serialization.h"
#include <drx3D/CoreX/Serialization/Decorators/ResourceFolderPath.h>
#include <drx3D/CoreX/Serialization/Decorators/ResourceFolderPathImpl.h>

using Serialization::ResourceFolderPath;

class PropertyRowResourceFolderPath : public PropertyRowField
{
public:
	PropertyRowResourceFolderPath() : handle_() {}
	bool                  isLeaf() const override   { return true; }
	bool                  isStatic() const override { return false; }
	bool                  onActivate(const PropertyActivationEvent& e) override;
	void                  setValueAndContext(const Serialization::SStruct& ser, Serialization::IArchive& ar) override;
	bool                  assignTo(const Serialization::SStruct& ser) const override;
	string                valueAsString() const;
	bool                  onContextMenu(IMenu& menu, PropertyTree* tree);

	i32                   buttonCount() const override     { return 1; }
	property_tree::Icon   buttonIcon(const PropertyTree* tree, i32 index) const override;
	bool                  usePathEllipsis() const override { return true; }
	void                  serializeValue(Serialization::IArchive& ar) override;
	ukk           searchHandle() const override    { return handle_; }
	Serialization::TypeID typeId() const override          { return Serialization::TypeID::get<string>(); }
	void                  clear();
private:
	string      path_;
	string      startFolder_;
	ukk handle_;
};

struct ResourceFolderPathMenuHandler : PropertyRowMenuHandler
{
	PropertyTree*                  tree;
	PropertyRowResourceFolderPath* self;

	ResourceFolderPathMenuHandler(PropertyTree* tree, PropertyRowResourceFolderPath* container);

	void onMenuClear();
};

