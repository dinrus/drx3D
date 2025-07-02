// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "SceneModelCommon.h"
#include <vector>

namespace FbxTool { class CScene; }

class CSkeleton
{
public:
	typedef std::function<void()> Callback;

	CSkeleton()
		: m_pExportRoot(nullptr)
	{}

	void SetScene(const FbxTool::CScene* pScene);

	void SetNodeInclusion(const FbxTool::SNode* pNode, bool bIncluded);

	const FbxTool::SNode* GetExportRoot() const;
	const std::vector<bool>& GetNodesInclusion() const;

	void SetCallback(const Callback& callback);
private:
	std::vector<bool> m_nodesInclusion;
	const FbxTool::SNode* m_pExportRoot;
	Callback m_callback;
};

class CSceneModelSkeleton : public CSceneModelCommon
{
private:
	enum EColumnType
	{
		eColumnType_Name,
		eColumnType_SourceNodeAttribute,
		eColumnType_COUNT
	};

public:
	CSceneModelSkeleton(CSkeleton* pNodeSkeleton)
		: m_pNodeSkeleton(pNodeSkeleton)
	{}

	// QAbstractItemModel implementation.
	i32           columnCount(const QModelIndex& index) const override;
	QVariant      data(const QModelIndex& index, i32 role) const override;
	QVariant      headerData(i32 column, Qt::Orientation orientation, i32 role) const override;
	Qt::ItemFlags flags(const QModelIndex& modelIndex) const override;
	bool          setData(const QModelIndex& index, const QVariant& value, i32 role) override;

private:
	CItemModelAttribute* GetColumnAttribute(i32 col) const;

private:
	CSkeleton* m_pNodeSkeleton;
};
