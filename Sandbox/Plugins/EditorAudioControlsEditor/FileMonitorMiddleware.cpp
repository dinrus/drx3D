// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "FileMonitorMiddleware.h"

#include "AudioControlsEditorPlugin.h"
#include "ImplementationManager.h"

#include <drx3D/CoreX/String/DrxPath.h>

namespace ACE
{
//////////////////////////////////////////////////////////////////////////
CFileMonitorMiddleware::CFileMonitorMiddleware(i32 const delay, QObject* const pParent)
	: CFileMonitorBase(delay, pParent)
{
	Enable();
}

//////////////////////////////////////////////////////////////////////////
void CFileMonitorMiddleware::Enable()
{
	if (g_pIImpl != nullptr)
	{
		stop();
		m_monitorFolders.clear();
		GetIEditor()->GetFileMonitor()->UnregisterListener(this);

		m_monitorFolders.push_back(g_pIImpl->GetAssetsPath());
		m_monitorFolders.push_back(PathUtil::GetLocalizationFolder().c_str());

		if (g_pIImpl->SupportsProjects())
		{
			m_monitorFolders.push_back(g_pIImpl->GetProjectPath());
		}

		for (auto const& folder : m_monitorFolders)
		{
			GetIEditor()->GetFileMonitor()->RegisterListener(this, folder);
		}
	}
}
} //endns ACE
