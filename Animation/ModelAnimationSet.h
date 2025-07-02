// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Containers/VectorMap.h>
#include <drx3D/CoreX/String/NameCRCHelper.h>
#include <drx3D/CoreX/Containers/DrxListenerSet.h>

enum
{
	CAF_File,
	AIM_File,
	LMG_File,
};

//////////////////////////////////////////////////////////////////////////
// Custom hash map class.
//////////////////////////////////////////////////////////////////////////
struct CHashMap_AmimNameCRC
{
	typedef std::map<u32, size_t> NameHashMap;

	//----------------------------------------------------------------------------------
	// Returns the index of the animation from crc value
	//----------------------------------------------------------------------------------
	size_t GetValueCRC(u32 crc) const
	{
		NameHashMap::const_iterator it = m_HashMap.find(crc);
		if (it == m_HashMap.end())
			return -1;
		return it->second;
	}

	// Returns the index of the animation from name. Name converted in lower case in this function
	size_t GetValue(tukk name) const
	{
		u32 crc32 = CCrc32::ComputeLowercase(name);
		return GetValueCRC(crc32);
	}

	bool InsertValue(u32 crc32, size_t num)
	{
		bool res = m_HashMap.find(crc32) == m_HashMap.end();
		m_HashMap[crc32] = num;
		return res;
	}

	size_t GetAllocMemSize() const
	{
		return m_HashMap.size() * (sizeof(u32) + sizeof(size_t));
	}

	size_t GetMapSize() const
	{
		return m_HashMap.size();
	}

	void GetMemoryUsage(class IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_HashMap);
	}

	void Clear()
	{
		m_HashMap.clear();
	}
protected:
	NameHashMap m_HashMap;
};

//! this structure contains info about loaded animations
struct ModelAnimationHeader
{
#ifdef STORE_ANIMATION_NAMES
	string m_Name;    // the name of the animation (not the name of the file) - unique per-model
#endif
	u32 m_CRC32Name;     //hash value for searching animations
	i16  m_nGlobalAnimId;
	i16  m_nAssetType;

	size_t SizeOfAnimationHeader() const
	{
		size_t size = 0;//sizeof(ModelAnimationHeader);
#ifdef STORE_ANIMATION_NAMES
		size += m_Name.length();
#endif
		return size;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
#ifdef STORE_ANIMATION_NAMES
		pSizer->AddObject(m_Name);
#endif
	}

	ModelAnimationHeader() : m_nGlobalAnimId(-1), m_nAssetType(CAF_File){};
	~ModelAnimationHeader() {}

	// the name of the animation when STORE_ANIMATION_NAMES; otherwise a shortened filepath
	// (should only be used for debugging)
	tukk GetAnimName() const
	{
#ifdef STORE_ANIMATION_NAMES
		return m_Name.c_str();
#else
		tukk filePath = GetFilePath();
		if (filePath)
		{
			tukk lastSeparator = strrchr(filePath, '/');
			tukk fileName = lastSeparator ? lastSeparator + 1 : filePath;
			return fileName;
		}
		else
		{
			extern tukk strEmpty;
			return strEmpty;
		}
#endif
	};

	tukk GetFilePath() const;

	void        SetAnimName(tukk name)
	{
#ifdef STORE_ANIMATION_NAMES
		m_Name = name;
#endif
		m_CRC32Name = CCrc32::ComputeLowercase(name);
	}

};

//////////////////////////////////////////////////////////////////////////
// Implementation of IAnimationSet, holding the information about animations
// and bones for a single model. Animations also include the subclass of morph targets
//////////////////////////////////////////////////////////////////////////
class CAnimationSet final : public IAnimationSet
{
public:
	CAnimationSet(tukk pSkeletonFilePath);

	////////////////////////////////////////////////////////////////////////////
	// IAnimationSet implementation
	virtual ~CAnimationSet() override;

	virtual void AddRef() override
	{
		++m_nRefCounter;
	}
	virtual void Release() override
	{
		--m_nRefCounter;
		if (m_nRefCounter == 0)
			delete this;
	}

	virtual u32 GetAnimationCount() const override
	{
		return m_arrAnimations.size();
	}

