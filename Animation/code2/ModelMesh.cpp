// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/ModelMesh.h>

#include <drx3D/CoreX/Renderer/IRenderAuxGeom.h>
#include <drx3D/CoreX/Math/QTangent.h>
#include <drx3D/Animation/LoaderCHR.h>

u32 CModelMesh::InitMesh(CMesh* pMesh, CNodeCGF* pMeshNode, _smart_ptr<IMaterial> pMaterial, CSkinningInfo* pSkinningInfo, tukk szFilePath, u32 nLOD)
{
	m_vRenderMeshOffset = pMeshNode ? pMeshNode->worldTM.GetTranslation() : Vec3(ZERO);
	if (pMaterial)
		m_pIDefaultMaterial = pMaterial;
	else
		g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, szFilePath, "No material in lod %d", nLOD);

	m_softwareMesh.GetVertexFrames().Create(*pSkinningInfo, -m_vRenderMeshOffset);

	if (pMesh)
	{
		m_geometricMeanFaceArea = pMesh->m_geometricMeanFaceArea;
		m_faceCount = pMesh->GetIndexCount() / 3;

		u32 numIntFaces = pSkinningInfo->m_arrIntFaces.size();
		u32 numExtFaces = pMesh->GetIndexCount() / 3;
		if (numExtFaces != numIntFaces)
		{
			g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, szFilePath, "external and internal face-amount is different. Probably caused by degenerated faces");
		}

		PrepareMesh(pMesh);
	}

	if (!Console::GetInst().ca_StreamCHR && pMesh)
	{
		m_pIRenderMesh = CreateRenderMesh(pMesh, szFilePath, nLOD, true);
		PrepareRenderChunks(*pMesh, m_arrRenderChunks);
	}

	return 1;
}

_smart_ptr<IRenderMesh> CModelMesh::InitRenderMeshAsync(CMesh* pMesh, tukk szFilePath, i32 nLod, DynArray<RChunk>& arrNewRenderChunks, bool useComputeSkinningBuffers /* = false */)
{
	PrepareMesh(pMesh);

	_smart_ptr<IRenderMesh> pNewRenderMesh = CreateRenderMesh(pMesh, szFilePath, nLod, false, useComputeSkinningBuffers);
	PrepareRenderChunks(*pMesh, arrNewRenderChunks);

	return pNewRenderMesh;
}

u32 CModelMesh::InitRenderMeshSync(DynArray<RChunk>& arrNewRenderChunks, _smart_ptr<IRenderMesh> pNewRenderMesh)
{
	m_arrRenderChunks.swap(arrNewRenderChunks);
	m_pIRenderMesh = pNewRenderMesh;

	return 1;
}

u32 CModelMesh::IsVBufferValid()
{
	if (m_pIRenderMesh == 0)
		return 0;
	m_pIRenderMesh->LockForThreadAccess();
	u32 numIndices = m_pIRenderMesh->GetIndicesCount();
	u32 numVertices = m_pIRenderMesh->GetVerticesCount();
	vtx_idx* pIndices = m_pIRenderMesh->GetIndexPtr(FSL_READ);
	if (pIndices == 0)
		return 0;
	i32 nPositionStride;
	u8* pPositions = m_pIRenderMesh->GetPosPtr(nPositionStride, FSL_READ);
	if (pPositions == 0)
		return 0;
	i32 nSkinningStride;
	u8* pSkinningInfo = m_pIRenderMesh->GetHWSkinPtr(nSkinningStride, FSL_READ);        //pointer to weights and bone-id
	if (pSkinningInfo == 0)
		return 0;
	++m_iThreadMeshAccessCounter;

	m_pIRenderMesh->UnLockForThreadAccess();
	--m_iThreadMeshAccessCounter;
	if (m_iThreadMeshAccessCounter == 0)
	{
		m_pIRenderMesh->UnlockStream(VSF_GENERAL);
		m_pIRenderMesh->UnlockStream(VSF_HWSKIN_INFO);
	}

	return 1;
}

void CModelMesh::AbortStream()
{
	if (m_stream.pStreamer)
		m_stream.pStreamer->AbortLoad();
}

