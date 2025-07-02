// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Eng3D/StdAfx.h>

#if defined(RESOURCE_COMPILER) || defined(INCLUDE_SAVECGF)

	#include <drx3D/Eng3D/CGFSaver.h>
	#include <drx3D/Eng3D/ChunkData.h>
	#include <drx3D/Eng3D/ChunkFile.h>
	#include <drx3D/CoreX/Math/QTangent.h>
	#include <drx3D/Eng3D/VClothSaver.h>

	#if defined(RESOURCE_COMPILER)

		#include <drx3D/Tools/RC/ResourceCompilerPC/CGF/LoaderCAF.h>
		#include <drx3D/Tools/RC/ResourceCompilerPC/CGF/AnimSaver.h>

	#endif

	#define SCALE_TO_CGF 100.0f

//////////////////////////////////////////////////////////////////////////
CSaverCGF::CSaverCGF(CChunkFile& chunkFile)
{
	assert(&chunkFile);
	m_pChunkFile = &chunkFile;
	m_bDoNotSaveMeshData = false;
	m_bDoNotSaveNonMeshData = false;
	m_bSavePhysicsMeshes = true;
	m_bCompactVertexStreams = false;
	m_bComputeSubsetTexelDensity = false;
	m_pCGF = 0;
}

//////////////////////////////////////////////////////////////////////////
void CSaverCGF::SetMeshDataSaving(bool bEnable)
{
	m_bDoNotSaveMeshData = !bEnable;
}

void CSaverCGF::SetNonMeshDataSaving(bool bEnable)
{
	m_bDoNotSaveNonMeshData = !bEnable;
}

void CSaverCGF::SetSavePhysicsMeshes(bool bEnable)
{
	m_bSavePhysicsMeshes = bEnable;
}

void CSaverCGF::SetVertexStreamCompacting(bool bEnable)
{
	m_bCompactVertexStreams = bEnable;
}

void CSaverCGF::SetSubsetTexelDensityComputing(bool bEnable)
{
	m_bComputeSubsetTexelDensity = bEnable;
}

//////////////////////////////////////////////////////////////////////////
void CSaverCGF::SaveContent(
  CContentCGF* pCGF,
  bool bSwapEndian,
  bool bStorePositionsAsF16,
  bool bUseQtangents,
  bool bStoreIndicesAsU16)
{
	SetContent(pCGF);

	SaveExportFlags(bSwapEndian);
	SaveMaterials(bSwapEndian);
	#if defined(RESOURCE_COMPILER)
	SaveNodes(bSwapEndian, bStorePositionsAsF16, bUseQtangents, bStoreIndicesAsU16, 0);
	#else
	SaveNodes(bSwapEndian, bStorePositionsAsF16, bUseQtangents, bStoreIndicesAsU16);
	#endif
	SaveBreakablePhysics(bSwapEndian);
	SaveFoliage();
}

//////////////////////////////////////////////////////////////////////////
void CSaverCGF::SetContent(CContentCGF* pCGF)
{
	m_pCGF = pCGF;
}

//////////////////////////////////////////////////////////////////////////
i32 CSaverCGF::SaveCompiledBones(bool bSwapEndian, uk pData, i32 nSize, i32 version)
{
	COMPILED_BONE_CHUNK_DESC_0800 chunk;
	ZeroStruct(chunk);
	SwapEndian(chunk, bSwapEndian);

	CChunkData chunkData;
	chunkData.Add(chunk);
	chunkData.AddData(pData, nSize);

	return m_pChunkFile->AddChunk(
	  ChunkType_CompiledBones,
	  version,
	  (bSwapEndian ? eEndianness_NonNative : eEndianness_Native),
	  chunkData.GetData(), chunkData.GetSize());
}

//////////////////////////////////////////////////////////////////////////
i32 CSaverCGF::SaveCompiledPhysicalBones(bool bSwapEndian, uk pData, i32 nSize, i32 version)
{
	COMPILED_PHYSICALBONE_CHUNK_DESC_0800 chunk;
	ZeroStruct(chunk);
	SwapEndian(chunk, bSwapEndian);

	CChunkData chunkData;
	chunkData.Add(chunk);
	chunkData.AddData(pData, nSize);

	return m_pChunkFile->AddChunk(
	  ChunkType_CompiledPhysicalBones,
	  version,
	  (bSwapEndian ? eEndianness_NonNative : eEndianness_Native),
	  chunkData.GetData(), chunkData.GetSize());
}

//////////////////////////////////////////////////////////////////////////
i32 CSaverCGF::SaveCompiledPhysicalProxis(bool bSwapEndian, uk pData, i32 nSize, u32 numPhysicalProxies, i32 version)
{
	COMPILED_PHYSICALPROXY_CHUNK_DESC_0800 chunk;
	ZeroStruct(chunk);
	chunk.numPhysicalProxies = numPhysicalProxies;
	SwapEndian(chunk, bSwapEndian);

	CChunkData chunkData;
	chunkData.Add(chunk);
	chunkData.AddData(pData, nSize);

	return m_pChunkFile->AddChunk(
	  ChunkType_CompiledPhysicalProxies,
	  version,
	  (bSwapEndian ? eEndianness_NonNative : eEndianness_Native),
	  chunkData.GetData(), chunkData.GetSize());
}

//////////////////////////////////////////////////////////////////////////
i32 CSaverCGF::SaveCompiledMorphTargets(bool bSwapEndian, uk pData, i32 nSize, u32 numMorphTargets, i32 version)
{
	COMPILED_MORPHTARGETS_CHUNK_DESC_0800 chunk;
	ZeroStruct(chunk);
	chunk.numMorphTargets = numMorphTargets;
	SwapEndian(chunk, bSwapEndian);

	CChunkData chunkData;
	chunkData.Add(chunk);
	chunkData.AddData(pData, nSize);

	return m_pChunkFile->AddChunk(
	  ChunkType_CompiledMorphTargets,
	  version,
	  (bSwapEndian ? eEndianness_NonNative : eEndianness_Native),
	  chunkData.GetData(), chunkData.GetSize());
}

//////////////////////////////////////////////////////////////////////////
i32 CSaverCGF::SaveCompiledIntSkinVertices(bool bSwapEndian, uk pData, i32 nSize, i32 version)
{
	COMPILED_INTSKINVERTICES_CHUNK_DESC_0800 chunk;
	ZeroStruct(chunk);
	SwapEndian(chunk, bSwapEndian);

	CChunkData chunkData;
	chunkData.Add(chunk);
	chunkData.AddData(pData, nSize);

	return m_pChunkFile->AddChunk(
	  ChunkType_CompiledIntSkinVertices,
	  version,
	  (bSwapEndian ? eEndianness_NonNative : eEndianness_Native),
	  chunkData.GetData(), chunkData.GetSize());
}

//////////////////////////////////////////////////////////////////////////
i32 CSaverCGF::SaveCompiledIntFaces(bool bSwapEndian, uk pData, i32 nSize, i32 version)
{
	CChunkData chunkData;
	chunkData.AddData(pData, nSize);

	return m_pChunkFile->AddChunk(
	  ChunkType_CompiledIntFaces,
	  version,
	  (bSwapEndian ? eEndianness_NonNative : eEndianness_Native),
	  chunkData.GetData(), chunkData.GetSize());
}

//////////////////////////////////////////////////////////////////////////
i32 CSaverCGF::SaveCompiledBoneBox(bool bSwapEndian, uk pData, i32 nSize, i32 version)
{
	CChunkData chunkData;
	chunkData.AddData(pData, nSize);

	return m_pChunkFile->AddChunk(
	  ChunkType_BonesBoxes,
	  version,
	  (bSwapEndian ? eEndianness_NonNative : eEndianness_Native),
	  chunkData.GetData(), chunkData.GetSize());
}

//////////////////////////////////////////////////////////////////////////
i32 CSaverCGF::SaveCompiledExt2IntMap(bool bSwapEndian, uk pData, i32 nSize, i32 version)
{
	CChunkData chunkData;
	chunkData.AddData(pData, nSize);

	return m_pChunkFile->AddChunk(
	  ChunkType_CompiledExt2IntMap,
	  version,
	  (bSwapEndian ? eEndianness_NonNative : eEndianness_Native),
	  chunkData.GetData(), chunkData.GetSize());
}

