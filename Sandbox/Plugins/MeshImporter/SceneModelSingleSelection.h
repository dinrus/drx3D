// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "SceneModelCommon.h"

namespace FbxTool {
struct SNode;
}

//! Scene model that allows selection of a single node.
class CSceneModelSingleSelection : public CSceneModelCommon
{
public:
	typedef std::function<void (const FbxTool::SNode*)> SetNode;
	typedef std::function<const FbxTool::SNode*(void)>  GetNode;

	void SetNodeAccessors(const SetNode& setNode, const GetNode& getNode)
	{
		m_setNode = setNode;
		m_getNode = getNode;
	}

	// QAbstractItemModel implementation.
	i32           columnCount(const QModelIndex& index) const override;
	QVariant      data(const QModelIndex& index, i32 role) const override;
	QVariant      headerData(i32 column, Qt::Orientation orientation, i32 role) const override;
	Qt::ItemFlags flags(const QModelIndex& modelIndex) const override;
	bool          setData(const QModelIndex& index, const QVariant& value, i32 role) override;
private:
	SetNode m_setNode;
	GetNode m_getNode;
};

