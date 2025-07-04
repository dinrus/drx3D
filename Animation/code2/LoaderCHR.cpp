// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/LoaderCHR.h>

#include <drx3D/Eng3D/I3DEngine.h>
#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Animation/ModelMesh.h>
#include <drx3D/Animation/ModelSkin.h>
#include <drx3D/Animation/Model.h>
#include<drx3D/Animation/FaceAnimation.h>
#include<drx3D/Animation/FacialModel.h>
#include<drx3D/Animation/FaceEffectorLibrary.h>
#include <drx3D/Animation/AnimEventLoader.h>
#include <drx3D/Animation/CharacterUpr.h>
#include <drx3D/CoreX/Platform/IPlatformOS.h>

namespace DrxCHRLoader_LoadNewCHR_Helpers
{

static bool FindFirstMesh(CMesh*& pMesh, CNodeCGF*& pGFXNode, CContentCGF* pContent, tukk filenameNoExt, i32 lod)
{
	u32 numNodes = pContent->GetNodeCount();
	pGFXNode = 0;
	for (u32 n = 0; n < numNodes; n++)
	{
		CNodeCGF* pNode = pContent->GetNode(n);
		if (pNode->type == CNodeCGF::NODE_MESH && pNode->pMesh)
		{
			pGFXNode = pNode;
			break;
		}
	}
	if (pGFXNode == 0)
	{
		g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, filenameNoExt, "Failed to Load Character file. GFXNode not found");
		return false;
	}

	pMesh = pGFXNode->pMesh;
	if (pMesh && pMesh->m_pBoneMapping == 0)
	{
		g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, filenameNoExt, "Failed to Load Character file. Skeleton-Initial-Positions are missing");
		return false;
	}

	return true;
}

} //endns DrxCHRLoader_LoadNewCHR_Helpers

bool DrxCHRLoader::BeginLoadCHRRenderMesh(CDefaultSkeleton* pSkel, const DynArray<CCharInstance*>& pCharInstances, EStreamTaskPriority estp)
{
	tukk szFilePath = pSkel->GetModelFilePath();

	LOADING_TIME_PROFILE_SECTION(g_pISystem);

	static_assert(sizeof(TFace) == 6, "Invalid type size!");

	MEMSTAT_CONTEXT_FMT(EMemStatContextTypes::MSC_CHR, 0, "LoadCharacter %s", szFilePath);

	DRX_DEFINE_ASSET_SCOPE(DRX_SKEL_FILE_EXT, szFilePath);

	tukk szExt = PathUtil::GetExt(szFilePath);
	m_strGeomFileNameNoExt.assign(szFilePath, *szExt ? szExt - 1 : szExt);

	if (m_strGeomFileNameNoExt.empty())
	{
		g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, szFilePath, "Wrong character filename");
		return false;
	}

	//--------------------------------------------------------------------------------------------

	CModelMesh* pModelMesh = pSkel->GetModelMesh();
	assert(pModelMesh);

	const stack_string filename = stack_string(m_strGeomFileNameNoExt.c_str()) + "." + szExt;
	const stack_string lodName = (pModelMesh->m_stream.bHasMeshFile) ? (filename + "m") : (filename);

	m_pModelSkel = pSkel;
	m_RefByInstances = pCharInstances;

	StreamReadParams params;
	params.nFlags = 0;//IStreamEngine::FLAGS_NO_SYNC_CALLBACK;
	params.dwUserData = 0;
	params.ePriority = estp;
	IStreamEngine* pStreamEngine = gEnv->pSystem->GetStreamEngine();
	m_pStream = pStreamEngine->StartRead(eStreamTaskTypeAnimation, lodName.c_str(), this, &params);

	return m_pStream != NULL;
}

void DrxCHRLoader::AbortLoad()
{
	if (m_pStream)
		m_pStream->Abort();
}

void DrxCHRLoader::StreamAsyncOnComplete(IReadStream* pStream, unsigned nError)
{
	if (!nError)
	{
		if (m_pModelSkel)
		{
			EndStreamSkel(pStream);
		}
		else if (m_pModelSkin)
		{
			EndStreamSkinAsync(pStream);
		}
	}

	pStream->FreeTemporaryMemory();
}

