// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __MannequinChangeMonitor_h__
#define __MannequinChangeMonitor_h__

#include <drx3D/Sys/File/IFileChangeMonitor.h>

class CMannequinChangeMonitor : public IFileChangeListener
{
public:
	CMannequinChangeMonitor();
	~CMannequinChangeMonitor();

	virtual void OnFileChange(tukk sFilename, EChangeType eType) override;

	class CMannequinFileChangeWriter* m_pFileChangeWriter;

};

#endif //__MannequinChangeMonitor_h__