ClosestTri CModelMesh::GetAttachmentTriangle(const Vec3& RMWPosition, const JointIdType* const pRemapTable)
{
	ClosestTri cf;
	if (m_pIRenderMesh == 0)
		return cf;
	m_pIRenderMesh->LockForThreadAccess();
	u32 numIndices = m_pIRenderMesh->GetIndicesCount();
	u32 numVertices = m_pIRenderMesh->GetVerticesCount();
	vtx_idx* pIndices = m_pIRenderMesh->GetIndexPtr(FSL_READ);
	if (pIndices == 0)
		return cf;
	i32 nPositionStride;
	u8* pPositions = m_pIRenderMesh->GetPosPtr(nPositionStride, FSL_READ);
	if (pPositions == 0)
		return cf;
	i32 nSkinningStride;
	u8* pSkinningInfo = m_pIRenderMesh->GetHWSkinPtr(nSkinningStride, FSL_READ, 0, false);          //pointer to weights and bone-id
	if (pSkinningInfo == 0)
		return cf;
	++m_iThreadMeshAccessCounter;

	f32 distance = 99999999.0f;
	u32 foundPos(~0);
	for (u32 d = 0; d < numIndices; d += 3)
	{
		Vec3 v0 = *(Vec3*)(pPositions + pIndices[d + 0] * nPositionStride);
		Vec3 v1 = *(Vec3*)(pPositions + pIndices[d + 1] * nPositionStride);
		Vec3 v2 = *(Vec3*)(pPositions + pIndices[d + 2] * nPositionStride);
		Vec3 TriMiddle = (v0 + v1 + v2) / 3.0f + m_vRenderMeshOffset;
		f32 sqdist = (TriMiddle - RMWPosition) | (TriMiddle - RMWPosition);
		if (distance > sqdist)
			distance = sqdist, foundPos = d;
	}

	for (u32 t = 0; t < 3; t++)
	{
		u32 e = pIndices[foundPos + t];
		Vec3 hwPosition = *(Vec3*)(pPositions + e * nPositionStride);
		u16* hwBoneIDs = ((SVF_W4B_I4S*)(pSkinningInfo + e * nSkinningStride))->indices;
		ColorB hwWeights = *(ColorB*)&((SVF_W4B_I4S*)(pSkinningInfo + e * nSkinningStride))->weights;
		cf.v[t].m_attTriPos = hwPosition + m_vRenderMeshOffset;
		cf.v[t].m_attWeights[0] = hwWeights[0] / 255.0f;
		cf.v[t].m_attWeights[1] = hwWeights[1] / 255.0f;
		cf.v[t].m_attWeights[2] = hwWeights[2] / 255.0f;
		cf.v[t].m_attWeights[3] = hwWeights[3] / 255.0f;
		if (pRemapTable)
		{
			cf.v[t].m_attJointIDs[0] = pRemapTable[hwBoneIDs[0]];
			cf.v[t].m_attJointIDs[1] = pRemapTable[hwBoneIDs[1]];
			cf.v[t].m_attJointIDs[2] = pRemapTable[hwBoneIDs[2]];
			cf.v[t].m_attJointIDs[3] = pRemapTable[hwBoneIDs[3]];
		}
		else
		{
			cf.v[t].m_attJointIDs[0] = hwBoneIDs[0];
			cf.v[t].m_attJointIDs[1] = hwBoneIDs[1];
			cf.v[t].m_attJointIDs[2] = hwBoneIDs[2];
			cf.v[t].m_attJointIDs[3] = hwBoneIDs[3];
		}
	}

	m_pIRenderMesh->UnLockForThreadAccess();
	--m_iThreadMeshAccessCounter;
	if (m_iThreadMeshAccessCounter == 0)
	{
		m_pIRenderMesh->UnlockStream(VSF_GENERAL);
		m_pIRenderMesh->UnlockStream(VSF_HWSKIN_INFO);
	}

	return cf;
}

