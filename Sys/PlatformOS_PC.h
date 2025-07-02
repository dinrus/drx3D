// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   IPlatformOS_PC.h
//  Created:     18/12/2009 by Alex Weighell.
//  Описание: Interface to the Platform OS
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __PLATFORMOS_PC_H_
#define __PLATFORMOS_PC_H_

#if !DRX_PLATFORM_DURANGO

	#include <drx3D/CoreX/Platform/IPlatformOS.h>
	#include <drx3D/CoreX/Containers/DrxListenerSet.h>
	#include <drx3D/CoreX/Game/IGameFramework.h>

namespace ZipDir
{
class FileEntryTree;

class CacheRW;
TYPEDEF_AUTOPTR(CacheRW);
typedef CacheRW_AutoPtr CacheRWPtr;
}

class CPlatformOS_PC : public IPlatformOS, public IPlatformOS::IPlatformListener, public ISystemEventListener, public IGameFrameworkListener
{
public:

	CPlatformOS_PC(u8k createParams);
	~CPlatformOS_PC();

	// ~IPlatformOS

	// Called each frame to update the platform listener
	virtual void        Tick(float realFrameTime);

	virtual tukk GetPlatformName() const { return "PC"; };

	// Local user profile functions to check/initiate user sign in:
	// See IPlatformOS.h for documentation.
	virtual u32                                         UserGetMaximumSignedInUsers() const;
	virtual bool                                                 UserIsSignedIn(u32 userIndex) const;
	virtual bool                                                 UserIsSignedIn(const IPlatformOS::TUserName& userName, u32& outUserIndex) const { if (!m_bSignedIn) return false; outUserIndex = 0; return true; }
	virtual bool                                                 UserDoSignIn(u32 numUsersRequested, u32 controllerIndex);
	virtual void                                                 UserSignOut(u32 user)                                                           {}
	virtual i32                                                  GetFirstSignedInUser() const;
	virtual u32                                         UserGetPlayerIndex(tukk userName) const                                           { return 0; }
	virtual bool                                                 UserGetName(u32 userIndex, IPlatformOS::TUserName& outName) const;
	virtual bool                                                 UserGetOnlineName(u32 userIndex, IPlatformOS::TUserName& outName) const;
	virtual bool                                                 UserSelectStorageDevice(u32 userIndex, bool bForceUI = false) { return true; } // always hard drive
	virtual bool                                                 GetUserProfilePreference(u32 user, IPlatformOS::EUserProfilePreference ePreference, SUserProfileVariant& outResult) const;
	virtual bool                                                 MountSaveFile(u32 userIndex)                                  { return true; }
	virtual IFileFinderPtr                                       GetFileFinder(u32 user);
	virtual void                                                 MountDLCContent(IDLCListener* pCallback, u32 user, u8k keyData[16]);

	virtual bool                                                 CanRestartTitle() const;
	virtual void                                                 RestartTitle(tukk pTitle);

	virtual bool                                                 UsePlatformSavingAPI() const;
	virtual bool                                                 BeginSaveLoad(u32 user, bool bSave);
	virtual void                                                 EndSaveLoad(u32 user);
	virtual IPlatformOS::ISaveReaderPtr                          SaveGetReader(tukk fileName, u32 user);
	virtual IPlatformOS::ISaveWriterPtr                          SaveGetWriter(tukk fileName, u32 user);

	virtual bool                                                 KeyboardStart(u32 inUserIndex, u32 flags, tukk title, tukk initialInput, i32 maxInputLength, IVirtualKeyboardEvents* pInCallback);
	virtual bool                                                 KeyboardIsRunning();
	virtual bool                                                 KeyboardCancel();

	virtual bool                                                 StringVerifyStart(tukk inString, IStringVerifyEvents* pInCallback);
	virtual bool                                                 IsVerifyingString();

	virtual void                                                 AddListener(IPlatformOS::IPlatformListener* pListener, tukk szName);
	virtual void                                                 RemoveListener(IPlatformOS::IPlatformListener* pListener);
	virtual void                                                 NotifyListeners(SPlatformEvent& event);

