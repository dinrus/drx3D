// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __PLAYERPROFILEIMPLFS_H__
#define __PLAYERPROFILEIMPLFS_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include "PlayerProfileUpr.h"

struct ICommonProfileImpl
{
	virtual ~ICommonProfileImpl(){}
	virtual void                   InternalMakeFSPath(CPlayerProfileUpr::SUserEntry* pEntry, tukk profileName, string& outPath) = 0;
	virtual void                   InternalMakeFSSaveGamePath(CPlayerProfileUpr::SUserEntry* pEntry, tukk profileName, string& outPath, bool bNeedFolder) = 0;
	virtual CPlayerProfileUpr* GetUpr() = 0;
};

#if 0
template<class T>
class CCommonSaveGameHelper
{
public:
	CCommonSaveGameHelper(T* pImpl) : m_pImpl(pImpl) {}
	bool       GetSaveGames(CPlayerProfileUpr::SUserEntry* pEntry, CPlayerProfileUpr::TSaveGameInfoVec& outVec, tukk altProfileName);
	ISaveGame* CreateSaveGame(CPlayerProfileUpr::SUserEntry* pEntry);
	ILoadGame* CreateLoadGame(CPlayerProfileUpr::SUserEntry* pEntry);

protected:
	bool FetchMetaData(XmlNodeRef& root, CPlayerProfileUpr::SSaveGameMetaData& metaData);
	T* m_pImpl;
};
#endif

class CPlayerProfileImplFS : public CPlayerProfileUpr::IPlatformImpl, public ICommonProfileImpl
{
public:
	CPlayerProfileImplFS();

	// CPlayerProfileUpr::IPlatformImpl
	virtual bool       Initialize(CPlayerProfileUpr* pMgr);
	virtual void       Release() { delete this; }
	virtual bool       LoginUser(SUserEntry* pEntry);
	virtual bool       LogoutUser(SUserEntry* pEntry);
	virtual bool       SaveProfile(SUserEntry* pEntry, CPlayerProfile* pProfile, tukk name, bool initialSave = false, i32 /*reason*/ = ePR_All);
	virtual bool       LoadProfile(SUserEntry* pEntry, CPlayerProfile* pProfile, tukk name);
	virtual bool       DeleteProfile(SUserEntry* pEntry, tukk name);
	virtual bool       RenameProfile(SUserEntry* pEntry, tukk newName);
	virtual bool       GetSaveGames(SUserEntry* pEntry, CPlayerProfileUpr::TSaveGameInfoVec& outVec, tukk altProfileName);
	virtual ISaveGame* CreateSaveGame(SUserEntry* pEntry);
	virtual ILoadGame* CreateLoadGame(SUserEntry* pEntry);
	virtual bool       DeleteSaveGame(SUserEntry* pEntry, tukk name);
	virtual bool       GetSaveGameThumbnail(SUserEntry* pEntry, tukk saveGameName, SThumbnail& thumbnail);
	// ~CPlayerProfileUpr::IPlatformImpl

	// ICommonProfileImpl
	void                           InternalMakeFSPath(SUserEntry* pEntry, tukk profileName, string& outPath);
	void                           InternalMakeFSSaveGamePath(SUserEntry* pEntry, tukk profileName, string& outPath, bool bNeedFolder);
	virtual CPlayerProfileUpr* GetUpr() { return m_pMgr; }
	// ~ICommonProfileImpl

protected:
	virtual ~CPlayerProfileImplFS();

private:
	CPlayerProfileUpr* m_pMgr;
};

class CPlayerProfileImplFSDir : public CPlayerProfileUpr::IPlatformImpl, public ICommonProfileImpl
{
public:
	CPlayerProfileImplFSDir();

