// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "AssetManagerHelpers.h"
#include <drx3D/Sys/File/IFileChangeMonitor.h>
#include <drx3D/CoreX/Renderer/IRenderer.h>
#include <drx3D/CoreX/ToolsHelpers/ResourceCompilerHelper.h>

#include <mutex>

class CProgressNotification;

namespace AssetManagerHelpers
{

// ! Creates and updates cryassets for asset data files.
class CAssetGenerator : public IFileChangeListener, public IAsyncTextureCompileListener
{
public:
	static void RegisterFileListener();

	virtual void OnFileChange(tukk szFilename, EChangeType changeType) override;

	virtual void OnCompilationStarted(tukk szSource, tukk szTarget, i32 nPending) override;
	virtual void OnCompilationFinished(tukk szSource, tukk szTarget, ERcExitCode eReturnCode) override;
	virtual void OnCompilationQueueTriggered(i32 nPending) override;
	virtual void OnCompilationQueueDepleted() override;

	//! Generates/repair *.cryasset files for the current project.
	static bool GenerateDrxassets();

private:
	CAssetGenerator();
	void GenerateDrxasset(const string& filePath);

private:
	CProcessingQueue m_fileQueue;
	std::unique_ptr<CProgressNotification> m_pProgress;
	std::unique_ptr<CProgressNotification> m_pTextureCompilerProgress;
	string m_rcSettings;
	std::mutex m_textureCompilerMutex;
};

};
