// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "PropertyRowImpl.h"
#include "PropertyTree.h"
#include "IDrawContext.h"
#include <drx3D/CoreX/Serialization/yasli/decorators/HorizontalLine.h>

using yasli::HorizontalLine;

class PropertyRowHorizontalLine : public PropertyRowImpl<HorizontalLine>{
public:
	void redraw(IDrawContext& context) override;
	bool isSelectable() const override{ return false; }
};

void PropertyRowHorizontalLine::redraw(IDrawContext& context)
{
	Rect rc = context.lineRect;
	rc.h = context.tree->_defaultRowHeight();
	rc.y -= rc.h;
	context.drawHorizontalLine(rc);
}

DECLARE_SEGMENT(PropertyRowHorizontalLine)
REGISTER_PROPERTY_ROW(HorizontalLine, PropertyRowHorizontalLine);


