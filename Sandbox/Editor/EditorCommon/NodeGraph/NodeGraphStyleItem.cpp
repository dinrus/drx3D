// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "NodeGraphStyleItem.h"

#include "NodeGraphViewStyle.h"

namespace DrxGraphEditor {

CNodeGraphViewStyleItem::CNodeGraphViewStyleItem(tukk szStyleId)
	: m_styleId(szStyleId)
	, m_styleIdHash(CCrc32::Compute(szStyleId))
	, m_pViewStyle(nullptr)
{
	DRX_ASSERT(szStyleId);
	setObjectName(szStyleId);
}

void CNodeGraphViewStyleItem::SetParent(CNodeGraphViewStyle* pViewStyle)
{
	m_pViewStyle = pViewStyle;
	setParent(pViewStyle);
}

}

