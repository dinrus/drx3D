// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>
#include <drx3D/Sys/File/IFileChangeMonitor.h>
#include <drx3D/CoreX/ToolsHelpers/ResourceCompilerHelper.h>

namespace AssetManagerHelpers
{

bool IsFileOpened(const string& path);

struct EDITOR_COMMON_API RCLogger : IResourceCompilerListener
{
	virtual void OnRCMessage(MessageSeverity severity, tukk szText) override;
};

// A FIFO data processing queue with the ability to wait for deferred processing of items without blocking the calling thread.
class CProcessingQueue
{
public:
	typedef std::function<bool(const string& /*key*/)> Predicate;

public:
	CProcessingQueue() = default;

	// Inserts a new item into the processing queue.
	// The predicate object must return true if the item should be removed from the queue,
	// if the function object returns false, the queue will try to check the item again, somewhat later.
	// In any case the items will be processed in order of adding to the queue (first-in, first-out).
	void ProcessItemAsync(const string& key, Predicate fn);

	// Inserts a new item into the processing queue, if the queue doesn't already contain an item with an equivalent key.
	// see ProcessFileAsync
	void ProcessItemUniqueAsync(const string& key, Predicate fn);

private:
	void ProcessQueue();

private:
	std::deque<std::pair<string, Predicate>> m_queue;
};

//! The base implementation of the IFileChangeListener with the ability to wait until the file locking operation ends, without blocking the calling thread.
//! Implements a FIFO file processing queue.
class EDITOR_COMMON_API CAsyncFileListener : public IFileChangeListener
{
public:

	//! Return true if the file should be processed. The predicate should not try to open file.
	virtual bool AcceptFile(tukk szFilename, EChangeType eType) = 0;

	//! Return true if the file processing is complete.
	//! If the function returns false, the listener will try to call the processing of the file again, somewhat later.
	//! In any case the files will be processed in FIFO order (first-in, first-out).
	//! The implementation should expect that the method can be called asynchronously in the main thread.
	virtual bool ProcessFile(tukk szFilename, EChangeType eType) = 0;

private:
	// Inherited via IFileChangeListener
	virtual void OnFileChange(tukk szFilename, EChangeType eType) override;

private:
	CProcessingQueue m_fileQueue;
};

};
