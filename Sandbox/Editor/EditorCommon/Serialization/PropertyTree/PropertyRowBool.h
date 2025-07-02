/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <drx3D/CoreX/Serialization/yasli/Config.h>
#include "PropertyRow.h"
#include "Unicode.h"

class PropertyRowBool : public PropertyRow
{
public:
	PropertyRowBool();
	bool assignToPrimitive(uk val, size_t size) const override;
	bool assignToByPointer(uk instance, const yasli::TypeID& type) const override;
	void setVal(bool value, ukk handle, const yasli::TypeID& typeId) { value_ = value; serializer_.setPointer((uk )handle); serializer_.setType(yasli::TypeID::get<bool>()); }

	void redraw(IDrawContext& context) override;
	bool isLeaf() const override{ return true; }
	bool isStatic() const override{ return false; }

	bool onActivate(const PropertyActivationEvent& ev) override;
	bool onKeyDown(PropertyTree* tree, const property_tree::KeyEvent* ev) override;
	DragCheckBegin onMouseDragCheckBegin() override;
	bool onMouseDragCheck(PropertyTree* tree, bool value) override;
	yasli::wstring valueAsWString() const override{ return value_ ? L"true" : L"false"; }
	yasli::string valueAsString() const override{ return value_ ? "true" : "false"; }

	// changed placement from WIDGET_ICON to WIDGET_AFTER_PULLED to achieve an alignement
	// with other controls like comboboxes/textfields/numericfields etc.
	// this change breakes the packing of the checkboxes in two columns (Tomas Oresky)
	WidgetPlacement widgetPlacement() const override{ return WIDGET_AFTER_PULLED; }
	void serializeValue(yasli::Archive& ar) override;
	i32 widgetSizeMin(const PropertyTree* tree) const override;
protected:
	bool value_;
};