//////////////////////////////////////////////////////////////////////////
i32 CSaverCGF::SaveBones(bool bSwapEndian, uk pData, i32 numBones, i32 nSize)
{
	BONEANIM_CHUNK_DESC_0290 chunk;
	ZeroStruct(chunk);
	chunk.nBones = numBones;
	SwapEndian(chunk, bSwapEndian);

	CChunkData chunkData;
	chunkData.Add(chunk);
	chunkData.AddData(pData, nSize);

	return m_pChunkFile->AddChunk(
	  ChunkType_BoneAnim,
	  BONEANIM_CHUNK_DESC_0290::VERSION,
	  (bSwapEndian ? eEndianness_NonNative : eEndianness_Native),
	  chunkData.GetData(), chunkData.GetSize());
}

//////////////////////////////////////////////////////////////////////////
i32 CSaverCGF::SaveBoneNames(bool bSwapEndian, tuk boneList, i32 numBones, i32 listSize)
{
	BONENAMELIST_CHUNK_DESC_0745 chunk;
	ZeroStruct(chunk);
	chunk.numEntities = numBones;
	SwapEndian(chunk, bSwapEndian);

	CChunkData chunkData;
	chunkData.Add(chunk);
	chunkData.AddData(boneList, listSize);

	return m_pChunkFile->AddChunk(
	  ChunkType_BoneNameList,
	  BONENAMELIST_CHUNK_DESC_0745::VERSION,
	  (bSwapEndian ? eEndianness_NonNative : eEndianness_Native),
	  chunkData.GetData(), chunkData.GetSize());
}

//////////////////////////////////////////////////////////////////////////
i32 CSaverCGF::SaveBoneInitialMatrices(bool bSwapEndian, SBoneInitPosMatrix* matrices, i32 numBones, i32 nSize)
{
	BONEINITIALPOS_CHUNK_DESC_0001 chunk;
	ZeroStruct(chunk);
	chunk.numBones = numBones;
	SwapEndian(chunk, bSwapEndian);

	CChunkData chunkData;
	chunkData.Add(chunk);
	chunkData.AddData(matrices, nSize);

	return m_pChunkFile->AddChunk(
	  ChunkType_BoneInitialPos,
	  BONEINITIALPOS_CHUNK_DESC_0001::VERSION,
	  (bSwapEndian ? eEndianness_NonNative : eEndianness_Native),
	  chunkData.GetData(), chunkData.GetSize());
}

//////////////////////////////////////////////////////////////////////////
i32 CSaverCGF::SaveBoneMesh(bool bSwapEndian, const PhysicalProxy& proxy)
{
	// uncompiled mesh chunk
	MESH_CHUNK_DESC_0745 chunk;
	ZeroStruct(chunk);

	i32 numVertices = proxy.m_arrPoints.size();
	i32 numFaces = proxy.m_arrMaterials.size();

	chunk.nFaces = numFaces;
	chunk.nTVerts = 0;
	chunk.nVerts = numVertices;
	chunk.VertAnimID = -1;

	// add chunk members
	CChunkData chunkData;
	chunkData.Add(chunk);

	// copy positions and vertices to a DrxVertex array -----------------------------------
	DrxVertex* vertices = new DrxVertex[numVertices];
	for (i32 i = 0; i < numVertices; i++)
	{
		vertices[i].p = proxy.m_arrPoints[i] * 100.0f;
		vertices[i].n = Vec3(ZERO);
	}
	chunkData.AddData(vertices, numVertices * sizeof(DrxVertex));
	SAFE_DELETE_ARRAY(vertices);

	// copy faces to a DrxFace array ------------------------------------------------------
	DrxFace* faces = new DrxFace[numFaces];
	for (i32 i = 0; i < numFaces; i++)
	{
		faces[i].v0 = proxy.m_arrIndices[i * 3 + 0];
		faces[i].v1 = proxy.m_arrIndices[i * 3 + 1];
		faces[i].v2 = proxy.m_arrIndices[i * 3 + 2];
		faces[i].MatID = proxy.m_arrMaterials[i];
	}
	chunkData.AddData(faces, numFaces * sizeof(DrxFace));
	SAFE_DELETE_ARRAY(faces);

	i32 nMeshChunkId = m_pChunkFile->AddChunk(
	  ChunkType_BoneMesh,
	  MESH_CHUNK_DESC_0745::VERSION,
	  eEndianness_Native,
	  chunkData.GetData(), chunkData.GetSize());

	return nMeshChunkId;
}

//////////////////////////////////////////////////////////////////////////
void CSaverCGF::SaveNodes(
  bool bSwapEndian,
  bool bStorePositionsAsF16,
  bool bUseQtangents,
  bool bStoreIndicesAsU16
	#if defined(RESOURCE_COMPILER)
  , CInternalSkinningInfo* pSkinningInfo
	#endif
  )
{
	m_savedNodes.clear();
	i32 numNodes = m_pCGF->GetNodeCount();
	for (i32 i = 0; i < numNodes; i++)
	{
		CNodeCGF* pNode = m_pCGF->GetNode(i);
		tukk pNodeName = pNode->name;

		// Check if not yet saved.
		u32 numSavedNodes = m_savedNodes.size();
		//	if (m_savedNodes.find(pNode) == m_savedNodes.end())
	#if defined(RESOURCE_COMPILER)
		SaveNode(pNode, bSwapEndian, bStorePositionsAsF16, bUseQtangents, bStoreIndicesAsU16, pSkinningInfo);
	#else
		SaveNode(pNode, bSwapEndian, bStorePositionsAsF16, bUseQtangents, bStoreIndicesAsU16);
	#endif
	}
}

//////////////////////////////////////////////////////////////////////////
	#if defined(RESOURCE_COMPILER)
void CSaverCGF::SaveUncompiledNodes()
{
	m_savedNodes.clear();
	i32 numNodes = m_pCGF->GetNodeCount();
	for (i32 i = 0; i < numNodes; i++)
	{
		CNodeCGF* pNode = m_pCGF->GetNode(i);
		SaveUncompiledNode(pNode);
	}
}
	#endif

