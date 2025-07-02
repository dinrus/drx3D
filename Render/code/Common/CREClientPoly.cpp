// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*=============================================================================
   CREClientPoly.cpp : implementation of 3D Client polygons RE.

   Revision история:
* Created by Honitch Andrey

   =============================================================================*/

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/RendElements/CREClientPoly.h>
#include <drx3D/Render/RenderView.h>

#include <drx3D/Render/D3D/DriverD3D.h>

//////////////////////////////////////////////////////////////////////////
void CRenderPolygonDataPool::Clear()
{
	m_vertexCount = 0;
	m_vertices.resize(0);
	m_tangents.resize(0);
	m_indices.resize(0);

	if (m_vertexBuffer)
		CDeviceBufferUpr::Instance()->Destroy(m_vertexBuffer);
	if (m_tangentsBuffer)
		CDeviceBufferUpr::Instance()->Destroy(m_tangentsBuffer);
	if (m_indexBuffer)
		CDeviceBufferUpr::Instance()->Destroy(m_indexBuffer);

	m_vertexBuffer = 0;
	m_tangentsBuffer = 0;
	m_indexBuffer = 0;
}

//////////////////////////////////////////////////////////////////////////
void CRenderPolygonDataPool::UpdateAPIBuffers()
{
	DRX_PROFILE_FUNCTION(PROFILE_RENDERER)

	// Already allocated.
	if (m_vertexBuffer)
		return;

	// Update GPU buffers from CPU only temporary buffers
	i32 numVertices = m_vertices.size() / sizeof(VertexFormat);
	if (numVertices > 0)
	{
		m_vertexBuffer = CDeviceBufferUpr::Instance()->Create(BBT_VERTEX_BUFFER, BU_STATIC, numVertices * sizeof(VertexFormat));
		CDeviceBufferUpr::Instance()->UpdateBuffer(m_vertexBuffer, &m_vertices[0], CDeviceBufferUpr::AlignBufferSizeForStreaming(numVertices * sizeof(VertexFormat)));

		assert(numVertices == m_tangents.size());

		m_tangentsBuffer = CDeviceBufferUpr::Instance()->Create(BBT_VERTEX_BUFFER, BU_STATIC, numVertices * sizeof(SPipTangents));
		CDeviceBufferUpr::Instance()->UpdateBuffer(m_tangentsBuffer, &m_tangents[0], CDeviceBufferUpr::AlignBufferSizeForStreaming(numVertices * sizeof(SPipTangents)));
	}

	if (m_indices.size() > 0)
	{
		m_indexBuffer = CDeviceBufferUpr::Instance()->Create(BBT_INDEX_BUFFER, BU_STATIC, m_indices.size() * sizeof(u16));
		CDeviceBufferUpr::Instance()->UpdateBuffer(m_indexBuffer, &m_indices[0], CDeviceBufferUpr::AlignBufferSizeForStreaming(m_indices.size() * sizeof(u16)));
	}
}

//////////////////////////////////////////////////////////////////////////

//===============================================================

CRenderElement* CREClientPoly::mfCopyConstruct(void)
{
	CREClientPoly* cp = new CREClientPoly;
	*cp = *this;
	return cp;
}

