// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __PerforceSourceControl_h__
#define __PerforceSourceControl_h__

#pragma once

#define USERNAME_LENGTH 64

#include "IEditorClassFactory.h"

#pragma warning(push)
#pragma warning(disable: 4244) // 'conversion' conversion from 'type1' to 'type2', possible loss of data
#include "p4/clientapi.h"
#include "p4/errornum.h"
#pragma warning(pop)

#include "ISourceControl.h"

#include <QToolButton>
#include <drx3D/CoreX/Sandbox/DrxSignal.h>

struct CPerforceThread;

class CMyClientUser : public ClientUser
{
public:
	CMyClientUser()
	{
		m_initDesc = "Automatic.";
		drx_strcpy(m_desc, m_initDesc);
		Init();
	}
	void HandleError(Error* e);
	void OutputStat(StrDict* varList);
	void Edit(FileSys* f1, Error* e);
	void OutputInfo(char level, tukk data);
	void Init();
	void PreCreateFileName(tukk file);
	void InputData(StrBuf* buf, Error* e);

	Error          m_e;
	char           m_action[64];
	char           m_headAction[64];
	char           m_otherAction[64];
	char           m_clientHasLatestRev[64];
	char           m_desc[1024];
	char           m_file[MAX_PATH];
	char           m_findedFile[MAX_PATH];
	char           m_depotFile[MAX_PATH];
	char           m_otherUser[USERNAME_LENGTH];
	char           m_lockedBy[USERNAME_LENGTH];
	string        m_clientRoot;
	string        m_clientName;
	string        m_clientHost;
	string        m_currentDirectory;
	bool           m_bIsClientUnknown;

	tukk    m_initDesc;
	bool           m_bIsSetup;
	bool           m_bIsPreCreateFile;
	bool           m_bIsCreatingChangelist;
	string        m_output;
	string        m_input;

	int64          m_nFileHeadRev;
	int64          m_nFileHaveRev;
	ESccLockStatus m_lockStatus;
};

class CMyClientApi : public ClientApi
{
public:
	void Run(tukk func);
	void Run(tukk func, ClientUser* ui);
};

class CPerforceSourceControl : public ISourceControl
{
public:

	// constructor
	CPerforceSourceControl();
	virtual ~CPerforceSourceControl();

	bool Connect();
	bool Reconnect();
	void FreeData();
	void Init();
	bool CheckConnection();

	// from ISourceControl
	u32 GetFileAttributes(tukk filename);

	// Thread processing
	void           GetFileAttributesThread(tukk filename);

	bool           DoesChangeListExist(tukk pDesc, tuk changeid, i32 nLen);
	bool           CreateChangeList(tukk pDesc, tuk changeid, i32 nLen);
	bool           Add(tukk filename, tukk desc, i32 nFlags, tuk changelistId = NULL);
	bool           CheckIn(tukk filename, tukk desc, i32 nFlags);
	bool           CheckOut(tukk filename, i32 nFlags, tuk changelistId = NULL);
	bool           UndoCheckOut(tukk filename, i32 nFlags);
	bool           Rename(tukk filename, tukk newfilename, tukk desc, i32 nFlags);
	bool           Delete(tukk filename, tukk desc, i32 nFlags, tuk changelistId = NULL);
	bool           GetLatestVersion(tukk filename, i32 nFlags);
	bool           GetInternalPath(tukk filename, tuk outPath, i32 nOutPathSize);
	bool           GetOtherUser(tukk filename, tuk outUser, i32 nOutUserSize);
	bool           History(tukk filename);

	virtual void   ShowSettings() override;

	bool           GetOtherLockOwner(tukk filename, tuk outUser, i32 nOutUserSize);
	ESccLockStatus GetLockStatus(tukk filename);
	bool           Lock(tukk filename, i32 nFlags = 0);
	bool           Unlock(tukk filename, i32 nFlags = 0);
	bool           GetUserName(tuk outUser, i32 nOutUserSize);
	bool           GetFileRev(tukk sFilename, int64* pHaveRev, int64* pHeadRev);
	bool           GetRevision(tukk filename, int64 nRev, i32 nFlags = 0);
	bool           SubmitChangeList(tuk changeid);
	bool           DeleteChangeList(tuk changeid);
	bool           Reopen(tukk filename, tuk changeid = NULL);

	bool           Run(tukk func, i32 nArgs, tuk argv[], bool bOnlyFatal = false);
	u32         GetFileAttributesAndFileName(tukk filename, tuk FullFileName);
	bool           IsFolder(tukk filename, tuk FullFileName);

	// from IClassDesc
	virtual ESystemClassID SystemClassID() { return ESYSTEM_CLASS_SCM_PROVIDER; };
	virtual tukk    ClassName()       { return "Perforce source control"; };
	virtual tukk    Category()        { return "SourceControl"; };
	virtual CRuntimeClass* GetRuntimeClass() { return 0; };

	CMyClientApi& GetClientApi() { return m_client; }
	bool IsWorkOffline() const { return m_bIsWorkOffline; }
	void SetWorkOffline(bool bWorkOffline);

protected:
	bool        IsFileManageable(tukk sFilename, bool bCheckFatal = true);
	bool        IsFileExistsInDatabase(tukk sFilename);
	bool        IsFileCheckedOutByUser(tukk sFilename, bool* pIsByAnotherUser = 0);
	bool        IsFileLatestVersion(tukk sFilename);
	void        ConvertFileNameCS(tuk sDst, tukk sSrcFilename);
	void        MakePathCS(tuk sDst, tukk sSrcFilename);
	void        RenameFolders(tukk path, tukk oldPath);
	bool        FindFile(tuk clientFile, tukk folder, tukk file);
	bool        FindDir(tuk clientFile, tukk folder, tukk dir);
	bool        IsSomeTimePassed();

	tukk GetErrorByGenericCode(i32 nGeneric);

public:
	CDrxSignal<void()> signalWorkOfflineChanged;

private:
	CMyClientUser    m_ui;
	CMyClientApi     m_client;
	Error            m_e;

	bool             m_bIsWorkOffline;
	bool             m_bIsFailConnectionLogged;

	CPerforceThread* m_thread;
	u32           m_unRetValue;
	bool             m_isSetuped;
	bool             m_isSecondThread;
	bool             m_isSkipThread;
	DWORD            m_dwLastAccessTime;

	ULONG            m_ref;
};

class CPerforceTrayWidget : public QToolButton
{
	Q_OBJECT
public:
	CPerforceTrayWidget(QWidget* pParent = nullptr);
	void OnClicked(bool bChecked);
	void OnConnectionStatusChanged();

private:
	class QPopupWidget* m_pPopUpMenu;
};

#endif //__PerforceSourceControl_h__

