// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>

#include "CommandHistory.h"
#include "RecursionLoopGuard.h"
#include "IUndoManager.h"
#include <QMouseEvent>
#include <QLayout>
#include <QAdvancedTreeView.h>
#include <EditorStyleHelper.h>
#include "IEditorClassFactory.h"

REGISTER_HIDDEN_VIEWPANE_FACTORY(CHistoryPanel, "Undo History", "Tools", true)

namespace Private_CommandHistory
{
class QHistoryModel : public QAbstractTableModel
{
public:
	QHistoryModel(QWidget* parent, IUndoManager* pCommandManager);
	~QHistoryModel();
	i32      rowCount(const QModelIndex& parent = QModelIndex()) const;
	i32      columnCount(const QModelIndex& parent = QModelIndex()) const;
	QVariant data(const QModelIndex& index, i32 role = Qt::DisplayRole) const;
	QVariant headerData(i32 section, Qt::Orientation orientation, i32 role) const;
	void     OnCommandBufferChange(CUndoStep* pCommand, IUndoManager::ECommandChangeType change, i32 index);
	void     FillHistoryFromCommandBuffer();

	CDrxSignal<void(CUndoStep*, IUndoManager::ECommandChangeType, i32)> signalBufferChanged;

private:
	IUndoManager*           m_pCommandManager;
	std::vector<CUndoStep*> m_commands;
	QFont                   m_font;
	QFont                   m_disabledFont;

	i32 getCurrentIndex() const
	{
		if (m_pCommandManager)
			return m_pCommandManager->GetUndoStackLen();
		return 0;
	}
};

QHistoryModel::QHistoryModel(QWidget* parent, IUndoManager* pCommandManager)
	: QAbstractTableModel(parent)
	, m_pCommandManager(pCommandManager)
{
	FillHistoryFromCommandBuffer();
	if (m_pCommandManager)
		m_pCommandManager->signalBufferChanged.Connect(this, &QHistoryModel::OnCommandBufferChange);

	m_font = QFont(parent->font());

}

QHistoryModel::~QHistoryModel()
{
	if (m_pCommandManager)
		m_pCommandManager->signalBufferChanged.DisconnectObject(this);
}

void QHistoryModel::FillHistoryFromCommandBuffer()
{
	m_commands.clear();

	if (m_pCommandManager)
	{
		for (CUndoStep* pCommand : m_pCommandManager->GetUndoStack())
			m_commands.push_back(pCommand);
		for (CUndoStep* pCommand : m_pCommandManager->GetRedoStack())
			m_commands.push_back(pCommand);
	}

	dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

void QHistoryModel::OnCommandBufferChange(CUndoStep* pCommand, IUndoManager::ECommandChangeType changeType, i32 changedIndex)
{
	if (!m_pCommandManager)
		return;

	if (IUndoManager::eCommandChangeType_Move & changeType)
	{
		dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
	}

	if (IUndoManager::eCommandChangeType_Insert & changeType)
	{
		beginInsertRows(QModelIndex(), changedIndex + 1, changedIndex + 1);
		m_commands.insert(m_commands.begin() + changedIndex, pCommand);
		endInsertRows();
	}

	if (IUndoManager::eCommandChangeType_Remove & changeType)
	{
		if (-1 != changedIndex && 0 != m_commands.size())
		{
			if (pCommand)
			{
				beginRemoveRows(QModelIndex(), changedIndex + 1, changedIndex + 1);
				m_commands.erase(m_commands.begin() + changedIndex);
				endRemoveRows();
			}
			else if (IUndoManager::eCommandChangeType_Undo & changeType)
			{
				beginRemoveRows(QModelIndex(), 1, changedIndex);
				m_commands.erase(m_commands.begin(), m_commands.begin() + changedIndex);
				endRemoveRows();
			}
			else if (IUndoManager::eCommandChangeType_Redo & changeType)
			{
				beginRemoveRows(QModelIndex(), rowCount() - (changedIndex + 1), rowCount() - 1);
				m_commands.erase(m_commands.end() - changedIndex, m_commands.end());
				endRemoveRows();
			}
		}
	}

	signalBufferChanged(pCommand, changeType, changedIndex);
}

i32 QHistoryModel::rowCount(const QModelIndex& parent) const
{
	if (!parent.isValid() && m_pCommandManager)
		return 1 + m_commands.size();
	return 1;
}

i32 QHistoryModel::columnCount(const QModelIndex& parent) const
{
	if (!parent.isValid())
		return 1;
	return 0;
}

QVariant QHistoryModel::data(const QModelIndex& index, i32 role) const
{
	i32 row = index.row();
	i32 col = index.column();

	switch (role)
	{
	case Qt::DisplayRole:
		if (0 == row)
		{
			return QString("Starting Point");
		}
		else if (m_commands.size() >= row && m_pCommandManager)
		{
			string name;
			m_pCommandManager->GetCommandName(m_commands[row - 1], name);
			return QString(name.GetBuffer());
		}
		break;

	case Qt::FontRole:
		if (row > getCurrentIndex())
		{
			QFont font;
			font.setItalic(true);
			return font;
		}
		break;

	case Qt::TextColorRole:
		if (row > getCurrentIndex())
		{
			QColor color = GetStyleHelper()->disabledWindowTextColor();
			return color;
		}
		break;
	}
	return QVariant();
}

QVariant QHistoryModel::headerData(i32 section, Qt::Orientation orientation, i32 role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch (section)
		{
		case 0:
			return QString("Commands");
		}
	}

	return QVariant();
}

class QHistoryView : public QAdvancedTreeView
{
public:
	QHistoryView(IUndoManager* pCommandManager, QHistoryModel* pModel)
		: m_pCommandManager(pCommandManager)
		, m_validating(false)
		, m_pModel(pModel)
	{
		setSelectionMode(QAbstractItemView::SingleSelection);

		if (m_pCommandManager && pModel)
		{
			pModel->signalBufferChanged.Connect(this, &QHistoryView::OnCommandBufferChange);
		}
	}