//////////////////////////////////////////////////////////////////////////
i32 CSaverCGF::SaveNode(
  CNodeCGF* const pNode,
  const bool bSwapEndian,
  const bool bStorePositionsAsF16,
  const bool bUseQtangents,
  const bool bStoreIndicesAsU16
	#if defined(RESOURCE_COMPILER)
  , CInternalSkinningInfo* const pSkinningInfo
	#endif
  )
{
	if (m_savedNodes.find(pNode) != m_savedNodes.end())
	{
		return pNode->nChunkId;
	}

	if (m_bDoNotSaveNonMeshData && (pNode->bPhysicsProxy || !pNode->pMesh))
	{
		return -1;
	}

	m_savedNodes.insert(pNode);

	NODE_CHUNK_DESC_0824 chunk;
	ZeroStruct(chunk);

	for (i32 i = 0; i < m_pCGF->GetNodeCount(); ++i)
	{
		CNodeCGF* const pOtherNode = m_pCGF->GetNode(i);
		if (pOtherNode->pParent == pNode)
		{
			chunk.nChildren++;
		}
	}

	drx_strcpy(chunk.name, pNode->name);

	// Set matrix to node chunk.
	float* pMat = (float*)&chunk.tm;
	pMat[0] = pNode->localTM(0, 0);
	pMat[1] = pNode->localTM(1, 0);
	pMat[2] = pNode->localTM(2, 0);
	pMat[4] = pNode->localTM(0, 1);
	pMat[5] = pNode->localTM(1, 1);
	pMat[6] = pNode->localTM(2, 1);
	pMat[8] = pNode->localTM(0, 2);
	pMat[9] = pNode->localTM(1, 2);
	pMat[10] = pNode->localTM(2, 2);
	pMat[12] = pNode->localTM.GetTranslation().x * SCALE_TO_CGF;
	pMat[13] = pNode->localTM.GetTranslation().y * SCALE_TO_CGF;
	pMat[14] = pNode->localTM.GetTranslation().z * SCALE_TO_CGF;

	if (pNode->pMaterial)
	{
		chunk.MatID = pNode->pMaterial->nChunkId;
	}

	chunk.ObjectID = -1;
	chunk.ParentID = -1;

	if (pNode->pParent)
	{
	#if defined(RESOURCE_COMPILER)
		pNode->nParentChunkId = SaveNode(pNode->pParent, bSwapEndian, bStorePositionsAsF16, bUseQtangents, bStoreIndicesAsU16, pSkinningInfo);
	#else
		pNode->nParentChunkId = SaveNode(pNode->pParent, bSwapEndian, bStorePositionsAsF16, bUseQtangents, bStoreIndicesAsU16);
	#endif
		chunk.ParentID = pNode->nParentChunkId;
	}

	if (pNode->type == CNodeCGF::NODE_MESH ||
	    (pNode->type == CNodeCGF::NODE_HELPER && pNode->helperType == HP_GEOMETRY))
	{
		pNode->nObjectChunkId = SaveNodeMesh(pNode, bSwapEndian, bStorePositionsAsF16, bUseQtangents, bStoreIndicesAsU16);
	}
	else if (pNode->type == CNodeCGF::NODE_HELPER)
	{
		pNode->nObjectChunkId = SaveHelperChunk(pNode, bSwapEndian);
	}

	i32 nextChunk = m_pChunkFile->NumChunks();

	i32 positionIndex = pNode->pos_cont_id;
	i32 rotationIndex = pNode->rot_cont_id;
	i32 scaleIndex = pNode->scl_cont_id;

	#if defined(RESOURCE_COMPILER)
	i32 controllerIndex = -1;
	if (pSkinningInfo)
	{
		// Get the node index.
		for (i32 i = 0; i < m_pCGF->GetNodeCount(); ++i)
		{
			CNodeCGF* const pOtherNode = m_pCGF->GetNode(i);
			if (pOtherNode == pNode)
			{
				controllerIndex = 3 * i;
				break;
			}
		}

		assert(controllerIndex > -1);

		if (pSkinningInfo->m_arrControllers[controllerIndex].m_controllertype == 0x55)
		{
			positionIndex = ++nextChunk;
		}

		if (pSkinningInfo->m_arrControllers[controllerIndex + 1].m_controllertype == 0xaa)
		{
			rotationIndex = ++nextChunk;
		}

		if (pSkinningInfo->m_arrControllers[controllerIndex + 2].m_controllertype == 0x55)
		{
			scaleIndex = ++nextChunk;
		}
	}
	#endif

	chunk.pos_cont_id = positionIndex;
	chunk.rot_cont_id = rotationIndex;
	chunk.scl_cont_id = scaleIndex;

	chunk.ObjectID = pNode->nObjectChunkId;
	chunk.PropStrLen = pNode->properties.length();

	i32k PropLen = chunk.PropStrLen;

	SwapEndian(chunk, bSwapEndian);

	CChunkData chunkData;
	chunkData.Add(chunk);
	// Copy property string
	chunkData.AddData(pNode->properties.c_str(), PropLen);

	pNode->nChunkId = m_pChunkFile->AddChunk(
	  ChunkType_Node,
	  NODE_CHUNK_DESC_0824::VERSION,
	  (bSwapEndian ? eEndianness_NonNative : eEndianness_Native),
	  chunkData.GetData(), chunkData.GetSize());

	#if defined(RESOURCE_COMPILER)
	if (pSkinningInfo)
	{
		if (positionIndex != -1)
		{
			SaveTCB3Track(pSkinningInfo, pSkinningInfo->m_arrControllers[controllerIndex].m_index);
		}
		if (rotationIndex != -1)
		{
			SaveTCBQTrack(pSkinningInfo, pSkinningInfo->m_arrControllers[controllerIndex + 1].m_index);
		}
		if (scaleIndex != -1)
		{
			SaveTCB3Track(pSkinningInfo, pSkinningInfo->m_arrControllers[controllerIndex + 2].m_index);
		}
	}
	#endif

	return pNode->nChunkId;
}

//////////////////////////////////////////////////////////////////////////
	#if defined(RESOURCE_COMPILER)
i32 CSaverCGF::SaveUncompiledNode(CNodeCGF* pNode)
{
	if (m_savedNodes.find(pNode) != m_savedNodes.end())
	{
		return pNode->nChunkId;
	}

	m_savedNodes.insert(pNode);

	NODE_CHUNK_DESC_0824 chunk;
	ZeroStruct(chunk);

	for (i32 i = 0; i < m_pCGF->GetNodeCount(); i++)
	{
		CNodeCGF* pOtherNode = m_pCGF->GetNode(i);
		if (pOtherNode->pParent == pNode)
			chunk.nChildren++;
	}

	drx_strcpy(chunk.name, pNode->name);

	chunk.pos_cont_id = pNode->pos_cont_id;
	chunk.rot_cont_id = pNode->rot_cont_id;
	chunk.scl_cont_id = pNode->scl_cont_id;

	// Set matrix to node chunk.
	float* pMat = (float*)&chunk.tm;
	pMat[0] = pNode->localTM(0, 0);
	pMat[1] = pNode->localTM(1, 0);
	pMat[2] = pNode->localTM(2, 0);
	pMat[4] = pNode->localTM(0, 1);
	pMat[5] = pNode->localTM(1, 1);
	pMat[6] = pNode->localTM(2, 1);
	pMat[8] = pNode->localTM(0, 2);
	pMat[9] = pNode->localTM(1, 2);
	pMat[10] = pNode->localTM(2, 2);
	pMat[12] = pNode->localTM.GetTranslation().x * SCALE_TO_CGF;
	pMat[13] = pNode->localTM.GetTranslation().y * SCALE_TO_CGF;
	pMat[14] = pNode->localTM.GetTranslation().z * SCALE_TO_CGF;

	if (pNode->pMaterial)
	{
		chunk.MatID = pNode->pMaterial->nChunkId;
	}

	chunk.ObjectID = -1;
	chunk.ParentID = -1;
	if (pNode->pParent)
	{
		pNode->nParentChunkId = SaveUncompiledNode(pNode->pParent);
		chunk.ParentID = pNode->nParentChunkId;
	}
	if (pNode->type == CNodeCGF::NODE_MESH && pNode->pMesh && pNode->pMesh->GetFaceCount() > 0)
	{
		pNode->nObjectChunkId = SaveUncompiledNodeMesh(pNode);
	}
	if (pNode->type == CNodeCGF::NODE_HELPER && pNode->helperType == HP_GEOMETRY && pNode->pMesh)
	{
		pNode->nObjectChunkId = SaveUncompiledNodeMesh(pNode);
	}
	else if (pNode->type == CNodeCGF::NODE_HELPER)
	{
		pNode->nObjectChunkId = SaveUncompiledHelperChunk(pNode);
	}
	else if (pNode->type == CNodeCGF::NODE_LIGHT)
	{
		// Save Light chunk
	}

	chunk.ObjectID = pNode->nObjectChunkId;
	chunk.PropStrLen = pNode->properties.length();

	CChunkData chunkData;
	chunkData.Add(chunk);
	// Copy property string
	chunkData.AddData(pNode->properties.c_str(), chunk.PropStrLen);

	pNode->nChunkId = m_pChunkFile->AddChunk(
	  ChunkType_Node,
	  NODE_CHUNK_DESC_0824::VERSION,
	  eEndianness_Native,
	  chunkData.GetData(), chunkData.GetSize());

	return pNode->nChunkId;
}
	#endif

//////////////////////////////////////////////////////////////////////////
	#if defined(RESOURCE_COMPILER)
void CSaverCGF::SaveUncompiledMorphTargets()
{
	assert(m_pCGF);
	CSkinningInfo* const skinningInfo = m_pCGF->GetSkinningInfo();
	for (i32 morphIndex = 0, morphCount = skinningInfo ? i32(skinningInfo->m_arrMorphTargets.size()) : 0; morphIndex < morphCount; ++morphIndex)
	{
		assert(skinningInfo);        // Loop invariant prevents skinningInfo from being nullptr here
		MorphTargets* morph = skinningInfo->m_arrMorphTargets[morphIndex];
		assert(morph);

		MESHMORPHTARGET_CHUNK_DESC_0001 chunk;
		ZeroStruct(chunk);
		chunk.nChunkIdMesh = -1;     // TODO: Save this properly!
		chunk.numMorphVertices = i32(morph->m_arrIntMorph.size());

		CChunkData chunkData;
		chunkData.Add(chunk);
		if (morph->m_arrIntMorph.size())
			chunkData.AddData(&morph->m_arrIntMorph[0], i32(morph->m_arrIntMorph.size() * sizeof(SMeshMorphTargetVertex)));
		chunkData.AddData(morph->m_strName.c_str(), i32(morph->m_strName.size() + 1));

		m_pChunkFile->AddChunk(
		  ChunkType_MeshMorphTarget,
		  MESHMORPHTARGET_CHUNK_DESC_0001::VERSION,
		  eEndianness_Native,
		  chunkData.GetData(), chunkData.GetSize());
	}
}
	#endif

