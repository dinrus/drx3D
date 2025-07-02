// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//
////////////////////////////////////////////////////////////////////////////
#ifndef __ANIMATION_DATABASE_MANAGER_H__
#define __ANIMATION_DATABASE_MANAGER_H__

#include "AnimationDatabase.h"
#include "IDrxMannequinEditor.h"

#include <drx3D/CoreX/Containers/DrxListenerSet.h>
#include <drx3D/Sys/IResourceUpr.h>

class CAnimationDatabaseLibrary
{
public:
	typedef const CAnimationDatabase* THandle;

	CAnimationDatabaseLibrary();
	~CAnimationDatabaseLibrary();

	void Insert(u32 nCrc, CAnimationDatabase* pDb);

public:
	THandle LoadResource(tukk name, u32 nFlags);
	THandle TryOpenResource(tukk name, u32 nFlags);
	void    CreateResource(THandle& hdlOut, tukk name, u32 nFlags);
	void    PublishResource(THandle& hdlOut);

protected:
	typedef std::map<u32, CAnimationDatabase*> TAnimDatabaseList;

protected:
	bool LoadDatabase(const XmlNodeRef& xmlNode, CAnimationDatabase& animDB, bool recursive);
	bool LoadDatabaseDefinitions(const XmlNodeRef& root, tukk filename, const CTagDefinition** ppFragDefs, const CTagDefinition** ppTagDefs);

	void LoadDatabaseData
	(
	  CAnimationDatabase& animDB,
	  const XmlNodeRef root,
	  const CTagDefinition& fragIDs,
	  const CTagDefinition& tagIDs,
	  const TagState& adbFilter,
	  bool recursive,
	  CAnimationDatabase::SSubADB* subAnimDB = NULL
	);

	void AnimEntryToXML(XmlNodeRef outNode, const SAnimationEntry& animEntry, tukk name) const;
	bool XMLToAnimEntry(SAnimationEntry& animEntry, XmlNodeRef root) const;

	bool XMLToFragment(CFragment& fragment, XmlNodeRef root, bool transition) const;
	void FragmentToXML(XmlNodeRef outNode, const CFragment* fragment, bool transition) const;

	void ProceduralToXml(XmlNodeRef pOutNode, const SProceduralEntry& proceduralEntry) const;
	bool XmlToProcedural(XmlNodeRef pInNode, SProceduralEntry& outProceduralEntry) const;

	void XMLToBlend(SAnimBlend& animBlend, XmlNodeRef xmlBlend) const;
	void BlendToXML(XmlNodeRef outNode, const SAnimBlend& animBlend, tukk name) const;

private:
	CAnimationDatabaseLibrary(const CAnimationDatabaseLibrary&);
	CAnimationDatabaseLibrary& operator=(const CAnimationDatabaseLibrary&);

protected:
	TAnimDatabaseList m_databases;

	CTagDefinition    m_animFlags;
	CTagDefinition    m_transitionFlags;
};

class CAnimationTagDefLibrary
{
public:
	typedef const CTagDefinition* THandle;

	CAnimationTagDefLibrary();
	~CAnimationTagDefLibrary();

	void Insert(u32 nCrc, CTagDefinition* pDef);

public:
	THandle LoadResource(tukk name, u32 nFlags);
	THandle TryOpenResource(tukk name, u32 nFlags);
	void    CreateResource(THandle& hdlOut, tukk name, u32 nFlags);
	void    PublishResource(THandle& hdlOut);

protected:
	typedef std::map<u32, CTagDefinition*> TTagDefList;

private:
	CAnimationTagDefLibrary(const CAnimationTagDefLibrary&);
	CAnimationTagDefLibrary& operator=(const CAnimationTagDefLibrary&);

protected:
	TTagDefList m_tagDefs;
};

class CAnimationControllerDefLibrary
{
public:
	typedef const SControllerDef* THandle;

	CAnimationControllerDefLibrary();
	~CAnimationControllerDefLibrary();

