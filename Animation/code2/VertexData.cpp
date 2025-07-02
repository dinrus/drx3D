// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/VertexData.h>

#include <drx3D/CoreX/Math/QTangent.h>
#include <drx3D/CoreX/Renderer/VertexFormats.h>
#include <drx3D/Animation/ModelMesh.h>

/*
   CSoftwareMesh
 */

CSoftwareMesh::CSoftwareMesh() :
	m_blendCount(0)
{
}

//

bool CSoftwareMesh::Create(const CMesh& mesh, const DynArray<RChunk>& renderChunks, const Vec3& positionOffset)
{
	if (IsInitialized())
		return 1;
	if (mesh.m_pExtraBoneMapping == 0)
		return false;

	//its a mesh with 8-weights. install it in system memory
	if (mesh.m_pIndices == 0)
		return false;
	if (mesh.m_pPositions == 0)
		return false;
	if (mesh.m_pTexCoord == 0)
		return false;
	if (mesh.m_pQTangents == 0)
		return false;
	if (mesh.m_pBoneMapping == 0)
		return false;

	// pre-calculate the UV vectors for recomputing tangents on 8 weight skinning meshes
	if (mesh.m_pColor0)
	{
		const uint triCount = mesh.GetIndexCount() / 3;
		m_precTangentData.reserve(triCount);

		std::set<vtx_idx> uniqueIds;
		for (uint i = 0; i < triCount; ++i)
		{
			const uint base = i * 3;
			const uint idx1 = mesh.m_pIndices[base];

			// blue color used for updating tangents
			const ColorB clr = mesh.m_pColor0[idx1].GetRGBA();
			if (clr.b == 0xFF && clr.r == 0 && clr.g == 0)
			{
				const uint idx2 = mesh.m_pIndices[base + 1];
				const uint idx3 = mesh.m_pIndices[base + 2];

				const Vec2 uv1 = mesh.m_pTexCoord[idx1].GetUV();
				const Vec2 uv2 = mesh.m_pTexCoord[idx2].GetUV();
				const Vec2 uv3 = mesh.m_pTexCoord[idx3].GetUV();

				STangentUpdateTriangles precomData;
				precomData.idx1 = idx1;
				precomData.idx2 = idx2;
				precomData.idx3 = idx3;

				const float s1 = uv2.x - uv1.x;
				const float s2 = uv3.x - uv1.x;

				precomData.t1 = uv2.y - uv1.y;
				precomData.t2 = uv3.y - uv1.y;

				const float denom = s1 * precomData.t2 - s2 * precomData.t1;
				precomData.r = fabsf(denom) > FLT_EPSILON ? 1.0f / denom : 1.0f;

				m_precTangentData.push_back(precomData);

				uniqueIds.insert(idx1);
				uniqueIds.insert(idx2);
				uniqueIds.insert(idx3);
			}
		}

		m_precTangentVertIds.reserve(uniqueIds.size());
		for (std::set<vtx_idx>::const_iterator it = uniqueIds.begin(); it != uniqueIds.end(); ++it)
			m_precTangentVertIds.push_back(*it);
	}

	u32k vertexCount = mesh.GetVertexCount();
	AllocateVertices(vertexCount);

	strided_pointer<Vec3> positions = GetWritePositions();
	strided_pointer<u32> colors = GetWriteColors();
	strided_pointer<Vec2> coords = GetWriteCoords();
	strided_pointer<Quat> tangents = GetWriteTangents();
	for (uint i = 0; i < vertexCount; ++i)
	{
		positions[i] = mesh.m_pPositions[i] + positionOffset;
		if (mesh.m_pColor0)
			colors[i] = *(u32*)&mesh.m_pColor0[i];
		else
			colors[i] = 0xffffffff;
		coords[i] = mesh.m_pTexCoord[i].GetUV();
		tangents[i] = mesh.m_pQTangents[i].GetQ();
		assert(tangents[i].IsUnit());
	}

#if defined(SUBDIVISION_ACC_ENGINE)
	const DynArray<SMeshSubset>& subsets = mesh.m_bIsSubdivisionMesh ? mesh.m_subdSubsets : mesh.m_subsets;
#else
	const DynArray<SMeshSubset>& subsets = mesh.m_subsets;
#endif
	const uint subsetCount = uint(subsets.size());

	strided_pointer<SoftwareVertexBlendIndex> blendIndices = GetWriteBlendIndices();
	strided_pointer<SoftwareVertexBlendWeight> blendWeights = GetWriteBlendWeights();
	const uint blendCount = 8;
	for (uint i = 0; i < vertexCount; ++i)
	{
		SoftwareVertexBlendIndex* pBlendIndices = &blendIndices[i];
		SoftwareVertexBlendWeight* pBlendWeights = &blendWeights[i];
		for (uint j = 0; j < blendCount; ++j)
		{
			pBlendIndices[j] = 0;
			pBlendWeights[j] = 0;
		}
	}
	for (u32 i = 0; i < subsetCount; i++)
	{
		const uint startIndex = renderChunks[i].m_nFirstIndexId;
		const uint endIndex = renderChunks[i].m_nFirstIndexId + renderChunks[i].m_nNumIndices;
		for (u32 j = startIndex; j < endIndex; ++j)
		{
#if defined(SUBDIVISION_ACC_ENGINE)
			const uint subIndex = mesh.m_bIsSubdivisionMesh ? mesh.m_pSubdIndices[j] : mesh.m_pIndices[j];
#else
			const uint subIndex = mesh.m_pIndices[j];
#endif
			SoftwareVertexBlendIndex* const pBlendIndices = &blendIndices[subIndex];
			SoftwareVertexBlendWeight* const pBlendWeights = &blendWeights[subIndex];

			{
				const SMeshBoneMapping_u16::BoneId* const pSourceBlendIndices = &mesh.m_pBoneMapping[subIndex].boneIds[0];
				const SMeshBoneMapping_u16::Weight* const pSourceBlendWeights = &mesh.m_pBoneMapping[subIndex].weights[0];
				for (uint k = 0; k < 4; ++k)
				{
					if (pSourceBlendWeights[k])
					{
						pBlendIndices[k] = pSourceBlendIndices[k];
					}
					pBlendWeights[k] = pSourceBlendWeights[k];
				}
			}

			{
				const SMeshBoneMapping_u16::BoneId* const pSourceBlendIndices = &mesh.m_pExtraBoneMapping[subIndex].boneIds[0];
				const SMeshBoneMapping_u16::Weight* const pSourceBlendWeights = &mesh.m_pExtraBoneMapping[subIndex].weights[0];
				for (uint k = 0; k < 4; ++k)
				{
					if (pSourceBlendWeights[k])
					{
						pBlendIndices[4 + k] = pSourceBlendIndices[k];
					}
					pBlendWeights[4 + k] = pSourceBlendWeights[k];
				}
			}
		}
	}

	const uint indexCount = mesh.GetIndexCount();
	AllocateIndices(indexCount);
	for (uint i = 0; i < indexCount; ++i)
		m_indices[i] = mesh.m_pIndices[i];

	m_blendCount = blendCount;
	return true;
}

