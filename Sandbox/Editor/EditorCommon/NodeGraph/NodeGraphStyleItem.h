// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "IDrxGraphEditor.h"
#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

#include <QWidget>

namespace DrxGraphEditor {

class CNodeGraphViewStyle;

typedef u32 StyleIdHash;

class EDITOR_COMMON_API CNodeGraphViewStyleItem : public QWidget
{
	Q_OBJECT;

public:
	CNodeGraphViewStyleItem(tukk szStyleId);

	tukk          GetId() const               { return m_styleId.c_str(); }
	StyleIdHash          GetIdHash() const           { return m_styleIdHash; }

	CNodeGraphViewStyle* GetViewStyle() const        { return m_pViewStyle; }

	QWidget*             GetParent() const           { return parentWidget(); }
	void                 SetParent(QWidget* pParent) { setParent(pParent); }
	void                 SetParent(CNodeGraphViewStyle* pViewStyle);

private:
	const string         m_styleId;
	const StyleIdHash    m_styleIdHash;
	CNodeGraphViewStyle* m_pViewStyle;
};

}

