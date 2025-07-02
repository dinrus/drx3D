// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <Serialization/PropertyTree/IDrawContext.h>
#include "Serialization/QPropertyTree/QPropertyTree.h"
#include <Serialization/PropertyTree/PropertyTreeModel.h>
#include <Serialization/PropertyTree/PropertyRowField.h>
#include <Serialization.h>
#include <drx3D/CoreX/Serialization/Decorators/Resources.h>
#include "IResourceSelectorHost.h"

using Serialization::IResourceSelector;
namespace Serialization {
struct INavigationProvider;
}

class PropertyRowResourceSelector : public PropertyRowField
{
public:
	PropertyRowResourceSelector();
	void                          clear();

	bool                          isLeaf() const override   { return true; }
	bool                          isStatic() const override { return false; }

	void                          jumpTo(PropertyTree* tree);
	bool                          createFile(PropertyTree* tree);
	void                          setValueAndContext(const Serialization::SStruct& ser, Serialization::IArchive& ar) override;
	bool                          assignTo(const Serialization::SStruct& ser) const override;
	bool                          onShowToolTip() override;
	void                          onHideToolTip() override;
	bool                          onActivate(const PropertyActivationEvent& ev) override;
	bool                          onActivateButton(i32 button, const PropertyActivationEvent& e) override;
	bool                          getHoverInfo(PropertyHoverInfo* hover, const Point& cursorPos, const PropertyTree* tree) const override;
	ukk                   searchHandle() const override { return searchHandle_; }
	Serialization::TypeID         typeId() const override       { return wrappedType_; }
	property_tree::InplaceWidget* createWidget(PropertyTree* tree) override;
	void                          setVal(PropertyTree* tree, tukk str, ukk handle, const yasli::TypeID& type);

	i32                           buttonCount() const override;
	virtual property_tree::Icon   buttonIcon(const PropertyTree* tree, i32 index) const override;
	virtual bool                  usePathEllipsis() const override { return true; }
	string                        valueAsString() const;
	void                          serializeValue(Serialization::IArchive& ar);
	void                          redraw(property_tree::IDrawContext& context);
	bool                          onMouseDown(PropertyTree* tree, Point point, bool& changed) override;

	virtual bool                  onDataDragMove(PropertyTree* tree) override;
	virtual bool                  onDataDrop(PropertyTree* tree) override;

	bool                          onContextMenu(IMenu& menu, PropertyTree* tree);
	bool                          pickResource(PropertyTree* tree);
	bool                          editResource(PropertyTree* tree);
	tukk                   typeNameForFilter(PropertyTree* tree) const override { return !type_.empty() ? type_.c_str() : "ResourceSelector"; }
	const yasli::string&          value() const                                        { return value_; }

private:
	mutable SResourceSelectorContext    context_;
	Serialization::INavigationProvider* provider_;
	ukk                         searchHandle_;
	Serialization::TypeID               wrappedType_;
	property_tree::Icon                 icon_;

	string                              value_;
	string                              type_;
	const SStaticResourceSelectorEntry*	selector_;
	string                              defaultPath_;
	bool                                bActive_; // A state to prevent re-entrancy if the selector is already active.
	i32 id_;
};

struct ResourceSelectorMenuHandler : PropertyRowMenuHandler
{
	PropertyTree*                tree;
	PropertyRowResourceSelector* self;

	ResourceSelectorMenuHandler(PropertyTree* tree, PropertyRowResourceSelector* container);

	void onMenuCreateFile();
	void onMenuJumpTo();
	void onMenuClear();
	void onMenuPickResource();
};