u32 CModelMesh::InitSWSkinBuffer()
{
	bool valid = m_softwareMesh.IsInitialized();
	if (!valid && m_pIRenderMesh)
	{
		m_pIRenderMesh->LockForThreadAccess();
		++m_iThreadMeshAccessCounter;

		valid = m_softwareMesh.Create(*m_pIRenderMesh, m_arrRenderChunks, m_vRenderMeshOffset);
		m_pIRenderMesh->UnLockForThreadAccess();
		--m_iThreadMeshAccessCounter;
		if (m_iThreadMeshAccessCounter == 0)
		{
			m_pIRenderMesh->UnlockStream(VSF_GENERAL);
			m_pIRenderMesh->UnlockStream(VSF_TANGENTS);
			m_pIRenderMesh->UnlockStream(VSF_HWSKIN_INFO);
		}
	}
	return valid;
}

size_t CModelMesh::SizeOfModelMesh() const
{
	u32 nSize = sizeof(CModelMesh);
	u32 numRenderChunks = m_arrRenderChunks.size();
	nSize += m_arrRenderChunks.get_alloc_size();

	return nSize;
}

void CModelMesh::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(m_arrRenderChunks);
}

void CModelMesh::PrepareMesh(CMesh* pMesh)
{
	//HACK-HACK-HACK-HACK-HACK-HACK-HACK-HACK-HACK-HACK-HACK-HACK-HACK-
	//fix this in the exporter
	u32 numMeshSubsets = pMesh->m_subsets.size();
	for (u32 i = 0; i < numMeshSubsets; i++)
	{
		pMesh->m_subsets[i].FixRanges(pMesh->m_pIndices);
	}

	// TEMP: Convert tangent frame from matrix to quaternion based.
	// This code should go into RC and not be performed at loading time!
	if (pMesh->m_pTangents && !pMesh->m_pQTangents)
	{
		pMesh->m_pQTangents = (SMeshQTangents*)pMesh->m_pTangents;
		MeshTangentsFrameToQTangents(
		  pMesh->m_pTangents, sizeof(pMesh->m_pTangents[0]), pMesh->GetVertexCount(),
		  pMesh->m_pQTangents, sizeof(pMesh->m_pQTangents[0]));
	}

	CreateMorphsBuffer(pMesh);
}

void CModelMesh::PrepareRenderChunks(CMesh& mesh, DynArray<RChunk>& renderChunks)
{
#if defined(SUBDIVISION_ACC_ENGINE)
	DynArray<SMeshSubset>& subsets = mesh.m_bIsSubdivisionMesh ? mesh.m_subdSubsets : mesh.m_subsets;
#else
	DynArray<SMeshSubset>& subsets = mesh.m_subsets;
#endif

	const uint subsetCount = uint(subsets.size());
	renderChunks.resize(subsetCount);
	for (uint i = 0; i < subsetCount; i++)
	{
		renderChunks[i].m_nFirstIndexId = subsets[i].nFirstIndexId;
		renderChunks[i].m_nNumIndices = subsets[i].nNumIndices;
	}

	m_softwareMesh.Create(mesh, renderChunks, m_vRenderMeshOffset);
}

_smart_ptr<IRenderMesh> CModelMesh::CreateRenderMesh(CMesh* pMesh, tukk szFilePath, i32 nLod, bool bCreateDeviceMesh, bool needsComputeSkinningBuffers)
{
	//////////////////////////////////////////////////////////////////////////
	// Create the RenderMesh
	//////////////////////////////////////////////////////////////////////////
	ERenderMeshType eRMType = eRMT_Static;

	bool bMultiGPU;
	gEnv->pRenderer->EF_Query(EFQ_MultiGPUEnabled, bMultiGPU);

	if (bMultiGPU && gEnv->pRenderer->GetRenderType() != ERenderType::Direct3D12
		          && gEnv->pRenderer->GetRenderType() != ERenderType::Vulkan)
		eRMType = eRMT_Dynamic;

	_smart_ptr<IRenderMesh> pRenderMesh = g_pIRenderer->CreateRenderMesh("Character", szFilePath, NULL, eRMType);
	assert(pRenderMesh != 0);
	if (pRenderMesh == 0)
	{
		g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, szFilePath, "Failed to create render mesh for lod %d", nLod);
		return 0;
	}

	u32 nFlags = FSM_VERTEX_VELOCITY;
	if (bCreateDeviceMesh)
		nFlags |= FSM_CREATE_DEVICE_MESH;
