// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "IDrxGraphEditor.h"
#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

#include "NodeGraphStyleItem.h"

#include <QWidget>
#include <QColor>
#include <QMargins>

#include <unordered_map>

namespace DrxGraphEditor {

class CNodePinWidgetStyle;
class CNodeWidgetStyle;

class EDITOR_COMMON_API CNodeContentWidgetStyle : protected QWidget
{
	Q_OBJECT;

public:
	Q_PROPERTY(QColor backgroundColor READ GetBackgroundColor WRITE SetBackgroundColor DESIGNABLE true);
	Q_PROPERTY(QMargins margins READ GetMargins WRITE SetMargins DESIGNABLE true);

public:
	CNodeContentWidgetStyle(CNodeWidgetStyle& nodeWidgetStyle);

	const CNodeWidgetStyle&    GetNodeWidgetStyle() const              { return m_nodeWidgetStyle; }

	const QColor&              GetBackgroundColor() const              { return m_backgroundColor; }
	void                       SetBackgroundColor(const QColor& color) { m_backgroundColor = color; }

	const QMargins&            GetMargins() const                      { return m_margins; }
	void                       SetMargins(const QMargins& margins)     { m_margins = margins; }

	void                       RegisterPinWidgetStyle(CNodePinWidgetStyle* pStyle);
	const CNodePinWidgetStyle* GetPinWidgetStyle(tukk styleId) const;

private:
	const CNodeWidgetStyle& m_nodeWidgetStyle;

	QColor                  m_backgroundColor;
	QMargins                m_margins;

	std::unordered_map<StyleIdHash, CNodePinWidgetStyle*> m_pinWidgetStylesById;
};

}

