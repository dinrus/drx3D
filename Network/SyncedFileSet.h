// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SYNCEDFILESET_H__
#define __SYNCEDFILESET_H__

#pragma once

#include <drx3D/Network/Config.h>

#if SERVER_FILE_SYNC_MODE

	#include <drx3D/Network/SyncedFile.h>

class CNetChannel;

class CSyncedFileSet
{
public:
	static u32k MAX_SEND_CHUNK_SIZE = 32;

	CSyncedFileSet();
	~CSyncedFileSet();

	void           AddFile(tukk name);
	CSyncedFilePtr GetSyncedFile(tukk name);
	void           SendFilesTo(CNetChannel* pSender);

	// update file implementation
	bool BeginUpdateFile(tukk name, u32 id);
	bool AddFileData(u32 id, u8k* pData, u32 length);
	bool EndUpdateFile(u32 id);

	// do we need to update a file?
	bool NeedToUpdateFile(tukk name, u8k* hash);

private:
	u8 m_syncsPerformed;

	typedef std::map<string, CSyncedFilePtr> TFiles;
	TFiles m_files;

	typedef std::map<u32, CSyncedFilePtr> TUpdatingFiles;
	TUpdatingFiles m_updating;
};

#endif // SERVER_FILE_SYNC_MODE

#endif
