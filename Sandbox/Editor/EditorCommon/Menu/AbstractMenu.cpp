// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "AbstractMenu.h"
#include "QtUtil.h"
#include "IEditor.h"
#include <drx3D/Sandbox/Editor/EditorCommon/ICommandManager.h>

struct CAbstractMenu::SMenuItem
{
	SMenuItem(i32 priority, i32 section)
		: m_priority(priority)
		, m_section(section)
	{
	}

	virtual void Build(IWidgetBuilder& widgetBuilder) const = 0;

	std::pair<i32, i32> MakeKey() const { return std::make_pair(m_section, m_priority); }

	i32 m_priority;
	i32 m_section;
};

struct CAbstractMenu::SActionItem : SMenuItem
{
	SActionItem(QAction* pAction, i32 priority, i32 section)
		: SMenuItem(priority, section)
		, m_pAction(pAction)
	{
	}

	virtual void Build(IWidgetBuilder& widgetBuilder) const
	{
		widgetBuilder.AddAction(m_pAction);
	}

	QAction* const m_pAction;
};

struct CAbstractMenu::SSubMenuItem : SMenuItem
{
	SSubMenuItem(CAbstractMenu* pMenu, i32 priority, i32 section)
		: SMenuItem(priority, section)
		, m_pMenu(pMenu)
	{
	}

	virtual void Build(IWidgetBuilder& widgetBuilder) const
	{
		std::unique_ptr<IWidgetBuilder> pSubMenuBuilder = widgetBuilder.AddMenu(m_pMenu);
		if (pSubMenuBuilder)
		{
			m_pMenu->Build(*pSubMenuBuilder);
		}
	}

	CAbstractMenu* const m_pMenu;
};

struct CAbstractMenu::SNamedSection
{
	explicit SNamedSection(i32 section)
		: m_section(section)
	{
	}

	i32k m_section;
	string m_name;
};

CAbstractMenu::CAbstractMenu(tukk szName)
	: m_name(szName)
	, m_bEnabled(true)
{
}

CAbstractMenu::CAbstractMenu()
	: m_bEnabled(true)
{
}

CAbstractMenu::~CAbstractMenu()
{
}

bool CAbstractMenu::IsEmpty() const
{
	return m_sortedItems.empty();
}

i32 CAbstractMenu::GetNextEmptySection() const
{
	return IsEmpty() ? 0 : (GetMaxSection() + 1);
}

i32 CAbstractMenu::GetMaxSection() const
{
	DRX_ASSERT(!IsEmpty());
	return m_sortedItems.back()->m_section;
}

void CAbstractMenu::Clear()
{
	m_sortedItems.clear();

	m_subMenuItems.clear();
	m_actionItems.clear();

	m_subMenus.clear();
	m_actions.clear();
}

i32 CAbstractMenu::GetMaxPriority(i32 section) const
{
	DRX_ASSERT(!IsEmpty());
	DRX_ASSERT(section >= eSections_Min);
	const std::pair<i32, i32> adj(section + 1, ePriorities_Min);
	auto maxItemInSection = --std::lower_bound(m_sortedItems.begin(), m_sortedItems.end(), adj, [](const auto& lhp, const auto& adj)
	{
		return lhp->MakeKey() < adj;
	});
	return (*maxItemInSection)->m_priority;
}

i32 CAbstractMenu::GetDefaultSection() const
{
	return IsEmpty() ? 0 : GetMaxSection();
}

i32 CAbstractMenu::GetSectionFromHint(i32 sectionHint) const
{
	return sectionHint == eSections_Default ? GetDefaultSection() : sectionHint;
}

i32 CAbstractMenu::GetPriorityFromHint(i32 priorityHint, i32 section) const
{
	if (priorityHint == ePriorities_Append)
	{
		return !IsEmpty() ? GetMaxPriority(section) : 0;
	}
	else
	{
		return priorityHint;
	}
}

void CAbstractMenu::AddAction(QAction* pAction, i32 sectionHint, i32 priorityHint)
{
	i32k section = GetSectionFromHint(sectionHint);
	m_actionItems.emplace_back(new SActionItem(pAction, GetPriorityFromHint(priorityHint, section), section));
	InsertItem(m_actionItems.back().get());
	signalActionAdded();
}

void CAbstractMenu::AddCommandAction(tukk szCommand, i32 sectionHint /*= eSections_Default*/, i32 priorityHint /*= ePriorities_Append*/)
{
	QAction* pAction = GetIEditor()->GetICommandManager()->GetAction(szCommand);

	if (!pAction)
		DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_ERROR_DBGBRK, "Command not found");

	AddAction(pAction, sectionHint, priorityHint);
}

QAction* CAbstractMenu::CreateAction(const QString& name, i32 sectionHint, i32 priorityHint)
{
	QAction* const pAction = new QAction(name, nullptr);
	m_actions.emplace_back(pAction);
	AddAction(pAction, sectionHint, priorityHint);
	return pAction;
}

QAction* CAbstractMenu::CreateAction(const QIcon& icon, const QString& name, i32 sectionHint, i32 priorityHint)
{
	QAction* const pAction = new QAction(icon, name, nullptr);
	m_actions.emplace_back(pAction);
	AddAction(pAction, sectionHint, priorityHint);
	return pAction;
}

