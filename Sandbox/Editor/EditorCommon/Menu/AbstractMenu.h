// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

#include <drx3D/CoreX/String/DrxString.h>
#include <drx3D/CoreX/Sandbox/DrxSignal.h>

#include <memory>
#include <vector>

class QAction;

//! CAbstractMenu is a simple menu abstraction which is similar to QMenu and provides the following benefits:
//! - Priority based ordering
//! - Sections instead of manually adding separators
//! - Removing of menu items
//! An abstract menu is a list of menu items. Each menu item is either an action or another menu.
//! Each menu item has a priority and a section. Within a menu, menu items are lexicographically sorted
//! by (section, priority, order of insertion).
//! Sections are intended to be visually separated.
class EDITOR_COMMON_API CAbstractMenu : public _i_reference_target_t
{
public:
	enum EPriorities : i32
	{
		ePriorities_Append = INT_MIN, //! See CAbstractMenu::AddAction.
		ePriorities_Min //! Priorities are in the range [PRIORITY_MIN, INT_MAX].
	};

	enum ESections: i32
	{
		eSections_Default = INT_MIN, //! See CAbstractMenu::AddAction.
		eSections_Min //! Sections are in the range [SECTION_MIN, INT_MAX].
	};

	//! Creates a menu-like widget (e.g., QMenu, QMenuBar, ...) from an abstract menu.
	//! \sa CAbstractMenu::Build()
	struct IWidgetBuilder
	{
		struct SSection
		{
			i32 m_section;
			string m_name;
		};

		//! Adds this action to widget. Does not take ownership of the action.
		virtual void AddAction(QAction* pAction) {}

		//! Adds a section to the widget.
		virtual void AddSection(const SSection& sec) {}

		//! Enables or disables the menu
		virtual void SetEnabled(bool enabled) {}

		//! Adds a sub-menu to the widget.
		//! \return A widget builder for the sub-menu; or a pointer that owns nothing, if no sub-menu should be created.
		virtual std::unique_ptr<IWidgetBuilder> AddMenu(const CAbstractMenu* pMenu) { return std::unique_ptr<IWidgetBuilder>(); }
	};

private:
	struct SActionItem;
	struct SMenuItem;
	struct SSubMenuItem;
	struct SNamedSection;

public:
	CAbstractMenu();
	~CAbstractMenu();

	CAbstractMenu(const CAbstractMenu&) = delete;
	CAbstractMenu& operator=(const CAbstractMenu&) = delete;

	i32 GetNextEmptySection() const;
	bool IsEmpty() const;
	i32 GetMaxSection() const;
	i32 GetMaxPriority(i32 section) const;

	void Clear();

	//! Add an action to the menu.
	//! \param priority If value is EPriorities::ePriorities_Append, the item is assigned the next integer that is
	//! greater than the maximum priority of all existing items. This effectively appends the item
	//! to the menu. Otherwise the value is the priority of the menu item.
	//! \param section If value is ESections::eSections_Default, the item will be added to the default section.
	//! Otherwise the menu item is added to this section.
	//! \sa CreateMenu
	void AddAction(QAction* pAction, i32 sectionHint = eSections_Default, i32 priorityHint = ePriorities_Append);

	//! Add an action based on editor commands to the menu. The command must be registered and be a UI command.
	//! \param priority If value is EPriorities::ePriorities_Append, the item is assigned the next integer that is
	//! greater than the maximum priority of all existing items. This effectively appends the item
	//! to the menu. Otherwise the value is the priority of the menu item.
	//! \param section If value is ESections::eSections_Default, the item will be added to the default section.
	//! Otherwise the menu item is added to this section.
	//! \sa ICommandManager
	void AddCommandAction(tukk szCommand, i32 sectionHint = eSections_Default, i32 priorityHint = ePriorities_Append);

