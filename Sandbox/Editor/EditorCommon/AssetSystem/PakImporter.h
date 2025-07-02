// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sys/File/IFileChangeMonitor.h>
#include <functional>
#include <deque>

namespace AssetManagerHelpers
{

//! Monitors creation of new pak files, creates cryassets for asset data files located in paks.
class CPakImporter : public IFileChangeListener
{
public:
	static void RegisterFileListener();
	virtual void OnFileChange(tukk szFilename, EChangeType changeType) override;

private:
	void ProcessQueue();
	bool TryToProcessPak(const string& pak);
	bool IsPakOpened(const string& pakPath);
	static void ImportAssets(const string& pak);
	static string GetTemporaryDirectoryPath();
	static void Unpak(tukk szArchivePath, tukk szDestPath, std::function<void(float)> progress);
	static void GenerateDrxassets(tukk szPath, std::function<void(float)> progress);
	CPakImporter();

private:
	std::deque<string> m_queue;
};

};