	virtual ILocalizationUpr::EPlatformIndependentLanguageID GetSystemLanguage();
	virtual tukk                                          GetSKUId();
	virtual ILocalizationUpr::TLocalizationBitfield          GetSystemSupportedLanguages();

	virtual bool                                                 PostLocalizationBootChecks();

	virtual void                                                 GetMemoryUsage(IDrxSizer* pSizer) const;

	virtual bool                                                 DebugSave(SDebugDump& dump);

	virtual bool                                                 ConsoleLoadGame(IConsoleCmdArgs* pArgs);

	virtual tukk const                                    GetHostName();

	virtual void                                                 InitEncryptionKey(tukk pMagic, size_t magicLength, u8k* pKey, size_t keyLength);

	virtual void                                                 GetEncryptionKey(const std::vector<char>** pMagic = NULL, const std::vector<u8>** pKey = NULL);

	virtual EUserPIIStatus                                       GetUserPII(u32 inUser, SUserPII* pOutPII);

	virtual IPlatformOS::ECDP_Start                              StartUsingCachePaks(i32k user, bool* outWritesOccurred);
	virtual IPlatformOS::ECDP_End                                EndUsingCachePaks(i32k user);
	virtual IPlatformOS::ECDP_Open                               DoesCachePakExist(tukk const filename, const size_t size, u8 md5[16]);
	virtual IPlatformOS::ECDP_Open                               OpenCachePak(tukk const filename, tukk const bindRoot, const size_t size, u8 md5[16]);
	virtual IPlatformOS::ECDP_Close                              CloseCachePak(tukk const filename);
	virtual IPlatformOS::ECDP_Delete                             DeleteCachePak(tukk const filename);
	virtual IPlatformOS::ECDP_Write                              WriteCachePak(tukk const filename, ukk const pData, const size_t numBytes);

	virtual IPlatformOS::EZipExtractFail                         ExtractZips(tukk path);

	virtual void                                                 SetOpticalDriveIdle(bool bIdle);
	virtual void                                                 AllowOpticalDriveUsage(bool bAllow);

	virtual bool                                                 GetLocalIPAddress(tuk ipAddress, u32& ip, i32 length) const;

	// ~IPlatformOS

	// IPlatformOS::IPlatformListener
	virtual void OnPlatformEvent(const IPlatformOS::SPlatformEvent& _event);
	// ~IPlatformOS::IPlatformListener

	// ISystemEventListener
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam);
	// ~ISystemEventListener

	// IGameFrameworkListener
	virtual void OnPostUpdate(float fDeltaTime)    {}
	virtual void OnSaveGame(ISaveGame* pSaveGame)  {}
	virtual void OnLoadGame(ILoadGame* pLoadGame)  {}
	virtual void OnLevelEnd(tukk nextLevel) {}
	virtual void OnActionEvent(const SActionEvent& event);
	// ~IGameFrameworkListener

private:

	void                         SaveDirtyFiles();
	IPlatformOS::EZipExtractFail RecurseZipContents(ZipDir::FileEntryTree* pSourceDir, tukk currentPath, ZipDir::CacheRWPtr pCache);
	bool                         SxmlMissingFromHDD(ZipDir::FileEntryTree* pSourceDir, tukk currentPath, ZipDir::CacheRWPtr pCache);

	bool                         DecryptAndCheckSigning(tukk pInData, i32 inDataLen, tukk* pOutData, i32* pOutDataLen, u8k key[16]);
	
private:
	CStableFPSWatcher                             m_fpsWatcher;
	CListenerSet<IPlatformOS::IPlatformListener*> m_listeners;
	std::vector<char>                             m_encryptionMagic;
	std::vector<u8>                            m_encryptionKey;
	float m_delayLevelStartIcon;
	i32   m_cachePakStatus;
	i32   m_cachePakUser;
	bool  m_bSignedIn;
	bool  m_bSaving;
	bool  m_bLevelLoad;
	bool  m_bSaveDuringLevelLoad;
};

#endif
#endif