// IZF: Once all initialization will take place from the CMesh overloaded method, this
// can be removed.
bool CSoftwareMesh::Create(IRenderMesh& renderMesh, const DynArray<RChunk>& renderChunks, const Vec3& positionOffset)
{
	uint indexCount = renderMesh.GetIndicesCount();
	uint vertexCount = renderMesh.GetVerticesCount();

	vtx_idx* pIndices = renderMesh.GetIndexPtr(FSL_READ);

	i32 positionStride;
	u8* pPositions = renderMesh.GetPosPtr(positionStride, FSL_READ);
	if (pPositions == 0)
		return false;

	i32 colorStride;
	u8* pColors = renderMesh.GetColorPtr(colorStride, FSL_READ);
	if (pColors == 0)
		return false;

	i32 coordStride;
	u8* pCoords = renderMesh.GetUVPtr(coordStride, FSL_READ);
	if (pCoords == 0)
		return false;

	i32 tangentStride;
	u8* pTangents = renderMesh.GetQTangentPtr(tangentStride, FSL_READ);
	if (pTangents == 0)
		return false;

	i32 indicesWeightsStride;
	u8* pIndicesWeights = renderMesh.GetHWSkinPtr(indicesWeightsStride, FSL_READ, 0, false);       //pointer to weights and bone-id
	if (pIndicesWeights == 0)
		return false;

	AllocateVertices(vertexCount);

	const bool texCoordsAre32Bits = renderMesh.GetVertexFormat() == EDefaultInputLayouts::P3F_C4B_T2F;
	DRX_ASSERT(texCoordsAre32Bits || renderMesh.GetVertexFormat() == EDefaultInputLayouts::P3S_C4B_T2S);

	strided_pointer<Vec3> positions = GetWritePositions();
	strided_pointer<u32> colors = GetWriteColors();
	strided_pointer<Vec2> coords = GetWriteCoords();
	strided_pointer<Quat> tangents = GetWriteTangents();
	for (uint i = 0; i < vertexCount; ++i)
	{
		positions[i] = *((const Vec3*)(pPositions + i * positionStride)) + positionOffset;
		colors[i] = *((u32k*)(pColors + i * colorStride));
		if (texCoordsAre32Bits)
			coords[i] = *((const Vec2*)(pCoords + i * coordStride));
		else
			coords[i] = ((const Vec2f16*)(pCoords + i * coordStride))->ToVec2();

		const SPipQTangents& tangent = *(const SPipQTangents*)(pTangents + i * tangentStride);
		tangents[i] = SPipQTangents(tangent).GetQ();
		assert(tangents[i].IsUnit());
	}

	strided_pointer<SoftwareVertexBlendIndex> blendIndices = GetWriteBlendIndices();
	strided_pointer<SoftwareVertexBlendWeight> blendWeights = GetWriteBlendWeights();
	const uint blendCount = 4;
	for (uint i = 0; i < vertexCount; ++i)
	{
		SoftwareVertexBlendIndex* pBlendIndices = &blendIndices[i];
		SoftwareVertexBlendWeight* pBlendWeights = &blendWeights[i];

		for (uint j = 0; j < blendCount; ++j)
		{
			pBlendIndices[j] = 0;
			pBlendWeights[j] = 0;
		}

		const ColorB& weights = *(const ColorB*)&((const SVF_W4B_I4S*)(pIndicesWeights + i * indicesWeightsStride))->weights;
		for (uint j = 0; j < 4; ++j)
			pBlendWeights[j] = weights[j];
	}

	uint subsetCount = uint(renderChunks.size());
	for (uint i = 0; i < subsetCount; i++)
	{
		uint startIndex = renderChunks[i].m_nFirstIndexId;
		uint endIndex = renderChunks[i].m_nFirstIndexId + renderChunks[i].m_nNumIndices;
		for (u32 j = startIndex; j < endIndex; ++j)
		{
			u32k index = pIndices[j];
			u16k* subsetBlendIndices = ((const SVF_W4B_I4S*)(pIndicesWeights + index * indicesWeightsStride))->indices;
			const ColorB& subsetWeightIndices = *(const ColorB*)&((const SVF_W4B_I4S*)(pIndicesWeights + index * indicesWeightsStride))->weights;

			SoftwareVertexBlendIndex* pBlendIndices = &blendIndices[index];
			for (uint k = 0; k < 4; ++k)
			{
				if (subsetWeightIndices[k])
					pBlendIndices[k] = subsetBlendIndices[k];
			}
		}
	}

	AllocateIndices(indexCount);
	for (uint i = 0; i < indexCount; ++i)
		m_indices[i] = pIndices[i];

	m_blendCount = blendCount;
	return true;
}