//////////////////////////////////////////////////////////////////////////
i32 CSaverCGF::SaveNodeMesh(
  CNodeCGF* pNode,
  bool bSwapEndian,
  bool bStorePositionsAsF16,
  bool bUseQTangents,
  bool bStoreIndicesAsU16)
{
	if (pNode->pMesh)
	{
		if (m_mapMeshToChunk.find(pNode->pMesh) != m_mapMeshToChunk.end())
		{
			// This mesh already saved.
			return m_mapMeshToChunk.find(pNode->pMesh)->second;
		}
	}

	MESH_CHUNK_DESC_0801 chunk;
	ZeroStruct(chunk);

	if (pNode->pMesh)
	{
		const CMesh& mesh = *pNode->pMesh;

		i32k nVerts = mesh.GetVertexCount();
		i32k nIndices = mesh.GetIndexCount();

		chunk.nVerts = nVerts;
		chunk.nIndices = nIndices;
		chunk.nSubsets = mesh.GetSubSetCount();
		chunk.bboxMin = mesh.m_bbox.min;
		chunk.bboxMax = mesh.m_bbox.max;
	}
	else
	{
		chunk.nVerts = pNode->meshInfo.nVerts;
		chunk.nIndices = pNode->meshInfo.nIndices;
		chunk.nSubsets = pNode->meshInfo.nSubsets;
		chunk.bboxMax = pNode->meshInfo.bboxMin;
		chunk.bboxMax = pNode->meshInfo.bboxMax;
	}

	const bool bEmptyMesh = m_bDoNotSaveMeshData || pNode->bPhysicsProxy || !pNode->pMesh;

	if (bEmptyMesh)
	{
		chunk.nFlags |= MESH_CHUNK_DESC_0801::MESH_IS_EMPTY;
	}
	chunk.nFlags2 = pNode->nPhysicalizeFlags;

	if (m_bSavePhysicsMeshes)
	{
		for (i32 i = 0; i < 4; ++i)
		{
			if (!pNode->physicalGeomData[i].empty())
			{
				chunk.nPhysicsDataChunkId[i] = SavePhysicalDataChunk(&pNode->physicalGeomData[i][0], pNode->physicalGeomData[i].size(), bSwapEndian);
			}
		}
	}

	if (!bEmptyMesh)
	{
		CMesh& mesh = *pNode->pMesh;

		mesh.RecomputeTexMappingDensity();
		chunk.texMappingDensity = mesh.m_texMappingDensity;
		chunk.nFlags |= MESH_CHUNK_DESC_0801::HAS_TEX_MAPPING_DENSITY;

		chunk.nSubsetsChunkId = SaveMeshSubsetsChunk(mesh, bSwapEndian);

		i32k vertexCount = mesh.GetVertexCount();
		bool bInterleaved = false;

		if (m_bCompactVertexStreams && mesh.m_pPositions && mesh.m_pColor0 && mesh.m_pTexCoord)
		{
			std::vector<SVF_P3S_C4B_T2S> interleavedVertices(vertexCount);
			memset(&interleavedVertices[0], 0, sizeof(interleavedVertices[0]) * interleavedVertices.size());

			const Vec3* const pPositions = mesh.m_pPositions;
			const SMeshColor* const pColors = mesh.m_pColor0;
			const SMeshTexCoord* const pTexCoords = mesh.m_pTexCoord;

			for (i32 vi = 0; vi < vertexCount; ++vi)
			{
				SVF_P3S_C4B_T2S& vert = interleavedVertices[vi];
				const ColorB clr = pColors[vi].GetRGBA();
				const Vec2 uv = pTexCoords[vi].GetUV();

				vert.xyz = Vec3f16(pPositions[vi].x, pPositions[vi].y, pPositions[vi].z);
				vert.color.dcolor = clr.pack_abgr8888();
				vert.st = Vec2f16(uv.x, uv.y);

				SwapEndian(vert.xyz, bSwapEndian);
				SwapEndian(vert.color.dcolor, bSwapEndian);
				SwapEndian(vert.st, bSwapEndian);
			}

			chunk.nStreamChunkID[CGF_STREAM_P3S_C4B_T2S] = SaveStreamDataChunk(&interleavedVertices[0], CGF_STREAM_P3S_C4B_T2S, vertexCount, sizeof(interleavedVertices[0]), bSwapEndian);

			bInterleaved = true;
		}

		// We don't support writing m_pPositionsF16 (although we can)
		if (mesh.m_pPositionsF16)
		{
			assert(0);
		}

		if (mesh.m_pPositions && !bInterleaved)
		{
			static_assert(sizeof(mesh.m_pPositions[0]) == sizeof(Vec3), "Invalid type size!");
			if (bStorePositionsAsF16)
			{
				const Vec3* pRealData = mesh.m_pPositions;
				std::vector<Vec3f16> tmpVec(vertexCount);
				for (i32 i = 0; i < vertexCount; ++i, ++pRealData)
				{
					tmpVec[i] = Vec3f16(pRealData->x, pRealData->y, pRealData->z);
					SwapEndian(tmpVec[i], bSwapEndian);
				}
				chunk.nStreamChunkID[CGF_STREAM_POSITIONS] = SaveStreamDataChunk(&tmpVec[0], CGF_STREAM_POSITIONS, vertexCount, sizeof(tmpVec[0]), bSwapEndian);
			}
			else
			{
				SwapEndian(mesh.m_pPositions, vertexCount, bSwapEndian);
				chunk.nStreamChunkID[CGF_STREAM_POSITIONS] = SaveStreamDataChunk(mesh.m_pPositions, CGF_STREAM_POSITIONS, vertexCount, sizeof(mesh.m_pPositions[0]), bSwapEndian);
				SwapEndian(mesh.m_pPositions, vertexCount, bSwapEndian);
			}
		}

		if (mesh.m_pNorms && !m_bCompactVertexStreams)
		{
			SwapEndian(mesh.m_pNorms, vertexCount, bSwapEndian);
			chunk.nStreamChunkID[CGF_STREAM_NORMALS] = SaveStreamDataChunk(mesh.m_pNorms, CGF_STREAM_NORMALS, vertexCount, sizeof(Vec3), bSwapEndian);
			SwapEndian(mesh.m_pNorms, vertexCount, bSwapEndian);
		}

		if (mesh.m_pTexCoord && !bInterleaved)
		{
			SwapEndian(mesh.m_pTexCoord, mesh.GetTexCoordCount(), bSwapEndian);
			chunk.nStreamChunkID[CGF_STREAM_TEXCOORDS] = SaveStreamDataChunk(mesh.m_pTexCoord, CGF_STREAM_TEXCOORDS, mesh.GetTexCoordCount(), sizeof(DrxUV), bSwapEndian);
			SwapEndian(mesh.m_pTexCoord, mesh.GetTexCoordCount(), bSwapEndian);
		}

		if (mesh.m_pColor0 && !bInterleaved)
		{
			SwapEndian(mesh.m_pColor0, vertexCount, bSwapEndian);
			chunk.nStreamChunkID[CGF_STREAM_COLORS] = SaveStreamDataChunk(mesh.m_pColor0, CGF_STREAM_COLORS, vertexCount, sizeof(SMeshColor), bSwapEndian);
			SwapEndian(mesh.m_pColor0, vertexCount, bSwapEndian);
		}

		if (mesh.m_pColor1)
		{
			SwapEndian(mesh.m_pColor1, vertexCount, bSwapEndian);
			chunk.nStreamChunkID[CGF_STREAM_COLORS2] = SaveStreamDataChunk(mesh.m_pColor1, CGF_STREAM_COLORS2, vertexCount, sizeof(SMeshColor), bSwapEndian);
			SwapEndian(mesh.m_pColor1, vertexCount, bSwapEndian);
		}

		if (mesh.m_pVertMats)
		{
			SwapEndian(mesh.m_pVertMats, vertexCount, bSwapEndian);
			chunk.nStreamChunkID[CGF_STREAM_VERT_MATS] = SaveStreamDataChunk(mesh.m_pVertMats, CGF_STREAM_VERT_MATS, vertexCount, sizeof(mesh.m_pVertMats[0]), bSwapEndian);
			SwapEndian(mesh.m_pVertMats, vertexCount, bSwapEndian);
		}

		if (mesh.m_pIndices)
		{
			i32k indexCount = mesh.GetIndexCount();

			static_assert(sizeof(mesh.m_pIndices[0]) == sizeof(vtx_idx), "Invalid type size!");
			static_assert(sizeof(vtx_idx) == 2 || sizeof(vtx_idx) == 4, "Invalid type size!");

			if (sizeof(mesh.m_pIndices[0]) == (bStoreIndicesAsU16 ? 2 : 4))
			{
				SwapEndian(mesh.m_pIndices, indexCount, bSwapEndian);
				chunk.nStreamChunkID[CGF_STREAM_INDICES] = SaveStreamDataChunk(mesh.m_pIndices, CGF_STREAM_INDICES, indexCount, sizeof(mesh.m_pIndices[0]), bSwapEndian);
				SwapEndian(mesh.m_pIndices, indexCount, bSwapEndian);
			}
			else if (bStoreIndicesAsU16)
			{
				if (vertexCount > 0xffff)
				{
					// 0xffff is used instead of 0x10000 to reserve index 0xffff for special cases
	#if defined(RESOURCE_COMPILER)
					RCLogError("Saving mesh with 16-bit vertex indices is impossible - 16-bit indices cannot address %i vertices", vertexCount);
	#endif
					return -1;
				}
				std::vector<u16> tmp(indexCount);
				for (i32 i = 0; i < indexCount; ++i)
				{
					tmp[i] = (u16)mesh.m_pIndices[i];
					SwapEndian(tmp[i], bSwapEndian);
				}
				chunk.nStreamChunkID[CGF_STREAM_INDICES] = SaveStreamDataChunk(&tmp[0], CGF_STREAM_INDICES, indexCount, sizeof(u16), bSwapEndian);
			}
			else
			{
				std::vector<u32> tmp(indexCount);
				for (i32 i = 0; i < indexCount; ++i)
				{
					tmp[i] = mesh.m_pIndices[i];
					SwapEndian(tmp[i], bSwapEndian);
				}
				chunk.nStreamChunkID[CGF_STREAM_INDICES] = SaveStreamDataChunk(&tmp[0], CGF_STREAM_INDICES, indexCount, sizeof(u32), bSwapEndian);
			}
		}

		if (mesh.m_pTangents)
		{
			if (bUseQTangents)
			{
				std::vector<SMeshQTangents> qTangents(vertexCount);
				MeshTangentsFrameToQTangents(
				  &mesh.m_pTangents[0], sizeof(mesh.m_pTangents[0]), vertexCount,
				  &qTangents[0], sizeof(qTangents[0]));
				SwapEndian(&qTangents[0], vertexCount, bSwapEndian);
				chunk.nStreamChunkID[CGF_STREAM_QTANGENTS] = SaveStreamDataChunk(&qTangents[0], CGF_STREAM_QTANGENTS, vertexCount, sizeof(SMeshQTangents), bSwapEndian);
			}
			else
			{
				SwapEndian(mesh.m_pTangents, vertexCount, bSwapEndian);
				chunk.nStreamChunkID[CGF_STREAM_TANGENTS] = SaveStreamDataChunk(mesh.m_pTangents, CGF_STREAM_TANGENTS, vertexCount, sizeof(SMeshTangents), bSwapEndian);
				SwapEndian(mesh.m_pTangents, vertexCount, bSwapEndian);
			}
		}

		if (mesh.m_pBoneMapping)
		{
			if (mesh.m_pExtraBoneMapping)
			{
				chunk.nFlags |= MESH_CHUNK_DESC_0801::HAS_EXTRA_WEIGHTS;

				std::vector<SMeshBoneMapping_u16> tempBoneMapping;
				static_assert(sizeof(tempBoneMapping[0]) == sizeof(mesh.m_pBoneMapping[0]), "Invalid type size!");
				tempBoneMapping.resize(vertexCount * 2);
				memcpy(&tempBoneMapping[0], mesh.m_pBoneMapping, sizeof(tempBoneMapping[0]) * vertexCount);
				memcpy(&tempBoneMapping[vertexCount], mesh.m_pExtraBoneMapping, sizeof(tempBoneMapping[0]) * vertexCount);
				SwapEndian(&tempBoneMapping[0], vertexCount * 2, bSwapEndian);
				chunk.nStreamChunkID[CGF_STREAM_BONEMAPPING] = SaveStreamDataChunk(&tempBoneMapping[0], CGF_STREAM_BONEMAPPING, vertexCount * 2, sizeof(tempBoneMapping[0]), bSwapEndian);
			}
			else
			{
				SwapEndian(mesh.m_pBoneMapping, vertexCount, bSwapEndian);
				chunk.nStreamChunkID[CGF_STREAM_BONEMAPPING] = SaveStreamDataChunk(mesh.m_pBoneMapping, CGF_STREAM_BONEMAPPING, vertexCount, sizeof(mesh.m_pBoneMapping[0]), bSwapEndian);
				SwapEndian(mesh.m_pBoneMapping, vertexCount, bSwapEndian);
			}
		}

		if (pNode->pSkinInfo)
		{
			SwapEndian(pNode->pSkinInfo, vertexCount + 1, bSwapEndian);
			chunk.nStreamChunkID[CGF_STREAM_SKINDATA] = SaveStreamDataChunk(pNode->pSkinInfo, CGF_STREAM_SKINDATA, vertexCount + 1, sizeof(pNode->pSkinInfo[0]), bSwapEndian);
			SwapEndian(pNode->pSkinInfo, vertexCount + 1, bSwapEndian);
		}
	}

	SwapEndian(chunk, bSwapEndian);

	i32k nMeshChunkId = m_pChunkFile->AddChunk(
	  ChunkType_Mesh,
	  MESH_CHUNK_DESC_0801::VERSION,
	  (bSwapEndian ? eEndianness_NonNative : eEndianness_Native),
	  &chunk, sizeof(chunk));

	m_mapMeshToChunk[pNode->pMesh] = nMeshChunkId;

	return nMeshChunkId;
}