void DrxCHRLoader::EndStreamSkel(IReadStream* pStream)
{
	using namespace DrxCHRLoader_LoadNewCHR_Helpers;

	tukk lodName = pStream->GetName();

	SmartContentCGF pCGF = g_pI3DEngine->CreateChunkfileContent(lodName);
	bool bLoaded = g_pI3DEngine->LoadChunkFileContentFromMem(pCGF, pStream->GetBuffer(), pStream->GetBytesRead(), IStatObj::ELoadingFlagsJustGeometry, false, false);

	if (!bLoaded || !pCGF)
	{
		g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, lodName, "%s: The Chunk-Loader failed to load the file.", __FUNCTION__);
		return;
	}

	CContentCGF* pLodContent = pCGF;

	CNodeCGF* pMeshNode = 0;
	CMesh* pMesh = 0;
	if (!FindFirstMesh(pMesh, pMeshNode, pLodContent, m_strGeomFileNameNoExt.c_str(), 0))
		return;

	CModelMesh* pModelMesh = m_pModelSkel->GetModelMesh();
	assert(pModelMesh);

	m_pNewRenderMesh = pModelMesh->InitRenderMeshAsync(pMesh, m_pModelSkel->GetModelFilePath(), 0, m_arrNewRenderChunks);
}

void DrxCHRLoader::StreamOnComplete(IReadStream* pStream, unsigned nError)
{
	if (m_pModelSkel)
	{
		CModelMesh* pModelMesh = m_pModelSkel->GetModelMesh();
		assert(pModelMesh);

		pModelMesh->InitRenderMeshSync(m_arrNewRenderChunks, m_pNewRenderMesh);
		m_pNewRenderMesh = nullptr;
		m_arrNewRenderChunks.clear();

		pModelMesh->m_stream.pStreamer = nullptr;
	}
	else if (m_pModelSkin)
	{
		EndStreamSkinSync(pStream);

		i32 nRenderLod = (i32)pStream->GetParams().dwUserData;
		m_pModelSkin->m_arrModelMeshes[nRenderLod].m_stream.pStreamer = NULL;
	}
	m_pStream = NULL;
	delete this;
}

// cleans up the resources allocated during load
void DrxCHRLoader::ClearModel()
{
	m_strGeomFileNameNoExt = "";
	//	m_animListIDs.clear();
}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----              this belongs into another file (i.e.CDefaultSkeleton)   -------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
namespace SkelLoader_Helpers
{

struct LevelOfDetail
{
	CContentCGF* m_pContentCGF;
	CContentCGF* m_pContentMeshCGF;

	LevelOfDetail()
	{
		m_pContentCGF = 0;
		m_pContentMeshCGF = 0;
	}

	~LevelOfDetail()
	{
		if (m_pContentCGF)
			g_pI3DEngine->ReleaseChunkfileContent(m_pContentCGF), m_pContentCGF = 0;
		if (m_pContentMeshCGF)
			g_pI3DEngine->ReleaseChunkfileContent(m_pContentMeshCGF), m_pContentMeshCGF = 0;
	}
};

static bool InitializeBones(CDefaultSkeleton* pDefaultSkeleton, CSkinningInfo* pSkinningInfo, tukk szFilePath)
{
	i32k lod = 0;
	u32 numBones = pSkinningInfo->m_arrBonesDesc.size();
	assert(numBones);
	if (numBones >= MAX_JOINT_AMOUNT)
	{
		g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, szFilePath, "Too many Joints in model. Current Limit is: %d", MAX_JOINT_AMOUNT);
		return false;
	}

