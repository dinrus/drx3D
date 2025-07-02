// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "DynamicPopupMenu.h"

#include "EditorCommonInit.h"
#include <drx3D/Sandbox/Editor/EditorCommon/ICommandManager.h>

#include <algorithm>

#include <QMenu>
#include <QCursor>
#include <QAction>

//////////////////////////////////////////////////////////////////////////
class CPopupMenuItemCommand : public CPopupMenuItem
{
public:
	CPopupMenuItemCommand(tukk text, tukk command)
		: CPopupMenuItem(text)
		, m_command(command)
	{
		m_function = [&]() {
			GetIEditor()->GetICommandManager()->Execute(m_command);
		};
	}

	virtual bool IsEditorCommand() { return true; }
	tukk GetCommand() const { return m_command; }

private:
	string m_command;
};

/////////////////////////////////////////////////////////////////////////
class QPopupMenuItemAction : public QAction
{
public:
	
	QPopupMenuItemAction(const std::shared_ptr<CPopupMenuItem> item, QObject* parent);
	~QPopupMenuItemAction();

	void OnTriggered();
	void OnHovered();

private:
	std::shared_ptr<CPopupMenuItem> m_item;
};

QPopupMenuItemAction::QPopupMenuItemAction(const std::shared_ptr<CPopupMenuItem> item, QObject* parent)
	: QAction(QString(item->Text()), parent)
	, m_item(item)
{
	//TODO : handle hotkeys through commands or having hotkeys as a KeyboardShortcut embedded here
	if (m_item->IsChecked())
	{
		setCheckable(true);
		setChecked(true);
	}
	setEnabled(m_item->IsEnabled());
	if (m_item->IsDefault())
	{
		QFont boldFont = font();
		boldFont.setBold(true);
		setFont(boldFont);
	}

	connect(this, &QPopupMenuItemAction::triggered, this, &QPopupMenuItemAction::OnTriggered);
	connect(this, &QPopupMenuItemAction::hovered, this, &QPopupMenuItemAction::OnHovered);
}

QPopupMenuItemAction::~QPopupMenuItemAction()
{
}

void QPopupMenuItemAction::OnTriggered()
{
	m_item->Call();
}

void QPopupMenuItemAction::OnHovered()
{
	if(m_item->m_hoverFunc)
		m_item->m_hoverFunc();
}

CDynamicPopupMenu::CDynamicPopupMenu()
	: m_root(CPopupMenuItem(""))
{
}

void CDynamicPopupMenu::Clear()
{
	m_root.GetChildren().clear();
}

CPopupMenuItem::~CPopupMenuItem()
{
}


CPopupMenuItem& CPopupMenuItem::Add(tukk text)
{
	CPopupMenuItem* item = new CPopupMenuItem(text);
	AddChildren(item);
	return *item;
}

CPopupMenuItem& CPopupMenuItem::Add(tukk text, const std::function<void()>& function)
{
	CPopupMenuItem* item = new CPopupMenuItem(text);
	item->m_function = function;
	AddChildren(item);
	return *item;
}

CPopupMenuItem& CPopupMenuItem::Add(tukk text,tukk icon, const std::function<void()>& function)
{
	CPopupMenuItem* item = new CPopupMenuItem(text);
	item->m_function = function;
	item->m_icon = icon;
	AddChildren(item);
	return *item;
}

CPopupMenuItem& CPopupMenuItem::AddSeparator()
{
	return Add("-");
}

CPopupMenuItem& CPopupMenuItem::AddCommand(tukk text, string commandToExecute)
{
	CPopupMenuItemCommand* item = new CPopupMenuItemCommand(text, commandToExecute);
	AddChildren(item);
	return *item;
}

CPopupMenuItem* CPopupMenuItem::Find(tukk text)
{
	for (Children::iterator it = m_children.begin(); it != m_children.end(); ++it)
	{
		CPopupMenuItem* item = it->get();
		if (strcmp(item->Text(), text) == 0)
			return item;
	}
	return 0;
}

QMenu* CDynamicPopupMenu::CreateQMenu()
{
	QMenu* menu = new QMenu();
	PopulateQMenu(menu);
	if (m_onHide)
	{
		QObject::connect(menu, &QMenu::aboutToHide, m_onHide);
	}
	return menu;
}

void CDynamicPopupMenu::PopulateQMenu(class QMenu* menu)
{
	if (m_root.GetChildren().empty())
		return;
	else
		PopulateQMenu(menu, &m_root);
}

void CDynamicPopupMenu::PopulateQMenu(class QMenu* menu, CPopupMenuItem* parentItem)
{
	for (auto& childItem : parentItem->GetChildren())
	{
		if (childItem->GetChildren().empty())
		{
			string text = childItem->Text();
			if (text == "-")
				menu->addSeparator();
			else if (childItem->IsEditorCommand())
			{
				tukk cmd = static_cast<CPopupMenuItemCommand*>(childItem.get())->GetCommand();
				QAction* action = GetIEditor()->GetICommandManager()->GetAction(cmd, text.c_str());
				action->setEnabled(childItem->IsEnabled());
				menu->addAction(action);
			}
			else
			{
				QAction* action = new QPopupMenuItemAction(childItem, menu);
				if (!childItem->m_icon.empty())
				{
					action->setIcon(DrxIcon(childItem->m_icon.c_str()));
				}
				menu->addAction(action);
			}
		}
		else
		{
			QMenu* subMenu = menu->addMenu(childItem->Text());
			PopulateQMenu(subMenu, childItem.get());
		}
	}
}

void CDynamicPopupMenu::Spawn(i32 x, i32 y)
{
	QMenu* menu = CreateQMenu();
	menu->exec(QPoint(x, y)); // Should already trigger an action
}

void CDynamicPopupMenu::SpawnAtCursor()
{
	Spawn(QCursor::pos().x(), QCursor::pos().y());
}


