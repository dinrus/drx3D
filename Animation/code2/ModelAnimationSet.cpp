// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/ModelAnimationSet.h>

#include <drx3D/Animation/CharacterUpr.h>
#include <drx3D/Animation/Model.h>

tukk strEmpty = "";

//////////////////////////////////////////////////////////////////////////
tukk ModelAnimationHeader::GetFilePath() const
{
	if (m_nAssetType == CAF_File)
		return g_AnimationUpr.m_arrGlobalCAF[m_nGlobalAnimId].GetFilePath();
	else if (m_nAssetType == AIM_File)
		return g_AnimationUpr.m_arrGlobalAIM[m_nGlobalAnimId].GetFilePath();
	else if (m_nAssetType == LMG_File)
		return g_AnimationUpr.m_arrGlobalLMG[m_nGlobalAnimId].GetFilePath();

	assert(0);
	return 0;
}

//////////////////////////////////////////////////////////////////////////
CAnimationSet::CAnimationSet(tukk pSkeletonFilePath) :
	m_strSkeletonFilePath(pSkeletonFilePath),
	m_listeners(1)
{
	m_nRefCounter = 0;
}

CAnimationSet::~CAnimationSet()
{
	ReleaseAnimationData();

	for (CListenerSet<IAnimationSetListener*>::Notifier notifier(m_listeners); notifier.IsValid(); notifier.Next())
		notifier->m_pIAnimationSet = 0;
}

void CAnimationSet::ReleaseAnimationData()
{
	// now there are no controllers referred to by this object, we can release the animations
	u32 numAnimations = m_arrAnimations.size();
	for (u32 nAnimId = 0; nAnimId < numAnimations; nAnimId++)
	{
		const ModelAnimationHeader* anim = GetModelAnimationHeader(nAnimId);
		if (anim)
		{
			if (anim->m_nAssetType == CAF_File)
			{
				GlobalAnimationHeaderCAF& rCAF = g_AnimationUpr.m_arrGlobalCAF[anim->m_nGlobalAnimId];
				g_AnimationUpr.AnimationReleaseCAF(rCAF);
			}
			if (anim->m_nAssetType == AIM_File)
			{
				GlobalAnimationHeaderAIM& rAIM = g_AnimationUpr.m_arrGlobalAIM[anim->m_nGlobalAnimId];
				g_AnimationUpr.AnimationReleaseAIM(rAIM);
			}
			if (anim->m_nAssetType == LMG_File)
			{
				GlobalAnimationHeaderLMG& rLMG = g_AnimationUpr.m_arrGlobalLMG[anim->m_nGlobalAnimId];
				g_AnimationUpr.AnimationReleaseLMG(rLMG);
			}

		}
	}
	m_arrAnimations.clear();
	stl::free_container(m_AnimationHashMapKey);
	stl::free_container(m_AnimationHashMapValue);
	m_hashToNameMap.clear();
}

#ifdef EDITOR_PCDEBUGCODE
void CAnimationSet::RegisterListener(IAnimationSetListener* pListener)
{
	if (pListener->m_pIAnimationSet == this)
		return;
	if (pListener->m_pIAnimationSet != 0)
		pListener->m_pIAnimationSet->UnregisterListener(pListener);

	m_listeners.Add(pListener);

	pListener->m_pIAnimationSet = this;
}

void CAnimationSet::UnregisterListener(IAnimationSetListener* pListener)
{
	m_listeners.Remove(pListener);
	pListener->m_pIAnimationSet = 0;
}
#endif

i32 CAnimationSet::GetNumFacialAnimations() const
{
	return m_facialAnimations.size();
}

tukk CAnimationSet::GetFacialAnimationPathByName(tukk szName) const
{
	FacialAnimationSet::const_iterator itFacialAnim = std::lower_bound(m_facialAnimations.begin(), m_facialAnimations.end(), szName, stl::less_stricmp<tukk >());
	if (itFacialAnim != m_facialAnimations.end() && stl::less_stricmp<tukk >()(szName, *itFacialAnim))
		itFacialAnim = m_facialAnimations.end();
	tukk szPath = (itFacialAnim != m_facialAnimations.end() ? (*itFacialAnim).path.c_str() : 0);
	return szPath;
}

tukk CAnimationSet::GetFacialAnimationName(i32 index) const
{
	if (index < 0 || index >= (i32)m_facialAnimations.size())
		return 0;
	return m_facialAnimations[index].name.c_str();
}

tukk CAnimationSet::GetFacialAnimationPath(i32 index) const
{
	if (index < 0 || index >= (i32)m_facialAnimations.size())
		return 0;
	return m_facialAnimations[index].path.c_str();
}

void CAnimationSet::PrepareToReuseAnimations(size_t amount)
{
	m_AnimationHashMapKey.reserve(m_AnimationHashMapKey.size() + amount);
	m_AnimationHashMapValue.reserve(m_AnimationHashMapValue.size() + amount);
}

void CAnimationSet::ReuseAnimation(const ModelAnimationHeader& header)
{
	m_arrAnimations.push_back(header);

	i32 nLocalAnimId2 = m_arrAnimations.size() - 1;
	InsertHash(header.m_CRC32Name, nLocalAnimId2);
}

//////////////////////////////////////////////////////////////////////////
// Loads animation file. Returns the global anim id of the file, or -1 if error
// SIDE EFFECT NOTES:
//  THis function does not put up a warning in the case the animation couldn't be loaded.
//  It returns an error (false) and the caller must process it.
i32 CAnimationSet::LoadFileCAF(tukk szFilePath, tukk szAnimName)
{
	//NEW RULE: every single CAF file is now an OnDemand streaming animation
	i32 nAnimId = GetAnimIDByName(szAnimName);
	if (nAnimId != -1)
	{
		i32 nGlobalAnimID = m_arrAnimations[nAnimId].m_nGlobalAnimId;
		g_pILog->LogWarning("DinrusXAnimation:: Trying to load animation with alias \"%s\" from file \"%s\" into the animation container. Such animation alias already exists and uses file \"%s\". Please use another animation alias.", szAnimName, szFilePath, g_AnimationUpr.m_arrGlobalCAF[nGlobalAnimID].GetFilePath());
		return nGlobalAnimID;
	}

	i32 nGlobalAnimID = g_AnimationUpr.CreateGAH_CAF(szFilePath); //create the global header. the data will be streamed in later.

	ModelAnimationHeader localAnim;
	localAnim.m_nGlobalAnimId = nGlobalAnimID;
	localAnim.m_nAssetType = CAF_File;
	localAnim.SetAnimName(szAnimName);
	m_arrAnimations.push_back(localAnim);

	StoreAnimName(localAnim.m_CRC32Name, szAnimName);

	i32 nLocalAnimId2 = m_arrAnimations.size() - 1;
	InsertHash(localAnim.m_CRC32Name, nLocalAnimId2);

	GlobalAnimationHeaderCAF& rCAF = g_AnimationUpr.m_arrGlobalCAF[nGlobalAnimID];
	rCAF.AddRef();

	u32 IsCreated = rCAF.IsAssetCreated();
	if (IsCreated == 0)
	{
		//asset is not created, so let's create by loading the CAF
		assert(rCAF.GetControllersCount() == 0);
		u8 status = rCAF.LoadCAF();
		if (status)
		{
			rCAF.ClearAssetRequested();
			assert(rCAF.IsAssetLoaded());
			if (Console::GetInst().ca_UseIMG_CAF)
				g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, rCAF.GetFilePath(), "Unnecessary Loading of CAF-files: Probably no valid IMG-file available");
		}
	}
	else
	{
		//this is only for production mode
		if (Console::GetInst().ca_UseIMG_CAF == 0)
		{
			if (rCAF.IsAssetOnDemand() == 0)
			{
				//asset is already in a DBA
				u8 exist = rCAF.DoesExistCAF();  //check if we also have a CAF-file
				if (exist)
				{
					//overload the DBA animation.
					u8 status = rCAF.LoadCAF();
					if (status)
					{
						rCAF.ClearAssetRequested();
						assert(rCAF.IsAssetLoaded());
						if (Console::GetInst().ca_UseIMG_CAF)
							g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, rCAF.GetFilePath(), "Unnecessary Loading of CAF-files: Probably no valid IMG-file availbale");
					}
				}
			}
		}
	}

	for (CListenerSet<IAnimationSetListener*>::Notifier notifier(m_listeners); notifier.IsValid(); notifier.Next())
		notifier->OnAnimationSetAddAnimation(szFilePath, szAnimName);

	return nGlobalAnimID;
}