	if (!numBones)
	{
		g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, szFilePath, "Model has no Joints.");
		return false;
	}

	pDefaultSkeleton->m_poseDefaultData.Initialize(numBones);
	pDefaultSkeleton->m_arrModelJoints.resize(numBones);

	for (u32 nBone = 0; nBone < numBones; ++nBone)
	{
		const DrxBoneDescData& boneDesc = pSkinningInfo->m_arrBonesDesc[nBone];
		assert(boneDesc.m_DefaultB2W.IsOrthonormalRH());
		if (!boneDesc.m_DefaultB2W.IsOrthonormalRH())
		{
			g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, szFilePath, "Bone %d: B2W is not orthonormal RH in lod %d", nBone, lod);
			return false;
		}

		CDefaultSkeleton::SJoint& joint = pDefaultSkeleton->m_arrModelJoints[nBone];
		joint.m_idxParent = -1;
		i32 offset = boneDesc.m_nOffsetParent;
		if (offset)
			joint.m_idxParent = i32(nBone + offset);

		joint.m_strJointName = &boneDesc.m_arrBoneName[0];
		joint.m_nJointCRC32Lower = CCrc32::ComputeLowercase(&boneDesc.m_arrBoneName[0]);
		joint.m_nJointCRC32 = boneDesc.m_nControllerID;
		joint.m_PhysInfo = boneDesc.m_PhysInfo[0];
		joint.m_fMass = boneDesc.m_fMass;

		pDefaultSkeleton->m_poseDefaultData.GetJointsAbsolute()[nBone] = QuatT(boneDesc.m_DefaultB2W);
		pDefaultSkeleton->m_poseDefaultData.GetJointsAbsolute()[nBone].q.Normalize();
		pDefaultSkeleton->m_poseDefaultData.GetJointsRelative()[nBone] = pDefaultSkeleton->m_poseDefaultData.GetJointsAbsolute()[nBone];
		if (joint.m_idxParent >= 0)
			pDefaultSkeleton->m_poseDefaultData.GetJointsRelative()[nBone] = pDefaultSkeleton->m_poseDefaultData.GetJointsAbsolute()[joint.m_idxParent].GetInverted() * pDefaultSkeleton->m_poseDefaultData.GetJointsAbsolute()[nBone];
		pDefaultSkeleton->m_poseDefaultData.GetJointsRelative()[nBone].q.Normalize();
	}

	u32 nRootCounter = 0;
	for (u32 j = 0; j < numBones; j++)
	{
		i32 idxParent = pDefaultSkeleton->m_arrModelJoints[j].m_idxParent;
		if (idxParent < 0)
			nRootCounter++;   //count the root joints
	}
	if (nRootCounter != 1)
	{
		//something went wrong. this model has multiple roots
		for (u32 j = 0; j < numBones; j++)
			pDefaultSkeleton->m_arrModelJoints[j].m_PhysInfo.pPhysGeom = 0; //delete all "Chunk-IDs" to prevent the FatalError in the destructor.
		g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, szFilePath, "DinrusXAnimation: multiple root in skeleton. Please fix the model.");
		return false;
	}

	tukk RootName = pDefaultSkeleton->m_arrModelJoints[0].m_strJointName.c_str();
	if (RootName[0] == 'B' && RootName[1] == 'i' && RootName[2] == 'p' && RootName[3] == '0' && RootName[4] == '1')
	{
		//This character-model is a biped and it was made in CharacterStudio.
		//CharacterStudio is not allowing us to project the Bip01 on the ground and change its orientation; thats why we have to do it here.
		pDefaultSkeleton->m_poseDefaultData.GetJointsAbsolute()[0].SetIdentity();
		pDefaultSkeleton->m_poseDefaultData.GetJointsRelative()[0].SetIdentity();
		u32 numJoints = pDefaultSkeleton->m_arrModelJoints.size();
		for (u32 bone = 1; bone < numJoints; bone++)
		{
			//adjust the pelvis (and all other joints directly linked Bip01)
			i32 idxParent = pDefaultSkeleton->m_arrModelJoints[bone].m_idxParent;
			if (idxParent == 0)
				pDefaultSkeleton->m_poseDefaultData.GetJointsRelative()[bone] = pDefaultSkeleton->m_poseDefaultData.GetJointsAbsolute()[bone];
		}
	}

	QuatT orientation = pDefaultSkeleton->m_poseDefaultData.GetJointsAbsolute()[0];
	u32 IsIdentiy = Quat::IsEquivalent(Quat(IDENTITY), orientation.q, 0.14f);  // 0.14 radians == ~8.0 degrees
	if (IsIdentiy == 0)
	{
		g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, szFilePath, "DinrusXAnimation: Default-Skeleton has wrong orientation. Please use the drx3D conventions when building a model");
		//DRX_ASSERT_MESSAGE(!"Inverted model orientation!", strGeomFileName);
		return 0;
	}

	return true;
}

static tukk FindFirstMesh(CMesh*& pMesh, CNodeCGF*& pGFXNode, CContentCGF* pContent)
{
	u32 numNodes = pContent->GetNodeCount();
	pGFXNode = 0;
	for (u32 n = 0; n < numNodes; n++)
	{
		CNodeCGF* pNode = pContent->GetNode(n);
		if (pNode->type == CNodeCGF::NODE_MESH && pNode->pMesh)
		{
			pGFXNode = pNode;
			break;
		}
	}
	if (pGFXNode == 0)
	{
		return "Failed to Load Character file. GFXNode not found";
	}

	pMesh = pGFXNode->pMesh;
	if (pMesh && pMesh->m_pBoneMapping == 0)
	{
		return "Failed to Load Character file. Skeleton-Initial-Positions are missing";
	}

	return nullptr;
}

static tukk FindFirstMeshAndMaterial(CMesh*& pMesh, CNodeCGF*& pGFXNode, _smart_ptr<IMaterial>& pMaterial, CContentCGF* pContent, tukk filenameNoExt)
{
	pGFXNode = 0;
	for (i32 i = 0, limit = pContent->GetNodeCount(); i < limit; ++i)
	{
		CNodeCGF* pNode = pContent->GetNode(i);
		if (pNode->type == CNodeCGF::NODE_MESH)
		{
			pGFXNode = pNode;
			break;
		}
	}
	if (!pGFXNode)
	{
		return "Failed to Load Character file. GFXNode not found";
	}

	pMesh = pGFXNode->pMesh;
	if (pMesh && !pMesh->m_pBoneMapping)
	{
		return "Failed to Load Character file. Skeleton-Initial-Positions are missing";
	}

	if (!pMaterial)
	{
		if (pGFXNode->pMaterial)
			pMaterial = g_pI3DEngine->GetMaterialUpr()->LoadCGFMaterial(pGFXNode->pMaterial->name, PathUtil::GetPathWithoutFilename(filenameNoExt).c_str());
		else
			pMaterial = g_pI3DEngine->GetMaterialUpr()->GetDefaultMaterial();
	}
	return nullptr;
}