//////////////////////////////////////////////////////////////////////////
	#if defined(RESOURCE_COMPILER)
i32 CSaverCGF::SaveUncompiledNodeMesh(CNodeCGF* pNode)
{
	if (m_mapMeshToChunk.find(pNode->pMesh) != m_mapMeshToChunk.end())
	{
		// This mesh already saved.
		return m_mapMeshToChunk.find(pNode->pMesh)->second;
	}

	const CMesh* const mesh = pNode->pMesh;

	const bool HasBoneInfo = (mesh->m_pBoneMapping != 0);
	const bool HasVertexColors = (mesh->m_pColor0 != 0);
	const bool HasVertexAlphas = HasVertexColors;
	const bool WriteVCol = true;

	// uncompiled mesh chunk
	MESH_CHUNK_DESC_0745 chunk;
	ZeroStruct(chunk);

	i32k numVertices = mesh->GetVertexCount();
	i32k numFaces = mesh->GetFaceCount();
	i32k numUVs = mesh->GetTexCoordCount();

	if (numUVs != 0 && numUVs != numVertices)
	{
		RCLogError("Mesh for node \"%s\" has mismatching number of vertices and texture coordinates", pNode->name);
		return -1;
	}

	if (!mesh->m_pTopologyIds)
	{
		RCLogError("Mesh for node \"%s\" has no topology info. Contact an RC programmer.", pNode->name);
		return -1;
	}

	assert(mesh->m_pPositions);
	assert(mesh->m_pNorms);
	assert(mesh->m_pFaces);

	chunk.flags1 = 0;
	chunk.flags2 = MESH_CHUNK_DESC_0745::FLAG2_HAS_TOPOLOGY_IDS;
	chunk.flags1 |= HasBoneInfo ? MESH_CHUNK_DESC_0745::FLAG1_BONE_INFO : 0;
	chunk.flags2 |= (WriteVCol && HasVertexColors) ? MESH_CHUNK_DESC_0745::FLAG2_HAS_VERTEX_COLOR : 0;
	chunk.flags2 |= (WriteVCol && HasVertexAlphas) ? MESH_CHUNK_DESC_0745::FLAG2_HAS_VERTEX_ALPHA : 0;
	chunk.nFaces = numFaces;
	chunk.nTVerts = numUVs;
	chunk.nVerts = numVertices;
	chunk.VertAnimID = -1;         // nVertexAnimChunkID;

	// add chunk members
	CChunkData chunkData;
	chunkData.Add(chunk);

	// copy positions and vertices to a DrxVertex array -----------------------------------
	{
		DrxVertex* vertices = new DrxVertex[numVertices];
		for (i32 i = 0; i < numVertices; i++)
		{
			vertices[i].p = mesh->m_pPositions[i];
			vertices[i].n = mesh->m_pNorms[i].GetN();
		}
		chunkData.AddData(vertices, numVertices * sizeof(DrxVertex));
		SAFE_DELETE_ARRAY(vertices);
	}

	// copy faces to a DrxFace array ------------------------------------------------------
	{
		DrxFace* faces = new DrxFace[numFaces];
		for (i32 i = 0; i < numFaces; i++)
		{
			faces[i].v0 = mesh->m_pFaces[i].v[0];
			faces[i].v1 = mesh->m_pFaces[i].v[1];
			faces[i].v2 = mesh->m_pFaces[i].v[2];
			faces[i].MatID = mesh->m_subsets[mesh->m_pFaces[i].nSubset].nMatID;
		}
		chunkData.AddData(faces, numFaces * sizeof(DrxFace));
		SAFE_DELETE_ARRAY(faces);
	}

	// topology info ----------------------------------------------------------------------
	{
		chunkData.AddData(mesh->m_pTopologyIds, numVertices * sizeof(mesh->m_pTopologyIds[0]));
	}

	// copy texture coordinates to a DrxUV array ------------------------------------------
	if (numUVs)
	{
		assert(numUVs == numVertices);

		DrxUV* uvs = new DrxUV[numUVs];
		for (i32 i = 0; i < numUVs; i++)
		{
			mesh->m_pTexCoord[i].ExportTo(uvs[i].u, uvs[i].v);
		}
		chunkData.AddData(uvs, numUVs * sizeof(DrxUV));
		SAFE_DELETE_ARRAY(uvs);
	}

	if (HasBoneInfo)
	{
		CSkinningInfo* skinningInfo = m_pCGF->GetSkinningInfo();
		Matrix34 objectTransform = pNode->worldTM;

		typedef std::map<i32, std::set<i32>> BadBoneIDs;
		BadBoneIDs badBoneIDs;

		std::vector<DrxLink> arrLinks;

		for (i32 k = 0; k < numVertices; k++)
		{
			Vec3 point = mesh->m_pPositions[k];
			Vec3 worldVertex = objectTransform.TransformPoint(point);

			arrLinks.clear();

			float totalweight = 0.0f;

			for (i32 j = 0; j < 8; j++)
			{
				i32 boneID = -1;
				float blending = 0.0f;

				if (j < 4)
				{
					boneID = mesh->m_pBoneMapping[k].boneIds[j];
					blending = (float)(mesh->m_pBoneMapping[k].weights[j]) / 255.0f;
				}
				else if (mesh->m_pExtraBoneMapping)
				{
					boneID = mesh->m_pExtraBoneMapping[k].boneIds[j - 4];
					blending = (float)(mesh->m_pExtraBoneMapping[k].weights[j - 4]) / 255.0f;
				}

				if (blending < 0.01f)
				{
					continue;
				}

				if (boneID >= skinningInfo->m_arrBonesDesc.size())
				{
					badBoneIDs[boneID].insert(k);
					continue;
				}

				totalweight += blending;

				const Matrix34& boneTransform = skinningInfo->m_arrBonesDesc[boneID].m_DefaultW2B;
				const Vec3 offset = boneTransform.TransformPoint(worldVertex);

				static float const metersToCentimeters = 100.0f;

				DrxLink link;
				link.BoneID = boneID;
				link.Blending = blending;
				link.offset = offset * metersToCentimeters;

				arrLinks.push_back(link);
			}

			i32k nLinks = arrLinks.size();

			for (i32 j = 0; j < nLinks; j++)
			{
				arrLinks[j].Blending /= totalweight;
			}

			chunkData.AddData(&nLinks, sizeof(i32));
			chunkData.AddData(&arrLinks[0], nLinks * sizeof(DrxLink));
		}

		if (!badBoneIDs.empty())
		{
			RCLogError("Skinned mesh for node \"%s\" contains references to missing bones:", pNode->name);
			BadBoneIDs::iterator it;
			string message;
			string vertexStr;
			for (it = badBoneIDs.begin(); it != badBoneIDs.end(); ++it)
			{
				const std::set<i32>& vertexIndices = it->second;
				message.Format("  Bone %i, vertices:", it->first);
				for (std::set<i32>::const_iterator vit = vertexIndices.begin(); vit != vertexIndices.end(); ++vit)
				{
					vertexStr.Format(" %i", *vit);
					message += vertexStr;
				}
				RCLogError("%s", message.c_str());
			}
		}
	}

	if (WriteVCol)
	{
		if (HasVertexColors)
		{
			DrxIRGB* vertexcolors = new DrxIRGB[numVertices];
			for (i32 i = 0; i < numVertices; i++)
			{
				const ColorB clr = mesh->m_pColor0[i].GetRGBA();

				vertexcolors[i].r = clr.r;
				vertexcolors[i].g = clr.g;
				vertexcolors[i].b = clr.b;
			}
			chunkData.AddData(vertexcolors, numVertices * sizeof(DrxIRGB));
			SAFE_DELETE_ARRAY(vertexcolors);
		}

		if (HasVertexAlphas)
		{
			u8* vertexalphas = new u8[numVertices];
			for (i32 i = 0; i < numVertices; i++)
			{
				const ColorB clr = mesh->m_pColor0[i].GetRGBA();

				vertexalphas[i] = clr.a;
			}
			chunkData.AddData(vertexalphas, numVertices * sizeof(u8));
			SAFE_DELETE_ARRAY(vertexalphas);
		}
	}

	if (0)     // save morph targets - the loader load this?
	{}

	if (0)     // save bone initial pose - the loader load this?
	{}

	i32 nMeshChunkId = m_pChunkFile->AddChunk(
	  ChunkType_Mesh,
	  MESH_CHUNK_DESC_0745::VERSION,
	  eEndianness_Native,
	  chunkData.GetData(), chunkData.GetSize());

	m_mapMeshToChunk[pNode->pMesh] = nMeshChunkId;

	return nMeshChunkId;
}
	#endif

