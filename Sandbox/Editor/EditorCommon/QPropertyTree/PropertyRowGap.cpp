// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

//#include "stdafx.h"
#include <Serialization/PropertyTree/IDrawContext.h>
#include <Serialization/PropertyTree/PropertyRow.h>
#include "Serialization/QPropertyTree/QPropertyTree.h"
#include "Serialization.h"
#include "DrxSerialization/Math.h"

class PropertyRowGap : public PropertyRow
{
public:
	bool            isLeaf() const override                                { return true; }
	bool            isStatic() const override                              { return false; }

	WidgetPlacement widgetPlacement() const override                       { return WIDGET_AFTER_PULLED; }
	i32             widgetSizeMax(const PropertyTree* tree) const override { return 10; }
};

namespace Serialization
{
REGISTER_PROPERTY_ROW(SGap, PropertyRowGap);
}

