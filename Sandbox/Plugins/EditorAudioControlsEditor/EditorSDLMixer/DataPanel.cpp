// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "DataPanel.h"

#include "common.h"
#include "Impl.h"
#include "FilterProxyModel.h"
#include "ItemModel.h"
#include "TreeView.h"

#include <ModelUtils.h>
#include <QFilteringPanel.h>
#include <QSearchBox.h>
#include <QtUtil.h>
#include <FileDialogs/SystemFileDialog.h>

#include <QHeaderView>
#include <QMenu>
#include <QPushButton>
#include <QVBoxLayout>

namespace ACE
{
namespace Impl
{
namespace SDLMixer
{
//////////////////////////////////////////////////////////////////////////
CDataPanel::CDataPanel(CImpl const& impl)
	: m_impl(impl)
	, m_pFilterProxyModel(new CFilterProxyModel(this))
	, m_pModel(new CItemModel(impl, impl.GetRootItem(), this))
	, m_pTreeView(new CTreeView(this))
	, m_pImportButton(new QPushButton(tr("Import Files"), this))
	, m_nameColumn(static_cast<i32>(CItemModel::EColumns::Name))
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	m_pFilterProxyModel->setSourceModel(m_pModel);
	m_pFilterProxyModel->setFilterKeyColumn(m_nameColumn);

	m_pTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_pTreeView->setDragEnabled(true);
	m_pTreeView->setDragDropMode(QAbstractItemView::DragDrop);
	m_pTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_pTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pTreeView->setTreePosition(m_nameColumn);
	m_pTreeView->setUniformRowHeights(true);
	m_pTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
	m_pTreeView->setModel(m_pFilterProxyModel);
	m_pTreeView->sortByColumn(m_nameColumn, Qt::AscendingOrder);
	m_pTreeView->header()->setMinimumSectionSize(25);
	m_pTreeView->header()->setSectionResizeMode(static_cast<i32>(CItemModel::EColumns::Notification), QHeaderView::ResizeToContents);
	m_pTreeView->header()->setSectionResizeMode(static_cast<i32>(CItemModel::EColumns::PakStatus), QHeaderView::ResizeToContents);
	m_pTreeView->SetNameColumn(m_nameColumn);
	m_pTreeView->SetNameRole(static_cast<i32>(ModelUtils::ERoles::Name));
	m_pTreeView->TriggerRefreshHeaderColumns();

	m_pFilteringPanel = new QFilteringPanel("ACESDLMixerDataPanel", m_pFilterProxyModel, this);
	m_pFilteringPanel->SetContent(m_pTreeView);
	m_pFilteringPanel->GetSearchBox()->SetAutoExpandOnSearch(m_pTreeView);

	auto const pMainLayout = new QVBoxLayout(this);
	pMainLayout->setContentsMargins(0, 0, 0, 0);
	pMainLayout->addWidget(m_pImportButton);
	pMainLayout->addWidget(m_pFilteringPanel);

