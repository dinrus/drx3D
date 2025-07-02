// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "DrxLinkCommands.h"

#include "MainWindow.h"

namespace Schematyc
{
static void RpcEditorShow(IConsoleCmdArgs* pArgs)
{
	if (gEnv->IsEditor())
	{
		DrxGUID elementGUID;
		if (pArgs->GetArgCount() >= 2)
		{
			elementGUID = DrxGUID::FromString(pArgs->GetArg(1));
		}

		DrxGUID detailGUID;
		if (pArgs->GetArgCount() >= 3)
		{
			detailGUID = DrxGUID::FromString(pArgs->GetArg(2));
		}

		if (!GUID::IsEmpty(elementGUID))
		{
			tukk szPaneTitle = "Schematyc";
			GetIEditor()->OpenView(szPaneTitle);
			// TODO: Try to find an opened instance or open a new editor window.
			/*CMainWindow* pMainWindow = CMainWindow::GetInstance();
			   if (pMainWindow)
			   {
			   pMainWindow->Show(elementGUID, detailGUID);
			   }*/
		}
	}
}

CDrxLinkCommands::CDrxLinkCommands()
	: m_pConsole(nullptr)
	, m_bRegistered(false)
{}

CDrxLinkCommands::~CDrxLinkCommands()
{
	Unregister();
}

void CDrxLinkCommands::Register(IConsole* pConsole)
{
	DRX_ASSERT(pConsole);
	m_pConsole = pConsole;
	if (m_pConsole && !m_bRegistered)
	{
		REGISTER_COMMAND("sc_rpcShow", RpcEditorShow, VF_NULL, "Show in Schematyc editor");
		m_bRegistered = true;
	}
}

void CDrxLinkCommands::Unregister()
{
	if (m_pConsole && m_bRegistered)
	{
		m_pConsole->RemoveCommand("sc_rpcShow");
		m_bRegistered = false;
	}
}

CDrxLinkCommands& CDrxLinkCommands::GetInstance()
{
	return ms_instance;
}

CDrxLinkCommands CDrxLinkCommands::ms_instance;
} // Schematyc