//////////////////////////////////////////////////////////////////////////
i32 CSaverCGF::SaveHelperChunk(CNodeCGF* pNode, bool bSwapEndian)
{
	HELPER_CHUNK_DESC chunk;
	ZeroStruct(chunk);

	chunk.type = pNode->helperType;
	chunk.size = pNode->helperSize;

	SwapEndian(chunk, bSwapEndian);

	return m_pChunkFile->AddChunk(
	  ChunkType_Helper,
	  HELPER_CHUNK_DESC::VERSION,
	  (bSwapEndian ? eEndianness_NonNative : eEndianness_Native),
	  &chunk, sizeof(chunk));
}

//////////////////////////////////////////////////////////////////////////
	#if defined(RESOURCE_COMPILER)
i32 CSaverCGF::SaveUncompiledHelperChunk(CNodeCGF* pNode)
{
	HELPER_CHUNK_DESC chunk;
	ZeroStruct(chunk);

	chunk.type = pNode->helperType;
	chunk.size = pNode->helperSize;

	return m_pChunkFile->AddChunk(
	  ChunkType_Helper,
	  HELPER_CHUNK_DESC::VERSION,
	  eEndianness_Native,
	  &chunk, sizeof(chunk));
}
	#endif

//////////////////////////////////////////////////////////////////////////
// TODO:!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

i32 CSaverCGF::SaveBreakablePhysics(bool bSwapEndian)
{
	const CPhysicalizeInfoCGF* const pPi = m_pCGF->GetPhysicalizeInfo();

	if (!pPi || pPi->nGranularity == -1)
	{
		return 0;
	}

	BREAKABLE_PHYSICS_CHUNK_DESC chunk;
	ZeroStruct(chunk);

	chunk.granularity = pPi->nGranularity;
	chunk.nMode = pPi->nMode;
	chunk.nRetVtx = pPi->nRetVtx;
	chunk.nRetTets = pPi->nRetTets;

	CChunkData chunkData;
	chunkData.Add(chunk);
	if (pPi->pRetVtx)
	{
		chunkData.AddData(pPi->pRetVtx, pPi->nRetVtx * sizeof(Vec3));
	}
	if (pPi->pRetTets)
	{
		chunkData.AddData(pPi->pRetTets, pPi->nRetTets * sizeof(i32) * 4);
	}

	SwapEndian(chunk, bSwapEndian);

	return m_pChunkFile->AddChunk(
	  ChunkType_BreakablePhysics,
	  BREAKABLE_PHYSICS_CHUNK_DESC::VERSION,
	  (bSwapEndian ? eEndianness_NonNative : eEndianness_Native),
	  chunkData.GetData(), chunkData.GetSize());
}