//////////////////////////////////////////////////////////////////////////
void CREClientPoly::AssignPolygon(const SRenderPolygonDescription& poly, const SRenderingPassInfo& passInfo, CRenderPolygonDataPool* pPolygonDataPool)
{
	auto& vertexPool = pPolygonDataPool->m_vertices;
	auto& indexPool = pPolygonDataPool->m_indices;
	auto& tangentsPool = pPolygonDataPool->m_tangents;

	m_pPolygonDataPool = pPolygonDataPool;

	m_Shader = poly.shaderItem;
	m_vertexCount = poly.numVertices;

	m_nCPFlags = 0;

	if (poly.afterWater)
		m_nCPFlags |= CREClientPoly::efAfterWater;
	if (passInfo.IsShadowPass())
		m_nCPFlags |= CREClientPoly::efShadowGen;

	auto& verts = poly.pVertices;
	auto& tangs = poly.pTangents;

	size_t nSize = CDeviceObjectFactory::GetInputLayoutDescriptor(EDefaultInputLayouts::P3F_C4B_T2F)->m_Strides[0] * poly.numVertices;
	i32 nOffs = vertexPool.size();
	vertexPool.resize(nOffs + nSize);
	SVF_P3F_C4B_T2F* vt = (SVF_P3F_C4B_T2F*)&vertexPool[nOffs];
	m_vertexOffset = nOffs;
	for (i32 i = 0; i < poly.numVertices; i++, vt++)
	{
		vt->xyz = verts[i].xyz;
		vt->st = verts[i].st;
		vt->color.dcolor = verts[i].color.dcolor;
	}

	m_tangentOffset = tangentsPool.size();
	tangentsPool.resize(tangentsPool.size() + poly.numVertices);

	if (tangs)
	{
		for (i32 i = 0; i < poly.numVertices; i++)
		{
			tangentsPool[m_tangentOffset + i] = tangs[i];
		}
	}
	else
	{
		m_tangentOffset = -1;
	}

	m_indexOffset = indexPool.size();

	i32 nBaseVertexOffset = pPolygonDataPool->m_vertexCount;

	if (poly.pIndices && poly.numIndices)
	{
		indexPool.resize(m_indexOffset + poly.numIndices);
		u16* poolIndices = &indexPool[m_indexOffset];
		m_indexCount = poly.numIndices;
		for (i32 i = 0; i < poly.numIndices; ++i)
		{
			poolIndices[i] = nBaseVertexOffset + poly.pIndices[i];
		}
	}
	else
	{
		indexPool.resize(m_indexOffset + (poly.numVertices - 2) * 3);
		u16* poolIndices = &indexPool[m_indexOffset];
		for (i32 i = 0; i < poly.numVertices - 2; ++i, poolIndices += 3)
		{
			poolIndices[0] = nBaseVertexOffset + 0;
			poolIndices[1] = nBaseVertexOffset + 1;
			poolIndices[2] = nBaseVertexOffset + i + 2;
		}
		m_indexCount = (poly.numVertices - 2) * 3;
	}

	pPolygonDataPool->m_vertexCount += poly.numVertices;
}

//////////////////////////////////////////////////////////////////////////
bool CREClientPoly::GetGeometryInfo(SGeometryInfo& geomInfo, bool bSupportTessellation)
{
	geomInfo.bonesRemapGUID = 0;

	geomInfo.primitiveType = eptTriangleList;
	geomInfo.eVertFormat = EDefaultInputLayouts::P3F_C4B_T2F;

	geomInfo.nFirstIndex = m_indexOffset;
	geomInfo.nNumIndices = m_indexCount;

	geomInfo.nFirstVertex = m_vertexOffset;
	geomInfo.nNumVertices = m_vertexCount;

	geomInfo.nNumVertexStreams = 2;

	geomInfo.indexStream.nStride = Index16;
	geomInfo.indexStream.hStream = m_pPolygonDataPool->m_indexBuffer;

	geomInfo.vertexStreams[VSF_GENERAL].nSlot = VSF_GENERAL;
	geomInfo.vertexStreams[VSF_GENERAL].nStride = CDeviceObjectFactory::GetInputLayoutDescriptor(EDefaultInputLayouts::P3F_C4B_T2F)->m_Strides[0];
	geomInfo.vertexStreams[VSF_GENERAL].hStream = m_pPolygonDataPool->m_vertexBuffer;

	geomInfo.vertexStreams[VSF_TANGENTS].nSlot = VSF_TANGENTS;
	geomInfo.vertexStreams[VSF_TANGENTS].nStride = CDeviceObjectFactory::GetInputLayoutDescriptor(EDefaultInputLayouts::T4S_B4S)->m_Strides[0];
	geomInfo.vertexStreams[VSF_TANGENTS].hStream = m_pPolygonDataPool->m_tangentsBuffer;

	return true;
}