static tukk FindFirstMaterial(_smart_ptr<IMaterial>& pMaterial, CContentCGF* pContent, tukk szFilenameNoExt)
{
	if (!pContent->GetMaterialCount())
	{
		return "Failed to Load Character file. No material found for physicalization.";
	}

	pMaterial = g_pI3DEngine->GetMaterialUpr()->LoadCGFMaterial(pContent->GetMaterial(0)->name, PathUtil::GetPathWithoutFilename(szFilenameNoExt).c_str());
	
	return nullptr;
}

} //endns SkelLoader_Helpers

bool CDefaultSkeleton::LoadNewSKEL(tukk szFilePath, u32 nLoadingFlags)
{
	using namespace SkelLoader_Helpers;

	LOADING_TIME_PROFILE_SECTION_ARGS(szFilePath);

	static_assert(sizeof(TFace) == 6, "Invalid type size!");

	MEMSTAT_CONTEXT_FMT(EMemStatContextTypes::MSC_CHR, 0, "LoadCharacter %s", szFilePath);

	DRX_DEFINE_ASSET_SCOPE(DRX_SKEL_FILE_EXT, szFilePath);

	tukk szExt = PathUtil::GetExt(szFilePath);
	stack_string strGeomFileNameNoExt;
	strGeomFileNameNoExt.assign(szFilePath, *szExt ? szExt - 1 : szExt);
	if (strGeomFileNameNoExt.empty())
	{
		g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, szFilePath, "Wrong character filename");
		return 0;
	}

	//--------------------------------------------------------------------------------------------
#ifndef _RELEASE
	{
		u32 isSKEL = stricmp(szExt, DRX_SKEL_FILE_EXT) == 0;
		if (isSKEL)
		{
			string strFilePathLOD1 = strGeomFileNameNoExt + "_lod1" + "." + szExt;
			bool exist = g_pISystem->GetIPak()->IsFileExist(strFilePathLOD1.c_str());
			if (exist)
				g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, 0, "!DinrusXAnimation: LODs in CHRs are not supported any more. Please remove them from the build: %s", strFilePathLOD1.c_str());
		}
	}