	//! Creates an action and adds it to the menu.
	//! \param priority If value is EPriorities::ePriorities_Append, the item is assigned the next integer that is
	//! greater than the maximum priority of all existing items. This effectively appends the item
	//! to the menu. Otherwise the value is the priority of the menu item.
	//! \param section If value is ESections::eSections_Default, the item will be added to the default section.
	//! Otherwise the menu item is added to this section.
	QAction* CreateAction(const QString& name, i32 sectionHint = eSections_Default, i32 priorityHint = ePriorities_Append);

	//! Creates an action and adds it to the menu.
	//! \param priority If value is EPriorities::ePriorities_Append, the item is assigned the next integer that is
	//! greater than the maximum priority of all existing items. This effectively appends the item
	//! to the menu. Otherwise the value is the priority of the menu item.
	//! \param section If value is ESections::eSections_Default, the item will be added to the default section.
	//! Otherwise the menu item is added to this section.
	QAction* CreateAction(const QIcon& icon, const QString& name, i32 sectionHint = eSections_Default, i32 priorityHint = ePriorities_Append);

	//! Creates and adds a new sub-menu.
	//! \param priority If value is EPriorities::ePriorities_Append, the item is assigned the next integer that is
	//! greater than the maximum priority of all existing items. This effectively appends the item
	//! to the menu. Otherwise the value is the priority of the menu item.
	//! \param section If value is ESections::eSections_Default, the item will be added to the default section.
	//! Otherwise the menu item is added to this section.
	//! \sa AddAction
	CAbstractMenu* CreateMenu(tukk szName, i32 sectionHint = eSections_Default, i32 priorityHint = ePriorities_Append);

	//! Creates and adds a new sub-menu.
	//! \param priority If value is EPriorities::ePriorities_Append, the item is assigned the next integer that is
	//! greater than the maximum priority of all existing items. This effectively appends the item
	//! to the menu. Otherwise the value is the priority of the menu item.
	//! \param section If value is ESections::eSections_Default, the item will be added to the default section.
	//! Otherwise the menu item is added to this section.
	//! \sa AddAction
	CAbstractMenu* CreateMenu(const QString& name, i32 sectionHint = eSections_Default, i32 priorityHint = ePriorities_Append);

	void SetSectionName(i32 section, tukk szName);

	bool IsNamedSection(i32 section) const;
	tukk GetSectionName(i32 section) const;
	// Returns index of the section with the name equal to the szName value or eSections_Default if no such section is found. 
	i32 FindSectionByName(tukk szName) const;

	CAbstractMenu* FindMenu(tukk szName);
	CAbstractMenu* FindMenuRecursive(tukk szName);

	bool ContainsAction(const QAction* pAction) const;

	//! Will disable all its child actions, leaving them in the menu but greyed out
	void SetEnabled(bool enabled) { m_bEnabled = enabled; }
	bool IsEnabled() const { return m_bEnabled; }

	tukk GetName() const;

	void Build(IWidgetBuilder& widgetBuilder) const;

	CDrxSignal<void()> signalActionAdded;
	CDrxSignal<void(CAbstractMenu*)> signalMenuAdded;
	CDrxSignal<void(CAbstractMenu*)> signalAboutToShow;

private:
	CAbstractMenu(tukk szName);

	i32 GetSectionFromHint(i32 sectionHint) const;
	i32 GetPriorityFromHint(i32 priorityHint, i32 section) const;
	i32 GetDefaultSection() const;

	void InsertItem(SMenuItem* pItem);
	void InsertNamedSection(std::unique_ptr<SNamedSection>&& namedSection);

private:
	std::vector<_smart_ptr<CAbstractMenu>> m_subMenus;
	std::vector<std::unique_ptr<QAction>> m_actions;
	std::vector<std::unique_ptr<SSubMenuItem>> m_subMenuItems;
	std::vector<std::unique_ptr<SActionItem>> m_actionItems;
	std::vector<SMenuItem*> m_sortedItems;
	std::vector<std::unique_ptr<SNamedSection>> m_sortedNamedSections;
	string m_name;
	bool m_bEnabled;
};

