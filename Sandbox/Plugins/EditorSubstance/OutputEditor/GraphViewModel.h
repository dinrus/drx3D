// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <NodeGraph/AbstractNodeGraphViewModel.h>

#include <NodeGraph/AbstractPinItem.h>
#include "NodeGraph/AbstractNodeItem.h"
#include "NodeGraph/AbstractConnectionItem.h"

#include "Pins/SubstanceInPinItem.h"
#include "Pins/SubstanceOutPinItem.h"
#include "Nodes/SubstanceOutputNodeBase.h"

#include "SubstanceCommon.h"


namespace EditorSubstance
{
	namespace OutputEditor
	{

		class CSubstanceOutputNodeBase;
		class CVirtualOutputNode;
		class CSubstanceConnectionItem;

		class CGraphViewModel : public DrxGraphEditor::CNodeGraphViewModel
		{
			Q_OBJECT
				typedef std::unordered_map<u32 /* name crc */, CSubstanceOutputNodeBase*> NodeItemByNameCrc;
		public:
			CGraphViewModel(const std::vector<SSubstanceOutput>& originalOutputs, const std::vector<SSubstanceOutput>& customOutputs, bool showPreviews = true);

			virtual DrxGraphEditor::INodeGraphRuntimeContext& GetRuntimeContext() override;
			virtual QString                                   GetGraphName() override;
			virtual u32                                    GetNodeItemCount() const override;
			virtual DrxGraphEditor::CAbstractNodeItem*        GetNodeItemByIndex(u32 index) const override;
			virtual DrxGraphEditor::CAbstractNodeItem*        GetNodeItemById(QVariant id) const override;
			virtual DrxGraphEditor::CAbstractNodeItem*        CreateNode(QVariant identifier, const QPointF& position = QPointF()) override;
			virtual bool                                      RemoveNode(DrxGraphEditor::CAbstractNodeItem& node) override;

			virtual u32                                    GetConnectionItemCount() const override;
			virtual DrxGraphEditor::CAbstractConnectionItem*  GetConnectionItemByIndex(u32 index) const override;
			virtual DrxGraphEditor::CAbstractConnectionItem*  GetConnectionItemById(QVariant id) const override;
			virtual DrxGraphEditor::CAbstractConnectionItem*  CreateConnection(DrxGraphEditor::CAbstractPinItem& sourcePin, DrxGraphEditor::CAbstractPinItem& targetPin) override;
			virtual bool                                      RemoveConnection(DrxGraphEditor::CAbstractConnectionItem& connection) override;

			CSubstanceOutputNodeBase*                       CreateNode(ESubstanceGraphNodeType nodeType, const QPointF& position);
			CSubstanceConnectionItem*                 CreateConnection(CSubstanceOutPinItem& sourcePin, CSubstanceInPinItem& targetPin);
			CSubstanceOutputNodeBase*                       GetNodeItemById(string identifier) const;
			void                             UpdateNodeCrc(tukk szOldName, tukk szNewName, const ESubstanceGraphNodeType& nodeType);
			std::vector<SSubstanceOutput*> GetOutputs() const;
			bool GetShowPreviews() { return m_showPreviews; }
			bool IsNameUnique(const string& name, const ESubstanceGraphNodeType& nodeType);
		Q_SIGNALS:
			void SignalOutputsChanged();

		private:
			static u32k GetNodeCRC(const string& name, const ESubstanceGraphNodeType& nodeType);
			static u32k GetNodeCRC(const string& name);
			NodeItemByNameCrc             m_nodeItemByNameCrc;
			std::unique_ptr<DrxGraphEditor::INodeGraphRuntimeContext>		      m_pRuntimeContext;
			std::vector<CSubstanceOutputNodeBase*>       m_nodes;
			std::vector<DrxGraphEditor::CAbstractConnectionItem*> m_connections;
			bool m_showPreviews;
		};
	}
}
