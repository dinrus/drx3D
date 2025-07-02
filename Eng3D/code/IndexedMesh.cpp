// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   IndexedMesh.cpp
//  Created:     28/5/2001 by Vladimir Kajalin
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Eng3D/StdAfx.h>

#include <drx3D/Eng3D/IndexedMesh.h>
#include <drx3D/Eng3D/MeshCompiler.h>

DEFINE_INTRUSIVE_LINKED_LIST(CIndexedMesh)

CIndexedMesh::CIndexedMesh()
{
}

CIndexedMesh::~CIndexedMesh()
{
}

void CIndexedMesh::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(this, sizeof(*this));
	CMesh::GetMemoryUsage(pSizer);
}

void CIndexedMesh::RestoreFacesFromIndices()
{
	i32k indexCount = GetIndexCount();
	SetFaceCount(indexCount / 3);
	memset(m_pFaces, 0, GetFaceCount() * sizeof(m_pFaces[0]));

	i32 nFaceId = 0;
	for (i32 i = 0; i < indexCount; i += 3)
	{
		if (m_pIndices[i] != (vtx_idx) - 1)  // deleted faces have -1 here
		{
			for (i32 v = 0; v < 3; ++v)
			{
				assert((i32)m_pIndices[i + v] < GetVertexCount());
				m_pFaces[nFaceId].v[v] = m_pIndices[i + v];
			}
			++nFaceId;
		}
	}

	SetFaceCount(nFaceId);
}

#if DRX_PLATFORM_WINDOWS
void CIndexedMesh::Optimize(tukk szComment) PREFAST_SUPPRESS_WARNING(6262) //function uses > 32k stack space
{
	mesh_compiler::CMeshCompiler meshCompiler;

	if (szComment)
	{
		// mesh_compiler::MESH_COMPILE_OPTIMIZE is a bit expensive so we show a warning if it's used at run time
		Warning("CIndexedMesh::Optimize is called at run time by %s", szComment);
	}

	if (!meshCompiler.Compile(*this, (mesh_compiler::MESH_COMPILE_TANGENTS | mesh_compiler::MESH_COMPILE_OPTIMIZE)))
	{
		Warning("CIndexedMesh::Optimize failed: %s", meshCompiler.GetLastError());
	}
}
#endif

void CIndexedMesh::CalcBBox()
{
	i32k vertexCount = GetVertexCount();

	if (vertexCount == 0 || !m_pPositions)
	{
		m_bbox = AABB(Vec3(0, 0, 0), Vec3(0, 0, 0));
		return;
	}

	assert(m_pPositionsF16 == 0);

	m_bbox.Reset();

	i32k faceCount = GetFaceCount();

	if (faceCount > 0)
	{
		for (i32 i = 0; i < faceCount; ++i)
		{
			for (i32 v = 0; v < 3; ++v)
			{
				i32k nIndex = m_pFaces[i].v[v];
				assert(nIndex >= 0 && nIndex < vertexCount);
				m_bbox.Add(m_pPositions[nIndex]);
			}
		}
	}
	else
	{
		i32k indexCount = GetIndexCount();

		for (i32 i = 0; i < indexCount; ++i)
		{
			m_bbox.Add(m_pPositions[m_pIndices[i]]);
		}
	}
}
