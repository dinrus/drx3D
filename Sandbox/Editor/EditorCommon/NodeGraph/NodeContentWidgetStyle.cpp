// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "NodeContentWidgetStyle.h"

#include "NodeWidgetStyle.h"
#include "NodePinWidgetStyle.h"

namespace DrxGraphEditor {

CNodeContentWidgetStyle::CNodeContentWidgetStyle(CNodeWidgetStyle& nodeWidgetStyle)
	: QWidget(&nodeWidgetStyle)
	, m_nodeWidgetStyle(nodeWidgetStyle)
{
	m_backgroundColor = QColor(32, 32, 32);
	m_margins = QMargins();
}

void CNodeContentWidgetStyle::RegisterPinWidgetStyle(CNodePinWidgetStyle* pStyle)
{
	DRX_ASSERT_MESSAGE(pStyle->GetId(), "StyleId must be non-zero!");
	if (pStyle->GetId())
	{
		const StyleIdHash styleIdHash = pStyle->GetIdHash();
		const bool iconExists = (m_pinWidgetStylesById.find(styleIdHash) != m_pinWidgetStylesById.end());

		if (iconExists == false)
		{
			pStyle->SetParent(this);
			m_pinWidgetStylesById[styleIdHash] = pStyle;
		}
		else
		{
			auto result = m_pinWidgetStylesById.find(styleIdHash);
			const stack_string resultStyleId = result->second->GetId();
			if (pStyle->GetId() == resultStyleId)
			{
				DRX_ASSERT_MESSAGE(false, "Style id already exists.");
			}
			else
			{
				DRX_ASSERT_MESSAGE(false, "Hash collison of style id '%s' and '%s'", pStyle->GetId(), resultStyleId.c_str());
			}
		}
	}
}

const CNodePinWidgetStyle* CNodeContentWidgetStyle::GetPinWidgetStyle(tukk styleId) const
{
	const StyleIdHash styleIdHash = CCrc32::Compute(styleId);
	auto result = m_pinWidgetStylesById.find(styleIdHash);
	if (result != m_pinWidgetStylesById.end())
	{
		return result->second;
	}
	return m_nodeWidgetStyle.GetViewStyle()->GetPinWidgetStyle(styleId);
}

}

