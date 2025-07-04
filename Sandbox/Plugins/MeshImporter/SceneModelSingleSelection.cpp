// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "SceneModelSingleSelection.h"
#include "Scene/SceneElementCommon.h"
#include "Scene/SceneElementSourceNode.h"
#include "Scene/SceneElementTypes.h"

i32 CSceneModelSingleSelection::columnCount(const QModelIndex& index) const
{
	return 1;
}

QVariant CSceneModelSingleSelection::data(const QModelIndex& index, i32 role) const
{
	DRX_ASSERT(index.isValid() && index.column() == 0);
	CSceneElementCommon* const pSceneElement = GetSceneElementFromModelIndex(index);
	if (role == Qt::DisplayRole)
	{
		return QtUtil::ToQString(pSceneElement->GetName());
	}
	else if (role == Qt::CheckStateRole && pSceneElement->GetType() == ESceneElementType::SourceNode)
	{
		CSceneElementSourceNode* const pSourceNodeElement = (CSceneElementSourceNode*)pSceneElement;
		return m_getNode && m_getNode() == pSourceNodeElement->GetNode();
	}
	return QVariant();
}

QVariant CSceneModelSingleSelection::headerData(i32 column, Qt::Orientation orientation, i32 role) const
{
	DRX_ASSERT(column == 0);
	if (role == Qt::DisplayRole)
	{
		return QString("Node name");
	}
	return QVariant();
}

Qt::ItemFlags CSceneModelSingleSelection::flags(const QModelIndex& modelIndex) const
{
	return Qt::ItemIsUserCheckable | CSceneModelCommon::flags(modelIndex);
}

bool CSceneModelSingleSelection::setData(const QModelIndex& modelIndex, const QVariant& value, i32 role)
{
	if (!m_setNode || !m_getNode)
	{
		return false;
	}

	if (role == Qt::CheckStateRole)
	{
		CSceneElementCommon* const pSceneElement = GetSceneElementFromModelIndex(modelIndex);

		if (pSceneElement->GetType() != ESceneElementType::SourceNode)
		{
			return false;
		}
		const auto checkState = (Qt::CheckState)value.toUInt();

		const FbxTool::SNode* const pNode = ((CSceneElementSourceNode*)pSceneElement)->GetNode();

		if (m_getNode() == pNode)
		{
			m_setNode(nullptr);
		}
		else
		{
			m_setNode(pNode);
		}

		Reset();
	}

	return false;
}

