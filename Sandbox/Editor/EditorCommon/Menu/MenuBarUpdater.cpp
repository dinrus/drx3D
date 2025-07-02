// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "MenuBarUpdater.h"
#include "AbstractMenu.h"
#include "MenuWidgetBuilders.h"

#include <QMenuBar>

CMenuBarUpdater::CMenuBarUpdater(CAbstractMenu* pAbstractMenu, QMenuBar* pMenuBar)
	: m_pAbstractMenu(pAbstractMenu)
{
	auto rebuild = [pAbstractMenu, pMenuBar]()
	{
		pMenuBar->clear();
		pAbstractMenu->Build(MenuWidgetBuilders::CMenuBarBuilder(pMenuBar));
	};

	pAbstractMenu->signalActionAdded.Connect(rebuild, GetId());
	pAbstractMenu->signalMenuAdded.Connect([rebuild](CAbstractMenu*) { rebuild(); }, GetId());
}

CMenuBarUpdater::~CMenuBarUpdater()
{
	m_pAbstractMenu->signalActionAdded.DisconnectById(GetId());
}

