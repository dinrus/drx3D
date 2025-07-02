// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "NodeGraphViewStyle.h"

#include "NodeWidgetStyle.h"
#include "ConnectionWidgetStyle.h"
#include "NodePinWidgetStyle.h"

#include "NodeGraphView.h"

namespace DrxGraphEditor {

CNodeGraphViewStyle::CNodeGraphViewStyle(tukk szStyleId)
	: QWidget()
{
	setObjectName(szStyleId);

	m_selectionColor = QColor(97, 172, 237);
	m_highlightColor = QColor(140, 140, 140);

	m_gridBackgroundColor = QColor(75, 75, 75);
	m_gridSegmentLineColor = QColor(70, 70, 70);
	m_gridSegmentLineWidth = 1.f;
	m_gridSegmentSize = 400.f;

	m_gridSubSegmentLineColor = QColor(70, 70, 70);
	m_gridSubSegmentCount = 5;
	m_gridSubSegmentLineWidth = 1.f;

	CNodeWidgetStyle* pNodeStyle = new CNodeWidgetStyle("Node", *this);
	CConnectionWidgetStyle* pConnectionStyle = new CConnectionWidgetStyle("Connection", *this);
	CNodePinWidgetStyle* pPinStyle = new CNodePinWidgetStyle("Pin", *this);
}

void CNodeGraphViewStyle::RegisterNodeWidgetStyle(CNodeWidgetStyle* pStyle)
{
	DRX_ASSERT_MESSAGE(pStyle->GetId(), "StyleId must be non-zero!");
	if (pStyle->GetId())
	{
		u32k styleIdHash = pStyle->GetIdHash();
		const bool iconExists = (m_nodeWidgetStylesById.find(styleIdHash) != m_nodeWidgetStylesById.end());

		if (iconExists == false)
		{
			pStyle->SetParent(this);
			m_nodeWidgetStylesById[styleIdHash] = pStyle;
		}
		else
		{
			auto result = m_nodeWidgetStylesById.find(styleIdHash);
			const stack_string resultStyleId = result->second->GetId();
			if (pStyle->GetId() == resultStyleId)
			{
				//DRX_ASSERT_MESSAGE(false, "Style id already exists.");
			}
			else
			{
				DRX_ASSERT_MESSAGE(false, "Hash collison of style id '%s' and '%s'", pStyle->GetId(), resultStyleId.c_str());
			}
		}
	}
}

const CNodeWidgetStyle* CNodeGraphViewStyle::GetNodeWidgetStyle(tukk styleId) const
{
	DRX_ASSERT(styleId);
	const StyleIdHash styleIdHash = CCrc32::Compute(styleId);
	auto result = m_nodeWidgetStylesById.find(styleIdHash);
	if (result != m_nodeWidgetStylesById.end())
	{
		return result->second;
	}
	return nullptr;
}

void CNodeGraphViewStyle::RegisterConnectionWidgetStyle(CConnectionWidgetStyle* pStyle)
{
	DRX_ASSERT_MESSAGE(pStyle->GetId(), "StyleId must be non-zero!");
	if (pStyle->GetId())
	{
		const StyleIdHash styleIdHash = pStyle->GetIdHash();
		const bool iconExists = (m_connectionWidgetStylesById.find(styleIdHash) != m_connectionWidgetStylesById.end());

		if (iconExists == false)
		{
			pStyle->SetParent(this);
			m_connectionWidgetStylesById[styleIdHash] = pStyle;
		}
		else
		{
			auto result = m_connectionWidgetStylesById.find(styleIdHash);
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

const CConnectionWidgetStyle* CNodeGraphViewStyle::GetConnectionWidgetStyle(tukk styleId) const
{
	DRX_ASSERT(styleId);
	const StyleIdHash styleIdHash = CCrc32::Compute(styleId);
	auto result = m_connectionWidgetStylesById.find(styleIdHash);
	if (result != m_connectionWidgetStylesById.end())
	{
		return result->second;
	}
	return nullptr;
}

void CNodeGraphViewStyle::RegisterPinWidgetStyle(CNodePinWidgetStyle* pStyle)
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

const CNodePinWidgetStyle* CNodeGraphViewStyle::GetPinWidgetStyle(tukk styleId) const
{
	DRX_ASSERT(styleId);
	const StyleIdHash styleIdHash = CCrc32::Compute(styleId);
	auto result = m_pinWidgetStylesById.find(styleIdHash);
	if (result != m_pinWidgetStylesById.end())
	{
		return result->second;
	}
	return nullptr;
}

}