	virtual i32           GetAnimIDByName(tukk szAnimationName) const override;
	virtual tukk   GetNameByAnimID(i32 nAnimationId) const override;
	virtual i32           GetAnimIDByCRC(u32 animationCRC) const override;
	virtual u32        GetCRCByAnimID(i32 nAnimationId) const override;
	virtual u32        GetFilePathCRCByAnimID(i32 nAnimationId) const override;
	virtual tukk   GetFilePathByName(tukk szAnimationName) const override;
	virtual tukk   GetFilePathByID(i32 nAnimationId) const override;
	virtual f32           GetDuration_sec(i32 nAnimationId) const override;
	virtual u32        GetAnimationFlags(i32 nAnimationId) const override;
	virtual u32        GetAnimationSize(u32k nAnimationId) const override;
	virtual bool          IsAnimLoaded(i32 nAnimationId) const override;
	virtual bool          IsAimPose(i32 nAnimationId, const IDefaultSkeleton& defaultSkeleton) const override;
	virtual bool          IsLookPose(i32 nAnimationId, const IDefaultSkeleton& defaultSkeleton) const override;
	virtual void          AddRef(i32k nAnimationId) const override;
	virtual void          Release(i32k nAnimationId) const override;
	virtual bool          GetAnimationDCCWorldSpaceLocation(tukk szAnimationName, QuatT& startLocation) const override;
	virtual bool          GetAnimationDCCWorldSpaceLocation(i32 AnimID, QuatT& startLocation) const override;
	virtual bool          GetAnimationDCCWorldSpaceLocation(const CAnimation* pAnim, QuatT& startLocation, u32 nControllerID) const override;
	virtual ESampleResult SampleAnimation(i32 animationId, float animationNormalizedTime, u32 controllerId, QuatT& relativeLocationOutput) const override;

#ifdef EDITOR_PCDEBUGCODE
	virtual void        GetSubAnimations(DynArray<i32>& animIdsOut, i32 animId) const override;
	virtual i32         GetNumFacialAnimations() const override;
	virtual tukk GetFacialAnimationPathByName(tukk szName) const override;
	virtual tukk GetFacialAnimationName(i32 index) const override;
	virtual i32       GetGlobalIDByName(tukk szAnimationName) const override;
	virtual i32       GetGlobalIDByAnimID(i32 nAnimationId) const override;
	virtual tukk GetAnimationStatus(i32 nAnimationId) const override;
	virtual u32      GetTotalPosKeys(u32k nAnimationId) const override;
	virtual u32      GetTotalRotKeys(u32k nAnimationId) const override;
	virtual tukk GetDBAFilePath(u32k nAnimationId) const override;
	virtual i32         AddAnimationByPath(tukk animationName, tukk animationPath, const IDefaultSkeleton* pIDefaultSkeleton) override;
	virtual void        RebuildAimHeader(tukk szAnimationName, const IDefaultSkeleton* pIDefaultSkeleton) override;
	virtual void        RegisterListener(IAnimationSetListener* pListener) override;
	virtual void        UnregisterListener(IAnimationSetListener* pListener) override;
	virtual bool        GetMotionParameters(u32 nAnimationId, i32 parameterIndex, IDefaultSkeleton* pDefaultSkeleton, Vec4& outParameters) const override;
	virtual bool        GetMotionParameterRange(u32 nAnimationId, EMotionParamID paramId, float& outMin, float& outMax) const override;
	virtual bool        IsBlendSpace(i32 animId) const override;
	virtual bool        IsCombinedBlendSpace(i32 animId) const override;
#endif
	////////////////////////////////////////////////////////////////////////////

	// gets called when the given animation (by global id) is unloaded.
	// the animation controls need to be unbound to free up the memory
	void ReleaseAnimationData();
	// prepares to load the specified number of CAFs by reserving the space for the controller pointers
	void prepareLoadCAFs(unsigned nReserveAnimations);
	void prepareLoadANMs(unsigned nReserveAnimations);

	// when the animinfo is given, it's used to set the values of the global animation as if the animation has already been loaded once -
	// so that the next time the anim info is available and there's no need to load the animation synchronously
	// Returns the global anim id of the file, or -1 if error
	i32  LoadFileCAF(tukk szFileName, tukk szAnimName);
	i32  LoadFileAIM(tukk szFileName, tukk szAnimName, const IDefaultSkeleton* pIDefaultSkeleton);
	i32  LoadFileANM(tukk szFileName, tukk szAnimName, DynArray<CControllerTCB>& m_LoadCurrAnimation, DrxCGALoader* pCGA, const IDefaultSkeleton* pIDefaultSkeleton);
	i32  LoadFileLMG(tukk szFileName, tukk szAnimName);
	// Reuse an animation that is already loaded in the global animation set for this model
	void PrepareToReuseAnimations(size_t amount);
	void ReuseAnimation(const ModelAnimationHeader& header);

	// tries to load the animation info if isn't present in memory; returns NULL if can't load
	const ModelAnimationHeader* GetModelAnimationHeader(i32 i) const;

	i32                       FindAimposeByGlobalID(u32 nGlobalIDAimPose)
	{
		u32 numAnims = GetAnimationCount();
		for (u32 nMID = 0; nMID < numAnims; nMID++)
		{
			if (m_arrAnimations[nMID].m_nAssetType != AIM_File)
				continue;
			if (m_arrAnimations[nMID].m_nGlobalAnimId == nGlobalIDAimPose)
				return 1;
		}
		return -1;
	}

	size_t SizeOfAnimationSet() const;
	void   GetMemoryUsage(IDrxSizer* pSizer) const;

	struct FacialAnimationEntry
	{
		FacialAnimationEntry(const string& name, const string& path) : name(name), path(path) {}
		string name;
		string path;
		u32 crc;
		operator tukk () const { return name.c_str(); }

