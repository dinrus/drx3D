// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <stdio.h>     // strlen()
#include <functional>
#include <drx3D/CoreX/StlUtils.h>
#include <string>
#include <deque>

#include <drx3D/CoreX/ToolsHelpers/ResourceCompilerHelper.h>

//////////////////////////////////////////////////////////////////////////
// Provides settings and functions to make calls to RC to compile textures.
class CTextureCompiler : public CResourceCompilerHelper, NoCopy
{

private:
	static DrxMutex          g_RCmutex;
	static CTextureCompiler* g_pRCInstance;

	static void __cdecl DestroyInstance();

	CTextureCompiler();
	~CTextureCompiler();

public:
	// factory-pattern
	static CTextureCompiler& GetInstance();

	// run compiler only on developer platform
#if defined(DRX_ENABLE_RC_HELPER)
public:
	enum class EResult
	{
		//! Texture was already compiled, no need to run RC
		AlreadyCompiled,
		//! Texture was queued for compilation, will not be available immediately
		Queued,
		//! Texture was compiled immediately, and can be loaded right away.
		Available,
		//! Texture compilation failed
		Failed
	};

	// checks file date and existence
	// Return:
	//   fills processedFilename[] with name of the file that should be loaded
	//   boolean for success
	EResult ProcessTextureIfNeeded(
	  tukk originalFilename,
	  tuk processedFilename,
	  size_t processedFilenameSizeInBytes,
	  bool immediate = true);

private:
	class CQueuedRC
	{
	public:
		CQueuedRC()
			: src()
			, dst()
			, windowed(false)
			, refresh(false)
			, returnval(eRcExitCode_Success)
			, process(INVALID_HANDLE_VALUE)
		{
		}

		string      src;
		string      dst;
		bool        windowed;
		bool        refresh;

		ERcExitCode returnval;
		HANDLE      process;
	};

	// notify-list, members will be notified about changes to the task-queue
	typedef std::set<IAsyncTextureCompileListener*> TProcListeners;

	// task-queue, can be used to query progress/todo, will deplete automagically
	typedef std::deque<CQueuedRC> TProcQueue;
	typedef CQueuedRC             TProcItem;

	// watch-list, can be used to reject duplicate requests, will deplete automagically
	#if 1
	typedef std::map<string, string>                                                               TWatchSet;
	#else
	typedef std::unordered_map<string, string, stl::hash_strcmp<string>, stl::hash_strcmp<string>> TWatchSet;
	#endif
	typedef TWatchSet::iterator                                                                    TWatchItem;

	// there are multiple instances of the helper, use global variables (1x per render-DLL)
	TWatchSet      m_mWatchList;
	TProcQueue     m_qProcessingList;
	TProcListeners m_sNotifyList;

	DrxRWLock      m_rwLockWatch;
	DrxRWLock      m_rwLockProcessing;
	DrxRWLock      m_rwLockNotify;

	uk          m_jobqueue;

public:
	void ConsumeQueuedResourceCompiler(TProcItem* item);
	void ForkOffResourceCompiler(tukk szSrcFile, tukk szDstFile, const bool bWindow, const bool bRefresh);

	// Arguments:
	//   szSrcFile usually the path to a TIFF
	//   szDstFile usually the path to a DDS
	bool          HasQueuedResourceCompiler(tukk szSrcFile, tukk szDstFile);
	ERcCallResult QueueResourceCompiler(tukk szSrcFile, tukk szDstFile, const bool bWindow, const bool bRefresh);

#endif // DRX_ENABLE_RC_HELPER

	void AddAsyncTextureCompileListener(IAsyncTextureCompileListener* pListener);
	void RemoveAsyncTextureCompileListener(IAsyncTextureCompileListener* pListener);

private:
#if defined(DRX_ENABLE_RC_HELPER)
	bool AddToWatchList(tukk szDstFile, tukk szSrcFile);
	void NotifyCompilationQueueTriggered(i32 pending);
	void NotifyCompilationStarted(TProcItem* item, i32 pending);
	void NotifyCompilationFinished(TProcItem* item);
	void NotifyCompilationQueueDepleted();
	void GetNextItem(TProcItem* &item, i32 &pending);

	// Arguments:
	//   szFilePath - could be source or destination filename
	//   dwIndex - used to iterate through all input filenames, start with 0 and increment by 1
	// Return:
	//   fills inputFilename[] with a filename (or empty string if that was the last input format)
	static void GetInputFilename(
	  tukk filename,
	  u32k index,
	  tuk inputFilename,
	  size_t inputFilenameSizeInBytes);

	// little helper function (to stay independent)
	static string AddSuffix(string in, tukk suffix)
	{
		string out = in;
		tukk extension = GetExtension(out.c_str());
		if (!extension)
			return out + suffix;

		size_t position = out.size() - strlen(extension) - 1;
		return out.insert(position, suffix);
	}

	static bool IsFileOpened(tukk szPath);
#endif

public:
	// only for image formats supported by the resource compiler
	// Arguments:
	//   szExtension - e.g. ".tif", can be 0
	static bool IsImageFormatSupported(tukk szExtension)
	{
		if (szExtension && (strlen(szExtension) > 0))
		{
			if (stricmp(szExtension, "dds") == 0 ||    // DirectX surface format
			    stricmp(szExtension, "hdr") == 0 ||    // High Dynamic Range image format
			    stricmp(szExtension, "tif") == 0)      // Dinrus resource compiler image format
			{
				return (CRenderer::CV_r_texturecompiling > 0);
			}
		}

		return false;
	}
};

