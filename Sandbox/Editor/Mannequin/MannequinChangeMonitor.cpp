// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "MannequinChangeMonitor.h"
#include "helper/MannequinFileChangeWriter.h"

CMannequinChangeMonitor::CMannequinChangeMonitor()
	:
	m_pFileChangeWriter(new CMannequinFileChangeWriter(true))
{
	GetIEditorImpl()->GetFileMonitor()->RegisterListener(this, "animations/mannequin", "adb");
}

CMannequinChangeMonitor::~CMannequinChangeMonitor()
{
	SAFE_DELETE(m_pFileChangeWriter);
	GetIEditorImpl()->GetFileMonitor()->UnregisterListener(this);
}

void CMannequinChangeMonitor::OnFileChange(tukk sFilename, EChangeType eType)
{
	DrxLog("CMannequinChangeMonitor - %s has changed", sFilename);

	if (CMannequinFileChangeWriter::UpdateActiveWriter() == false)
	{
		m_pFileChangeWriter->ShowFileManager();
	}
}

