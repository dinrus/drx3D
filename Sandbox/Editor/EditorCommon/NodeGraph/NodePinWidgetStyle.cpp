// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "NodePinWidgetStyle.h"

namespace DrxGraphEditor {

CNodePinWidgetStyle::CNodePinWidgetStyle(tukk szStyleId, CNodeGraphViewStyle& viewStyle)
	: CNodeGraphViewStyleItem(szStyleId)
{
	m_textFont = QFont("Arial", 12, QFont::Bold);
	m_iconSize = QSize(9, 9);

	viewStyle.RegisterPinWidgetStyle(this);

	SetColor(QColor(94, 94, 94));
	SetIcon(QIcon("icons:Graph/Node_connection_circle.ico"));
}

void CNodePinWidgetStyle::SetColor(const QColor& color)
{
	m_color = color;
	m_highlightColor = StyleUtils::ColorMuliply(m_color, GetViewStyle()->GetHighlightColor());

	GeneratePixmaps();
}

void CNodePinWidgetStyle::SetIcon(const QIcon& icon)
{
	m_icon = icon;
	GeneratePixmaps();
}

void CNodePinWidgetStyle::SetIconSize(const QSize& size)
{
	m_iconSize = size;
	GeneratePixmaps();
}

void CNodePinWidgetStyle::GeneratePixmaps()
{
	if (GetViewStyle())
	{
		m_iconPixmap[StyleState_Default] = StyleUtils::ColorizePixmap(m_icon.pixmap(m_iconSize), m_color);
		m_iconPixmap[StyleState_Highlighted] = StyleUtils::ColorizePixmap(m_icon.pixmap(m_iconSize), GetHighlightColor());
		m_iconPixmap[StyleState_Selected] = StyleUtils::ColorizePixmap(m_icon.pixmap(m_iconSize), GetSelectionColor());
		m_iconPixmap[StyleState_Disabled] = StyleUtils::ColorizePixmap(m_icon.pixmap(m_iconSize), QColor(32, 32, 32) /*GetHighlightColor()*/);
	}
}

}

