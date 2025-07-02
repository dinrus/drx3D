// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "AssetManagerHelpers.h"
#include <drx3D/Sys/File/IFileChangeMonitor.h>

namespace AssetManagerHelpers
{

//! Monitors changes of asset meta-data (.cryasset) files on disk and merges them.
class CAssetFileMonitor : public IFileChangeListener
{
public:
	static void RegisterFileListener();

	// IFileChangeListener implementation.
	virtual void OnFileChange(tukk szFilename, EChangeType changeType) override;

private:
	static void RemoveAsset(const string& assetPath);
	static void LoadAsset(const string& assetPath);
	CAssetFileMonitor();
private:
	CProcessingQueue m_fileQueue;
};

};