bool CSoftwareMesh::IsInitialized() const
{
	return m_vertices.size() != 0;
}

/*
   CSoftwareVertexFrames
 */

bool CSoftwareVertexFrames::Create(const CSkinningInfo& skinningInfo, const Vec3& positionOffset)
{
	uint frameCount = skinningInfo.m_arrMorphTargets.size();
	if (!frameCount)
		return false;

	m_numVertexDeltas = 0;
	m_frames.resize(frameCount);
	for (uint i = 0; i < frameCount; ++i)
	{
		SSoftwareVertexFrame& frame = m_frames[i];
		frame.index = i;

		frame.name = skinningInfo.m_arrMorphTargets[i]->m_strName;

		uint vertexCount = uint(skinningInfo.m_arrMorphTargets[i]->m_arrExtMorph.size());
		assert(vertexCount);
		if (vertexCount <= 0)
		{
			//			g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION,VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, szFilePath, "arrExtMorph[] is empty in lod %d", nLOD);
			return false;
		}

		float maxLengthSquared = 0.0f;
		frame.vertices.resize(vertexCount);
		for (uint j = 0; j < vertexCount; j++)
		{
			SSoftwareVertexFrameFormat& vf = frame.vertices[j];

			vf.index = skinningInfo.m_arrMorphTargets[i]->m_arrExtMorph[j].nVertexId;
			vf.position = skinningInfo.m_arrMorphTargets[i]->m_arrExtMorph[j].ptVertex + positionOffset;

			const float lengthSquared = frame.vertices[j].position.GetLengthSquared();
			maxLengthSquared = lengthSquared > maxLengthSquared ? lengthSquared : maxLengthSquared;

			++m_numVertexDeltas;
		}
		frame.vertexMaxLength = sqrt(maxLengthSquared);
	}
	return true;
}

uint CSoftwareVertexFrames::GetIndexByName(tukk name) const
{
	uint count = uint(m_frames.size());
	for (uint i = 0; i < count; ++i)
	{
		if (!stricmp(m_frames[i].name.c_str(), name) ||
		    !stricmp(m_frames[i].name.c_str() + 1, name))
			return i;
	}
	return -1;
}
