// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   IndexedMesh.h
//  Created:     28/5/2001 by Vladimir Kajalin
////////////////////////////////////////////////////////////////////////////

#ifndef IDX_MESH_H
#define IDX_MESH_H

#include <drx3D/CoreX/Containers/DrxArray.h>
#include <drx3D/Eng3D/DrxHeaders.h>
#include <drx3D/Eng3D/IIndexedMesh.h>

class CIndexedMesh
	: public IIndexedMesh
	  , public CMesh
	  , public stl::intrusive_linked_list_node<CIndexedMesh>
	  , public DinrusX3dEngBase
{
public:
	CIndexedMesh();
	virtual ~CIndexedMesh();

	//////////////////////////////////////////////////////////////////////////
	// IIndexedMesh
	//////////////////////////////////////////////////////////////////////////

	virtual void Release()
	{
		delete this;
	}

	// gives read-only access to mesh data
	virtual void GetMeshDescription(SMeshDescription& meshDesc) const
	{
		meshDesc.m_pFaces = m_pFaces;
		meshDesc.m_pVerts = m_pPositions;
		meshDesc.m_pVertsF16 = m_pPositionsF16;
		meshDesc.m_pNorms = m_pNorms;
		meshDesc.m_pColor = m_pColor0;
		meshDesc.m_pTexCoord = m_pTexCoord;
		meshDesc.m_pIndices = m_pIndices;
		meshDesc.m_nFaceCount = GetFaceCount();
		meshDesc.m_nVertCount = GetVertexCount();
		meshDesc.m_nCoorCount = GetTexCoordCount();
		meshDesc.m_nIndexCount = GetIndexCount();
	}

	virtual CMesh* GetMesh()
	{
		return this;
	}

	virtual void SetMesh(CMesh& mesh)
	{
		CopyFrom(mesh);
	}

	virtual void FreeStreams()
	{
		return CMesh::FreeStreams();
	}

	virtual i32 GetFaceCount() const
	{
		return CMesh::GetFaceCount();
	}

	virtual void SetFaceCount(i32 nNewCount)
	{
		CMesh::SetFaceCount(nNewCount);
	}

	virtual i32 GetVertexCount() const
	{
		return CMesh::GetVertexCount();
	}

	virtual void SetVertexCount(i32 nNewCount)
	{
		CMesh::SetVertexCount(nNewCount);
	}

	virtual void SetColorCount(i32 nNewCount)
	{
		CMesh::ReallocStream(COLORS_0, nNewCount);
	}

	virtual i32 GetTexCoordCount() const
	{
		return CMesh::GetTexCoordCount();
	}

	virtual void SetTexCoordCount(i32 nNewCount)
	{
		CMesh::ReallocStream(TEXCOORDS, nNewCount);
	}

	virtual i32 GetTangentCount() const
	{
		return CMesh::GetTangentCount();
	}

	virtual void SetTangentCount(i32 nNewCount)
	{
		CMesh::ReallocStream(TANGENTS, nNewCount);
	}

	virtual void SetTexCoordsAndTangentsCount(i32 nNewCount)
	{
		CMesh::SetTexCoordsAndTangentsCount(nNewCount);
	}

	virtual i32 GetIndexCount() const
	{
		return CMesh::GetIndexCount();
	}

	virtual void SetIndexCount(i32 nNewCount)
	{
		CMesh::SetIndexCount(nNewCount);
	}

	virtual void AllocateBoneMapping()
	{
		ReallocStream(BONEMAPPING, GetVertexCount());
	}

	virtual i32 GetSubSetCount() const
	{
		return m_subsets.size();
	}

	virtual void SetSubSetCount(i32 nSubsets)
	{
		m_subsets.resize(nSubsets);
	}

	virtual const SMeshSubset& GetSubSet(i32 nIndex) const
	{
		return m_subsets[nIndex];
	}

	virtual void SetSubsetBounds(i32 nIndex, const Vec3& vCenter, float fRadius)
	{
		m_subsets[nIndex].vCenter = vCenter;
		m_subsets[nIndex].fRadius = fRadius;
	}

	virtual void SetSubsetIndexVertexRanges(i32 nIndex, i32 nFirstIndexId, i32 nNumIndices, i32 nFirstVertId, i32 nNumVerts)
	{
		m_subsets[nIndex].nFirstIndexId = nFirstIndexId;
		m_subsets[nIndex].nNumIndices = nNumIndices;
		m_subsets[nIndex].nFirstVertId = nFirstVertId;
		m_subsets[nIndex].nNumVerts = nNumVerts;
	}

	virtual void SetSubsetMaterialId(i32 nIndex, i32 nMatID)
	{
		m_subsets[nIndex].nMatID = nMatID;
	}

	virtual void SetSubsetMaterialProperties(i32 nIndex, i32 nMatFlags, i32 nPhysicalizeType)
	{
		m_subsets[nIndex].nMatFlags = nMatFlags;
		m_subsets[nIndex].nPhysicalizeType = nPhysicalizeType;
	}

	virtual AABB GetBBox() const
	{
		return m_bbox;
	}

	virtual void SetBBox(const AABB& box)
	{
		m_bbox = box;
	}

	virtual void CalcBBox();

#if DRX_PLATFORM_WINDOWS
	virtual void Optimize(tukk szComment = NULL);
#endif

	virtual void RestoreFacesFromIndices();

	//////////////////////////////////////////////////////////////////////////

	void GetMemoryUsage(class IDrxSizer* pSizer) const;
};

#endif // IDX_MESH_H