//////////////////////////////////////////////////////////////////////////
i32 CSaverCGF::SaveMeshSubsetsChunk(CMesh& mesh, bool bSwapEndian)
{
	MESH_SUBSETS_CHUNK_DESC_0800 chunk;
	ZeroStruct(chunk);

	chunk.nCount = mesh.GetSubSetCount();

	if (m_bComputeSubsetTexelDensity)
	{
		chunk.nFlags |= MESH_SUBSETS_CHUNK_DESC_0800::HAS_SUBSET_TEXEL_DENSITY;
	}

	SwapEndian(chunk, bSwapEndian);

	CChunkData chunkData;
	chunkData.Add(chunk);

	for (i32 i = 0; i < mesh.GetSubSetCount(); i++)
	{
		const SMeshSubset& srcSubset = mesh.m_subsets[i];

		MESH_SUBSETS_CHUNK_DESC_0800::MeshSubset subset;
		memset(&subset, 0, sizeof(subset));
		subset.nFirstIndexId = srcSubset.nFirstIndexId;
		subset.nNumIndices = srcSubset.nNumIndices;
		subset.nFirstVertId = srcSubset.nFirstVertId;
		subset.nNumVerts = srcSubset.nNumVerts;
		subset.nMatID = srcSubset.nMatID;
		subset.fRadius = srcSubset.fRadius;
		subset.vCenter = srcSubset.vCenter;
		SwapEndian(subset, bSwapEndian);
		chunkData.Add(subset);
	}

	// add subset texel densities
	if (m_bComputeSubsetTexelDensity)
	{
		for (i32 i = 0; i < mesh.GetSubSetCount(); ++i)
		{
			MESH_SUBSETS_CHUNK_DESC_0800::MeshSubsetTexelDensity subset;

			tukk err;
			float posArea, texArea;
			if (!mesh.ComputeSubsetTexMappingAreas((size_t)i, posArea, texArea, err))
			{
	#if defined(RESOURCE_COMPILER)
				RCLog("ComputeSubsetTexMappingAreas: %s", err);
	#endif
				subset.texelDensity = 0.0f;
			}
			else
			{
				subset.texelDensity = texArea / posArea;
			}

			SwapEndian(subset, bSwapEndian);
			chunkData.AddData(&subset, sizeof(subset));
		}
	}

	return m_pChunkFile->AddChunk(
	  ChunkType_MeshSubsets,
	  MESH_SUBSETS_CHUNK_DESC_0800::VERSION,
	  (bSwapEndian ? eEndianness_NonNative : eEndianness_Native),
	  chunkData.GetData(), chunkData.GetSize());
}

//////////////////////////////////////////////////////////////////////////
i32 CSaverCGF::SaveStreamDataChunk(ukk pStreamData, i32 nStreamType, i32 nCount, i32 nElemSize, bool bSwapEndian)
{
	STREAM_DATA_CHUNK_DESC_0800 chunk;
	ZeroStruct(chunk);

	chunk.nStreamType = nStreamType;
	chunk.nCount = nCount;
	chunk.nElementSize = nElemSize;

	SwapEndian(chunk, bSwapEndian);

	CChunkData chunkData;
	chunkData.Add(chunk);
	chunkData.AddData(pStreamData, nCount * nElemSize);

	return m_pChunkFile->AddChunk(
	  ChunkType_DataStream,
	  STREAM_DATA_CHUNK_DESC_0800::VERSION,
	  (bSwapEndian ? eEndianness_NonNative : eEndianness_Native),
	  chunkData.GetData(), chunkData.GetSize());
}

//////////////////////////////////////////////////////////////////////////
i32 CSaverCGF::SavePhysicalDataChunk(ukk pData, i32 nSize, bool bSwapEndian)
{
	MESH_PHYSICS_DATA_CHUNK_DESC_0800 chunk;
	ZeroStruct(chunk);

	chunk.nDataSize = nSize;

	SwapEndian(chunk, bSwapEndian);

	CChunkData chunkData;
	chunkData.Add(chunk);
	chunkData.AddData(pData, nSize);

	return m_pChunkFile->AddChunk(
	  ChunkType_MeshPhysicsData,
	  MESH_PHYSICS_DATA_CHUNK_DESC_0800::VERSION,
	  (bSwapEndian ? eEndianness_NonNative : eEndianness_Native),
	  chunkData.GetData(), chunkData.GetSize());
}

//////////////////////////////////////////////////////////////////////////
i32 CSaverCGF::SaveExportFlags(bool bSwapEndian)
{
	if (m_bDoNotSaveNonMeshData)
	{
		return -1;
	}

	EXPORT_FLAGS_CHUNK_DESC chunk;
	ZeroStruct(chunk);

	CExportInfoCGF* pExpInfo = m_pCGF->GetExportInfo();
	if (pExpInfo->bMergeAllNodes)
	{
		chunk.flags |= EXPORT_FLAGS_CHUNK_DESC::MERGE_ALL_NODES;
	}
	if (pExpInfo->bUseCustomNormals)
	{
		chunk.flags |= EXPORT_FLAGS_CHUNK_DESC::USE_CUSTOM_NORMALS;
	}
	if (pExpInfo->bHaveAutoLods)
	{
		chunk.flags |= EXPORT_FLAGS_CHUNK_DESC::HAVE_AUTO_LODS;
	}
	if (pExpInfo->bWantF32Vertices)
	{
		chunk.flags |= EXPORT_FLAGS_CHUNK_DESC::WANT_F32_VERTICES;
	}
	if (pExpInfo->b8WeightsPerVertex)
	{
		chunk.flags |= EXPORT_FLAGS_CHUNK_DESC::EIGHT_WEIGHTS_PER_VERTEX;
	}
	if (pExpInfo->bMakeVCloth)
	{
		chunk.flags |= EXPORT_FLAGS_CHUNK_DESC::MAKE_VCLOTH;
	}

	if (pExpInfo->bFromColladaXSI)
	{
		chunk.assetAuthorTool |= EXPORT_FLAGS_CHUNK_DESC::FROM_COLLADA_XSI;
	}
	if (pExpInfo->bFromColladaMAX)
	{
		chunk.assetAuthorTool |= EXPORT_FLAGS_CHUNK_DESC::FROM_COLLADA_MAX;
	}
	if (pExpInfo->bFromColladaMAYA)
	{
		chunk.assetAuthorTool |= EXPORT_FLAGS_CHUNK_DESC::FROM_COLLADA_MAYA;
	}

	chunk.authorToolVersion = pExpInfo->authorToolVersion;

	static_assert(sizeof(chunk.rc_version) == sizeof(pExpInfo->rc_version), "Invalid type size!");
	memcpy(&chunk.rc_version[0], &pExpInfo->rc_version[0], sizeof(pExpInfo->rc_version));

	drx_strcpy(chunk.rc_version_string, pExpInfo->rc_version_string);

	SwapEndian(chunk, bSwapEndian);

	return m_pChunkFile->AddChunk(
	  ChunkType_ExportFlags,
	  EXPORT_FLAGS_CHUNK_DESC::VERSION,
	  (bSwapEndian ? eEndianness_NonNative : eEndianness_Native),
	  &chunk, sizeof(chunk));
}

