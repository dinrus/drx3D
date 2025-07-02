// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/String/StringUtils.h>
#include <drx3D/Animation/ModelAnimationSet.h> //embedded

class CDefaultSkeleton;

struct CAF_ID
{
	i32 m_nCafID;
	i32 m_nType;  //regular CAF-file or AIM-Pose (which is also a CAF file)
	CAF_ID(i32 id, i32 type)
	{
		m_nCafID = id;
		m_nType = type;
	};
};

// information about an animation to load
struct SAnimFile
{
	char   m_FilePathQ[256];
	char   m_AnimNameQ[256];
	u32 m_crcAnim;

	SAnimFile(const stack_string& szFilePath, const stack_string& szAnimName)
	{
		stack_string tmp = szFilePath;
		PathUtil::UnifyFilePath(tmp);
		tmp.MakeLower();

		memset(m_FilePathQ, 0, sizeof(m_FilePathQ));
		u32 len1 = tmp.length();
		for (u32 i = 0; i < len1; i++)
			m_FilePathQ[i] = tmp[i];

		memset(m_AnimNameQ, 0, sizeof(m_AnimNameQ));
		u32 len2 = szAnimName.length();
		for (u32 i = 0; i < len2; i++)
			m_AnimNameQ[i] = szAnimName[i];

		m_crcAnim = CCrc32::ComputeLowercase(szAnimName);

	}
};

class DrxCHRLoader;

// holds information about a loaded animation list
// (the <AnimationList> section of a .chrparams file)
// this looks more scary than it is (memory wise)
// for a total level its about 150 kb in memory
struct SAnimListInfo
{
	string              fileName;
	DynArray<i32>       dependencies;

	string              faceLibFile;
	string              faceLibDir;
	string              animEventDatabase;
	DynArray<string>    modelTracksDatabases;
	DynArray<string>    lockedDatabases;
	DynArray<SAnimFile> arrAnimFiles;
	DynArray<SAnimFile> arrWildcardAnimFiles;
	CAnimationSet::FacialAnimationSet::container_type facialAnimations;

	// has the animation list been parsed and the animations been loaded?
	// then headers are available and just have to be loaded into an
	// AnimationSet
	bool headersLoaded;

	// this is the only thing that really takes up memory after loading
	DynArray<ModelAnimationHeader> arrLoadedAnimFiles;

	SAnimListInfo(tukk paramFileName)
	{
		fileName.append(paramFileName);
		// reserve a large block to prevent fragmentation
		// it gets deleted once the headers are loaded anyway
		arrAnimFiles.reserve(512);
		arrWildcardAnimFiles.reserve(512);
		arrLoadedAnimFiles.reserve(0);
		headersLoaded = false;
	}
	void HeadersLoaded()
	{
		arrAnimFiles.clear();
		arrLoadedAnimFiles.reserve(arrLoadedAnimFiles.size());
		headersLoaded = true;
	}
	void PrepareHeaderList()
	{
		arrWildcardAnimFiles.clear();
		arrLoadedAnimFiles.reserve(arrAnimFiles.size());
	}
};

class CParamLoader
{
public:
	CParamLoader();
	~CParamLoader(void);

	void                 ClearLists();

	bool                 LoadXML(CDefaultSkeleton* pDefaultSkeleton, string defaultAnimDir, tukk const paramFileName, DynArray<u32>& listIDs);
	bool                 ExpandWildcards(u32 listID);

	const SAnimListInfo& GetParsedList(u32k i)                                                    { return m_parsedLists[i]; };
	u32               GetParsedListNumber()                                                            { return m_parsedLists.size(); };

	void                 PrepareHeaderList(u32 listID)                                                 { m_parsedLists[listID].PrepareHeaderList(); };
	void                 SetHeadersLoaded(u32 listID)                                                  { m_parsedLists[listID].HeadersLoaded(); };
	void                 AddLoadedHeader(u32 listID, const ModelAnimationHeader& modelAnimationHeader) { m_parsedLists[listID].arrLoadedAnimFiles.push_back(modelAnimationHeader); }

#ifdef EDITOR_PCDEBUGCODE
	bool HasInjectedCHRPARAMS(tukk filename) const;
	void InjectCHRPARAMS(tukk filename, tukk content, size_t contentLength);
	void ClearCHRPARAMSCache();
#endif

private:
	void   ExpandWildcardsForPath(SAnimListInfo& animList, tukk szMask, u32 crcFolder, tukk szAnimNamePre, tukk szAnimNamePost);

	i32    LoadAnimList(const XmlNodeRef calNode, tukk paramFileName, string strAnimDirName);
	bool   BuildDependencyList(i32 rootListID, DynArray<u32>& listIDs);
	CAF_ID MemFindFirst(tukk* ppAnimPath, tukk szMask, u32 crcFolder, CAF_ID nCafID);

	bool   ParseIKDef(const XmlNodeRef ikNode);
	bool   LoadIKDefLimbIK(const XmlNodeRef limbNode);
	bool   LoadIKDefAnimDrivenIKTargets(const XmlNodeRef adNode);
	bool   LoadIKDefFeetLock(const XmlNodeRef limbNode);
	bool   LoadIKDefRecoil(const XmlNodeRef aimNode);
	bool   LoadIKDefLookIK(const XmlNodeRef aimNode);
	bool   LoadIKDefAimIK(const XmlNodeRef aimNode);
	bool   LoadLod(const XmlNodeRef lodNode);
	bool   LoadBBoxInclusionList(const XmlNodeRef node);
	bool   LoadBBoxExtension(const XmlNodeRef node);
	bool   LoadShadowCapsulesList(const XmlNodeRef node);
	i32    ListProcessed(tukk paramFileName);

	// helper functions for interfacing SAnimListInfo
	bool             AddIfNewAnimationAlias(SAnimListInfo& animList, tukk animName, tukk szFileName);
	const SAnimFile* FindAnimationAliasInDependencies(SAnimListInfo& animList, tukk szFileName);

	bool             AddIfNewFacialAnimationAlias(SAnimListInfo& animList, tukk animName, tukk szFileName);
	bool             NoFacialAnimationAliasInDependencies(SAnimListInfo& animList, tukk animName);
	bool             AddIfNewModelTracksDatabase(SAnimListInfo& animList, tukk dataBase);
	bool             NoModelTracksDatabaseInDependencies(SAnimListInfo& animList, tukk dataBase);

#ifdef EDITOR_PCDEBUGCODE
	typedef std::map<string, DynArray<char>> CachedCHRPARAMS;
	CachedCHRPARAMS         m_cachedCHRPARAMS;
#endif
	DynArray<SAnimListInfo> m_parsedLists;
	CDefaultSkeleton*       m_pDefaultSkeleton;
	string                  m_defaultAnimDir;
};