#endif

	//--------------------------------------------------------------------------------------------

	LevelOfDetail cgfs; //in case of early exit we call the destructor automatically and delete all CContentCGFs
	cgfs.m_pContentCGF = g_pI3DEngine->CreateChunkfileContent(szFilePath);
	if (cgfs.m_pContentCGF == 0)
		DrxFatalError("DinrusXAnimation error: issue in Chunk-Loader");
	bool bLoaded = g_pI3DEngine->LoadChunkFileContent(cgfs.m_pContentCGF, szFilePath, 0);
	if (bLoaded == 0)
	{
		g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, szFilePath, "DinrusXAnimation: The Chunk-Loader failed to load the file.");
		return 0;
	}

	if (!Console::GetInst().ca_StreamCHR)
	{
		string lodmName = string(szFilePath) + "m";
		bool bHasMeshFile = gEnv->pDrxPak->IsFileExist(lodmName.c_str());
		if (bHasMeshFile)
		{
			cgfs.m_pContentMeshCGF = g_pI3DEngine->CreateChunkfileContent(lodmName);
			g_pI3DEngine->LoadChunkFileContent(cgfs.m_pContentMeshCGF, lodmName.c_str());
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	//--- initialize the ModelSkel
	//----------------------------------------------------------------------------------------------------------------------
	CSkinningInfo* pSkinningInfo = cgfs.m_pContentCGF->GetSkinningInfo();
	if (pSkinningInfo == 0)
	{
		g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, szFilePath, "DinrusXAnimation: Skinning info is missing in lod %d", 0);
		return 0;
	}

	//--------------------------------------------------------------------------
	//----   initialize the default animation skeleton
	//--------------------------------------------------------------------------
	u32k isSkeletonValid = InitializeBones(this, pSkinningInfo, szFilePath);

	CMesh* pMesh = nullptr;
	CNodeCGF* pMeshNode = nullptr;
	_smart_ptr<IMaterial> pMaterial = nullptr;
	FindFirstMeshAndMaterial(pMesh, pMeshNode, pMaterial, cgfs.m_pContentCGF, strGeomFileNameNoExt.c_str());

	if (!pMaterial)
	{
		tukk const szError = FindFirstMaterial(pMaterial, cgfs.m_pContentCGF, strGeomFileNameNoExt.c_str());
		if (szError)
		{
			g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, 0, "!DinrusXAnimation: Error while loading material: %s", szError);
		}
	}

	//------------------------------------------------------------------------------------
	//--- initialize Physical Proxies
	//------------------------------------------------------------------------------------
	if (1)
	{
		u32 numJoints = m_arrModelJoints.size();
		m_arrBackupPhysInfo.resize(numJoints);
		for (u32 j = 0; j < numJoints; j++)
			m_arrBackupPhysInfo[j] = m_arrModelJoints[j].m_PhysInfo;
		m_arrBackupPhyBoneMeshes = pSkinningInfo->m_arrPhyBoneMeshes;
		m_arrBackupBoneEntities = pSkinningInfo->m_arrBoneEntities;
		if (!SetupPhysicalProxies(m_arrBackupPhyBoneMeshes, m_arrBackupBoneEntities, pMaterial, szFilePath))
			return 0;
	}
	if (isSkeletonValid == 0)
		return 0;

	//------------------------------------------------------------------------------------

	VerifyHierarchy();

	if (1)
	{
		//----------------------------------------------------------------------------------------------------------------------------------
		//---  initialize ModelMesh MorphTargets and RenderMesh
		//---  There can be only one Render LOD in the Base-Model (Even this is optional. Usually the Base-Model has an empty skeleton)
		//----------------------------------------------------------------------------------------------------------------------------------

		if (cgfs.m_pContentMeshCGF)
		{
			const auto szError = FindFirstMesh(pMesh, pMeshNode, cgfs.m_pContentMeshCGF);
			if (szError)
			{
				g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, strGeomFileNameNoExt.c_str(), szError);
				return false;
			}
		}

		if (pMesh)
		{
			m_ModelMeshEnabled = true;

			CModelMesh* pModelMesh = GetModelMesh();
			assert(pModelMesh);

			u32 success = pModelMesh->InitMesh(pMesh, pMeshNode, pMaterial, pSkinningInfo, m_strFilePath.c_str(), 0);
			if (success == 0)
				return 0;

			string meshFilename = string(szFilePath) + 'm';
			pModelMesh->m_stream.bHasMeshFile = gEnv->pDrxPak->IsFileExist(meshFilename.c_str());
		}
	}

	//---------------------------------------------------------------------------------------------------------------

	InitializeHardcodedJointsProperty();
	RebuildJointLookupCaches();
	stack_string paramFileName = strGeomFileNameNoExt + "." + DRX_CHARACTER_PARAM_FILE_EXT;
	u32 isPrevMode = nLoadingFlags & CA_PreviewMode;
	if (isPrevMode == 0)
		LoadCHRPARAMS(paramFileName);

	m_animListIDs.clear();
	return 1;
}

void CDefaultSkeleton::LoadCHRPARAMS(tukk paramFileName)
{
	CParamLoader& paramLoader = g_pCharacterUpr->GetParamLoader();
#ifdef EDITOR_PCDEBUGCODE
	bool hasInjectedParams = paramLoader.HasInjectedCHRPARAMS(paramFileName);
#else
	bool hasInjectedParams = false;
#endif
	// If the new .params file exists, load it since it can be done much faster and the file is cleaner (all XML)
	if (hasInjectedParams || g_pIPak->IsFileExist(paramFileName))
	{
		// this will be refilled by LoadXML
		m_animListIDs.clear();

		bool ok = paramLoader.LoadXML(this, GetDefaultAnimDir(), paramFileName, m_animListIDs);
		if (ok)
			LoadAnimations(paramLoader);
	}
}

