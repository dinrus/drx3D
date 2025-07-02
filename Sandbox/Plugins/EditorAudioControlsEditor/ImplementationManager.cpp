// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "ImplementationManager.h"

#include "AudioControlsEditorPlugin.h"

#include <IUndoManager.h>
#include <DrxSystem/IConsole.h>

string const g_sImplementationCVarName = "s_AudioImplName";

namespace ACE
{
typedef void (WINAPI * PGNSI)();
using TPfnGetAudioInterface = Impl::IImpl * (*)(ISystem*);

Impl::IImpl* g_pIImpl = nullptr;

//////////////////////////////////////////////////////////////////////////
CImplementationManager::CImplementationManager()
	: m_hMiddlewarePlugin(nullptr)
{
}

//////////////////////////////////////////////////////////////////////////
CImplementationManager::~CImplementationManager()
{
	if (m_hMiddlewarePlugin != nullptr)
	{
		FreeLibrary(m_hMiddlewarePlugin);
		m_hMiddlewarePlugin = nullptr;
	}

	g_pIImpl = nullptr;
}

//////////////////////////////////////////////////////////////////////////
bool CImplementationManager::LoadImplementation()
{
	SignalImplementationAboutToChange();
	GetIEditor()->GetIUndoManager()->Suspend();

	bool isLoaded = true;
	ICVar const* const pCVar = gEnv->pConsole->GetCVar(g_sImplementationCVarName);

	if (pCVar != nullptr)
	{
		if (m_hMiddlewarePlugin != nullptr)
		{
			// Need to flush the undo/redo queue to make sure we're not keeping data from
			// previous implementation there
			GetIEditor()->GetIUndoManager()->Flush();

			FreeLibrary(m_hMiddlewarePlugin);
			m_hMiddlewarePlugin = nullptr;
			g_pIImpl = nullptr;
		}

		char szExecutableDirPath[_MAX_PATH];
		DrxGetExecutableFolder(sizeof(szExecutableDirPath), szExecutableDirPath);
		drx_sprintf(szExecutableDirPath, "%sEditorPlugins\\ace\\Editor%s.dll", szExecutableDirPath, pCVar->GetString());

		m_hMiddlewarePlugin = LoadLibraryA(szExecutableDirPath);

		if (m_hMiddlewarePlugin == nullptr)
		{
			DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_WARNING, "[Audio Controls Editor] Couldn't load the middleware specific editor dll.");
			isLoaded = false;
		}
		else
		{
			auto const pfnAudioInterface = (TPfnGetAudioInterface)GetProcAddress(m_hMiddlewarePlugin, "GetAudioInterface");

			if (pfnAudioInterface == nullptr)
			{
				DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_WARNING, "[Audio Controls Editor] Couldn't get middleware interface from loaded dll.");
				FreeLibrary(m_hMiddlewarePlugin);
				isLoaded = false;
			}
			else
			{
				if (GetIEditor() != nullptr)
				{
					g_pIImpl = pfnAudioInterface(GetIEditor()->GetSystem());
					CAudioControlsEditorPlugin::ReloadData(EReloadFlags::ReloadImplData | EReloadFlags::SetPlatforms);
				}
			}
		}
	}
	else
	{
		DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_WARNING, "[Audio Controls Editor] CVar %s not defined. Needed to derive the Editor plugin name.", g_sImplementationCVarName);
		isLoaded = false;
	}

	GetIEditor()->GetIUndoManager()->Resume();
	SignalImplementationChanged();
	return isLoaded;
}

//////////////////////////////////////////////////////////////////////////
void CImplementationManager::Release()
{
	if (m_hMiddlewarePlugin)
	{
		FreeLibrary(m_hMiddlewarePlugin);
	}
}
} //endns ACE

