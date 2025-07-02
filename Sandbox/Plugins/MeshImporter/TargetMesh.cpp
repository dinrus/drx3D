// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "TargetMesh.h"
#include "QtCommon.h"
#include <IEditor.h>
#include <Drx3DEngine/I3DEngine.h>
#include "Drx3DEngine/CGF/DrxHeaders.h"
#include <Drx3DEngine/CGF/CGFContent.h>

#include <drx3D/CoreX/Serialization/yasli/Archive.h>
#include <drx3D/CoreX/Serialization/yasli/STL.h>
#include <drx3D/CoreX/Serialization/yasli/Enum.h>
#include <drx3D/CoreX/Serialization/yasli/JSONIArchive.h>
#include <drx3D/CoreX/Serialization/yasli/JSONOArchive.h>

void CTargetMeshModel::SItemProperties::Serialize(Serialization::IArchive& ar)
{
	ar(pItem->m_name, "name", "!Name");
}

CTargetMeshModel::CTargetMeshModel(QObject* pParent)
	: QAbstractItemModel(pParent)
{}

void CTargetMeshModel::LoadCgf(tukk filename)
{
	beginResetModel();

	m_items.clear();
	m_rootItems.clear();

	if (filename)
	{
		CContentCGF* pCGF = GetIEditor()->Get3DEngine()->CreateChunkfileContent(filename);
		if (GetIEditor()->Get3DEngine()->LoadChunkFileContent(pCGF, filename))
		{
			RebuildInternal(*pCGF);
		}
	}

	endResetModel();
}

void CTargetMeshModel::Clear()
{
	LoadCgf(nullptr);
}

CTargetMeshModel::SItemProperties* CTargetMeshModel::GetProperties(const QModelIndex& index)
{
	SItem* const pItem = (SItem*)index.internalPointer();
	return &pItem->m_properties;
}

const CTargetMeshModel::SItemProperties* CTargetMeshModel::GetProperties(const QModelIndex& index) const
{
	SItem* const pItem = (SItem*)index.internalPointer();
	return &pItem->m_properties;
}

QModelIndex CTargetMeshModel::index(i32 row, i32 column, const QModelIndex& parent) const
{
	if (parent.isValid())
	{
		SItem* const pParentItem = (SItem*)parent.internalPointer();
		return createIndex(row, 0, pParentItem->m_children[row]);
	}
	else
	{
		return createIndex(row, 0, m_rootItems[row]);
	}
}

QModelIndex CTargetMeshModel::parent(const QModelIndex& index) const
{
	if (!index.isValid())
	{
		return QModelIndex();
	}

	SItem* const pItem = (SItem*)index.internalPointer();
	SItem* const pParentItem = pItem->m_pParent;
	if (pParentItem)
	{
		return createIndex(pItem->m_siblingIndex, 0, pParentItem);
	}
	else
	{
		return QModelIndex();
	}
}

i32 CTargetMeshModel::rowCount(const QModelIndex& index) const
{
	if (index.isValid())
	{
		const SItem* const pItem = (SItem*)index.internalPointer();
		return pItem->m_children.size();
	}
	else
	{
		return m_rootItems.size();
	}
}

i32 CTargetMeshModel::columnCount(const QModelIndex& index) const
{
	return eColumnType_COUNT;
}

QVariant CTargetMeshModel::data(const QModelIndex& index, i32 role) const
{
	if (!index.isValid())
	{
		return QVariant();
	}

	if (role == Qt::DisplayRole)
	{
		const SItem* const pItem = (SItem*)index.internalPointer();
		return QLatin1String(pItem->m_name);
	}

	if (role == eItemDataRole_YasliSStruct)
	{
		std::unique_ptr<Serialization::SStruct> sstruct(new Serialization::SStruct(*GetProperties(index)));
		return QVariant::fromValue((uk )sstruct.release());
	}

	return QVariant();
}

QVariant CTargetMeshModel::headerData(i32 column, Qt::Orientation orientation, i32 role) const
{
	if (column == eColumnType_Name && role == Qt::DisplayRole)
	{
		return tr("Name");
	}
	return QVariant();
}

void CTargetMeshModel::RebuildInternal(CContentCGF& cgf)
{
	// Must avoid re-allocation of vector. Otherwise pointers to items might become invalid.
	m_items.clear();
	m_items.resize(cgf.GetNodeCount());

	for (i32 i = 0; i < cgf.GetNodeCount(); ++i)
	{
		CNodeCGF* pNode = cgf.GetNode(i);
		m_items[i].m_name = pNode->name;

		if (!pNode->pParent)
			continue;

		for (i32 j = 0; j < cgf.GetNodeCount(); ++j)
		{
			if (cgf.GetNode(j) == pNode->pParent)
			{
				m_items[j].AddChild(&m_items[i]);
				break;
			}
		}
	}

	m_rootItems.clear();
	for (auto& item : m_items)
	{
		if (!item.m_pParent)
		{
			m_rootItems.push_back(&item);
		}
	}
}

CTargetMeshModel* CTargetMeshView::model()
{
	return m_pModel.get();
}

void CTargetMeshView::reset()
{
	QAdvancedTreeView::reset();
	expandAll();
}

CTargetMeshView::CTargetMeshView(QWidget* pParent)
	: QAdvancedTreeView(QAdvancedTreeView::Behavior(QAdvancedTreeView::PreserveExpandedAfterReset | QAdvancedTreeView::PreserveSelectionAfterReset), pParent)
	, m_pModel(new CTargetMeshModel())
{
	setModel(m_pModel.get());
}

