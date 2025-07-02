// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QWidget>
#include <SharedData.h>

class QFilteringPanel;

namespace ACE
{
namespace Impl
{
namespace Wwise
{
class CFilterProxyModel;
class CItemModel;
class CTreeView;
class CImpl;

class CDataPanel final : public QWidget
{
public:

	explicit CDataPanel(CImpl const& impl);

	CDataPanel() = delete;

	void Reset();
	void OnAboutToReload();
	void OnReloaded();
	void OnConnectionAdded() const;
	void OnConnectionRemoved() const;
	void OnSelectConnectedItem(ControlId const id);

private:

	void OnContextMenu(QPoint const& pos);
	void ClearFilters();

	CImpl const&             m_impl;
	CFilterProxyModel* const m_pFilterProxyModel;
	CItemModel* const        m_pModel;
	CTreeView* const         m_pTreeView;
	QFilteringPanel*         m_pFilteringPanel;
	i32 const                m_nameColumn;
};
} //endns Wwise
} //endns Impl
} //endns ACE
