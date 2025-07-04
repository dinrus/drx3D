// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class CAbstractMenu;

class QMenuBar;

class CMenuBarUpdater
{
public:
	CMenuBarUpdater(CAbstractMenu* pAbstractMenu, QMenuBar* pMenuBar);
	~CMenuBarUpdater();

private:
	uintptr_t GetId() const { return (uintptr_t)this; }

private:
	CAbstractMenu* const m_pAbstractMenu;
};
