// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <StdAfx.h>
#include "LayerPicker.h"

#include "LevelModelsManager.h"
#include "Objects/ObjectLayer.h"
#include "Objects/ObjectLayerManager.h"

#include "ProxyModels/DeepFilterProxyModel.h"
#include "QAdvancedTreeView.h"
#include "QSearchBox.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

#include "IResourceSelectorHost.h"

//////////////////////////////////////////////////////////////////////////
// Register for yasli

namespace
{
dll_string ShowDialog(const SResourceSelectorContext& context, tukk szPreviousValue)
{
	CLayerPicker layerPicker;
	CObjectLayer* previousValue = GetIEditorImpl()->GetObjectManager()->GetLayersManager()->FindLayerByFullName(szPreviousValue);
	layerPicker.SetSelectedLayer(previousValue);
	if (layerPicker.exec() == QDialog::Accepted)
	{
		return layerPicker.GetSelectedLayer()->GetFullName().GetBuffer();
	}

	return szPreviousValue;
}

REGISTER_RESOURCE_SELECTOR("LevelLayer", ShowDialog, "")
}

//////////////////////////////////////////////////////////////////////////

CLayerPicker::CLayerPicker()
	: CEditorDialog("LayerPicker")
{
	auto model = CLevelModelsManager::GetInstance().GetLevelModel();

	QDeepFilterProxyModel::BehaviorFlags behavior = QDeepFilterProxyModel::AcceptIfChildMatches | QDeepFilterProxyModel::AcceptIfParentMatches;
	QDeepFilterProxyModel* proxy = new QDeepFilterProxyModel(behavior);
	proxy->setSourceModel(model);
	proxy->setFilterKeyColumn(1);

	auto searchBox = new QSearchBox();
	searchBox->SetModel(proxy);
	searchBox->EnableContinuousSearch(true);

	m_treeView = new QAdvancedTreeView();
	m_treeView->setModel(proxy);
	m_treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_treeView->setSelectionMode(QAbstractItemView::SingleSelection);
	m_treeView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_treeView->setDragDropMode(QAbstractItemView::NoDragDrop);
	m_treeView->setTreePosition((i32)eLayerColumns_Name);
	m_treeView->setSortingEnabled(true);
	m_treeView->sortByColumn((i32)eLayerColumns_Name, Qt::AscendingOrder);

	connect(m_treeView, &QTreeView::doubleClicked, this, &CLayerPicker::SelectLayerIndex);

	searchBox->SetAutoExpandOnSearch(m_treeView);

	i32k count = proxy->columnCount(QModelIndex());
	if (count > 1)
	{
		for (i32 i = 0; i < count; i++)
		{
			m_treeView->SetColumnVisible(i, false);
		}

		m_treeView->SetColumnVisible((i32)eLayerColumns_Name, true);
	}

	//TODO : make use of the standard button dialog bar
	QPushButton* okButton = new QPushButton(this);
	okButton->setText(tr("Ok"));
	okButton->setDefault(true);
	connect(okButton, &QPushButton::clicked, this, &CLayerPicker::OnOk);

	QPushButton* cancelButton = new QPushButton(this);
	cancelButton->setText(tr("Cancel"));
	connect(cancelButton, &QPushButton::clicked, [this]()
	{
		reject();
	});

	auto buttonsLayout = new QHBoxLayout();
	buttonsLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred));
	buttonsLayout->addWidget(okButton);
	buttonsLayout->addWidget(cancelButton);

	auto mainLayout = new QVBoxLayout();
	mainLayout->addWidget(searchBox);
	mainLayout->addWidget(m_treeView);
	mainLayout->addLayout(buttonsLayout);

	setLayout(mainLayout);
}

void CLayerPicker::SetSelectedLayer(CObjectLayer* layer)
{
	auto model = m_treeView->model();
	auto matchingList = model->match(model->index(0, 0, QModelIndex()), (i32)CLevelModel::Roles::InternalPointerRole, reinterpret_cast<intptr_t>(layer), 1, Qt::MatchRecursive | Qt::MatchExactly);
	if (matchingList.size() == 1)
	{
		m_treeView->expand(matchingList.first().parent());
		m_treeView->scrollTo(matchingList.first());
		m_treeView->selectionModel()->setCurrentIndex(matchingList.first(), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
	}
}

void CLayerPicker::OnOk()
{
	QModelIndex selected = m_treeView->selectionModel()->currentIndex();
	SelectLayerIndex(selected);
}

void CLayerPicker::SelectLayerIndex(const QModelIndex& index)
{
	if (index.isValid())
	{
		QVariant layerVar = index.data((i32)CLevelModel::Roles::InternalPointerRole);
		if (layerVar.isValid())
		{
			CObjectLayer* pLayer = reinterpret_cast<CObjectLayer*>(layerVar.value<intptr_t>());
			if (pLayer != nullptr && pLayer->GetLayerType() == eObjectLayerType_Layer)
			{
				m_selectedLayer = pLayer;
				accept();
			}
		}
	}
}

