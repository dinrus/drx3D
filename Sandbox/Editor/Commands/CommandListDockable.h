// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/QtViewPane.h>
#include "Commands/CommandModel.h"
#include <memory>

class QAdvancedTreeView;

class CCommandListModel : public CommandModel
{
	friend CommandModelFactory;

public:
	enum Roles
	{
		SortRole = static_cast<i32>(CommandModel::Roles::Max),
	};

	virtual ~CCommandListModel();

	virtual void          Initialize() override;

	virtual i32           columnCount(const QModelIndex& parent) const override { return s_ColumnCount; }
	virtual QVariant      data(const QModelIndex& index, i32 role) const override;
	virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
	virtual bool          hasChildren(const QModelIndex& parent) const override;
	virtual QVariant      headerData(i32 section, Qt::Orientation orientation, i32 role) const override;
	virtual QModelIndex   index(i32 row, i32 column, const QModelIndex& parent) const override;
	virtual QModelIndex   parent(const QModelIndex& index) const override;
	virtual i32           rowCount(const QModelIndex& parent) const override;

protected:
	CCommandListModel();

	virtual void Rebuild() override;

	static i32k                     s_ColumnCount = 2;
	static tukk                   s_ColumnNames[s_ColumnCount];

	std::vector<const CCommandArgument*> m_params;
};

class CCommandListDockable : public CDockableWidget
{
public:
	CCommandListDockable(QWidget* const pParent = nullptr);
	~CCommandListDockable();

	//////////////////////////////////////////////////////////
	// CDockableWidget implementation
	virtual IViewPaneClass::EDockingDirection GetDockingDirection() const override { return IViewPaneClass::DOCK_FLOAT; }
	virtual tukk                       GetPaneTitle() const override        { return "Console Commands"; };
	virtual QRect                             GetPaneRect() override               { return QRect(0, 0, 800, 500); }
	//////////////////////////////////////////////////////////

private:
	std::unique_ptr<CommandModel> m_pModel;
	QAdvancedTreeView*            m_pTreeView;
};

