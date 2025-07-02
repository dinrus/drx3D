// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "IDrxGraphEditor.h"

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

#include "QScrollableBox.h"

#include <drx3D/CoreX/Serialization/IArchive.h>

class QPropertyTree;

namespace DrxGraphEditor {

class CAbstractNodeGraphViewModelItem;

class EDITOR_COMMON_API CNodeGraphItemPropertiesWidget : public QScrollableBox
{
	Q_OBJECT

public:
	CNodeGraphItemPropertiesWidget(GraphItemSet& items);
	CNodeGraphItemPropertiesWidget(CAbstractNodeGraphViewModelItem& item);
	~CNodeGraphItemPropertiesWidget();

	virtual void showEvent(QShowEvent* pEvent) override;

protected:
	void SetupPropertyTree();

protected:
	QPropertyTree*          m_pPropertyTree;
	Serialization::SStructs m_structs;
};

}