//------------------------------------------------------------------------------------------------------------
// loads animations for a model by composing a complete animation list and loading it into an AnimationSet
//------------------------------------------------------------------------------------------------------------
bool CDefaultSkeleton::LoadAnimations(CParamLoader& paramLoader)
{
	//LOADING_TIME_PROFILE_SECTION(g_pISystem);

	// the number of animations loaded
	u32 numAnimAssets = 0;

	i32 numAnims = 0;

	DRX_PROFILE_FUNCTION(PROFILE_ANIMATION);

	if (m_animListIDs.size() == 0)
	{
		return false;
	}

	// attributes of the compound list
	// all those attributes have to be stored on a per list basis,
	// keeping it for the topmost list in memory is not enough
	// since another list might use it as a dependency which does not
	// override it
	DynArray<string> modelTracksDatabases;
	DynArray<string> lockedDatabases;
	string animEventDatabase;
	string faceLibFile;
	string faceLibDir;
	CAnimationSet::FacialAnimationSet::container_type facialAnimations;

	i32k nListIDs = m_animListIDs.size();
	for (i32 i = nListIDs - 1; i >= 0; --i) // by walking backwards the 'parent' lists come first
	{
		u32k animListID = m_animListIDs[i];
		if (animListID < paramLoader.GetParsedListNumber())
		{
			const SAnimListInfo& animList = paramLoader.GetParsedList(animListID);
			numAnims += animList.arrAnimFiles.size();
			modelTracksDatabases.push_back(animList.modelTracksDatabases);
			lockedDatabases.push_back(animList.lockedDatabases);

			if (animEventDatabase.empty())
			{
				animEventDatabase = animList.animEventDatabase; // first one wins --> so outermost animeventdb wins
			}

			if (faceLibFile.empty())
			{
				faceLibFile = animList.faceLibFile;
				faceLibDir = animList.faceLibDir;
			}
		}
	}

	m_pAnimationSet->prepareLoadCAFs(numAnims);

	u32 numTracksDataBases = modelTracksDatabases.size();
	if (numTracksDataBases)
		g_AnimationUpr.CreateGlobalHeaderDBA(modelTracksDatabases);

	// Lock Persistent Databases
	DynArray<string>::iterator it;
	for (it = lockedDatabases.begin(); it != lockedDatabases.end(); ++it)
	{
		g_AnimationUpr.DBA_LockStatus(it->c_str(), 1, false);
	}

	if (!animEventDatabase.empty())
	{
		SetModelAnimEventDatabase(animEventDatabase);
	}

	if (!faceLibFile.empty())
	{
		CreateFacialInstance();
		LoadFaceLib(faceLibFile, faceLibDir, this);
	}

	for (i32 i = nListIDs - 1; i >= 0; --i)
	{
		const SAnimListInfo& animList = paramLoader.GetParsedList(m_animListIDs[i]);
		u32 numAnimNames = animList.arrAnimFiles.size();

		// this is where the Animation List Cache kicks in
		if (!animList.headersLoaded)
		{
			paramLoader.ExpandWildcards(m_animListIDs[i]);
			numAnimAssets += LoadAnimationFiles(paramLoader, m_animListIDs[i]);
			// this list has been processed, dont do it again!
			paramLoader.SetHeadersLoaded(m_animListIDs[i]);
		}
		else
		{
			numAnimAssets += ReuseAnimationFiles(paramLoader, m_animListIDs[i]);
		}

		if (!animList.facialAnimations.empty())
			facialAnimations.insert(facialAnimations.end(), animList.facialAnimations.begin(), animList.facialAnimations.end());
	}

	// Store the list of facial animations in the model animation set.
	if (!facialAnimations.empty())
		m_pAnimationSet->GetFacialAnimations().SwapElementsWithVector(facialAnimations);

	AnimEventLoader::LoadAnimationEventDatabase(this, GetModelAnimEventDatabaseCStr());

	u32 dddd = g_AnimationUpr.m_arrGlobalCAF.size();

	if (numAnimAssets > 1)
	{
		g_pILog->UpdateLoadingScreen("  %u animation-assets loaded (total assets: %d)", numAnimAssets, g_AnimationUpr.m_arrGlobalCAF.size());
	}

	if (Console::GetInst().ca_MemoryUsageLog)
	{
		DrxModuleMemoryInfo info;
		DrxGetMemoryInfoForModule(&info);
		g_pILog->UpdateLoadingScreen("Memstat %i", (i32)(info.allocated - info.freed));
	}

	// If there is a facial model, but no expression library, we should create an empty library for it.
	// When we assign the library to the facial model it will automatically be assigned the morphs as expressions.
	{
		CFacialModel* pFacialModel = GetFacialModel();
		CFacialEffectorsLibrary* pEffectorsLibrary = (pFacialModel ? pFacialModel->GetFacialEffectorsLibrary() : 0);
		if (pFacialModel && !pEffectorsLibrary)
		{
			CFacialAnimation* pFacialAnimationUpr = (g_pCharacterUpr ? g_pCharacterUpr->GetFacialAnimation() : 0);
			CFacialEffectorsLibrary* pNewLibrary = new CFacialEffectorsLibrary(pFacialAnimationUpr);
			const string& filePath = GetModelFilePath();
			if (!filePath.empty())
			{
				string libraryFilePath = PathUtil::ReplaceExtension(filePath, ".fxl");
				pNewLibrary->SetName(libraryFilePath);
			}
			if (pFacialModel && pNewLibrary)
				pFacialModel->AssignLibrary(pNewLibrary);
		}
	}

	if (Console::GetInst().ca_PrecacheAnimationSets)
	{
		CAnimationSet& animSet = *m_pAnimationSet;
		u32 animCount = animSet.GetAnimationCount();
		for (u32 i = 0; i < animCount; ++i)
		{
			const ModelAnimationHeader* pHeader = animSet.GetModelAnimationHeader(i);
			if (pHeader)
			{
				if (pHeader->m_nAssetType == CAF_File)
				{
					u32 globalID = pHeader->m_nGlobalAnimId;
					if (globalID > 0)
					{
						PREFAST_ASSUME(g_pCharacterUpr);
						g_AnimationUpr.m_arrGlobalCAF[globalID].StartStreamingCAF();
					}
				}
			}
		}
	}

	m_pAnimationSet->VerifyLMGs();

	return m_arrModelJoints.size() != 0;
}