		void GetMemoryUsage(IDrxSizer* pSizer) const
		{
			pSizer->AddObject(name);
			pSizer->AddObject(path);
		}
	};
	typedef VectorSet<FacialAnimationEntry, stl::less_stricmp<FacialAnimationEntry>> FacialAnimationSet;
	FacialAnimationSet& GetFacialAnimations() { return m_facialAnimations; }
#ifndef EDITOR_PCDEBUGCODE
	i32                 GetNumFacialAnimations() const;
	tukk         GetFacialAnimationPathByName(tukk szName) const;
	tukk         GetFacialAnimationName(i32 index) const;
#endif
	tukk         GetFacialAnimationPath(i32 index) const;

	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------

	bool IsVEGLoaded(i32 nGlobalVEGId) const;

#ifndef EDITOR_PCDEBUGCODE
	tukk GetAnimationStatus(i32 nAnimationId) const;
#endif

	// for internal use only

	GlobalAnimationHeaderCAF* GetGAH_CAF(i32 nAnimationId) const;
	GlobalAnimationHeaderCAF* GetGAH_CAF(tukk AnimationName) const;
	GlobalAnimationHeaderAIM* GetGAH_AIM(i32 nAnimationId) const;
	GlobalAnimationHeaderAIM* GetGAH_AIM(tukk AnimationName) const;
	GlobalAnimationHeaderLMG* GetGAH_LMG(i32 nAnimationId) const;
	GlobalAnimationHeaderLMG* GetGAH_LMG(tukk AnimationName) const;

#ifndef EDITOR_PCDEBUGCODE
	i32        GetGlobalIDByAnimID(i32 nAnimationId) const;
	i32        GetGlobalIDByName(tukk szAnimationName) const;
#endif
	ILINE u32 GetGlobalIDByAnimID_Fast(i32 nAnimationId) const
	{
		assert(nAnimationId >= 0);
		assert(nAnimationId < m_arrAnimations.size());
		return m_arrAnimations[nAnimationId].m_nGlobalAnimId;
	};

	// Not safe method. Just direct access to m_arrAnimations
	const ModelAnimationHeader& GetModelAnimationHeaderRef(i32 i)
	{
		return m_arrAnimations[i];
	}

	tukk GetSkeletonFilePathDebug() const { return m_strSkeletonFilePath.c_str(); }

	void        VerifyLMGs();

#ifdef EDITOR_PCDEBUGCODE
	void NotifyListenersAboutReloadStarted();
	void NotifyListenersAboutReload();
#endif
private:

#ifdef STORE_ANIMATION_NAMES
	void VerifyAliases_Debug();
#endif

	void StoreAnimName(u32 nameCRC, tukk name)
	{
		if (Console::GetInst().ca_StoreAnimNamesOnLoad)
			m_hashToNameMap[nameCRC] = name;
	}

private:
	void InsertHash(u32 crc, size_t id)
	{
		DynArray<u32>::iterator it = std::lower_bound(m_AnimationHashMapKey.begin(), m_AnimationHashMapKey.end(), crc);
		ptrdiff_t index = it - m_AnimationHashMapKey.begin();
		m_AnimationHashMapKey.insert(it, crc);

		DynArray<u16>::iterator itValue = m_AnimationHashMapValue.begin();
		m_AnimationHashMapValue.insert(itValue + index, static_cast<u16>(id));
	}

	size_t GetValueCRC(u32 crc) const
	{
		DynArray<u32>::const_iterator itBegin = m_AnimationHashMapKey.begin();
		DynArray<u32>::const_iterator itEnd = m_AnimationHashMapKey.end();
		DynArray<u32>::const_iterator it = std::lower_bound(itBegin, itEnd, crc);
		if (it != itEnd && *it == crc)
		{
			ptrdiff_t index = it - itBegin;
			return m_AnimationHashMapValue[static_cast<i32>(index)];
		}
		return -1;
	}

	// Returns the index of the animation from name. Name converted in lower case in this function
	size_t GetValue(tukk name) const
	{
		u32 crc32 = CCrc32::ComputeLowercase(name);
		return GetValueCRC(crc32);
	}

private:
	// No more direct access to vector. No more LocalIDs!!!! This is just vector of registered animations!!!
	// When animation started we need build remap controllers\bones
	DynArray<ModelAnimationHeader> m_arrAnimations;
	FacialAnimationSet             m_facialAnimations;

	DynArray<u32>               m_AnimationHashMapKey;   //< Array of crc32 animation names (i.e. ModelAnimationHeader::m_CRC32Name), sorted by hash values.
	DynArray<u16>               m_AnimationHashMapValue; //< Mapping from indices of m_AnimationHashMapKey to indices of m_arrAnimations.

	typedef std::map<u32, string> HashToNameMap;
	HashToNameMap                        m_hashToNameMap; // Simple optional array of names that maps to m_arrAnimations. Filled if ca_StoreAnimNamesOnLoad == 1.
	i32                                  m_nRefCounter;
	//Just for debugging...Just for debugging...Just for debugging
	const string                         m_strSkeletonFilePath; //This was the original skeleton the animation set was created for. But the set might also work on an extended version of that skeleton
	CListenerSet<IAnimationSetListener*> m_listeners;
};
