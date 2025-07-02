// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <NodeGraph/ClipboardItemCollection.h>

namespace DrxSchematycEditor {

class CNodeGraphClipboard : public DrxGraphEditor::CClipboardItemCollection
{
public:
	CNodeGraphClipboard(DrxGraphEditor::CNodeGraphViewModel& model);

	virtual void                               SaveNodeDataToXml(DrxGraphEditor::CAbstractNodeItem& node, Serialization::IArchive& archive) override;
	virtual DrxGraphEditor::CAbstractNodeItem* RestoreNodeFromXml(Serialization::IArchive& archive) override;
};

}