//////////////////////////////////////////////////////////////////////////
// Loads animation file. Returns the global anim id of the file, or -1 if error
// SIDE EFFECT NOTES:
//  THis function does not put up a warning in the case the animation couldn't be loaded.
//  It returns an error (false) and the caller must process it.
i32 CAnimationSet::LoadFileAIM(tukk szFilePath, tukk szAnimName, const IDefaultSkeleton* pIDefaultSkeleton)
{
	//NEW RULE: every single AIM file is now an OnDemand streaming animation
	i32 nAnimId = GetAnimIDByName(szAnimName);
	if (nAnimId != -1)
	{
		i32 nGlobalAnimID = m_arrAnimations[nAnimId].m_nGlobalAnimId;
		g_pILog->LogWarning("DinrusXAnimation:: Trying to load animation with alias \"%s\" from file \"%s\" into the animation container. Such animation alias already exists and uses file \"%s\". Please use another animation alias.", szAnimName, szFilePath, g_AnimationUpr.m_arrGlobalAIM[nGlobalAnimID].GetFilePath());
		return nGlobalAnimID;
	}

	i32 nGlobalAnimID = g_AnimationUpr.CreateGAH_AIM(szFilePath);

	ModelAnimationHeader localAnim;
	localAnim.m_nGlobalAnimId = nGlobalAnimID;
	localAnim.m_nAssetType = AIM_File;
	localAnim.SetAnimName(szAnimName);
	m_arrAnimations.push_back(localAnim);

	StoreAnimName(localAnim.m_CRC32Name, szAnimName);

	i32 nLocalAnimId2 = m_arrAnimations.size() - 1;
	InsertHash(localAnim.m_CRC32Name, nLocalAnimId2);

	GlobalAnimationHeaderAIM& rAIM = g_AnimationUpr.m_arrGlobalAIM[nGlobalAnimID];
	rAIM.AddRef();

	for (CListenerSet<IAnimationSetListener*>::Notifier notifier(m_listeners); notifier.IsValid(); notifier.Next())
		notifier->OnAnimationSetAddAnimation(szFilePath, szAnimName);

	u32 IsCreated = rAIM.IsAssetCreated();
	if (IsCreated)
		return nGlobalAnimID;

	//----------------------------------------------------------------------------------------

	assert(rAIM.GetControllersCount() == 0);
	u8 status = rAIM.LoadAIM();
	if (status)
	{
		rAIM.ProcessPoses(pIDefaultSkeleton, szAnimName);
	}
	else
	{
		//Aim-Pose does not exist as file. this is an error
		if (rAIM.IsAssetNotFound() == 0)
		{
			tukk pPathName = rAIM.GetFilePath();
			g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, pPathName, "Failed to load animation CAF file");
			rAIM.OnAssetNotFound();
		}
	}

	return nGlobalAnimID;
}

//////////////////////////////////////////////////////////////////////////
// Loads animation file. Returns the global anim id of the file, or -1 if error
// SIDE EFFECT NOTES:
//  THis function does not put up a warning in the case the animation couldn't be loaded.
//  It returns an error (false) and the caller must process it.
i32 CAnimationSet::LoadFileANM(tukk szFileName, tukk szAnimName, DynArray<CControllerTCB>& m_LoadCurrAnimation, DrxCGALoader* pCGA, const IDefaultSkeleton* pIDefaultSkeleton)
{
	i32 nModelAnimId = GetAnimIDByName(szAnimName);
	if (nModelAnimId == -1)
	{
		i32 nGlobalAnimID = g_AnimationUpr.CreateGAH_CAF(szFileName);

		i32 nLocalAnimId = m_arrAnimations.size();
		ModelAnimationHeader LocalAnim;
		LocalAnim.m_nGlobalAnimId = nGlobalAnimID;
		LocalAnim.m_nAssetType = CAF_File;
		LocalAnim.SetAnimName(szAnimName);
		m_arrAnimations.push_back(LocalAnim);

		StoreAnimName(LocalAnim.m_CRC32Name, szAnimName);

		//m_arrAnimByGlobalId.insert (std::lower_bound(m_arrAnimByGlobalId.begin(), m_arrAnimByGlobalId.end(), nGlobalAnimID, AnimationGlobIdPred(m_arrAnimations)), LocalAnimId(nLocalAnimId));
		InsertHash(LocalAnim.m_CRC32Name, nLocalAnimId);
		//		m_arrAnimByLocalName.insert (std::lower_bound(m_arrAnimByLocalName.begin(), m_arrAnimByLocalName.end(), szAnimName, AnimationNamePred(m_arrAnimations)), nLocalAnimId);

		g_AnimationUpr.m_arrGlobalCAF[nGlobalAnimID].AddRef();

		u32 loaded = g_AnimationUpr.m_arrGlobalCAF[nGlobalAnimID].IsAssetLoaded();
		if (loaded)
		{
			// No asserts on data
			//assert(g_AnimationUpr.m_arrGlobalCAF[nGlobalAnimID].m_arrController.size());
		}
		else
		{
			//assert(g_AnimationUpr.m_arrGlobalCAF[nGlobalAnimID].m_arrController.size()==0);
			//assert(g_AnimationUpr.m_arrGlobalCAF[nGlobalAnimID].m_arrController);
			g_AnimationUpr.LoadAnimationTCB(nGlobalAnimID, m_LoadCurrAnimation, pCGA, pIDefaultSkeleton);
		}

		for (CListenerSet<IAnimationSetListener*>::Notifier notifier(m_listeners); notifier.IsValid(); notifier.Next())
			notifier->OnAnimationSetAddAnimation(szFileName, szAnimName);

		return nGlobalAnimID;
	}
	else
	{
		i32 nGlobalAnimID = m_arrAnimations[nModelAnimId].m_nGlobalAnimId;
		g_pILog->LogWarning("DinrusXAnimation:: Trying to load animation with alias \"%s\" from file \"%s\" into the animation container. Such animation alias already exists and uses file \"%s\". Please use another animation alias.", szAnimName, szFileName, g_AnimationUpr.m_arrGlobalCAF[nGlobalAnimID].GetFilePath());
		return nGlobalAnimID;
	}
}

//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------

i32 CAnimationSet::LoadFileLMG(tukk szFilePath, tukk szAnimName)
{
	i32 nModelAnimId = GetAnimIDByName(szAnimName);
	if (nModelAnimId == -1)
	{
		i32 nGlobalAnimID = g_AnimationUpr.CreateGAH_LMG(szFilePath);
		i32 nLocalAnimId = m_arrAnimations.size();

		ModelAnimationHeader LocalAnim;
		LocalAnim.m_nGlobalAnimId = nGlobalAnimID;
		LocalAnim.m_nAssetType = LMG_File;
		LocalAnim.SetAnimName(szAnimName);
		m_arrAnimations.push_back(LocalAnim);

		StoreAnimName(LocalAnim.m_CRC32Name, szAnimName);

		InsertHash(LocalAnim.m_CRC32Name, nLocalAnimId);

		g_AnimationUpr.m_arrGlobalLMG[nGlobalAnimID].AddRef();

		for (CListenerSet<IAnimationSetListener*>::Notifier notifier(m_listeners); notifier.IsValid(); notifier.Next())
			notifier->OnAnimationSetAddAnimation(szFilePath, szAnimName);

		return nGlobalAnimID;
	}
	else
	{

		const ModelAnimationHeader* anim = GetModelAnimationHeader(nModelAnimId);
		if (anim)
		{
			if (anim->m_nAssetType == CAF_File)
			{
				GlobalAnimationHeaderCAF& rCAF = g_AnimationUpr.m_arrGlobalCAF[anim->m_nGlobalAnimId];
				g_pILog->LogWarning("DinrusXAnimation:: weird access of CAF \"%s\"", rCAF.GetFilePath());
				return anim->m_nGlobalAnimId;
			}
			if (anim->m_nAssetType == AIM_File)
			{
				GlobalAnimationHeaderAIM& rAIM = g_AnimationUpr.m_arrGlobalAIM[anim->m_nGlobalAnimId];
				g_pILog->LogWarning("DinrusXAnimation:: weird access of AIM \"%s\"", rAIM.GetFilePath());
				return anim->m_nGlobalAnimId;
			}
			if (anim->m_nAssetType == LMG_File)
			{
				GlobalAnimationHeaderLMG& rLMG = g_AnimationUpr.m_arrGlobalLMG[anim->m_nGlobalAnimId];
				g_pILog->LogWarning("DinrusXAnimation:: Trying to load animation with alias \"%s\" from file \"%s\" into the animation container. Such animation alias already exists and uses file \"%s\". Please use another animation alias.", szAnimName, szFilePath, rLMG.GetFilePath());
				return anim->m_nGlobalAnimId;
			}

		}

		return -1;
	}

}

