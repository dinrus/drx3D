// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "NodeGraphClipboard.h"

#include "GraphViewModel.h"
#include "GraphNodeItem.h"

#include <DrxSchematyc/SerializationUtils/SerializationUtils.h>

namespace DrxSchematycEditor {

CNodeGraphClipboard::CNodeGraphClipboard(DrxGraphEditor::CNodeGraphViewModel& model)
	: CClipboardItemCollection(model)
{}

void CNodeGraphClipboard::SaveNodeDataToXml(DrxGraphEditor::CAbstractNodeItem& node, Serialization::IArchive& archive)
{
	CNodeItem& nodeItem = static_cast<CNodeItem&>(node);
	const Schematyc::IScriptGraphNode& scriptGraphNode = nodeItem.GetScriptElement();

	Schematyc::CStackString typeGuidString;
	Schematyc::GUID::ToString(typeGuidString, scriptGraphNode.GetTypeGUID());
	archive(typeGuidString, "typeGUID");
	archive(CopySerialize(scriptGraphNode), "dataBlob");
}

DrxGraphEditor::CAbstractNodeItem* CNodeGraphClipboard::RestoreNodeFromXml(Serialization::IArchive& archive)
{
	CNodeGraphViewModel* pModel = static_cast<CNodeGraphViewModel*>(GetModel());
	if (pModel)
	{
		Schematyc::CStackString typeGuidString;
		archive(typeGuidString, "typeGUID");

		const DrxGUID typeGuid = DrxGUID::FromString(typeGuidString.c_str());
		CNodeItem* pNodeItem = pModel->CreateNode(typeGuid);
		if (pNodeItem)
		{
			Schematyc::IScriptGraphNode& scriptGraphNode = pNodeItem->GetScriptElement();
			archive(PasteSerialize(scriptGraphNode), "dataBlob");
			pNodeItem->Refresh(true);
			return pNodeItem;
		}
	}
	return nullptr;
}

}

