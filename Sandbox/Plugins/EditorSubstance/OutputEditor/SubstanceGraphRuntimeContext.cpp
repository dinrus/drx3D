// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "SubstanceGraphRuntimeContext.h"

#include "NodeGraph/NodeWidgetStyle.h"
#include "NodeGraph/NodeHeaderWidgetStyle.h"
#include "NodeGraph/NodeGraphViewStyle.h"
#include "NodeGraph/ConnectionWidgetStyle.h"
#include "NodeGraph/NodePinWidgetStyle.h"

namespace EditorSubstance
{
	namespace OutputEditor
	{

		CVirtualOutputsNodesDictionary::CVirtualOutputsNodesDictionary()
		{
			m_nodes.emplace_back(new CNodesDictionaryNode("Texture Output", "VirtualOutput"));
			m_nodes.emplace_back(new CNodesDictionaryNode("Substance Graph Output", "OriginalOutput"));
		}

		CVirtualOutputsNodesDictionary::~CVirtualOutputsNodesDictionary()
		{

		}

		const CAbstractDictionaryEntry* CVirtualOutputsNodesDictionary::GetEntry(i32 index) const
		{
			return m_nodes[index];
		}

		QVariant CNodesDictionaryNode::GetColumnValue(i32 columnIndex) const
		{
			switch (columnIndex)
			{
			case CVirtualOutputsNodesDictionary::eColumn_Name:
			{
				return QVariant::fromValue(m_name);
			}
			case CVirtualOutputsNodesDictionary::eColumn_Identifier:
			{
				return GetIdentifier();
			}
			default:
				break;
			}

			return QVariant();
		}


		void AddNodeStyle(DrxGraphEditor::CNodeGraphViewStyle& viewStyle, tukk szStyleId, tukk szIcon, QColor color)
		{
			DrxGraphEditor::CNodeWidgetStyle* pStyle = new DrxGraphEditor::CNodeWidgetStyle(szStyleId, viewStyle);

			DrxGraphEditor::CNodeHeaderWidgetStyle& headerStyle = pStyle->GetHeaderWidgetStyle();

			headerStyle.SetNodeIconMenuColor(color);


			headerStyle.SetNameColor(color);
			headerStyle.SetLeftColor(QColor(26, 26, 26));
			headerStyle.SetRightColor(QColor(26, 26, 26));
			headerStyle.SetNodeIconViewDefaultColor(color);

			DrxIcon icon(szIcon, {
				{ QIcon::Mode::Normal, QColor(255, 255, 255) }
			});
			headerStyle.SetNodeIcon(icon);
		}

		void AddConnectionStyle(DrxGraphEditor::CNodeGraphViewStyle& viewStyle, tukk szStyleId, float width)
		{
			DrxGraphEditor::CConnectionWidgetStyle* pStyle = new DrxGraphEditor::CConnectionWidgetStyle(szStyleId, viewStyle);
			pStyle->SetWidth(width);
		}

		void AddPinStyle(DrxGraphEditor::CNodeGraphViewStyle& viewStyle, tukk szStyleId, tukk szIcon, QColor color)
		{
			DrxIcon icon(szIcon, {
				{ QIcon::Mode::Normal, color }
			});

			DrxGraphEditor::CNodePinWidgetStyle* pStyle = new DrxGraphEditor::CNodePinWidgetStyle(szStyleId, viewStyle);
			pStyle->SetIcon(icon);
			pStyle->SetColor(color);
		}


		CSubstanceOutputsGraphRuntimeContext::CSubstanceOutputsGraphRuntimeContext()
		{
			DrxGraphEditor::CNodeGraphViewStyle* pViewStyle = new DrxGraphEditor::CNodeGraphViewStyle("Substance");

			AddNodeStyle(*pViewStyle, "Node::Output", "icons:MaterialEditor/Material_Editor.ico", QColor(255, 0, 0));
			AddNodeStyle(*pViewStyle, "Node::Input", "icons:ObjectTypes/node_track_expression.ico", QColor(0, 255, 0));

			AddConnectionStyle(*pViewStyle, "Connection::Substance", 2.0);

			AddPinStyle(*pViewStyle, "Pin::Substance", "icons:Graph/Node_connection_arrow_R.ico", QColor(200, 200, 200));
			m_pStyle = pViewStyle;
		}

		CSubstanceOutputsGraphRuntimeContext::~CSubstanceOutputsGraphRuntimeContext()
		{
			m_pStyle->deleteLater();
		}



	}
}