// prepares to load the specified number of CAFs by reserving the space for the controller pointers
void CAnimationSet::prepareLoadCAFs(u32 nReserveAnimations)
{
	u32 numAnims = m_arrAnimations.size();
	if (numAnims)
		DrxFatalError("DinrusXAnimation: chrparams file loaded twice");

	m_arrAnimations.reserve(nReserveAnimations);
}

// prepares to load the specified number of CAFs by reserving the space for the controller pointers
void CAnimationSet::prepareLoadANMs(u32 nReserveAnimations)
{
	nReserveAnimations += m_arrAnimations.size();
	m_arrAnimations.reserve(nReserveAnimations);
}

const ModelAnimationHeader* CAnimationSet::GetModelAnimationHeader(i32 i) const
{
	DEFINE_PROFILER_FUNCTION();

	i32 numAnimation = (i32)m_arrAnimations.size();
	if (numAnimation == 0)
		return 0;

	if (i < 0 || i >= numAnimation)
	{
		if (Console::GetInst().ca_AnimWarningLevel > 0)
			gEnv->pLog->LogError("Invalid Animation ID: %d for Model: %s", i, m_strSkeletonFilePath.c_str());

		return 0;
	}
	return &m_arrAnimations[i];  //the animation-asset exists
}

//----------------------------------------------------------------------------------
// Returns the index of the animation in the set, -1 if there's no such animation
//----------------------------------------------------------------------------------
i32 CAnimationSet::GetAnimIDByName(tukk szAnimationName) const
{
	u32 numAnimsInList = m_arrAnimations.size();
	if (numAnimsInList == 0)
		return -1;
	if (szAnimationName == 0)
		return -1;

	return GetValue(szAnimationName);
}

// Returns the given animation name
tukk CAnimationSet::GetNameByAnimID(i32 nAnimationId) const
{
	if (nAnimationId < 0)
		return "!NEGATIVE ANIMATION ID!";

	i32 numAnimation = m_arrAnimations.size();
	if (nAnimationId >= numAnimation)
		return "!ANIMATION ID OUT OF RANGE!";

#ifdef STORE_ANIMATION_NAMES
	return m_arrAnimations[nAnimationId].GetAnimName();
#else
	HashToNameMap::const_iterator iter(m_hashToNameMap.find(m_arrAnimations[nAnimationId].m_CRC32Name));
	if (iter != m_hashToNameMap.end())
		return iter->second;
	else
		return m_arrAnimations[nAnimationId].GetAnimName();
#endif
}

//------------------------------------------------------------------------------
i32 CAnimationSet::GetAnimIDByCRC(u32 animationCRC) const
{
	size_t value = GetValueCRC(animationCRC);
	return value;
}

u32 CAnimationSet::GetCRCByAnimID(i32 nAnimationId) const
{
	if ((nAnimationId >= 0) && (nAnimationId < (i32)m_arrAnimations.size()))
	{
		return m_arrAnimations[nAnimationId].m_CRC32Name;
	}
	else
	{
		return 0;
	}
}

u32 CAnimationSet::GetFilePathCRCByAnimID(i32 nAnimationId) const
{
	const ModelAnimationHeader* anim = GetModelAnimationHeader(nAnimationId);
	if (anim == 0)
		return 0;

	u32 GlobalAnimationID = anim->m_nGlobalAnimId;
	if (anim->m_nAssetType == CAF_File)
		return g_AnimationUpr.m_arrGlobalCAF[GlobalAnimationID].GetFilePathCRC32();
	if (anim->m_nAssetType == AIM_File)
		return g_AnimationUpr.m_arrGlobalAIM[GlobalAnimationID].GetFilePathCRC32();
	if (anim->m_nAssetType == LMG_File)
		return g_AnimationUpr.m_arrGlobalLMG[GlobalAnimationID].GetFilePathCRC32();

	assert(0);
	return 0;
}

bool CAnimationSet::IsAnimLoaded(i32 nAnimationId) const
{
	bool bInMemory = false;
	const ModelAnimationHeader* pAnim = GetModelAnimationHeader(nAnimationId);
	if (pAnim)
	{
		if (pAnim->m_nAssetType == CAF_File)
		{
			i32 nGlobalID = pAnim->m_nGlobalAnimId;
			GlobalAnimationHeaderCAF& rGAHeader = g_AnimationUpr.m_arrGlobalCAF[nGlobalID];
			bInMemory = rGAHeader.IsAssetLoaded() != 0;
		}
		else if (pAnim->m_nAssetType == AIM_File)
		{
			i32 nGlobalID = pAnim->m_nGlobalAnimId;
			GlobalAnimationHeaderAIM& rGAHeader = g_AnimationUpr.m_arrGlobalAIM[nGlobalID];
			bInMemory = rGAHeader.IsAssetLoaded() != 0;
		}
		else if (pAnim->m_nAssetType == LMG_File)
		{
			i32 nGlobalID = pAnim->m_nGlobalAnimId;
			GlobalAnimationHeaderLMG& rGAHeader = g_AnimationUpr.m_arrGlobalLMG[nGlobalID];
			bInMemory = rGAHeader.IsAssetLoaded() != 0;
		}
	}
	return bInMemory;
}

bool CAnimationSet::IsVEGLoaded(i32 nGlobalVEGId) const
{
	GlobalAnimationHeaderLMG& rLMG = g_AnimationUpr.m_arrGlobalLMG[nGlobalVEGId];
	bool bInMemory = rLMG.IsAssetLoaded() != 0;

	if (rLMG.m_numExamples)
	{
		for (u32 i = 0; (i < rLMG.m_numExamples) && bInMemory; i++)
		{
			i32 exampleAnimID = GetAnimIDByCRC(rLMG.m_arrParameter[i].m_animName.m_CRC32);
			if (exampleAnimID >= 0)
			{
				bInMemory = IsAnimLoaded(exampleAnimID);
			}
			else
			{
				bInMemory = false;
			}
		}
	}
	else
	{
		u32 numBlendSpaces = rLMG.m_arrCombinedBlendSpaces.size();
		for (u32 bs = 0; (bs < numBlendSpaces) && bInMemory; bs++)
		{
			i32 nSubGlobalID = rLMG.m_arrCombinedBlendSpaces[bs].m_ParaGroupID;
			if (nSubGlobalID >= 0)
			{
				bInMemory = IsVEGLoaded(nSubGlobalID);
			}
			else
			{
				bInMemory = false;
			}
		}
	}

	return bInMemory;
}

static bool AnimTokenMatchesOneOfDirectionalBlends(const CAnimationSet* pAnimationSet, i32 nAnimationId, const DynArray<DirectionalBlends>& blends)
{
	const ModelAnimationHeader* pAnim = pAnimationSet->GetModelAnimationHeader(nAnimationId);
	if (pAnim && pAnim->m_nAssetType == AIM_File)
	{
		i32 nGlobalID = pAnim->m_nGlobalAnimId;
		u32 animTokenCRC32 = g_AnimationUpr.m_arrGlobalAIM[nGlobalID].m_AnimTokenCRC32;
		size_t numBlends = blends.size();
		for (size_t i = 0; i < numBlends; ++i)
			if (blends[i].m_AnimTokenCRC32 == animTokenCRC32)
				return true;
	}
	return false;
}

