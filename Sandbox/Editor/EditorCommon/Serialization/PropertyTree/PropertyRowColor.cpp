// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "PropertyRowColor.h"
#include "PropertyTree.h"
#include <drx3D/CoreX/Serialization/yasli/Archive.h>
#include <drx3D/CoreX/Serialization/yasli/ClassFactory.h>
#include "Rect.h"

using property_tree::Color;

PropertyRowColor::PropertyRowColor()
: value_(255, 255, 255, 255)
{
}

i32 PropertyRowColor::widgetSizeMin(const PropertyTree* tree) const
{
	return tree->_defaultRowHeight();
}

void PropertyRowColor::redraw(IDrawContext& context)
{
	context.drawColor(context.widgetRect.adjusted(1,1,-1,-3), value_);
}

void PropertyRowColor::closeNonLeaf(const yasli::Serializer& ser, yasli::Archive& ar)
{
	property_tree::Color* value = ser.cast<property_tree::Color>();
	if (!value)
		return;
	value_ = *value;
}


REGISTER_PROPERTY_ROW(Color, PropertyRowColor)
DECLARE_SEGMENT(PropertyRowColor)