	void Insert(u32 nCrc32, SControllerDef* pDef);

public:
	THandle LoadResource(tukk name, u32 nFlags);
	THandle TryOpenResource(tukk name, u32 nFlags);
	void    CreateResource(THandle& hdlOut, tukk name, u32 nFlags);
	void    PublishResource(THandle& hdlOut);

protected:
	typedef std::map<u32, SControllerDef*> TControllerDefList;

protected:
	SControllerDef* LoadControllerDef(const XmlNodeRef& xmlNode, tukk context);

private:
	CAnimationControllerDefLibrary(const CAnimationControllerDefLibrary&);
	CAnimationControllerDefLibrary& operator=(const CAnimationControllerDefLibrary&);

protected:
	TControllerDefList m_controllerDefs;
	CTagDefinition     m_fragmentFlags;
};

class CAnimationDatabaseUpr
	: public IAnimationDatabaseUpr
	  , public IMannequinEditorUpr
	  , protected CAnimationDatabaseLibrary
	  , protected CAnimationTagDefLibrary
	  , protected CAnimationControllerDefLibrary
{
public:
	CAnimationDatabaseUpr();
	~CAnimationDatabaseUpr();

	virtual i32 GetTotalDatabases() const
	{
		return m_databases.size();
	}
	virtual const IAnimationDatabase* GetDatabase(i32 idx) const
	{
		for (TAnimDatabaseList::const_iterator iter = m_databases.begin(); iter != m_databases.end(); ++iter)
		{
			if (idx-- == 0)
			{
				return iter->second;
			}
		}
		return NULL;
	}

	virtual const IAnimationDatabase* Load(tukk filename);
	virtual const SControllerDef*     LoadControllerDef(tukk filename);
	virtual const CTagDefinition*     LoadTagDefs(tukk filename, bool isTags);
	virtual void                      SaveAll(IMannequinWriter* pWriter) const;

	virtual const IAnimationDatabase* FindDatabase(u32k crcFilename) const; 

	virtual const SControllerDef*     FindControllerDef(u32k crcFilename) const;
	virtual const SControllerDef*     FindControllerDef(tukk filename) const;

	virtual const CTagDefinition*     FindTagDef(u32k crcFilename) const;
	virtual const CTagDefinition*     FindTagDef(tukk filename) const;

	virtual IAnimationDatabase*       Create(tukk filename, tukk defFilename);
	virtual CTagDefinition*           CreateTagDefinition(tukk filename);

	virtual void                      ReloadAll();
	virtual void                      UnloadAll();

	// IMannequinEditorUpr
	void                    GetAffectedFragmentsString(const CTagDefinition* pQueryTagDef, TagID tagID, tuk buffer, i32 bufferSize);
	void                    ApplyTagDefChanges(const CTagDefinition* pOriginal, CTagDefinition* pModified);
	void                    RenameTag(const CTagDefinition* pOriginal, i32 tagCRC, tukk newName);
	void                    RenameTagGroup(const CTagDefinition* pOriginal, i32 tagGroupCRC, tukk newName);
	void                    GetIncludedTagDefs(const CTagDefinition* pQueriedTagDef, DynArray<CTagDefinition*>& tagDefs) const;

	EModifyFragmentIdResult CreateFragmentID(const CTagDefinition& fragmentIds, tukk szFragmentIdName);
	EModifyFragmentIdResult RenameFragmentID(const CTagDefinition& fragmentIds, FragmentID fragmentID, tukk szFragmentIdName);
	EModifyFragmentIdResult DeleteFragmentID(const CTagDefinition& fragmentIds, FragmentID fragmentID);

	bool                    SetFragmentTagDef(const CTagDefinition& fragmentIds, FragmentID fragmentID, const CTagDefinition* pFragTagDefs);

	void                    SetFragmentDef(const SControllerDef& controllerDef, FragmentID fragmentID, const SFragmentDef& fragmentDef);

	void                    SaveDatabasesSnapshot(SSnapshotCollection& snapshotCollection) const;
	void                    LoadDatabasesSnapshot(const SSnapshotCollection& snapshotCollection);

	void                    GetLoadedTagDefs(DynArray<const CTagDefinition*>& tagDefs);
	void                    GetLoadedDatabases(DynArray<const IAnimationDatabase*>& animDatabases) const;
	void                    GetLoadedControllerDefs(DynArray<const SControllerDef*>& controllerDefs) const;

	void                    RevertDatabase(tukk szFilename);
	void                    RevertControllerDef(tukk szFilename);
	void                    RevertTagDef(tukk szFilename);

	bool                    DeleteFragmentEntry(IAnimationDatabase* pDatabase, FragmentID fragmentID, const SFragTagState& tagState, u32 optionIdx, bool logWarning);
	u32                  AddFragmentEntry(IAnimationDatabase* pDatabase, FragmentID fragmentID, const SFragTagState& tagState, const CFragment& fragment);
	void                    SetFragmentEntry(IAnimationDatabase* pDatabase, FragmentID fragmentID, const SFragTagState& tagState, u32 optionIdx, const CFragment& fragment);

	bool                    IsFileUsedByControllerDef(const SControllerDef& controllerDef, tukk szFilename) const;

	void                    RegisterListener(IMannequinEditorListener* pListener);
	void                    UnregisterListener(IMannequinEditorListener* pListener);

	void                    AddSubADBFragmentFilter(IAnimationDatabase* pDatabase, tukk szSubADBFilename, FragmentID fragmentID);
	void                    RemoveSubADBFragmentFilter(IAnimationDatabase* pDatabase, tukk szSubADBFilename, FragmentID fragmentID);
	u32                  GetSubADBFragmentFilterCount(const IAnimationDatabase* pDatabase, tukk szSubADBFilename) const;
	FragmentID              GetSubADBFragmentFilter(const IAnimationDatabase* pDatabase, tukk szSubADBFilename, u32 index) const;

	void                    SetSubADBTagFilter(IAnimationDatabase* pDatabase, tukk szSubADBFilename, TagState tagState);
	TagState                GetSubADBTagFilter(const IAnimationDatabase* pDatabase, tukk szSubADBFilename) const;

	void                    SetBlend(IAnimationDatabase* pDatabase, FragmentID fragmentIDFrom, FragmentID fragmentIDTo, const SFragTagState& tagFrom, const SFragTagState& tagTo, SFragmentBlendUid blendUid, const SFragmentBlend& fragBlend);
	SFragmentBlendUid       AddBlend(IAnimationDatabase* pDatabase, FragmentID fragmentIDFrom, FragmentID fragmentIDTo, const SFragTagState& tagFrom, const SFragTagState& tagTo, const SFragmentBlend& fragBlend);
	void                    DeleteBlend(IAnimationDatabase* pDatabase, FragmentID fragmentIDFrom, FragmentID fragmentIDTo, const SFragTagState& tagFrom, const SFragTagState& tagTo, SFragmentBlendUid blendUid);
	void                    GetFragmentBlends(const IAnimationDatabase* pDatabase, SEditorFragmentBlendID::TEditorFragmentBlendIDArray& outBlendIDs) const;
	void                    GetFragmentBlendVariants(const IAnimationDatabase* pDatabase, const FragmentID fragmentIDFrom, const FragmentID fragmentIDTo, SEditorFragmentBlendVariant::TEditorFragmentBlendVariantArray& outVariants) const;
	void                    GetFragmentBlend(const IAnimationDatabase* pIDatabase, const FragmentID fragmentIDFrom, const FragmentID fragmentIDTo, const SFragTagState& tagFrom, const SFragTagState& tagTo, const SFragmentBlendUid& blendUid, SFragmentBlend& outFragmentBlend) const;
	// ~IMannequinEditorUpr

private:
	CAnimationDatabaseUpr(const CAnimationDatabaseUpr&);
	CAnimationDatabaseUpr& operator=(const CAnimationDatabaseUpr&);

	//--- used for saving
	struct SSaveState
	{
		string m_sFileName;
		bool   m_bSavedByTags;  // Otherwise saved by ID... tags is higher priority;

		SSaveState() : m_sFileName(), m_bSavedByTags(false) {}
	};
	typedef std::vector<SSaveState> TSaveStateList;

	struct SFragmentSaveEntry
	{
		TSaveStateList vSaveStates;
	};
	typedef std::vector<SFragmentSaveEntry> TFragmentSaveList;

	struct SFragmentBlendSaveEntry
	{
		TSaveStateList vSaveStates;
	};
	typedef std::map<CAnimationDatabase::SFragmentBlendID, SFragmentBlendSaveEntry> TFragmentBlendSaveDatabase;

	void       Save(const IAnimationDatabase& iAnimDB, tukk databaseName) const;

	XmlNodeRef SaveDatabase
	(
	  const CAnimationDatabase& animDB,
	  const CAnimationDatabase::SSubADB* subAnimDB,
	  const TFragmentSaveList& vFragSaveList,
	  const TFragmentBlendSaveDatabase& mBlendSaveDatabase,
	  bool flatten = false
	) const;

	void                             ReloadDatabase(IAnimationDatabase* pDatabase);
	void                             RemoveDataFromParent(CAnimationDatabase* parentADB, const CAnimationDatabase::SSubADB* subADB);
	void                             ClearDatabase(CAnimationDatabase* pDatabase);
	void                             Compress(CAnimationDatabase& animDB);

	XmlNodeRef                       SaveControllerDef(const SControllerDef& controllerDef) const;
	void                             ReloadControllerDef(SControllerDef* pControllerDef);
	void                             ReloadTagDefinition(CTagDefinition* pTagDefinition);

	void                             FindRootSubADB(const CAnimationDatabase& animDB, const CAnimationDatabase::SSubADB* pSubADB, FragmentID fragID, TagState tagState, string& outRootADB) const;
	std::vector<CAnimationDatabase*> FindImpactedDatabases(const CAnimationDatabase* pWorkingDatabase, FragmentID fragId, TagState globalTags) const;
	bool                             HasAncestor(const CAnimationDatabase& database, const string& ancestorName) const;
	bool                             HasAncestor(const CAnimationDatabase::SSubADB& database, const string& ancestorName) const;

	void                             PrepareSave(const CAnimationDatabase& animDB, const CAnimationDatabase::SSubADB* subAnimDB, TFragmentSaveList& vFragSaveList, TFragmentBlendSaveDatabase& mBlendSaveDatabase) const;

	bool                             FragmentMatches(u32 fragCRC, const TagState& tagState, const CAnimationDatabase::SSubADB& subAnimDB, bool& bTagMatchFound) const;
	bool                             TransitionMatches(u32 fragCRCFrom, u32 fragCRCTo, const TagState& tagStateFrom, const TagState& tagStateTo, const CTagDefinition& parentTagDefs, const CAnimationDatabase::SSubADB& subAnimDB, bool& bTagMatchFound) const;
	bool                             CanSaveFragmentID(FragmentID fragID, const CAnimationDatabase& animDB, const CAnimationDatabase::SSubADB* subAnimDB) const;
	bool                             ShouldSaveFragment(FragmentID fragID, const TagState& tagState, const CAnimationDatabase& animDB, const CAnimationDatabase::SSubADB* subAnimDB, bool& bTagMatchFound) const;
	bool                             ShouldSaveTransition
	(
	  FragmentID fragIDFrom,
	  FragmentID fragIDTo,
	  const TagState& tagStateFrom,
	  const TagState& tagStateTo,
	  const CAnimationDatabase& animDB,
	  const CAnimationDatabase::SSubADB* subAnimDB,
	  bool& bTagMatchFound
	) const;

	void RevertSubADB(tukk szFilename, CAnimationDatabase* animDB, const CAnimationDatabase::SSubADB& subADB);

	void SaveSubADB
	(
	  IMannequinWriter* pWriter,
	  const CAnimationDatabase& animationDatabase,
	  const CAnimationDatabase::SSubADB& subADB,
	  const TFragmentSaveList& vFragSaveList,
	  const TFragmentBlendSaveDatabase& mBlendSaveDatabase
	) const;

	void SaveSubADB(const CAnimationDatabase& animDB, const CAnimationDatabase::SSubADB& subAnimDB, const TFragmentSaveList& vFragSaveList, const TFragmentBlendSaveDatabase& mBlendSaveDatabase) const;

	void SaveDatabase(IMannequinWriter* pWriter, const IAnimationDatabase* pAnimationDatabase) const;
	void SaveControllerDef(IMannequinWriter* pWriter, const SControllerDef* pControllerDef) const;
	void SaveTagDefinition(IMannequinWriter* pWriter, const CTagDefinition* pTagDefinition) const;

	void NotifyListenersTagDefinitionChanged(const CTagDefinition& tagDef);

	void ReconcileSubDatabases(const CAnimationDatabase* pSourceDatabase);
	void ReconcileSubDatabase(const CAnimationDatabase* pSourceDatabase, CAnimationDatabase* pTargetSubDatabase);

private:

	static CAnimationDatabaseUpr* s_Instance;

	typedef CListenerSet<IMannequinEditorListener*> TEditorListenerSet;
	TEditorListenerSet m_editorListenerSet;
};

#endif //__ANIMATION_DATABASE_MANAGER_H__
