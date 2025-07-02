// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include <drx3D/Sys/File/IFileChangeMonitor.h>
#include "Util/FileChangeMonitor.h"

//DO NOT USE, DEPRECATED !!
//This is currently only used by manneqin and hardcoded behavior in cpp file...
class CEditorFileMonitor
	: public IFileChangeMonitor
	  , public CFileChangeMonitorListener
	  , public IEditorNotifyListener
{
public:
	CEditorFileMonitor();
	~CEditorFileMonitor();

	//DO NOT USE, DEPRECATED !!
	bool RegisterListener(IFileChangeListener* pListener, tukk filename) override;
	bool RegisterListener(IFileChangeListener* pListener, tukk folderRelativeToGame, tukk ext) override;
	bool UnregisterListener(IFileChangeListener* pListener) override;

	// from CFileChangeMonitorListener
	void OnFileMonitorChange(const SFileChangeInfo& rChange) override;

	// from IEditorNotifyListener
	void OnEditorNotifyEvent(EEditorNotifyEvent ev) override;
private:

	void MonitorDirectories();
	// File Change Monitor stuff
	struct SFileChangeCallback
	{
		IFileChangeListener* pListener;
		string              item;
		string              extension;

		SFileChangeCallback()
			: pListener(NULL)
		{}

		SFileChangeCallback(IFileChangeListener* pListener, tukk item, tukk extension)
			: pListener(pListener)
			, item(item)
			, extension(extension)
		{}
	};

	std::vector<SFileChangeCallback> m_vecFileChangeCallbacks;
};

