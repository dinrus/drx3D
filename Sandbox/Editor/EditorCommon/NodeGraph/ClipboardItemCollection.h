// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "AbstractNodeGraphViewModelItem.h"
#include "ItemCollection.h"

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

namespace DrxGraphEditor {

class CAbstractNodeItem;
class CNodeGraphViewModel;
class CAbstractNodeItem;

class EDITOR_COMMON_API CClipboardItemCollection : public DrxGraphEditor::CItemCollection
{
protected:
	static u32k InvalidIndex = 0xffffffff;

	struct SClipboardConnectionItem
	{
		u32 sourceNodeIndex;
		u32 sourcePinIndex;
		u32 targetNodeIndex;
		u32 targetPinIndex;

		SClipboardConnectionItem()
			: sourceNodeIndex(InvalidIndex)
			, sourcePinIndex(InvalidIndex)
			, targetNodeIndex(InvalidIndex)
			, targetPinIndex(InvalidIndex)
		{}

		void Serialize(Serialization::IArchive& archive);
	};

	struct SClipboardNodeItem
	{
		CAbstractNodeItem* pNodeItem;

		float              positionX;
		float              positionY;

		SClipboardNodeItem()
			: pNodeItem(nullptr)
			, positionX(.0f)
			, positionY(.0f)
		{}

		void Serialize(Serialization::IArchive& archive);
	};

	typedef std::map<CAbstractNodeItem*, u32 /* index */> NodeIndexByInstance;
	typedef std::vector<CAbstractNodeItem*>                  NodesByIndex;

public:
	CClipboardItemCollection(CNodeGraphViewModel& model);

	virtual void               Serialize(Serialization::IArchive& archive) override;

	virtual void               SaveNodeDataToXml(CAbstractNodeItem& node, Serialization::IArchive& archive) = 0;
	virtual CAbstractNodeItem* RestoreNodeFromXml(Serialization::IArchive& archive) = 0;
};

}