	// CPlayerProfileUpr::IPlatformImpl
	virtual bool                Initialize(CPlayerProfileUpr* pMgr);
	virtual void                Release() { delete this; }
	virtual bool                LoginUser(SUserEntry* pEntry);
	virtual bool                LogoutUser(SUserEntry* pEntry);
	virtual bool                SaveProfile(SUserEntry* pEntry, CPlayerProfile* pProfile, tukk name, bool initialSave = false, i32 /*reason*/ = ePR_All);
	virtual bool                LoadProfile(SUserEntry* pEntry, CPlayerProfile* pProfile, tukk name);
	virtual bool                DeleteProfile(SUserEntry* pEntry, tukk name);
	virtual bool                RenameProfile(SUserEntry* pEntry, tukk newName);
	virtual bool                GetSaveGames(SUserEntry* pEntry, CPlayerProfileUpr::TSaveGameInfoVec& outVec, tukk altProfileName);
	virtual ISaveGame*          CreateSaveGame(SUserEntry* pEntry);
	virtual ILoadGame*          CreateLoadGame(SUserEntry* pEntry);
	virtual bool                DeleteSaveGame(SUserEntry* pEntry, tukk name);
	virtual ILevelRotationFile* GetLevelRotationFile(SUserEntry* pEntry, tukk name);
	virtual bool                GetSaveGameThumbnail(SUserEntry* pEntry, tukk saveGameName, SThumbnail& thumbnail);
	virtual void                GetMemoryStatistics(IDrxSizer* s);
	// ~CPlayerProfileUpr::IPlatformImpl

	// ICommonProfileImpl
	virtual void                   InternalMakeFSPath(SUserEntry* pEntry, tukk profileName, string& outPath);
	virtual void                   InternalMakeFSSaveGamePath(SUserEntry* pEntry, tukk profileName, string& outPath, bool bNeedFolder);
	virtual CPlayerProfileUpr* GetUpr() { return m_pMgr; }
	// ~ICommonProfileImpl

protected:
	virtual ~CPlayerProfileImplFSDir();

private:
	CPlayerProfileUpr* m_pMgr;
};

class CCommonSaveGameHelper
{
public:
	CCommonSaveGameHelper(ICommonProfileImpl* pImpl) : m_pImpl(pImpl) {}
	bool                GetSaveGames(CPlayerProfileUpr::SUserEntry* pEntry, CPlayerProfileUpr::TSaveGameInfoVec& outVec, tukk altProfileName);
	ISaveGame*          CreateSaveGame(CPlayerProfileUpr::SUserEntry* pEntry);
	ILoadGame*          CreateLoadGame(CPlayerProfileUpr::SUserEntry* pEntry);
	bool                DeleteSaveGame(CPlayerProfileUpr::SUserEntry* pEntry, tukk name);
	bool                GetSaveGameThumbnail(CPlayerProfileUpr::SUserEntry* pEntry, tukk saveGameName, CPlayerProfileUpr::SThumbnail& thumbnail);
	bool                MoveSaveGames(CPlayerProfileUpr::SUserEntry* pEntry, tukk oldProfileName, tukk newProfileName);
	ILevelRotationFile* GetLevelRotationFile(CPlayerProfileUpr::SUserEntry* pEntry, tukk name);

protected:
	bool FetchMetaData(XmlNodeRef& root, CPlayerProfileUpr::SSaveGameMetaData& metaData);
	ICommonProfileImpl* m_pImpl;
};

namespace PlayerProfileImpl
{
extern tukk PROFILE_ROOT_TAG;
extern tukk PROFILE_NAME_TAG;
extern tukk PROFILE_LAST_PLAYED;

// a simple serializer. manages sections (XmlNodes)
// is used during load and save
// CSerializerXML no longer supports sections being within the root node. In future it could be modified to but for now because of BinaryXML loading on consoles the ProfileImplFSDir is preferred.
class CSerializerXML : public CPlayerProfileUpr::IProfileXMLSerializer
{
public:
	CSerializerXML(XmlNodeRef& root, bool bLoading);

	// CPlayerProfileUpr::ISerializer
	inline bool IsLoading() { return m_bLoading; }

	XmlNodeRef  CreateNewSection(CPlayerProfileUpr::EPlayerProfileSection section, tukk name);
	void        SetSection(CPlayerProfileUpr::EPlayerProfileSection section, XmlNodeRef& node);
	XmlNodeRef  GetSection(CPlayerProfileUpr::EPlayerProfileSection section);

protected:
	bool       m_bLoading;
	XmlNodeRef m_root;
	XmlNodeRef m_sections[CPlayerProfileUpr::ePPS_Num];
};

// some helpers
bool       SaveXMLFile(const string& filename, const XmlNodeRef& rootNode);
XmlNodeRef LoadXMLFile(const string& filename);
bool       IsValidFilename(tukk filename);

} //endns PlayerProfileImpl

#endif