CAbstractMenu* CAbstractMenu::CreateMenu(tukk szName, i32 sectionHint, i32 priorityHint)
{
	m_subMenus.emplace_back(new CAbstractMenu(szName));
	CAbstractMenu* const pSubMenu = m_subMenus.back().get();

	i32k section = GetSectionFromHint(sectionHint);
	i32k priority = GetPriorityFromHint(priorityHint, section);

	std::unique_ptr<SSubMenuItem> pSubMenuItem(new SSubMenuItem(pSubMenu, priority, section));

	InsertItem(pSubMenuItem.get());
	m_subMenuItems.push_back(std::move(pSubMenuItem));

	signalMenuAdded(pSubMenu);

	return pSubMenu;
}

CAbstractMenu* CAbstractMenu::CreateMenu(const QString& name, i32 sectionHint, i32 priorityHint)
{
	return CreateMenu(QtUtil::ToString(name).c_str(), sectionHint, priorityHint);
}

CAbstractMenu* CAbstractMenu::FindMenu(tukk szName)
{
	auto it = std::find_if(m_subMenus.begin(), m_subMenus.end(), [szName](const auto& other)
	{
		return other->m_name == szName;
	});
	return it != m_subMenus.end() ? it->get() : nullptr;
}

CAbstractMenu* CAbstractMenu::FindMenuRecursive(tukk szName)
{
	std::deque<CAbstractMenu*> queue;
	queue.insert(queue.end(), m_subMenus.begin(), m_subMenus.end());
	while (!queue.empty())
	{
		if (queue.front()->m_name == szName)
		{
			return queue.front();
		}
		queue.insert(queue.end(), queue.front()->m_subMenus.begin(), queue.front()->m_subMenus.end());
		queue.pop_front();
	}
	return nullptr;
}

void CAbstractMenu::SetSectionName(i32 section, tukk szName)
{
	auto it = std::find_if(m_sortedNamedSections.begin(), m_sortedNamedSections.end(), [section](const auto& other)
	{
		return other->m_section == section;
	});
	if (it != m_sortedNamedSections.end())
	{
		(*it)->m_name = szName;
	}
	else
	{
		std::unique_ptr<SNamedSection> namedSection(new SNamedSection(section));
		namedSection->m_name = szName;
		InsertNamedSection(std::move(namedSection));
	}
}

bool CAbstractMenu::IsNamedSection(i32 section) const
{
	return std::find_if(m_sortedNamedSections.begin(), m_sortedNamedSections.end(), [section](const auto& other)
	{
		return other->m_section == section;
	}) != m_sortedNamedSections.end();
}

tukk CAbstractMenu::GetSectionName(i32 section) const
{
	auto it = std::find_if(m_sortedNamedSections.begin(), m_sortedNamedSections.end(), [section](const auto& other)
	{
		return other->m_section == section;
	});
	if (it == m_sortedNamedSections.end())
	{
		return "";
	}
	else
	{
		return (*it)->m_name.c_str();
	}
}

i32 CAbstractMenu::FindSectionByName(tukk szName) const
{
	for (const auto& section : m_sortedNamedSections)
	{
		if (section.get()->m_name.Compare(szName) == 0)
		{
			return section.get()->m_section;
		}
	}
	return eSections_Default;
}

bool CAbstractMenu::ContainsAction(const QAction* pAction) const
{
	return m_actionItems.end() != std::find_if(m_actionItems.begin(), m_actionItems.end(), [pAction](const auto& other)
	{
		return other->m_pAction == pAction;
	});
}

tukk CAbstractMenu::GetName() const
{
	return m_name.c_str();
}

void CAbstractMenu::Build(IWidgetBuilder& widgetBuilder) const
{
	const size_t N = m_sortedItems.size();
	size_t i = 0;
	while (i < N)
	{
		i32k section = m_sortedItems[i]->m_section;
		if (i > 0)
		{
			widgetBuilder.AddSection(IWidgetBuilder::SSection { section, GetSectionName(section) });
		}
		while (i < N && m_sortedItems[i]->m_section == section)
		{
			m_sortedItems[i]->Build(widgetBuilder);
			i++;
		}
	}

	//SetEnabled must be called last in order to iterate over all the actions and disabled them
	widgetBuilder.SetEnabled(m_bEnabled);
}

void CAbstractMenu::InsertItem(SMenuItem* pItem)
{
	m_sortedItems.insert(std::upper_bound(m_sortedItems.begin(), m_sortedItems.end(), pItem, [](const SMenuItem* lhp, const SMenuItem* rhp)
	{
		return lhp->MakeKey() < rhp->MakeKey();
	}), pItem);
}

void CAbstractMenu::InsertNamedSection(std::unique_ptr<SNamedSection>&& pNamedSection)
{
	const SNamedSection* const needle = pNamedSection.get();
	m_sortedNamedSections.insert(std::upper_bound(m_sortedNamedSections.begin(), m_sortedNamedSections.end(), needle, [](const auto& lhp, const auto& rhp)
	{
		return lhp->m_section < rhp->m_section;
	}), std::move(pNamedSection));
}