	~QHistoryView()
	{
	}

protected:
	void selectRow(i32 row)
	{
		if (selectionModel() && model())
			selectionModel()->select(model()->index(row, 0), QItemSelectionModel::ClearAndSelect);
	}

	void QHistoryView::OnCommandBufferChange(CUndoStep* pCommand, IUndoManager::ECommandChangeType changeType, i32 index)
	{
		RECURSION_GUARD(m_validating);

		if (IUndoManager::eCommandChangeType_Move & changeType)
		{
			selectRow(index + 1);
		}

		if (IUndoManager::eCommandChangeType_Insert & changeType)
		{
			selectRow(index + 1);
		}

		if (IUndoManager::eCommandChangeType_Remove & changeType)
		{
			if (-1 != index)
			{
				if (m_pCommandManager)
				{
					selectRow(index);
				}
				else if (IUndoManager::eCommandChangeType_Undo & changeType)
				{
					selectRow(model()->rowCount() - 1);
				}
				else if (IUndoManager::eCommandChangeType_Redo & changeType)
				{
					selectRow(model()->rowCount() - 1);
				}
			}
		}
	}

	virtual void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override
	{
		RECURSION_GUARD(m_validating);

		if (0 != selected.size() && m_pCommandManager)
		{
			i32 row = selected.front().top();
			i32 count = m_pCommandManager->GetRedoStackLen() + m_pCommandManager->GetUndoStackLen() + 1;

			if (0 <= row && count > row)
			{
				i32 undoCount = m_pCommandManager->GetUndoStackLen();
				i32 commandIndex = row - undoCount;

				if (commandIndex > 0)
					m_pCommandManager->Redo(commandIndex);
				else if (commandIndex < 0)
					m_pCommandManager->Undo(abs(commandIndex));
			}
		}

		__super::selectionChanged(selected, deselected);
	}

private:
	bool           m_validating;
	IUndoManager*  m_pCommandManager;
	QHistoryModel* m_pModel;
};
}

CHistoryPanel::CHistoryPanel()
	: CDockableWidget()
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(1, 1, 1, 1);
	setLayout(layout);

	IUndoManager* pCommandManager = GetIEditorImpl()->GetIUndoManager();
	if (pCommandManager)
	{
		Private_CommandHistory::QHistoryModel* pHistoryModel = new Private_CommandHistory::QHistoryModel(this, pCommandManager);
		Private_CommandHistory::QHistoryView* pHistoryList = new Private_CommandHistory::QHistoryView(pCommandManager, pHistoryModel);
		pHistoryList->setModel(pHistoryModel);
		layout->addWidget(pHistoryList);
	}
}

CHistoryPanel::~CHistoryPanel()
{
}

