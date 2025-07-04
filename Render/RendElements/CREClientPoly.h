// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CRECLIENTPOLY_H__
#define __CRECLIENTPOLY_H__

struct SClientPolyStat
{
	i32 NumOccPolys;
	i32 NumRendPolys;
	i32 NumVerts;
	i32 NumIndices;
};

// Pool of vertex/index data used by client polygons rendering.
class CRenderPolygonDataPool
{
public:
	typedef SVF_P3F_C4B_T2F VertexFormat;
	typedef SPipTangents    TangentFormat;

	//////////////////////////////////////////////////////////////////////////
	// CPU only temporary buffers
	std::vector<u8>        m_vertices;
	std::vector<u16>       m_indices;
	std::vector<SPipTangents> m_tangents;
	
	u32 m_vertexCount = 0;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// GPU buffers
	buffer_handle_t m_vertexBuffer = 0;
	buffer_handle_t m_tangentsBuffer = 0;
	buffer_handle_t m_indexBuffer = 0;
	//////////////////////////////////////////////////////////////////////////

public:
	void Clear();
	void UpdateAPIBuffers();
};

class CREClientPoly : public CRenderElement
{
public:
	enum EFlags
	{
		efAfterWater = 1 << 0,
		efShadowGen  = 1 << 1,
	};
	SShaderItem m_Shader;

	u32      m_nCPFlags = 0;
	u16      m_vertexCount = 0;
	u16      m_indexCount = 0;
	u32      m_vertexOffset = 0;
	u32      m_tangentOffset = 0;
	u32      m_indexOffset = 0;

	// Shared vertex/index buffers pool for all polygons.
	CRenderPolygonDataPool* m_pPolygonDataPool = nullptr;

public:
	CREClientPoly()
	{
		mfSetType(eDATA_ClientPoly);
		mfUpdateFlags(FCEF_TRANSFORM);
	}

	virtual ~CREClientPoly() {};

	virtual CRenderElement*     mfCopyConstruct(void) final;

	virtual bool              GetGeometryInfo(SGeometryInfo& geomInfo, bool bSupportTessellation = false) final;

	virtual void              GetMemoryUsage(IDrxSizer* pSizer) const final
	{
		pSizer->AddObject(this, sizeof(*this));
	}

	void AssignPolygon(const SRenderPolygonDescription& poly, const SRenderingPassInfo& passInfo, CRenderPolygonDataPool* pPolygonDataPool);
};

#endif  // __CRECLIENTPOLY2D_H__
