// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "ConnectionWidgetStyle.h"

namespace DrxGraphEditor {

CConnectionWidgetStyle::CConnectionWidgetStyle(tukk szStyleId, CNodeGraphViewStyle& viewStyle)
	: CNodeGraphViewStyleItem(szStyleId)
{
	m_color = QColor(110, 110, 110);
	m_width = 3.0;
	m_bezier = 120.0;
	m_usePinColors = true;

	viewStyle.RegisterConnectionWidgetStyle(this);
}

}

