// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// ------------------------------------------------------------------------
//  Имя файла:   CREGeomCache.h
//  Created:     17/10/2012 by Axel Gneiting
//  Описание: Backend part of geometry cache rendering
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#ifndef _CREGEOMCACHE_
#define _CREGEOMCACHE_

#pragma once

#if defined(USE_GEOM_CACHES)

class CREGeomCache : public CRenderElement
{
public:
	struct SMeshInstance
	{
		AABB     m_aabb;
		Matrix34 m_matrix;
		Matrix34 m_prevMatrix;
	};

	struct SMeshRenderData
	{
		DynArray<SMeshInstance> m_instances;
		_smart_ptr<IRenderMesh> m_pRenderMesh;
	};

public:
	CREGeomCache();
	~CREGeomCache();

	bool        Update(i32k flags, const bool bTesselation);
	static void UpdateModified();

	// CRenderElement interface
	virtual bool mfUpdate(InputLayoutHandle eVertFormat, i32 Flags, bool bTessellation) override;
	
	// CREGeomCache interface
	virtual void                       InitializeRenderElement(const uint numMeshes, _smart_ptr<IRenderMesh>* pMeshes, u16 materialId);
	virtual void                       SetupMotionBlur(CRenderObject* pRenderObject, const SRenderingPassInfo& passInfo);

	virtual  i32*              SetAsyncUpdateState(i32& threadId);
	virtual DynArray<SMeshRenderData>* GetMeshFillDataPtr();
	virtual DynArray<SMeshRenderData>* GetRenderDataPtr();
	virtual void                       DisplayFilledBuffer(i32k threadId);

	// accessors for new render pipeline
	virtual InputLayoutHandle GetVertexFormat() const override;
	virtual bool          GetGeometryInfo(SGeometryInfo& streams, bool bSupportTessellation = false) override;
	virtual void          DrawToCommandList(CRenderObject* pObj, const SGraphicsPipelinePassContext& ctx, CDeviceCommandList* commandList) override;

private:
	u16        m_materialId;
	 bool m_bUpdateFrame[2];
	 i32  m_transformUpdateState[2];

	//! We use a double buffered m_meshFillData array for input from the main thread. When data
	//! was successfully sent from the main thread it gets copied to m_meshRenderData
	//! This simplifies the cases where frame data is missing, e.g. meshFillData is not updated for a frame
	//! Note that meshFillData really needs to be double buffered because the copy occurs in render thread
	//! so the next main thread could already be touching the data again
	//! Note: m_meshRenderData is directly accessed for ray intersections via GetRenderDataPtr.
	//! This is safe, because it's only used in editor.
	DynArray<SMeshRenderData>         m_meshFillData[2];
	DynArray<SMeshRenderData>         m_meshRenderData;

	static DrxCriticalSection         ms_updateListCS[2];
	static std::vector<CREGeomCache*> ms_updateList[2];
};

#endif
#endif
