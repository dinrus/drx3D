// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <Controls/DictionaryWidget.h>
#include "NodeGraph/IDrxGraphEditor.h"

namespace EditorSubstance
{
	namespace OutputEditor
	{


		class CNodesDictionaryNode : public CAbstractDictionaryEntry
		{
		public:
			CNodesDictionaryNode(QString name, QString identifier)
				: CAbstractDictionaryEntry()
				, m_name(name)
				, m_identifier(identifier)
			{}
			virtual ~CNodesDictionaryNode() {}

			// CAbstractDictionaryEntry
			virtual u32                          GetType() const override { return Type_Entry; }
			virtual QVariant                        GetColumnValue(i32 columnIndex) const override;
			virtual QVariant                        GetIdentifier() const override { return QVariant::fromValue(m_identifier); }
			// ~CAbstractDictionaryEntry

			const QString& GetName() const { return m_name; }

		private:
			QString                   m_name;
			QString                   m_identifier;
		};


		class CVirtualOutputsNodesDictionary : public CAbstractDictionary
		{
		public:
			enum EColumn : i32
			{
				eColumn_Name,
				eColumn_Identifier,

				eColumn_COUNT
			};

		public:
			CVirtualOutputsNodesDictionary();
			virtual ~CVirtualOutputsNodesDictionary();

			// DrxGraphEditor::CAbstractDictionary
			virtual i32                           GetNumEntries() const override { return m_nodes.size(); }
			virtual const CAbstractDictionaryEntry* GetEntry(i32 index) const override;

			virtual i32                           GetNumColumns() const { return 1; }
			// ~DrxGraphEditor::CAbstractDictionary

		private:
			std::vector<CNodesDictionaryNode*>     m_nodes;
		};


		class CSubstanceOutputsGraphRuntimeContext : public DrxGraphEditor::INodeGraphRuntimeContext
		{
		public:
			CSubstanceOutputsGraphRuntimeContext();
			~CSubstanceOutputsGraphRuntimeContext();

			// DrxGraphEditor::INodeGraphRuntimeContext
			virtual tukk                                GetTypeName() const override { return "SubstanceOutputs"; }
			virtual CAbstractDictionary*                       GetAvailableNodesDictionary() override { return &m_nodeTreeModel; }
			virtual const DrxGraphEditor::CNodeGraphViewStyle* GetStyle() const { return m_pStyle; }
			// ~DrxGraphEditor::INodeGraphRuntimeContext

		private:
			CVirtualOutputsNodesDictionary                   m_nodeTreeModel;
			DrxGraphEditor::CNodeGraphViewStyle* m_pStyle;
		};


	}
}
