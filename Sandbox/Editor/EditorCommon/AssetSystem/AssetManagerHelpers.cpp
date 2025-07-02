// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "AssetManagerHelpers.h"
#include "Loader/AssetLoaderBackgroundTask.h"
#include "Loader/AssetLoaderHelpers.h"
#include "AssetSystem/AssetManager.h"
#include "Notifications/NotificationCenter.h"
#include "FilePathUtil.h"
#include "ThreadingUtils.h"
#include "QtUtil.h"

#include <drx3D/Sys/IProjectManager.h>
#include <drx3D/Sys/File/IDrxPak.h>
#include <drx3D/CoreX/ToolsHelpers/ResourceCompilerHelper.inl>
#include <drx3D/CoreX/ToolsHelpers/SettingsManagerHelpers.inl>
#include <drx3D/CoreX/ToolsHelpers/EngineSettingsManager.inl>
#include <drx3D/CoreX/String/DrxPath.h>

#include <deque>

namespace AssetManagerHelpers
{

bool IsFileOpened(const string& path)
{
	if (!gEnv->pDrxPak->IsFileExist(path, IDrxPak::eFileLocation_OnDisk))
	{
		return false;
	}

	FILE* pFile = GetISystem()->GetIPak()->FOpen(path, "r", IDrxPak::FOPEN_ONDISK | IDrxPak::FOPEN_LOCKED_OPEN);
	if (!pFile)
	{
		return true;
	}
	GetISystem()->GetIPak()->FClose(pFile);
	return false;
}

void RCLogger::OnRCMessage(MessageSeverity severity, tukk szText)
{
	if (severity == MessageSeverity_Error)
	{
		DrxWarning(EValidatorModule::VALIDATOR_MODULE_EDITOR, EValidatorSeverity::VALIDATOR_ERROR, "RC: %s", szText);
	}
	else if (severity == MessageSeverity_Warning)
	{
		DrxWarning(EValidatorModule::VALIDATOR_MODULE_EDITOR, EValidatorSeverity::VALIDATOR_WARNING, "RC: %s", szText);
	}
}

void CProcessingQueue::ProcessItemAsync(const string& key, Predicate fn)
{
	if (m_queue.empty())
	{
		m_queue.emplace_back(key, std::move(fn));
		ProcessQueue();
	}
	else // When the queue is not empty, a previous call to ProcessQueue() has already triggered the processing.
	{
		m_queue.emplace_back(key, std::move(fn));
	}
}

void CProcessingQueue::ProcessItemUniqueAsync(const string& key, Predicate fn)
{
	const bool bIsQueuedUp = std::any_of(m_queue.begin(), m_queue.end(), [&key](const auto& x)
	{
		return key.CompareNoCase(x.first) == 0;
	});

	if (bIsQueuedUp)
	{
		return;
	}

	ProcessItemAsync(key, std::move(fn));
}

void CProcessingQueue::ProcessQueue()
{
	if (m_queue.empty())
	{
		return;
	}

	auto path = m_queue.front();
		
	// Call the function object.
	const bool isDone = path.second(path.first);
	if (isDone)
	{
		m_queue.pop_front();
	}

	if (!m_queue.empty())
	{
		// Try in 0.5 seconds if the item has not been processed.
		i32k timeout = isDone ? 0 : 500;
		QTimer::singleShot(timeout, [this]()
		{
			ProcessQueue();
		});
	}
}

void CAsyncFileListener::OnFileChange(tukk szFilename, EChangeType eType)
{
	// Ignore events for files outside of the current game folder.
	if (GetISystem()->GetIPak()->IsAbsPath(szFilename))
	{
		return;
	}

	if (AcceptFile(szFilename, eType))
	{
		const string assetPath(szFilename);
		m_fileQueue.ProcessItemUniqueAsync(assetPath, [this, eType](const string& assetPath)
		{
			// It can be that the file is still being opened for writing.
			if (AssetManagerHelpers::IsFileOpened(assetPath))
			{
				// Try again
				return false;
			}

			return ProcessFile(assetPath.c_str(), eType);
		});
	}
}

};
