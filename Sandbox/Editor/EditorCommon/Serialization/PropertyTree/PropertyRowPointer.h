/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <drx3D/CoreX/Serialization/yasli/StringList.h>
using yasli::StringList;

#include "PropertyRow.h"

class PropertyTree;
class PropertyRowPointer;
struct CreatePointerMenuHandler : PropertyRowMenuHandler
{
public:
	PropertyTree* tree;
	PropertyRowPointer* row;
	i32 index;
	bool useDefaultValue;

	void onMenuCreateByIndex();
};

namespace property_tree { class IMenu; }
struct ClassMenuItemAdder
{
	virtual void addAction(property_tree::IMenu& menu, tukk text, i32 index);
	virtual property_tree::IMenu* addMenu(property_tree::IMenu& menu, tukk text);
	void generateMenu(property_tree::IMenu& createItem, const StringList& comboStrings);
};

class PropertyRowPointer : public PropertyRow
{
public:
	PropertyRowPointer();

	bool assignTo(yasli::PointerInterface &ptr);
	void setValueAndContext(const yasli::PointerInterface& ptr, yasli::Archive& ar);
	using PropertyRow::assignTo;

	yasli::TypeID baseType() const{ return baseType_; }
	void setBaseType(const yasli::TypeID& baseType) { baseType_ = baseType; }
	tukk derivedTypeName() const{ return derivedTypeName_.c_str(); }
	void setDerivedType(tukk typeName, yasli::ClassFactoryBase* factory);
	void setFactory(yasli::ClassFactoryBase* factory) { factory_ = factory; }
	yasli::ClassFactoryBase* factory() const{ return factory_; }
	bool onActivate(const PropertyActivationEvent& ev) override;
	bool onMouseDown(PropertyTree* tree, Point point, bool& changed) override;
	bool onContextMenu(IMenu &root, PropertyTree* tree) override;
	bool isStatic() const override{ return false; }
	bool isPointer() const override{ return true; }
	i32 widgetSizeMin(const PropertyTree*) const override;
	yasli::string generateLabel() const;
	yasli::string valueAsString() const override;
	tukk typeNameForFilter(PropertyTree* tree) const override { return baseType_.name(); }
	void redraw(IDrawContext& context) override;
	WidgetPlacement widgetPlacement() const override{ return WIDGET_VALUE; }
	void serializeValue(yasli::Archive& ar) override;
	ukk searchHandle() const override { return searchHandle_; }
	yasli::TypeID typeId() const override{ return pointerType_; }
protected:

	yasli::TypeID baseType_;
	yasli::string derivedTypeName_;
	yasli::string derivedLabel_;

	// this member is available for instances deserialized from clipboard:
	yasli::ClassFactoryBase* factory_;
	ukk searchHandle_;
	yasli::TypeID  pointerType_;
	Color colorOverride_;
};