void CDefaultSkeleton::LoadFaceLib(tukk faceLibFile, tukk animDirName, CDefaultSkeleton* pDefaultSkeleton)
{
	tukk pFilePath = GetModelFilePath();
	tukk szExt = PathUtil::GetExt(pFilePath);
	stack_string strGeomFileNameNoExt;
	strGeomFileNameNoExt.assign(pFilePath, *szExt ? szExt - 1 : szExt);

	LOADING_TIME_PROFILE_SECTION(g_pISystem);
	// Facial expressions library.
	_smart_ptr<IFacialEffectorsLibrary> pLib;

	pLib = g_pCharacterUpr->GetIFacialAnimation()->LoadEffectorsLibrary(faceLibFile);
	if (!pLib)
	{
		// Search in animation directory .chr file directory.
		pLib = g_pCharacterUpr->GetIFacialAnimation()->LoadEffectorsLibrary(
		  PathUtil::Make(animDirName, faceLibFile));
	}
	if (!pLib)
	{
		// Search in .chr file directory.
		pLib = g_pCharacterUpr->GetIFacialAnimation()->LoadEffectorsLibrary(
		  PathUtil::Make(PathUtil::GetParentDirectory(strGeomFileNameNoExt), stack_string(faceLibFile)));
	}
	if (pLib)
	{
		if (pDefaultSkeleton->GetFacialModel())
		{
			pDefaultSkeleton->GetFacialModel()->AssignLibrary(pLib);
		}
	}
	else
	{
		gEnv->pLog->LogError("DinrusXAnimation: Facial Effector Library %s not found (when loading %s.chrparams)", faceLibFile, strGeomFileNameNoExt.c_str());
	}
}

//-------------------------------------------------------------------------------------------------
#define MAX_STRING_LENGTH (256)

u32 CDefaultSkeleton::ReuseAnimationFiles(CParamLoader& paramLoader, u32 listID)
{
	LOADING_TIME_PROFILE_SECTION(g_pISystem);

	const SAnimListInfo& animList = paramLoader.GetParsedList(listID);
	i32 numAnims = animList.arrLoadedAnimFiles.size();
	u32 result = 0;

	m_pAnimationSet->PrepareToReuseAnimations(numAnims);

	for (i32 i = 0; i < numAnims; ++i)
	{
		const ModelAnimationHeader& animHeader = animList.arrLoadedAnimFiles[i];

		m_pAnimationSet->ReuseAnimation(animHeader);
		result++;
	}

	return result;
}

