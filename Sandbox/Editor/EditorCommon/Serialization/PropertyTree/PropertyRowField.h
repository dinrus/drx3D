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
#include "IDrawContext.h"

class PROPERTY_TREE_API PropertyRowField : public PropertyRow
{
public:
	WidgetPlacement widgetPlacement() const override{ return WIDGET_VALUE; }
	i32 widgetSizeMin(const PropertyTree* tree) const override;

	virtual i32 buttonCount() const{ return 0; }
	virtual property_tree::Icon buttonIcon(const PropertyTree* tree, i32 index) const;
	virtual bool usePathEllipsis() const { return false; }
	virtual bool onActivateButton(i32 buttonIndex, const PropertyActivationEvent& e) { return false; }
	i32 hitButton(const PropertyTree* tree, const Point& p) const;

	void redraw(IDrawContext& context) override;
	bool onActivate(const PropertyActivationEvent& e) override;
	bool onMouseDown(PropertyTree* tree, Point point, bool& changed) override;
	void onMouseUp(PropertyTree* tree, Point point) override;
protected:
	Rect fieldRect(const PropertyTree* tree) const;
	void drawButtons(i32* offset);

	mutable RowWidthCache widthCache_;
};


