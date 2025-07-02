// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "FileMonitorSystem.h"

#include "AudioControlsEditorPlugin.h"

namespace ACE
{
//////////////////////////////////////////////////////////////////////////
CFileMonitorSystem::CFileMonitorSystem(i32 const delay, QObject* const pParent)
	: CFileMonitorBase(delay, pParent)
	, m_delayTimer(new QTimer(this))
{
	m_delayTimer->setSingleShot(true);
	connect(m_delayTimer, &QTimer::timeout, this, &CFileMonitorSystem::Enable);
	Enable();
}

//////////////////////////////////////////////////////////////////////////
void CFileMonitorSystem::Enable()
{
	m_monitorFolder = g_assetsManager.GetConfigFolderPath();
	GetIEditor()->GetFileMonitor()->RegisterListener(this, m_monitorFolder, "xml");
}

//////////////////////////////////////////////////////////////////////////
void CFileMonitorSystem::EnableDelayed()
{
	m_delayTimer->start(m_delay);
}
} //endns ACE
