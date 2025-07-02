// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   MeshCompiler.h
//  Version:     v1.00
//  Created:     5/11/2004 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64 2003
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __MeshCompiler_h__
#define __MeshCompiler_h__
#pragma once

#include <drx3D/Eng3D/IIndexedMesh.h>

namespace mesh_compiler
{

enum EMeshCompileFlags
{
	MESH_COMPILE_OPTIMIZE                    = BIT(0),
	MESH_COMPILE_TANGENTS                    = BIT(1),
	MESH_COMPILE_IGNORE_TANGENT_SPACE_ERRORS = BIT(2),
	MESH_COMPILE_USE_CUSTOM_NORMALS          = BIT(3),
	MESH_COMPILE_VALIDATE                    = BIT(4),
};

//////////////////////////////////////////////////////////////////////////
class CMeshCompiler
{
public:
	CMeshCompiler();
	~CMeshCompiler();

#if DRX_PLATFORM_WINDOWS
	// for flags see EMeshCompilerFlags
	bool Compile(CMesh& mesh, i32 flags);
#endif

	void SetVertexRemapping(std::vector<i32>* pVertexMap)
	{
		m_pVertexMap = pVertexMap;
	}

	void SetIndexRemapping(std::vector<i32>* pIndexMap)
	{
		m_pIndexMap = pIndexMap;
	}

	static inline bool IsEquivalentVec3dCheckYFirst(const Vec3& v0, const Vec3& v1, float fEpsilon)
	{
		if (fabsf(v0.y - v1.y) < fEpsilon)
			if (fabsf(v0.x - v1.x) < fEpsilon)
				if (fabsf(v0.z - v1.z) < fEpsilon)
					return true;
		return false;
	}

	static inline const Vec3& ToVec3(const Vec3& vec)    { return vec; }
	static inline const Vec3  ToVec3(const Vec3f16& vec) { return vec.ToVec3(); }

	template<class T>
	static inline i32 FindInPosBuffer_VF_P3X(const Vec3& vPosToFind, const T* pVertBuff, std::vector<i32>* pHash, float fEpsilon)
	{
		for (u32 i = 0; i < pHash->size(); i++)
			if (IsEquivalentVec3dCheckYFirst(ToVec3(pVertBuff[(*pHash)[i]].xyz), vPosToFind, fEpsilon))
				return (*pHash)[i];

		return -1;
	}

	template<class V, class I>
	void WeldPos_VF_P3X(
	  PodArray<V>& vertices,
	  PodArray<SPipTangents>& tangents,
	  PodArray<SPipNormal>& normals,
	  PodArray<I>& indices,
	  float fEpsilon,
	  const AABB& boxBoundary)
	{
		i32k numVertices = vertices.Count();
		V* pTmpVerts = new V[numVertices];
		SPipTangents* pTmpTangents = new SPipTangents[tangents.Count()];

#if ENABLE_NORMALSTREAM_SUPPORT
		SPipNormal* pTmpNormals = NULL;
		if (normals.Count() > 0)
			pTmpNormals = new SPipNormal[normals.Count()];

		SPipNormal emptyNormal;
#endif

		i32 nCurVertex = 0;
		PodArray<I> newIndices;
		std::vector<i32> arrHashTable[256];
		newIndices.reserve(indices.size());
		std::vector<i32>* pHash = 0;

		float fHashElemSize = 256.0f / max(boxBoundary.max.x - boxBoundary.min.x, 0.01f);

		for (u32 i = 0; i < indices.size(); i++)
		{
			i32 v = indices[i];

			assert(v < vertices.Count());

			V& vPos = vertices[v];
			SPipTangents& vTang = tangents[v];
#if ENABLE_NORMALSTREAM_SUPPORT
			SPipNormal& vNorm = pTmpNormals ? normals[v] : emptyNormal;
#endif

			bool bInRange(
			  vPos.xyz.x > boxBoundary.min.x && vPos.xyz.y > boxBoundary.min.y && vPos.xyz.z > boxBoundary.min.z &&
			  vPos.xyz.x < boxBoundary.max.x && vPos.xyz.y < boxBoundary.max.y && vPos.xyz.z < boxBoundary.max.z);

			i32 nHashValue = i32((vPos.xyz.x - boxBoundary.min.x) * fHashElemSize);

			pHash = &arrHashTable[(u8)(nHashValue)];
			i32 nFind = FindInPosBuffer_VF_P3X(ToVec3(vPos.xyz), pTmpVerts, pHash, bInRange ? fEpsilon : 0.01f);
			if (nFind < 0)
			{
				pHash->push_back(nCurVertex);

				// make sure neighbor hashes also have this vertex
				if (bInRange && fEpsilon > 0.01f)
				{
					pHash = &arrHashTable[(u8)(nHashValue + 1)];
					if (FindInPosBuffer_VF_P3X(ToVec3(vPos.xyz), pTmpVerts, pHash, fEpsilon) < 0)
						pHash->push_back(nCurVertex);

					pHash = &arrHashTable[(u8)(nHashValue - 1)];
					if (FindInPosBuffer_VF_P3X(ToVec3(vPos.xyz), pTmpVerts, pHash, fEpsilon) < 0)
						pHash->push_back(nCurVertex);
				}

				PREFAST_ASSUME(nCurVertex < numVertices);

				// add new vertex
				pTmpVerts[nCurVertex] = vPos;
				pTmpTangents[nCurVertex] = vTang;
#if ENABLE_NORMALSTREAM_SUPPORT
				if (pTmpNormals)
					pTmpNormals[nCurVertex] = vNorm;
#endif
				newIndices.push_back(nCurVertex);
				nCurVertex++;
			}
			else
			{
				newIndices.push_back(nFind);
			}
		}

		indices.Clear();
		indices.AddList(newIndices);

		vertices.Clear();
		vertices.AddList(pTmpVerts, nCurVertex);

		tangents.Clear();
		tangents.AddList(pTmpTangents, nCurVertex);

#if ENABLE_NORMALSTREAM_SUPPORT
		if (pTmpNormals)
		{
			normals.Clear();
			normals.AddList(pTmpNormals, nCurVertex);

			delete[] pTmpNormals;
		}
#endif

		delete[] pTmpVerts;
		delete[] pTmpTangents;
	}

	static bool CompareMeshes(const CMesh& mesh1, const CMesh& mesh2);

	tukk GetLastError() const
	{
		return m_LastError;
	}

private:
#if DRX_PLATFORM_WINDOWS
	bool        CreateIndicesAndDeleteDuplicateVertices(CMesh& mesh);
	bool        StripifyMesh_Forsyth(CMesh& mesh);
	static bool CheckForDegenerateFaces(const CMesh& mesh);
	static void FindVertexRanges(CMesh& mesh);
#endif

private:
	struct SBasisFace
	{
		i32 v[3];
	};
	std::vector<const SMeshFace*> m_vhash_table[MAX_SUB_MATERIALS];
	std::vector<SBasisFace>       m_thash_table[MAX_SUB_MATERIALS];

	std::vector<i32>*             m_pVertexMap;
	std::vector<i32>*             m_pIndexMap;

	string                        m_LastError;
};

} //endns mesh_compiler

#endif // __MeshCompiler_h__