u32 CDefaultSkeleton::LoadAnimationFiles(CParamLoader& paramLoader, u32 listID)
{
	tukk pFullFilePath = GetModelFilePath();
	tukk szExt = PathUtil::GetExt(pFullFilePath);
	stack_string strGeomFileNameNoExt;
	strGeomFileNameNoExt.assign(pFullFilePath, *szExt ? szExt - 1 : szExt);

	LOADING_TIME_PROFILE_SECTION(g_pISystem);
	// go through all Anims on stack and load them
	u32 nAnimID = 0;
	i32 numAnims = 0;

	const SAnimListInfo& animList = paramLoader.GetParsedList(listID);

#if BLENDSPACE_VISUALIZATION
	tukk strInternalType_Para1D = "InternalPara1D";
	i32 nDefAnimID = m_pAnimationSet->GetAnimIDByName(strInternalType_Para1D);
	if (nDefAnimID < 0)
	{
		i32 nGAnimID = m_pAnimationSet->LoadFileLMG("InternalPara1D.LMG", strInternalType_Para1D);
		if (nGAnimID)
			DrxFatalError("The GlobalID of the default Para1D must be 0");
		i32 nAnimID0 = m_pAnimationSet->GetAnimIDByName(strInternalType_Para1D);
		if (nAnimID0 >= 0)
		{
			ModelAnimationHeader* pAnim = (ModelAnimationHeader*)m_pAnimationSet->GetModelAnimationHeader(nAnimID0);
			if (pAnim)
			{
				nAnimID++;
				paramLoader.AddLoadedHeader(listID, *pAnim);    // store ModelAnimationHeader to reuse it in the case that a second model will need it
			}
		}
	}
#endif

	numAnims = animList.arrAnimFiles.size();
	for (i32 i = 0; i < numAnims; i++)
	{
		tukk pFilePath = animList.arrAnimFiles[i].m_FilePathQ;
		tukk pAnimName = animList.arrAnimFiles[i].m_AnimNameQ;
		tukk fileExt = PathUtil::GetExt(pFilePath);

		bool IsBAD = (0 == stricmp(fileExt, ""));
		if (IsBAD)
			continue;

		bool IsCHR = (0 == stricmp(fileExt, DRX_SKEL_FILE_EXT));
		if (IsCHR)
			continue;
		bool IsSKIN = (0 == stricmp(fileExt, DRX_SKIN_FILE_EXT));
		if (IsSKIN)
			continue;
		bool IsCDF = (0 == stricmp(fileExt, "cdf"));
		if (IsCDF)
			continue;
		bool IsCGA = (0 == stricmp(fileExt, "cga"));
		if (IsCGA)
			continue;

		u32 IsCAF = (stricmp(fileExt, "caf") == 0);
		u32 IsLMG = (stricmp(fileExt, "lmg") == 0) || (stricmp(fileExt, "bspace") == 0) || (stricmp(fileExt, "comb") == 0);
		assert(IsCAF + IsLMG);

		bool IsAimPose = 0;
		bool IsLookPose = 0;
		if (IsCAF)
		{
			u32 numAimDB = m_poseBlenderAimDesc.m_blends.size();
			for (u32 d = 0; d < numAimDB; d++)
			{
				tukk strAimIK_Token = m_poseBlenderAimDesc.m_blends[d].m_AnimToken;
				IsAimPose = (DrxStringUtils::stristr(pAnimName, strAimIK_Token) != 0);
				if (IsAimPose)
					break;
			}
			u32 numLookDB = m_poseBlenderLookDesc.m_blends.size();
			for (u32 d = 0; d < numLookDB; d++)
			{
				tukk strLookIK_Token = m_poseBlenderLookDesc.m_blends[d].m_AnimToken;
				IsLookPose = (DrxStringUtils::stristr(pAnimName, strLookIK_Token) != 0);
				if (IsLookPose)
					break;
			}
		}

		if (IsCAF && IsAimPose)
			IsCAF = 0;
		if (IsCAF && IsLookPose)
			IsCAF = 0;

		// ben: it can happen that a AnimID is defined in a list and in one of its dependencies that
		// was included after the definition of that animation, this is a different behavior than
		// before, so errors might pop up that did not pop up before
		DrxFixedStringT<256> standupAnimType;

		i32 nGlobalAnimID = -1;
		if (IsCAF)
		{
			nGlobalAnimID = m_pAnimationSet->LoadFileCAF(pFilePath, pAnimName);
		}

		if (IsAimPose || IsLookPose)
		{
			nGlobalAnimID = m_pAnimationSet->LoadFileAIM(pFilePath, pAnimName, this);
		}

		if (IsLMG)
		{
			nGlobalAnimID = m_pAnimationSet->LoadFileLMG(pFilePath, pAnimName);
		}

		i32 nAnimID0 = m_pAnimationSet->GetAnimIDByName(pAnimName);
		if (nAnimID0 >= 0)
		{
			ModelAnimationHeader* pAnim = (ModelAnimationHeader*)m_pAnimationSet->GetModelAnimationHeader(nAnimID0);
			if (pAnim)
			{
				nAnimID++;
				// store ModelAnimationHeader to reuse it in the case that a second model will need it
				paramLoader.AddLoadedHeader(listID, *pAnim);
			}
		}
		else
		{
			gEnv->pLog->LogError("DinrusXAnimation: Animation (caf) file \"%s\" could not be read (it's an animation of \"%s.%s\")", animList.arrAnimFiles[i].m_FilePathQ, strGeomFileNameNoExt.c_str(), DRX_SKEL_FILE_EXT);
		}
	}

	return nAnimID;
}

const string CDefaultSkeleton::GetDefaultAnimDir()
{
	tukk pFilePath = GetModelFilePath();
	tukk szExt = PathUtil::GetExt(pFilePath);
	stack_string strGeomFileNameNoExt;
	strGeomFileNameNoExt.assign(pFilePath, *szExt ? szExt - 1 : szExt);

	stack_string animDirName = PathUtil::GetParentDirectory(strGeomFileNameNoExt, 3);
	if (!animDirName.empty())
		return animDirName += "/animations";
	else
		return animDirName += "animations";
}