	QObject::connect(m_pImportButton, &QPushButton::clicked, this, &CDataPanel::OnImportFiles);
	QObject::connect(m_pTreeView, &CTreeView::customContextMenuRequested, this, &CDataPanel::OnContextMenu);
}

//////////////////////////////////////////////////////////////////////////
void CDataPanel::OnContextMenu(QPoint const& pos)
{
	auto const pContextMenu = new QMenu(this);
	auto const& selection = m_pTreeView->selectionModel()->selectedRows(m_nameColumn);

	if (!selection.isEmpty())
	{
		if (selection.count() == 1)
		{
			ControlId const itemId = selection[0].data(static_cast<i32>(ModelUtils::ERoles::Id)).toInt();
			auto const pItem = static_cast<CItem const*>(m_impl.GetItem(itemId));

			if ((pItem != nullptr) && ((pItem->GetFlags() & EItemFlags::IsConnected) != 0))
			{
				SControlInfos controlInfos;
				m_impl.SignalGetConnectedSystemControls(pItem->GetId(), controlInfos);

				if (!controlInfos.empty())
				{
					auto const pConnectionsMenu = new QMenu(pContextMenu);

					for (auto const& info : controlInfos)
					{
						pConnectionsMenu->addAction(info.icon, QtUtil::ToQString(info.name), [=]()
									{
										m_impl.SignalSelectConnectedSystemControl(info.id, pItem->GetId());
						      });
					}

					pConnectionsMenu->setTitle(tr("Connections (" + ToString(controlInfos.size()) + ")"));
					pContextMenu->addMenu(pConnectionsMenu);
					pContextMenu->addSeparator();
				}
			}

			if ((pItem != nullptr) && ((pItem->GetPakStatus() & EPakStatus::OnDisk) != 0) && !pItem->GetFilePath().IsEmpty())
			{
				pContextMenu->addAction(tr("Show in File Explorer"), [=]()
							{
								QtUtil::OpenInExplorer((PathUtil::GetGameFolder() + "/" + pItem->GetFilePath()).c_str());
				      });

				pContextMenu->addSeparator();
			}
		}

		pContextMenu->addAction(tr("Expand Selection"), [=]() { m_pTreeView->ExpandSelection(); });
		pContextMenu->addAction(tr("Collapse Selection"), [=]() { m_pTreeView->CollapseSelection(); });
		pContextMenu->addSeparator();
	}

	pContextMenu->addAction(tr("Expand All"), [=]() { m_pTreeView->expandAll(); });
	pContextMenu->addAction(tr("Collapse All"), [=]() { m_pTreeView->collapseAll(); });

	pContextMenu->addSeparator();
	pContextMenu->addAction(tr("Import Files"), [=]() { OnImportFiles(); });

	pContextMenu->exec(QCursor::pos());
}

//////////////////////////////////////////////////////////////////////////
void CDataPanel::OnImportFiles()
{
	QString targetFolderName = "";

	if (!m_pTreeView->selectionModel()->selectedRows(m_nameColumn).isEmpty())
	{
		targetFolderName = m_pModel->GetTargetFolderName(m_pFilterProxyModel->mapToSource(m_pTreeView->currentIndex()));
	}

	m_impl.SignalImportFiles(s_extensionFilters, s_supportedFileTypes, targetFolderName);
}

//////////////////////////////////////////////////////////////////////////
void CDataPanel::ClearFilters()
{
	m_pFilteringPanel->GetSearchBox()->clear();
	m_pFilteringPanel->Clear();
}

//////////////////////////////////////////////////////////////////////////
void CDataPanel::Reset()
{
	ClearFilters();
	m_pFilterProxyModel->invalidate();
}

//////////////////////////////////////////////////////////////////////////
void CDataPanel::OnAboutToReload()
{
	m_pTreeView->BackupExpanded();
	m_pTreeView->BackupSelection();
	m_pModel->Reset();
}

//////////////////////////////////////////////////////////////////////////
void CDataPanel::OnReloaded()
{
	m_pModel->Reset();
	m_pTreeView->RestoreExpanded();
	m_pTreeView->RestoreSelection();
}

//////////////////////////////////////////////////////////////////////////
void CDataPanel::OnConnectionAdded() const
{
	m_pFilterProxyModel->invalidate();
}

//////////////////////////////////////////////////////////////////////////
void CDataPanel::OnConnectionRemoved() const
{
	m_pFilterProxyModel->invalidate();
}

//////////////////////////////////////////////////////////////////////////
void CDataPanel::OnSelectConnectedItem(ControlId const id)
{
	ClearFilters();
	auto const& matches = m_pFilterProxyModel->match(m_pFilterProxyModel->index(0, 0, QModelIndex()), static_cast<i32>(ModelUtils::ERoles::Id), id, 1, Qt::MatchRecursive);

	if (!matches.isEmpty())
	{
		m_pTreeView->setFocus();
		m_pTreeView->selectionModel()->setCurrentIndex(matches.first(), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
	}
}

//////////////////////////////////////////////////////////////////////////
void CDataPanel::OnFileImporterOpened()
{
	m_pTreeView->setDragDropMode(QAbstractItemView::NoDragDrop);
}

//////////////////////////////////////////////////////////////////////////
void CDataPanel::OnFileImporterClosed()
{
	m_pTreeView->setDragDropMode(QAbstractItemView::DragDrop);
}
} //endns SDLMixer
} //endns Impl
} //endns ACE
