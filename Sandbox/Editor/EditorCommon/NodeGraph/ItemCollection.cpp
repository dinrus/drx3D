// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "ItemCollection.h"

namespace DrxGraphEditor {

CItemCollection::CItemCollection()
	: m_pViewWidget(nullptr)
{
	contextContainer.set(this);
}

void CItemCollection::Serialize(Serialization::IArchive& archive)
{
	float top = m_itemsRect.top();
	archive(top, "itemRectTop");
	float left = m_itemsRect.left();
	archive(left, "itemRectLeft");
	float bottom = m_itemsRect.bottom();
	archive(bottom, "itemRectBottom");
	float right = m_itemsRect.right();
	archive(right, "itemRectRight");

	archive.setLastContext(&contextContainer);
}

void CItemCollection::AddNode(CAbstractNodeItem& node)
{
	m_nodes.push_back(&node);
}

void CItemCollection::AddConnection(CAbstractConnectionItem& connection)
{
	m_connections.push_back(&connection);
}

void CItemCollection::AddCustomItem(CAbstractNodeGraphViewModelItem& customItem)
{
	m_customItems.push_back(&customItem);
}

}