#ifdef MESH_TESSELLATION_ENGINE
	nFlags |= FSM_ENABLE_NORMALSTREAM;
#endif

	
	if (needsComputeSkinningBuffers)
	{
		static ICVar* cvar_gd = gEnv->pConsole->GetCVar("r_ComputeSkinning");
		if ((nLod == 0) && cvar_gd && cvar_gd->GetIVal())
			nFlags |= FSM_USE_COMPUTE_SKINNING;
	}

	pRenderMesh->SetMesh(*pMesh, 0, nFlags, 0, true);

	return pRenderMesh;
}

void CModelMesh::CreateMorphsBuffer(CMesh* pMesh)
{
	assert(pMesh);

	u32 numMorphs = m_softwareMesh.GetVertexFrames().GetCount();
	u32 numVertexDeltas = m_softwareMesh.GetVertexFrames().GetVertexDeltasCount();
	u32 numVertices = pMesh->GetVertexCount();
	if (!numMorphs || !numVertices || !numVertexDeltas)
	{
		return;
	}

	pMesh->m_numMorphs = numMorphs;

	u32 numMorphMaskBits = Align(numMorphs, (1U << 5));
	u32 numMorphMasks = numMorphMaskBits >> 5;

	pMesh->m_vertexDeltas.resize(numVertexDeltas);
	pMesh->m_vertexMorphsBitfield.resize(numVertices * numMorphMasks, 0);

	std::vector<u32> vertexIndices;
	vertexIndices.resize(numVertices, 0);

	std::vector<u32> vertexBuckets;
	vertexBuckets.resize(numVertices, 0);

	for (u32 i = 0; i < numMorphs; ++i)
	{
		const SSoftwareVertexFrame& frame = m_softwareMesh.GetVertexFrames().GetFrames()[i];
		u32 numMorphVerts = (u32)frame.vertices.size();
		for (u32 j = 0; j < numMorphVerts; ++j)
		{
			const Vec3& deltaPos = frame.vertices[j].position;
			u32 vertexIndex = (u32)frame.vertices[j].index;
			++vertexBuckets[vertexIndex];
		}
	}

	pMesh->m_verticesDeltaOffsets.resize(numVertices, 0);
	for (u32 i = 1; i < numVertices; ++i)
	{
		pMesh->m_verticesDeltaOffsets[i] = pMesh->m_verticesDeltaOffsets[i - 1] + vertexBuckets[i - 1];
	}

	for (u32 i = 0; i < numMorphs; ++i)
	{
		const SSoftwareVertexFrame& frame = m_softwareMesh.GetVertexFrames().GetFrames()[i];
		u32 numMorphVerts = (u32)frame.vertices.size();
		for (u32 j = 0; j < numMorphVerts; ++j)
		{
			const Vec3& deltaPos = frame.vertices[j].position;
			u32 vertexIndex = (u32)frame.vertices[j].index;
			pMesh->m_vertexDeltas[pMesh->m_verticesDeltaOffsets[vertexIndex] + vertexIndices[vertexIndex]] = Vec4(deltaPos, 0.0f);
			++vertexIndices[vertexIndex];

			u32 bidx = i >> 5;
			u32 bset = i & 31;
			pMesh->m_vertexMorphsBitfield[vertexIndex + (bidx * numVertices)] |= uint64(1ull << bset);
		}
	}

	for (u32 i = 0; i < numVertices; ++i)
	{
		u32 numBits = 0;
		for (u32 j = 0; j < numMorphMasks; ++j)
		{
			uint64 mask = pMesh->m_vertexMorphsBitfield[i + (numVertices * j)];
			pMesh->m_vertexMorphsBitfield[i + (numVertices * j)] = (uint64(numBits) << 32) | mask;
			numBits += CountBits(u32(mask));
		}
	}
}