bool CAnimationSet::IsAimPose(i32 nAnimationId, const IDefaultSkeleton& defaultSkeleton) const
{
	return AnimTokenMatchesOneOfDirectionalBlends(this, nAnimationId, static_cast<const CDefaultSkeleton&>(defaultSkeleton).m_poseBlenderAimDesc.m_blends);
}

bool CAnimationSet::IsLookPose(i32 nAnimationId, const IDefaultSkeleton& defaultSkeleton) const
{
	return AnimTokenMatchesOneOfDirectionalBlends(this, nAnimationId, static_cast<const CDefaultSkeleton&>(defaultSkeleton).m_poseBlenderLookDesc.m_blends);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
GlobalAnimationHeaderCAF* CAnimationSet::GetGAH_CAF(tukk AnimationName) const
{
	i32 subAnimID = GetAnimIDByName(AnimationName);
	if (subAnimID < 0)
		return 0; //error -> name not found
	return GetGAH_CAF(subAnimID);
}
GlobalAnimationHeaderAIM* CAnimationSet::GetGAH_AIM(tukk AnimationName) const
{
	i32 subAnimID = GetAnimIDByName(AnimationName);
	if (subAnimID < 0)
		return 0; //error -> name not found
	return GetGAH_AIM(subAnimID);
}
GlobalAnimationHeaderLMG* CAnimationSet::GetGAH_LMG(tukk AnimationName) const
{
	i32 subAnimID = GetAnimIDByName(AnimationName);
	if (subAnimID < 0)
		return 0; //error -> name not found
	return GetGAH_LMG(subAnimID);
}

GlobalAnimationHeaderCAF* CAnimationSet::GetGAH_CAF(i32 nAnimationId) const
{
	i32 numAnimation = (i32)m_arrAnimations.size();
	if ((nAnimationId < 0) || (nAnimationId >= numAnimation))
	{
		gEnv->pLog->LogError("illegal animation index '%d' for Model: %s", nAnimationId, m_strSkeletonFilePath.c_str());
		return NULL;
	}

	const ModelAnimationHeader* anim = GetModelAnimationHeader(nAnimationId);
	if (anim == NULL)
		return NULL;
	if (anim->m_nAssetType == CAF_File)
		return &g_AnimationUpr.m_arrGlobalCAF[anim->m_nGlobalAnimId];
	return 0;
}

GlobalAnimationHeaderAIM* CAnimationSet::GetGAH_AIM(i32 nAnimationId) const
{
	i32 numAnimation = (i32)m_arrAnimations.size();
	if ((nAnimationId < 0) || (nAnimationId >= numAnimation))
	{
		gEnv->pLog->LogError("illegal animation index '%d' for Model: %s", nAnimationId, m_strSkeletonFilePath.c_str());
		return NULL;
	}

	const ModelAnimationHeader* anim = GetModelAnimationHeader(nAnimationId);
	if (anim == NULL)
		return NULL;
	if (anim->m_nAssetType == AIM_File)
		return &g_AnimationUpr.m_arrGlobalAIM[anim->m_nGlobalAnimId];
	return 0;
}

GlobalAnimationHeaderLMG* CAnimationSet::GetGAH_LMG(i32 nAnimationId) const
{
	i32 numAnimation = (i32)m_arrAnimations.size();
	if ((nAnimationId < 0) || (nAnimationId >= numAnimation))
	{
		gEnv->pLog->LogError("illegal animation index '%d' for Model: %s", nAnimationId, m_strSkeletonFilePath.c_str());
		return NULL;
	}

	const ModelAnimationHeader* anim = GetModelAnimationHeader(nAnimationId);
	if (anim == NULL)
		return NULL;
	if (anim->m_nAssetType == LMG_File)
		return &g_AnimationUpr.m_arrGlobalLMG[anim->m_nGlobalAnimId];
	return 0;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
f32 CAnimationSet::GetDuration_sec(i32 nAnimationId) const
{
	const f32 INVALID_DURATION = 0.0f;

	i32 numAnimation = (i32)m_arrAnimations.size();
	if (nAnimationId < 0 || nAnimationId >= numAnimation)
	{
		if (Console::GetInst().ca_AnimWarningLevel > 0)
			gEnv->pLog->LogError("illegal animation index used in function GetDuration_sec'%d' for Model: %s", nAnimationId, m_strSkeletonFilePath.c_str());
		return INVALID_DURATION;
	}

	const ModelAnimationHeader* anim = GetModelAnimationHeader(nAnimationId);
	if (anim == 0)
		return INVALID_DURATION;

	if (anim->m_nAssetType == CAF_File)
	{
		u32 GlobalAnimationID = anim->m_nGlobalAnimId;
		GlobalAnimationHeaderCAF& rGlobalAnimHeader = g_AnimationUpr.m_arrGlobalCAF[GlobalAnimationID];
		f32 fDuration = rGlobalAnimHeader.m_fEndSec - rGlobalAnimHeader.m_fStartSec;
		return fDuration;
	}

	if (anim->m_nAssetType == AIM_File)
	{
		u32 GlobalAnimationID = anim->m_nGlobalAnimId;
		GlobalAnimationHeaderAIM& rGlobalAnimHeader = g_AnimationUpr.m_arrGlobalAIM[GlobalAnimationID];
		f32 fDuration = rGlobalAnimHeader.m_fEndSec - rGlobalAnimHeader.m_fStartSec;
		return fDuration;
	}

	if (anim->m_nAssetType == LMG_File)
	{
		u32 GlobalAnimationID = anim->m_nGlobalAnimId;
		GlobalAnimationHeaderLMG& rGlobalAnimHeader = g_AnimationUpr.m_arrGlobalLMG[GlobalAnimationID];
		u32 lmg = rGlobalAnimHeader.IsAssetLMG();
		assert(lmg);
		if (rGlobalAnimHeader.IsAssetLMGValid() == 0)
			return 0;
		if (rGlobalAnimHeader.IsAssetInternalType())
			return 1.0f; //there is no reliable way to compute the duration.
		u32k numBlendSpaces = rGlobalAnimHeader.m_arrCombinedBlendSpaces.size();
		if (numBlendSpaces > 0)
		{
			f32 fDuration = 0.0f;
			f32 totalExamples = 0.0f;
			for (u32 i = 0; i < numBlendSpaces; i++)
			{
				i32 bspaceID = rGlobalAnimHeader.m_arrCombinedBlendSpaces[i].m_ParaGroupID;
				if (bspaceID < 0)
					return INVALID_DURATION;

				GlobalAnimationHeaderLMG& rGlobalAnimHeaderBS = g_AnimationUpr.m_arrGlobalLMG[bspaceID];

				u32k numBS = rGlobalAnimHeaderBS.m_numExamples;
				for (u32 e = 0; e < numBS; e++)
				{
					i32 aid = GetAnimIDByCRC(rGlobalAnimHeaderBS.m_arrParameter[e].m_animName.m_CRC32);
					assert(aid >= 0);
					fDuration += GetDuration_sec(aid);
					totalExamples += 1.0f;
				}
			}
			return (totalExamples > 0) ? fDuration / totalExamples : INVALID_DURATION;
		}
		else
		{
			u32 numBS = rGlobalAnimHeader.m_numExamples;
			if (numBS == 0)
				return INVALID_DURATION;
			f32 fDuration = 0;
			for (u32 i = 0; i < numBS; i++)
			{
				i32 aid = GetAnimIDByCRC(rGlobalAnimHeader.m_arrParameter[i].m_animName.m_CRC32);
				assert(aid >= 0);
				fDuration += GetDuration_sec(aid);
			}
			return fDuration / numBS;
		}
	}

	assert(0);
	return INVALID_DURATION;
}

u32 CAnimationSet::GetAnimationFlags(i32 nAnimationId) const
{
	i32 numAnimation = (i32)m_arrAnimations.size();
	if (nAnimationId < 0 || nAnimationId >= numAnimation)
	{
		if (Console::GetInst().ca_AnimWarningLevel > 0)
			gEnv->pLog->LogError("illegal animation index used in function GetAnimationFlags '%d' for Model: %s", nAnimationId, m_strSkeletonFilePath.c_str());
		return 0;
	}
	const ModelAnimationHeader* pAnimHeader = GetModelAnimationHeader(nAnimationId);
	if (pAnimHeader == 0)
		return 0;

	u32 nGlobalAnimationID = pAnimHeader->m_nGlobalAnimId;

	if (pAnimHeader->m_nAssetType == CAF_File)
	{
		if (g_AnimationUpr.m_arrGlobalCAF[nGlobalAnimationID].IsAssetOnDemand())
			g_AnimationUpr.m_arrGlobalCAF[nGlobalAnimationID].m_nFlags |= CA_ASSET_ONDEMAND;
		return g_AnimationUpr.m_arrGlobalCAF[nGlobalAnimationID].m_nFlags;
	}
	if (pAnimHeader->m_nAssetType == AIM_File)
		return g_AnimationUpr.m_arrGlobalAIM[nGlobalAnimationID].m_nFlags;
	if (pAnimHeader->m_nAssetType == LMG_File)
		return g_AnimationUpr.m_arrGlobalLMG[nGlobalAnimationID].m_nFlags;

	assert(0);
	return 0;
}

tukk CAnimationSet::GetAnimationStatus(i32 nAnimationId) const
{
	i32 numAnimation = (i32)m_arrAnimations.size();
	if (nAnimationId < 0 || nAnimationId >= numAnimation)
	{
		if (Console::GetInst().ca_AnimWarningLevel > 0)
			gEnv->pLog->LogError("illegal animation index used in function GetAnimationFlags '%d' for Model: %s", nAnimationId, m_strSkeletonFilePath.c_str());
		return 0;
	}
	const ModelAnimationHeader* pAnimHeader = GetModelAnimationHeader(nAnimationId);
	if (pAnimHeader == 0)
		return 0;

	u32 nGlobalAnimationID = pAnimHeader->m_nGlobalAnimId;

	if (pAnimHeader->m_nAssetType == CAF_File)
		return 0;
	if (pAnimHeader->m_nAssetType == AIM_File)
		return 0;
	if (pAnimHeader->m_nAssetType == LMG_File)
		return g_AnimationUpr.m_arrGlobalLMG[nGlobalAnimationID].m_Status.c_str();

	assert(0);
	return 0;
}

void CAnimationSet::AddRef(i32k nAnimationId) const
{
	i32 numAnimation = (i32)m_arrAnimations.size();
	if (nAnimationId < 0 || nAnimationId >= numAnimation)
	{
		if (Console::GetInst().ca_AnimWarningLevel > 0)
			gEnv->pLog->LogError("illegal animation index used in function AddRef '%d' for Model: %s", nAnimationId, m_strSkeletonFilePath.c_str());
		return;
	}

	const ModelAnimationHeader* pAnimHeader = GetModelAnimationHeader(nAnimationId);
	if (pAnimHeader == 0)
		return;

	u32 nGlobalAnimationID = pAnimHeader->m_nGlobalAnimId;
	if (pAnimHeader->m_nAssetType == CAF_File)
	{
		g_pCharacterUpr->CAF_AddRefByGlobalId(nGlobalAnimationID);
	}
	else if (pAnimHeader->m_nAssetType == LMG_File)
	{
		GlobalAnimationHeaderLMG& rLMG = g_AnimationUpr.m_arrGlobalLMG[nGlobalAnimationID];

		rLMG.AddRef();
		if (rLMG.m_numExamples)
		{
			for (u32 i = 0; i < rLMG.m_numExamples; i++)
			{
				i32 exampleAnimID = GetAnimIDByCRC(rLMG.m_arrParameter[i].m_animName.m_CRC32);
				if (exampleAnimID >= 0)
				{
					const ModelAnimationHeader* pExampleAnimHeader = GetModelAnimationHeader(exampleAnimID);
					g_pCharacterUpr->CAF_AddRefByGlobalId(pExampleAnimHeader->m_nGlobalAnimId);
				}
			}
		}
		else
		{
			u32 totalExamples = 0;

			u32 numBlendSpaces = rLMG.m_arrCombinedBlendSpaces.size();
			for (u32 bs = 0; bs < numBlendSpaces; bs++)
			{
				i32 nGlobalID = rLMG.m_arrCombinedBlendSpaces[bs].m_ParaGroupID;
				if (nGlobalID < 0)
					continue;

				GlobalAnimationHeaderLMG& rsubLMG = g_AnimationUpr.m_arrGlobalLMG[nGlobalID];

				for (u32 i = 0; i < rsubLMG.m_numExamples; i++)
				{
					i32 exampleAnimID = GetAnimIDByCRC(rsubLMG.m_arrParameter[i].m_animName.m_CRC32);
					if (exampleAnimID >= 0)
					{
						const ModelAnimationHeader* pExampleAnimHeader = GetModelAnimationHeader(exampleAnimID);
						g_pCharacterUpr->CAF_AddRefByGlobalId(pExampleAnimHeader->m_nGlobalAnimId);
					}
				}
			}
		}
	}
}

void CAnimationSet::Release(i32k nAnimationId) const
{
	i32 numAnimation = (i32)m_arrAnimations.size();
	if (nAnimationId < 0 || nAnimationId >= numAnimation)
	{
		if (Console::GetInst().ca_AnimWarningLevel > 0)
			gEnv->pLog->LogError("illegal animation index used in function AddRef '%d' for Model: %s", nAnimationId, m_strSkeletonFilePath.c_str());
		return;
	}

	const ModelAnimationHeader* pAnimHeader = GetModelAnimationHeader(nAnimationId);
	if (pAnimHeader == 0)
		return;

	u32 nGlobalAnimationID = pAnimHeader->m_nGlobalAnimId;
	if (pAnimHeader->m_nAssetType == CAF_File)
	{
		g_pCharacterUpr->CAF_ReleaseByGlobalId(nGlobalAnimationID);
	}
	else if (pAnimHeader->m_nAssetType == LMG_File)
	{
		GlobalAnimationHeaderLMG& rLMG = g_AnimationUpr.m_arrGlobalLMG[nGlobalAnimationID];

		if (rLMG.m_numExamples)
		{
			for (u32 i = 0; i < rLMG.m_numExamples; i++)
			{
				i32 exampleAnimID = GetAnimIDByCRC(rLMG.m_arrParameter[i].m_animName.m_CRC32);
				if (exampleAnimID >= 0)
				{
					const ModelAnimationHeader* pExampleAnimHeader = GetModelAnimationHeader(exampleAnimID);
					g_pCharacterUpr->CAF_ReleaseByGlobalId(pExampleAnimHeader->m_nGlobalAnimId);
				}
			}
		}
		else
		{
			u32 totalExamples = 0;

			u32 numBlendSpaces = rLMG.m_arrCombinedBlendSpaces.size();
			for (u32 bs = 0; bs < numBlendSpaces; bs++)
			{
				i32 nGlobalID = rLMG.m_arrCombinedBlendSpaces[bs].m_ParaGroupID;
				if (nGlobalID >= 0)
				{
					GlobalAnimationHeaderLMG& rsubLMG = g_AnimationUpr.m_arrGlobalLMG[nGlobalID];

					for (u32 i = 0; i < rsubLMG.m_numExamples; i++)
					{
						i32 exampleAnimID = GetAnimIDByCRC(rsubLMG.m_arrParameter[i].m_animName.m_CRC32);
						if (exampleAnimID >= 0)
						{
							const ModelAnimationHeader* pExampleAnimHeader = GetModelAnimationHeader(exampleAnimID);
							g_pCharacterUpr->CAF_ReleaseByGlobalId(pExampleAnimHeader->m_nGlobalAnimId);
						}
					}
				}
			}
		}
		rLMG.Release();
	}
}

bool CAnimationSet::GetAnimationDCCWorldSpaceLocation(tukk szAnimationName, QuatT& startLocation) const
{
	i32 AnimID = GetAnimIDByName(szAnimationName);
	return GetAnimationDCCWorldSpaceLocation(AnimID, startLocation);
}

bool CAnimationSet::GetAnimationDCCWorldSpaceLocation(i32 AnimID, QuatT& startLocation) const
{
	startLocation.SetIdentity();
	if (AnimID < 0)
	{
		if (Console::GetInst().ca_AnimWarningLevel > 0)
			gEnv->pLog->LogError("animation-name not in chrparams file: %s for Model: %s", GetNameByAnimID(AnimID), m_strSkeletonFilePath.c_str());
		return false;
	}
	const ModelAnimationHeader* anim = GetModelAnimationHeader(AnimID);
	if ((anim == 0) || (anim->m_nAssetType != CAF_File))
		return false;
	u32 GlobalAnimationID = anim->m_nGlobalAnimId;
	GlobalAnimationHeaderCAF& rGlobalAnimHeader = g_AnimationUpr.m_arrGlobalCAF[GlobalAnimationID];
	if (rGlobalAnimHeader.IsAssetCreated() == 0)
		return false;
	startLocation = rGlobalAnimHeader.m_StartLocation;
	return true;
}

bool CAnimationSet::GetAnimationDCCWorldSpaceLocation(const CAnimation* pAnim, QuatT& startLocation, u32 nControllerID) const
{
	startLocation.SetIdentity();
	if (!pAnim)
		return false;

	i16 animID = pAnim->GetAnimationId();
	if (animID < 0)
	{
		if (Console::GetInst().ca_AnimWarningLevel > 0)
			gEnv->pLog->LogError("animation-name not in chrparams file: %s for Model: %s", GetNameByAnimID(animID), m_strSkeletonFilePath.c_str());
		return false;
	}

	const ModelAnimationHeader* pAnimHeader = GetModelAnimationHeader(animID);
	if (!pAnimHeader || pAnimHeader->m_nAssetType != CAF_File)
		return false;

	GlobalAnimationHeaderCAF& rCAF = g_AnimationUpr.m_arrGlobalCAF[pAnimHeader->m_nGlobalAnimId];
	if (rCAF.IsAssetCreated() == 0)
		return false;

	startLocation = rCAF.m_StartLocation;

	if (rCAF.IsAssetLoaded() == 0)
		return true; // the startLocation is already valid at this point

	const IController* pController = rCAF.GetControllerByJointCRC32(nControllerID);
	assert(pController);
	if (!pController)
		return true;

	QuatT key0(IDENTITY);
	QuatT key1(IDENTITY);
	const f32 fKeyTime0 = rCAF.NTime2KTime(0);
	pController->GetOP(fKeyTime0, key0.q, key0.t);  //the first key should be always identity, except in the case of raw-assets
	const float ntime = rCAF.GetNTimeforEntireClip(pAnim->GetCurrentSegmentIndex(), pAnim->GetCurrentSegmentNormalizedTime());

#ifndef _RELEASE
	// Debug code to try and figure out rare crash bug DT-2674, which crashes on a floating point error during the second GetOP call
	{
		if (!NumberValid(ntime))
		{
			static float lastntime = 0.0f;
			lastntime = ntime;
			DrxFatalError("CAnimationSet::GetAnimationDCCWorldSpaceLocation: Invalid nTime");
		}
	}
#endif

	const f32 fKeyTime1 = rCAF.NTime2KTime(ntime);

#ifndef _RELEASE
	// Debug code to try and figure out rare crash bug DT-2674, which crashes on a floating point error during the second GetOP call
	{
		if (!NumberValid(fKeyTime1))
		{
			static float lastfKeyTime1 = 0.0f;
			lastfKeyTime1 = fKeyTime1;
			DrxFatalError("CAnimationSet::GetAnimationDCCWorldSpaceLocation: Invalid fKeyTime1");
		}
	}
#endif

	pController->GetOP(fKeyTime1, key1.q, key1.t);
	const QuatT animationMovement = key0.GetInverted() * key1;
	startLocation = startLocation * animationMovement;
	return true;
}

CAnimationSet::ESampleResult CAnimationSet::SampleAnimation(i32k animationId, const float animationNormalizedTime, u32k controllerId, QuatT& relativeLocationOutput) const
{
	relativeLocationOutput.SetIdentity();

	const ModelAnimationHeader* pModelAnimationHeader = GetModelAnimationHeader(animationId);
	if (!pModelAnimationHeader)
		return eSR_InvalidAnimationId;

	if (pModelAnimationHeader->m_nAssetType != CAF_File)
		return eSR_UnsupportedAssetType;

	const GlobalAnimationHeaderCAF& globalAnimationHeader = g_AnimationUpr.m_arrGlobalCAF[pModelAnimationHeader->m_nGlobalAnimId];
	if (!globalAnimationHeader.IsAssetCreated() || !globalAnimationHeader.IsAssetLoaded())
		return eSR_NotInMemory;

	const IController* pController = globalAnimationHeader.GetControllerByJointCRC32(controllerId);
	if (!pController)
		return eSR_ControllerNotFound;

	pController->GetOP(globalAnimationHeader.NTime2KTime(animationNormalizedTime), relativeLocationOutput.q, relativeLocationOutput.t);

	return eSR_Success;
}

tukk CAnimationSet::GetFilePathByName(tukk szAnimationName) const
{
	i32 AnimID = GetAnimIDByName(szAnimationName);
	if (AnimID < 0)
	{
		gEnv->pLog->LogError("animation-name not in chrparams file: %s for Model: %s", szAnimationName, m_strSkeletonFilePath.c_str());
		return 0;
	}
	const ModelAnimationHeader* anim = GetModelAnimationHeader(AnimID);
	if (anim == 0)
		return 0;
	return anim->GetFilePath();
};

tukk CAnimationSet::GetFilePathByID(i32 nAnimationId) const
{
	const ModelAnimationHeader* anim = GetModelAnimationHeader(nAnimationId);
	if (anim == 0)
		return 0;
	return anim->GetFilePath();
};

#ifdef EDITOR_PCDEBUGCODE
i32 CAnimationSet::GetGlobalIDByName(tukk szAnimationName) const
{
	i32 AnimID = GetAnimIDByName(szAnimationName);
	if (AnimID < 0)
	{
		gEnv->pLog->LogError("DinrusXAnimation. animation-name not in chrparams file: %s for Model: %s", szAnimationName, m_strSkeletonFilePath.c_str());
		return -1;
	}
	const ModelAnimationHeader* anim = GetModelAnimationHeader(AnimID);
	if (anim == 0)
		return -1;
	return (i32)anim->m_nGlobalAnimId;
}
#endif

i32 CAnimationSet::GetGlobalIDByAnimID(i32 nAnimationId) const
{
	const ModelAnimationHeader* anim = GetModelAnimationHeader(nAnimationId);
	if (anim == 0)
		return -1;
	return (i32)anim->m_nGlobalAnimId;
}

u32 CAnimationSet::GetAnimationSize(u32k nAnimationId) const
{
	assert(nAnimationId >= 0);
	assert((i32)nAnimationId < m_arrAnimations.size());
	const ModelAnimationHeader& header = m_arrAnimations[nAnimationId];

	i32 globalID = header.m_nGlobalAnimId;
	if (globalID < 0)
		return 0;

	switch (header.m_nAssetType)
	{
	case LMG_File:
		return g_AnimationUpr.m_arrGlobalLMG[globalID].SizeOfLMG();
	case CAF_File:
		return g_AnimationUpr.m_arrGlobalCAF[globalID].SizeOfCAF(true);
	default:
		return 0;
	}
}

#ifdef EDITOR_PCDEBUGCODE
tukk CAnimationSet::GetDBAFilePath(u32k nAnimationId) const
{
	assert(nAnimationId >= 0);
	assert((i32)nAnimationId < m_arrAnimations.size());
	const ModelAnimationHeader& header = m_arrAnimations[nAnimationId];

	i32 globalID = header.m_nGlobalAnimId;
	if (header.m_nAssetType == CAF_File)
	{
		assert(globalID >= 0);
		assert(globalID < g_AnimationUpr.m_arrGlobalCAF.size());
		return g_pCharacterUpr->GetDBAFilePathByGlobalID(globalID);
	}
	else
		return NULL;
}
#endif

#ifdef EDITOR_PCDEBUGCODE
u32 CAnimationSet::GetTotalPosKeys(u32k nAnimationId) const
{
	assert(nAnimationId >= 0);
	assert((i32)nAnimationId < m_arrAnimations.size());
	const ModelAnimationHeader& header = m_arrAnimations[nAnimationId];

	i32 globalID = header.m_nGlobalAnimId;

	if (header.m_nAssetType == CAF_File)
	{
		assert(globalID >= 0);
		assert(globalID < g_AnimationUpr.m_arrGlobalCAF.size());
		return g_AnimationUpr.m_arrGlobalCAF[globalID].GetTotalPosKeys();
	}
	return 0;
}

u32 CAnimationSet::GetTotalRotKeys(u32k nAnimationId) const
{
	assert(nAnimationId >= 0);
	assert((i32)nAnimationId < m_arrAnimations.size());
	const ModelAnimationHeader& header = m_arrAnimations[nAnimationId];

	i32 globalID = header.m_nGlobalAnimId;

	if (header.m_nAssetType == CAF_File)
	{
		assert(globalID >= 0);
		assert(globalID < g_AnimationUpr.m_arrGlobalCAF.size());
		return g_AnimationUpr.m_arrGlobalCAF[globalID].GetTotalRotKeys();
	}
	return 0;
}

void CAnimationSet::RebuildAimHeader(tukk szAnimationName, const IDefaultSkeleton* pIDefaultSkeleton)
{
	i32k animationId = GetAnimIDByName(szAnimationName);
	const ModelAnimationHeader* pAnim = GetModelAnimationHeader(animationId);
	if (pAnim->m_nAssetType != AIM_File)
		return;

	GlobalAnimationHeaderAIM& aimHeader = g_AnimationUpr.m_arrGlobalAIM[pAnim->m_nGlobalAnimId];
	i32k loadSuccess = aimHeader.LoadAIM();
	if (loadSuccess == 0)
		return;

	aimHeader.ProcessPoses(pIDefaultSkeleton, szAnimationName);
}

i32 CAnimationSet::AddAnimationByPath(tukk animationName, tukk animationPath, const IDefaultSkeleton* pIDefaultSkeleton)
{
	{
		i32k localAnimationId = GetAnimIDByName(animationName);
		if (localAnimationId != -1)
		{
			return localAnimationId;
		}
	}

	tukk fileExtension = PathUtil::GetExt(animationPath);

	const bool isCaf = (stricmp(fileExtension, "caf") == 0);
	const bool isLmg = (stricmp(fileExtension, "lmg") == 0) || (stricmp(fileExtension, "bspace") == 0) || (stricmp(fileExtension, "comb") == 0);

	const bool isValidAnimationExtension = (isCaf || isLmg);
	if (!isValidAnimationExtension)
	{
		return -1;
	}

	i32 globalAnimationId = -1;
	if (isCaf)
	{
		const bool isAim = false;

		if (isAim)
		{
			globalAnimationId = LoadFileAIM(animationPath, animationName, pIDefaultSkeleton);
		}
		else
		{
			globalAnimationId = LoadFileCAF(animationPath, animationName);
		}
	}
	else
	{
		globalAnimationId = LoadFileLMG(animationPath, animationName);
	}

	if (globalAnimationId == -1)
	{
		return -1;
	}

	i32k localAnimationId = GetAnimIDByName(animationName);
	return localAnimationId;
}

void CAnimationSet::NotifyListenersAboutReloadStarted()
{
	for (CListenerSet<IAnimationSetListener*>::Notifier notifier(m_listeners); notifier.IsValid(); notifier.Next())
		notifier->OnAnimationSetAboutToBeReloaded();
}

void CAnimationSet::NotifyListenersAboutReload()
{
	for (CListenerSet<IAnimationSetListener*>::Notifier notifier(m_listeners); notifier.IsValid(); notifier.Next())
		notifier->OnAnimationSetReloaded();
}
#endif

//----------------------------------------------------------------------------------
//----      check if all animation-assets in a locomotion group are valid       ----
//----------------------------------------------------------------------------------
void CAnimationSet::VerifyLMGs()
{
	u32 numAnimNames = m_arrAnimations.size();
	for (u32 i = 0; i < numAnimNames; i++)
	{
		//tukk pAnimName = m_arrAnimations[i].m_Name;
		if (m_arrAnimations[i].m_nAssetType != LMG_File)
			continue;
		u32 GlobalAnimationID = m_arrAnimations[i].m_nGlobalAnimId;
		GlobalAnimationHeaderLMG& rLMG = g_AnimationUpr.m_arrGlobalLMG[GlobalAnimationID];
		u32 IsCreated = rLMG.IsAssetCreated();//Loaded();
		if (IsCreated)
			continue;
		tukk strFilePath = rLMG.GetFilePath();
		tukk szExt = PathUtil::GetExt(strFilePath);
		u32 IsLMG = (strcmp(szExt, "lmg") == 0) || (strcmp(szExt, "bspace") == 0) || (strcmp(szExt, "comb") == 0);
		if (IsLMG == 0)
			continue;
		rLMG.OnAssetLMG();
		rLMG.LoadAndParseXML(this);
	}
}

#ifdef STORE_ANIMATION_NAMES
void CAnimationSet::VerifyAliases_Debug()
{
	DynArray<string> arrAnimNames;

	{
		FILE* pFile = ::fopen("c:/MultipleAliases_LMG.txt", "w");
		if (!pFile)
			return;
		fprintf(pFile, "-------------------------------\n");

		u32 numLMG = g_AnimationUpr.m_arrGlobalLMG.size();
		for (u32 id = 0; id < numLMG; id++)
		{
			arrAnimNames.clear();

			u32 numAnimNames = m_arrAnimations.size();
			for (u32 i = 0; i < numAnimNames; i++)
			{
				if (m_arrAnimations[i].m_nAssetType != LMG_File)
					continue;
				u32 nGlobalAnimId = m_arrAnimations[i].m_nGlobalAnimId;
				if (nGlobalAnimId != id)
					continue;
				arrAnimNames.push_back(m_arrAnimations[i].m_Name);
			}

			u32 numNames = arrAnimNames.size();
			if (numNames > 1)
			{
				fprintf(pFile, "filepath: %s\n", g_AnimationUpr.m_arrGlobalLMG[id].GetFilePath());
				for (u32 i = 0; i < numNames; i++)
					fprintf(pFile, "animname: %s   \n", arrAnimNames[i].c_str());
				fprintf(pFile, "\n\n");
			}

		}

		fprintf(pFile, "-------------------------------------------\n");
		::fclose(pFile);
	}

	{
		FILE* pFile = ::fopen("c:/MultipleAliases_CAF.txt", "w");
		if (!pFile)
			return;
		fprintf(pFile, "-------------------------------\n");

		u32 numCAF = g_AnimationUpr.m_arrGlobalCAF.size();
		for (u32 id = 0; id < numCAF; id++)
		{
			arrAnimNames.clear();

			u32 numAnimNames = m_arrAnimations.size();
			for (u32 i = 0; i < numAnimNames; i++)
			{
				if (m_arrAnimations[i].m_nAssetType != CAF_File)
					continue;
				u32 nGlobalAnimId = m_arrAnimations[i].m_nGlobalAnimId;
				if (nGlobalAnimId != id)
					continue;
				arrAnimNames.push_back(m_arrAnimations[i].m_Name);
			}

			u32 numNames = arrAnimNames.size();
			if (numNames > 1)
			{
				fprintf(pFile, "filepath: %s \n", g_AnimationUpr.m_arrGlobalCAF[id].GetFilePath());
				for (u32 i = 0; i < numNames; i++)
					fprintf(pFile, "animname: %s  \n", arrAnimNames[i].c_str());
				fprintf(pFile, "\n\n");
			}

		}

		fprintf(pFile, "-------------------------------------------\n");
		::fclose(pFile);
	}

}
#endif

size_t CAnimationSet::SizeOfAnimationSet() const
{
	size_t nSize = sizeof(CAnimationSet);

	nSize += m_arrAnimations.get_alloc_size();
	u32 numAnimations = m_arrAnimations.size();
	for (u32 i = 0; i < numAnimations; i++)
		nSize += m_arrAnimations[i].SizeOfAnimationHeader();

	nSize += m_AnimationHashMapKey.get_alloc_size();
	nSize += m_AnimationHashMapValue.get_alloc_size();

	for (FacialAnimationSet::const_iterator it = m_facialAnimations.begin(), end = m_facialAnimations.end(); it != end; ++it)
	{
		nSize += it->name.capacity();
		nSize += it->path.capacity();
	}

	for (HashToNameMap::const_iterator iter(m_hashToNameMap.begin()), end(m_hashToNameMap.end()); iter != end; ++iter)
	{
		nSize += sizeof(HashToNameMap::key_type);
		nSize += iter->second.capacity();
	}

	return nSize;
}

void CAnimationSet::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(m_arrAnimations);
	pSizer->AddObject(m_AnimationHashMapKey);
	pSizer->AddObject(m_AnimationHashMapValue);
	pSizer->AddObject(m_facialAnimations);
	pSizer->AddObject(m_hashToNameMap);
}

#ifdef EDITOR_PCDEBUGCODE
void CAnimationSet::GetSubAnimations(DynArray<i32>& animIdsOut, i32k animId) const
{
	const ModelAnimationHeader* pHeader = GetModelAnimationHeader(animId);
	if (!pHeader)
		return;

	if (pHeader->m_nAssetType != LMG_File)
		return;

	const GlobalAnimationHeaderLMG& bspaceHeader = g_AnimationUpr.m_arrGlobalLMG[pHeader->m_nGlobalAnimId];
	assert(bspaceHeader.IsAssetLMG());

	for (u32 i = 0; i < bspaceHeader.m_numExamples; ++i)
	{
		const BSParameter& parameter = bspaceHeader.m_arrParameter[i];
		i32k exampleAnimId = GetAnimIDByCRC(parameter.m_animName.m_CRC32);
		animIdsOut.push_back(exampleAnimId);
	}

	i32 numCombinedBlendspaces = bspaceHeader.m_arrCombinedBlendSpaces.size();
	for (i32 i = 0; i < numCombinedBlendspaces; ++i)
	{
		i32k bspaceId = bspaceHeader.m_arrCombinedBlendSpaces[i].m_ParaGroupID;
		if (bspaceId < 0)
			continue;

		const GlobalAnimationHeaderLMG& subBspaceHeader = g_AnimationUpr.m_arrGlobalLMG[bspaceId];
		for (u32 e = 0; e < subBspaceHeader.m_numExamples; ++e)
		{
			const BSParameter& parameter = subBspaceHeader.m_arrParameter[e];
			i32k exampleAnimId = GetAnimIDByCRC(parameter.m_animName.m_CRC32);
			animIdsOut.push_back(exampleAnimId);
		}
	}
}

bool CAnimationSet::IsBlendSpace(i32 nAnimationId) const
{
	const ModelAnimationHeader* pHeader = GetModelAnimationHeader(nAnimationId);
	if (!pHeader)
		return false;

	if (pHeader->m_nAssetType != LMG_File)
		return false;

	size_t headerIndex = pHeader->m_nGlobalAnimId;
	if (headerIndex >= g_AnimationUpr.m_arrGlobalLMG.size())
		return false;

	GlobalAnimationHeaderLMG& bspaceHeader = g_AnimationUpr.m_arrGlobalLMG[headerIndex];
	if (!bspaceHeader.IsAssetLMG())
		return false;

	return true;
}

bool CAnimationSet::IsCombinedBlendSpace(i32 nAnimationId) const
{
	const ModelAnimationHeader* pHeader = GetModelAnimationHeader(nAnimationId);
	if (!pHeader)
		return false;

	if (pHeader->m_nAssetType != LMG_File)
		return false;

	size_t headerIndex = pHeader->m_nGlobalAnimId;
	if (headerIndex >= g_AnimationUpr.m_arrGlobalLMG.size())
		return false;

	GlobalAnimationHeaderLMG& bspaceHeader = g_AnimationUpr.m_arrGlobalLMG[headerIndex];
	if (!bspaceHeader.IsAssetLMG())
		return false;

	return bspaceHeader.m_arrCombinedBlendSpaces.size() != 0;
}

bool CAnimationSet::GetMotionParameters(u32 nAnimationId, i32 exampleIndex, IDefaultSkeleton* pDefaultSkeleton, Vec4& outParameters) const
{
	const ModelAnimationHeader* pHeader = GetModelAnimationHeader(nAnimationId);
	if (!pHeader)
		return false;

	if (pHeader->m_nAssetType != LMG_File)
		return false;

	size_t headerIndex = pHeader->m_nGlobalAnimId;
	if (headerIndex >= g_AnimationUpr.m_arrGlobalLMG.size())
		return false;

	GlobalAnimationHeaderLMG& bspaceHeader = g_AnimationUpr.m_arrGlobalLMG[headerIndex];
	if (!bspaceHeader.IsAssetLMG())
		return false;

	if (exampleIndex >= bspaceHeader.m_arrParameter.size())
		return false;

	size_t numExamples = bspaceHeader.m_arrParameter.size();
	for (i32 i = 0; i < numExamples; ++i)
	{
		i32 animID = GetAnimIDByCRC(bspaceHeader.m_arrParameter[i].m_animName.m_CRC32);
		if (animID < 0)
			return false;
		i32 globalID = GetGlobalIDByAnimID_Fast(animID);
		if (globalID < 0)
			return false;
		GlobalAnimationHeaderCAF& rCAF = g_AnimationUpr.m_arrGlobalCAF[globalID];
		if (rCAF.IsAssetLoaded() == 0 && rCAF.IsAssetRequested() == 0)
			rCAF.LoadCAF();
		if (rCAF.IsAssetLoaded() == 0) // possible in DBA case
			return false;
	}

	for (i32 i = 0; i < bspaceHeader.m_Dimensions; ++i)
		bspaceHeader.ParameterExtraction(this, static_cast<CDefaultSkeleton*>(pDefaultSkeleton), bspaceHeader.m_DimPara[i].m_ParaID, i);

	const BSParameter& example = bspaceHeader.m_arrParameter[exampleIndex];
	outParameters = example.m_Para;
	return true;
}

bool CAnimationSet::GetMotionParameterRange(u32 nAnimationId, EMotionParamID paramID, float& outMin, float& outMax) const
{
	const ModelAnimationHeader* pHeader = GetModelAnimationHeader(nAnimationId);
	if (!pHeader)
		return false;

	if (pHeader->m_nAssetType != LMG_File)
		return false;

	size_t headerIndex = pHeader->m_nGlobalAnimId;
	if (headerIndex >= g_AnimationUpr.m_arrGlobalLMG.size())
		return false;

	GlobalAnimationHeaderLMG& bspaceHeader = g_AnimationUpr.m_arrGlobalLMG[headerIndex];
	if (!bspaceHeader.IsAssetLMG())
		return false;

	if (bspaceHeader.m_arrCombinedBlendSpaces.size() > 0)
	{
		float resultMin = FLT_MAX;
		float resultMax = -FLT_MAX;
		size_t numCombinedBlendSpaces = bspaceHeader.m_arrCombinedBlendSpaces.size();
		for (size_t i = 0; i < numCombinedBlendSpaces; ++i)
		{
			u32 combinedHeaderIndex = bspaceHeader.m_arrCombinedBlendSpaces[i].m_ParaGroupID;
			if (combinedHeaderIndex >= g_AnimationUpr.m_arrGlobalLMG.size())
				continue;

			GlobalAnimationHeaderLMG& combinedHeader = g_AnimationUpr.m_arrGlobalLMG[combinedHeaderIndex];
			for (u32 i = 0; i < sizeof(combinedHeader.m_DimPara) / sizeof(combinedHeader.m_DimPara[0]); ++i)
			{
				if (combinedHeader.m_DimPara[i].m_ParaID == paramID)
				{
					float bspaceMin = combinedHeader.m_DimPara[i].m_min;
					float bspaceMax = combinedHeader.m_DimPara[i].m_max;
					if (bspaceMin < resultMin)
						resultMin = bspaceMin;
					if (bspaceMax > resultMax)
						resultMax = bspaceMax;
				}
			}
		}

		if (resultMin != FLT_MAX && resultMax != -FLT_MAX)
		{
			outMin = resultMin;
			outMax = resultMax;
			return true;
		}
		return false;
	}
	else
	{
		for (u32 i = 0; i < sizeof(bspaceHeader.m_DimPara) / sizeof(bspaceHeader.m_DimPara[0]); ++i)
		{
			if (bspaceHeader.m_DimPara[i].m_ParaID == paramID)
			{
				outMin = bspaceHeader.m_DimPara[i].m_min;
				outMax = bspaceHeader.m_DimPara[i].m_max;
				return true;
			}
		}
		return false;
	}
}

#endif
