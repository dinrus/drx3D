// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "IEditorClassFactory.h"

// Source control status of item.
enum ESccFileAttributes
{
	SCC_FILE_ATTRIBUTE_INVALID         = 0x0000, // File is not found.
	SCC_FILE_ATTRIBUTE_NORMAL          = 0x0001, // Normal file on disk.
	SCC_FILE_ATTRIBUTE_READONLY        = 0x0002, // Read only files cannot be modified at all, either read only file not under source control or file in packfile.
	SCC_FILE_ATTRIBUTE_INPAK           = 0x0004, // File is inside pack file.
	SCC_FILE_ATTRIBUTE_MANAGED         = 0x0008, // File is managed under source control.
	SCC_FILE_ATTRIBUTE_CHECKEDOUT      = 0x0010, // File is under source control and is checked out.
	SCC_FILE_ATTRIBUTE_BYANOTHER       = 0x0020, // File is under source control and is checked out by another user.
	SCC_FILE_ATTRIBUTE_FOLDER          = 0x0040, // Managed folder.
	SCC_FILE_ATTRIBUTE_LOCKEDBYANOTHER = 0x0080, // File is under source control and is checked out and locked by another user.
};

// Source control flags
enum ESccFlags
{
	GETLATEST_REVERT      = 1 << 0, // don't revert when perform GetLatestVersion() on opened files
	GETLATEST_ONLY_CHECK  = 1 << 1, // don't actually get latest version of the file, check if scc has more recent version
	ADD_WITHOUT_SUBMIT    = 1 << 2, // add the files to the default changelist
	ADD_CHANGELIST        = 1 << 3, // add a changelist with the description
	ADD_AS_BINARY_FILE    = 1 << 4, // add the files as binary files
	DELETE_WITHOUT_SUBMIT = 1 << 5, // mark for delete and don't submit
};

// Lock status of an item in source control
enum ESccLockStatus
{
	SCC_LOCK_STATUS_UNLOCKED,
	SCC_LOCK_STATUS_LOCKED_BY_OTHERS,
	SCC_LOCK_STATUS_LOCKED_BY_US,
};

//////////////////////////////////////////////////////////////////////////
// Description
//    This interface provide access to the source control functionality.
//////////////////////////////////////////////////////////////////////////
struct ISourceControl : public IClassDesc, public _i_reference_target_t
{
	// Description:
	//    Returns attributes of the file.
	// Return:
	//    Combination of flags from ESccFileAttributes enumeration.
	virtual u32 GetFileAttributes(tukk filename) = 0;

	virtual bool   DoesChangeListExist(tukk pDesc, tuk changeid, i32 nLen) = 0;
	virtual bool   CreateChangeList(tukk pDesc, tuk changeid, i32 nLen) = 0;
	virtual bool   Add(tukk filename, tukk desc = 0, i32 nFlags = 0, tuk changelistId = NULL) = 0;
	virtual bool   CheckIn(tukk filename, tukk desc = 0, i32 nFlags = 0) = 0;
	virtual bool   CheckOut(tukk filename, i32 nFlags = 0, tuk changelistId = NULL) = 0;
	virtual bool   UndoCheckOut(tukk filename, i32 nFlags = 0) = 0;
	virtual bool   Rename(tukk filename, tukk newfilename, tukk desc = 0, i32 nFlags = 0) = 0;
	virtual bool   Delete(tukk filename, tukk desc = 0, i32 nFlags = 0, tuk changelistId = NULL) = 0;
	virtual bool   GetLatestVersion(tukk filename, i32 nFlags = 0) = 0;

	// GetInternalPath - Get internal Source Control path for file filename
	// outPath: output char buffer, provided by user, MAX_PATH - recommended size of this buffer
	// nOutPathSize: size of outPath buffer, MAX_PATH is recommended
	// if a smaller size will be used, it may returned a truncated path.
	virtual bool GetInternalPath(tukk filename, tuk outPath, i32 nOutPathSize) = 0;

	// GetOtherUser - Get other user name who edit file filename
	// outUser: output char buffer, provided by user, 64 - recommended size of this buffer
	// nOutUserSize: size of outUser buffer, 64 is recommended
	virtual bool GetOtherUser(tukk filename, tuk outUser, i32 nOutUserSize) = 0;

	// Show file history
	virtual bool History(tukk filename) = 0;

	// Show settings dialog
	virtual void ShowSettings() = 0;

	// GetOtherLockOwner = Get the user name who has the specifed file locked.
	// outUser: output char buffer, provided by user, 64 - recommended size of this buffer
	// If the file is locked by me or nobody, it returns false.
	virtual bool           GetOtherLockOwner(tukk filename, tuk outUser, i32 nOutUserSize)                       { return false; }
	virtual ESccLockStatus GetLockStatus(tukk filename)                                                            { return SCC_LOCK_STATUS_UNLOCKED; }
	virtual bool           Lock(tukk filename, i32 nFlags = 0)                                                     { return false; }
	virtual bool           Unlock(tukk filename, i32 nFlags = 0)                                                   { return false; }
	virtual bool           Integrate(tukk filename, tukk newfilename, tukk desc = 0, i32 nFlags = 0) { return false; }
	virtual bool           GetFileRev(tukk sFilename, int64* pHaveRev, int64* pHeadRev)                            { return false; }
	virtual bool           GetUserName(tuk outUser, i32 nOutUserSize)                                                   { return false; }
	virtual bool           GetRevision(tukk filename, int64 nRev, i32 nFlags = 0)                                  { return false; }
	virtual bool           SubmitChangeList(tuk changeid)                                                               { return false; }
	virtual bool           DeleteChangeList(tuk changeid)                                                               { return false; }
	virtual bool           Reopen(tukk filename, tuk changeid = NULL)                                            { return false; }
};

