// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <Serialization/PropertyTree/IDrawContext.h>
#include <Serialization/PropertyTree/PropertyRowField.h>
#include <Serialization/PropertyTree/PropertyTreeModel.h>
#include "Serialization/QPropertyTree/QPropertyTree.h"
#include "Serialization.h"
#include <drx3D/CoreX/Serialization/Decorators/OutputFilePath.h>
#include <drx3D/CoreX/Serialization/Decorators/OutputFilePathImpl.h>

using Serialization::OutputFilePath;

class PropertyRowOutputFilePath : public PropertyRowField
{
public:
	void                clear();

	bool                isLeaf() const override   { return true; }
	bool                isStatic() const override { return false; }

	bool                onActivate(const PropertyActivationEvent& e) override;
	void                setValueAndContext(const Serialization::SStruct& ser, Serialization::IArchive& ar) override;
	bool                assignTo(const Serialization::SStruct& ser) const override;
	string              valueAsString() const;
	void                serializeValue(Serialization::IArchive& ar);
	bool                onContextMenu(IMenu& menu, PropertyTree* tree);
	ukk         searchHandle() const             { return handle_; }

	i32                 buttonCount() const override     { return 1; }
	property_tree::Icon buttonIcon(const PropertyTree* tree, i32 index) const override;
	bool                usePathEllipsis() const override { return true; }

private:
	string      path_;
	string      filter_;
	string      startFolder_;
	ukk handle_;
};

struct OutputFilePathMenuHandler : PropertyRowMenuHandler
{
	PropertyTree*              tree;
	PropertyRowOutputFilePath* self;

	OutputFilePathMenuHandler(PropertyTree* tree, PropertyRowOutputFilePath* container);

	void onMenuClear();
};