//////////////////////////////////////////////////////////////////////////
void CSaverCGF::SaveMaterials(bool bSwapEndian)
{
	for (i32 i = 0; i < m_pCGF->GetNodeCount(); ++i)
	{
		CMaterialCGF* const pMaterialCGF = m_pCGF->GetNode(i)->pMaterial;
		if (pMaterialCGF)
		{
			SaveMaterial(pMaterialCGF, bSwapEndian);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
i32 CSaverCGF::SaveMaterial(CMaterialCGF* pMtl, bool bSwapEndian)
{
	// Check whether the material has already been saved.
	if (m_savedMaterials.find(pMtl) != m_savedMaterials.end())
	{
		return pMtl->nChunkId;
	}

	m_savedMaterials.insert(pMtl);

	MTL_NAME_CHUNK_DESC_0802 chunk;
	ZeroStruct(chunk);

	drx_strcpy(chunk.name, pMtl->name);

	DynArray<i32> nPhysicalizeTypeArray;
	DynArray<char> names;

	i32 nSubMaterials = i32(pMtl->subMaterials.size());

	if (nSubMaterials == 0)
	{
		nPhysicalizeTypeArray.push_back(pMtl->nPhysicalizeType);
	}
	else
	{
		if (nSubMaterials > MAX_SUB_MATERIALS)
		{
	#if defined(RESOURCE_COMPILER)
			RCLogError(
			  "Material %s uses %d sub-materials but maximum allowed is %d.", pMtl->name, nSubMaterials, MAX_SUB_MATERIALS);
	#else
			DrxWarning(
			  VALIDATOR_MODULE_3DENGINE, VALIDATOR_ERROR,
			  "Material %s uses %d sub-materials but maximum allowed is %d.", pMtl->name, nSubMaterials, MAX_SUB_MATERIALS);
	#endif
			nSubMaterials = MAX_SUB_MATERIALS;
		}
		nPhysicalizeTypeArray.resize(nSubMaterials, PHYS_GEOM_TYPE_DEFAULT);

		for (i32 childIndex = 0; childIndex < nSubMaterials; ++childIndex)
		{
			CMaterialCGF* childMaterial = pMtl->subMaterials[childIndex];
			if (childMaterial)
			{
				nPhysicalizeTypeArray[childIndex] = childMaterial->nPhysicalizeType;
				for (i32 i = 0; childMaterial->name[i] != 0; ++i)
				{
					names.push_back(childMaterial->name[i]);
				}
			}
			names.push_back(0);
		}
	}

	chunk.nSubMaterials = nSubMaterials;

	SwapEndian(chunk, bSwapEndian);
	SwapEndian(&nPhysicalizeTypeArray[0], nPhysicalizeTypeArray.size(), bSwapEndian);

	CChunkData chunkData;
	chunkData.Add(chunk);
	chunkData.AddData(&nPhysicalizeTypeArray[0], nPhysicalizeTypeArray.size() * sizeof(nPhysicalizeTypeArray[0]));
	if (!names.empty())
	{
		chunkData.AddData(&names[0], names.size() * sizeof(names[0]));
	}

	pMtl->nChunkId = m_pChunkFile->AddChunk(
	  ChunkType_MtlName,
	  MTL_NAME_CHUNK_DESC_0802::VERSION,
	  (bSwapEndian ? eEndianness_NonNative : eEndianness_Native),
	  chunkData.GetData(), chunkData.GetSize());

	return pMtl->nChunkId;
}

i32 CSaverCGF::SaveController829(bool bSwapEndian, const CONTROLLER_CHUNK_DESC_0829& ctrlChunk, uk pData, i32 nSize)
{
	CChunkData chunkData;
	chunkData.Add(ctrlChunk);
	chunkData.AddData(pData, nSize);

	return m_pChunkFile->AddChunk(
	  ChunkType_Controller,
	  CONTROLLER_CHUNK_DESC_0829::VERSION,
	  (bSwapEndian ? eEndianness_NonNative : eEndianness_Native),
	  chunkData.GetData(), chunkData.GetSize());
}

i32 CSaverCGF::SaveController832(bool bSwapEndian, const CONTROLLER_CHUNK_DESC_0832& ctrlChunk, uk pData, i32 nSize)
{
	CChunkData chunkData;
	chunkData.Add(ctrlChunk);
	chunkData.AddData(pData, nSize);

	return m_pChunkFile->AddChunk(
	  ChunkType_Controller,
	  CONTROLLER_CHUNK_DESC_0832::VERSION,
	  (bSwapEndian ? eEndianness_NonNative : eEndianness_Native),
	  chunkData.GetData(), chunkData.GetSize());
}

i32 CSaverCGF::SaveControllerDB905(bool bSwapEndian, const CONTROLLER_CHUNK_DESC_0905& ctrlChunk, uk pData, i32 nSize)
{
	CChunkData chunkData;
	chunkData.Add(ctrlChunk);
	chunkData.AddData(pData, nSize);

	return m_pChunkFile->AddChunk(
	  ChunkType_Controller,
	  CONTROLLER_CHUNK_DESC_0905::VERSION,
	  (bSwapEndian ? eEndianness_NonNative : eEndianness_Native),
	  chunkData.GetData(), chunkData.GetSize());
}

//////////////////////////////////////////////////////////////////////////
i32 CSaverCGF::SaveFoliage()
{
	FOLIAGE_INFO_CHUNK_DESC chunk;
	ZeroStruct(chunk);

	SFoliageInfoCGF& fi = *m_pCGF->GetFoliageInfo();

	if (fi.nSpines > 0)
	{
		i32 i, j;
		chunk.nSpines = fi.nSpines;
		chunk.nSkinnedVtx = fi.nSkinnedVtx;
		chunk.nBoneIds = fi.chunkBoneIds.size();
		chunk.nSpineVtx = 0;

		for (i = 0; i < fi.nSpines; i++)
		{
			chunk.nSpineVtx += fi.pSpines[i].nVtx;
		}

		FOLIAGE_SPINE_SUB_CHUNK* const pSpineBuf = new FOLIAGE_SPINE_SUB_CHUNK[fi.nSpines];
		Vec3* const pSpineVtx = new Vec3[chunk.nSpineVtx];
		Vec4* const pSpineSegDim = new Vec4[chunk.nSpineVtx];

		memset(pSpineBuf, 0, sizeof(pSpineBuf[0]) * fi.nSpines);

		for (i = j = 0; i < fi.nSpines; j += fi.pSpines[i++].nVtx)
		{
			pSpineBuf[i].nVtx = fi.pSpines[i].nVtx;
			pSpineBuf[i].len = fi.pSpines[i].len;
			pSpineBuf[i].navg = fi.pSpines[i].navg;
			pSpineBuf[i].iAttachSpine = fi.pSpines[i].iAttachSpine + 1;
			pSpineBuf[i].iAttachSeg = fi.pSpines[i].iAttachSeg + 1;
			memcpy(pSpineVtx + j, fi.pSpines[i].pVtx, fi.pSpines[i].nVtx * sizeof(Vec3));
			memcpy(pSpineSegDim + j, fi.pSpines[i].pSegDim, fi.pSpines[i].nVtx * sizeof(Vec4));
		}

		CChunkData chunkData;
		chunkData.Add(chunk);
		chunkData.AddData(pSpineBuf, sizeof(pSpineBuf[0]) * fi.nSpines);
		chunkData.AddData(pSpineVtx, sizeof(pSpineVtx[0]) * chunk.nSpineVtx);
		chunkData.AddData(pSpineSegDim, sizeof(pSpineSegDim[0]) * chunk.nSpineVtx);
		chunkData.AddData(fi.pBoneMapping, sizeof(fi.pBoneMapping[0]) * fi.nSkinnedVtx);
		chunkData.AddData(&fi.chunkBoneIds[0], sizeof(fi.chunkBoneIds[0]) * fi.chunkBoneIds.size());

		delete[] pSpineSegDim;
		delete[] pSpineVtx;
		delete[] pSpineBuf;

		return m_pChunkFile->AddChunk(
		  ChunkType_FoliageInfo,
		  FOLIAGE_INFO_CHUNK_DESC::VERSION,
		  eEndianness_Native,
		  chunkData.GetData(), chunkData.GetSize());
	}
	else
	{
		return 0;
	}
}

//////////////////////////////////////////////////////////////////////////

i32 CSaverCGF::SaveVCloth(bool bSwapEndian)
{
	const SVClothInfoCGF* const pVClothInfo = m_pCGF->GetVClothInfo();
	CChunkData chunkData;

	CSaverVCloth saver(chunkData, pVClothInfo, bSwapEndian);
	saver.WriteChunkHeader();
	saver.WriteChunkVertices();
	saver.WriteTriangleData();
	saver.WriteNndcNotAttachedOrdered();
	saver.WriteLinks();

	return m_pChunkFile->AddChunk(
	  ChunkType_VCloth,
	  VCLOTH_CHUNK::VERSION,
	  (bSwapEndian ? eEndianness_NonNative : eEndianness_Native),
	  chunkData.GetData(), chunkData.GetSize());
}

	#if defined(RESOURCE_COMPILER)

i32 CSaverCGF::SaveTCB3Track(CInternalSkinningInfo* pSkinningInfo, i32 trackIndex)
{
	return CSaverAnim::SaveTCB3Track(m_pChunkFile, pSkinningInfo, trackIndex);
}

i32 CSaverCGF::SaveTCBQTrack(CInternalSkinningInfo* pSkinningInfo, i32 trackIndex)
{
	return CSaverAnim::SaveTCBQTrack(m_pChunkFile, pSkinningInfo, trackIndex);
}

i32 CSaverCGF::SaveTiming(CInternalSkinningInfo* pSkinningInfo)
{
	return CSaverAnim::SaveTiming(m_pChunkFile, pSkinningInfo->m_nStart, pSkinningInfo->m_nEnd);
}

	#endif

#endif
