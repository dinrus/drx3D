// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __RENDELEMENT_H__
#define __RENDELEMENT_H__

#include <drx3D/CoreX/Renderer/VertexFormats.h>
#include <drx3D/CoreX/Math/Drx_Color.h>

class CRenderElement;
struct CRenderChunk;
struct PrimitiveGroup;
class CShader;
struct SShaderTechnique;
class CParserBin;
struct SParserFrame;
class CRenderView;

enum EDataType
{
	eDATA_Unknown = 0,
	eDATA_Sky,
	eDATA_ClientPoly,
	eDATA_Flare,
	eDATA_Terrain,
	eDATA_SkyZone,
	eDATA_Mesh,
	eDATA_LensOptics,
	eDATA_OcclusionQuery,
	eDATA_Particle,
	eDATA_HDRSky,
	eDATA_FogVolume,
	eDATA_WaterVolume,
	eDATA_WaterOcean,
	eDATA_GameEffect,
	eDATA_BreakableGlass,
	eDATA_GeomCache,
};

enum ERenderElementFlags
{
	FCEF_TRANSFORM             = BIT(0),
	FCEF_DIRTY                 = BIT(1),
	FCEF_NODEL                 = BIT(2),
	FCEF_DELETED               = BIT(3),

	FCEF_UPDATEALWAYS          = BIT(8),

	FCEF_SKINNED               = BIT(11),
	FCEF_PRE_DRAW_DONE         = BIT(12),
};

typedef uintptr_t stream_handle_t;

struct SStreamInfo
{
	stream_handle_t hStream = ~0u;
	u32          nStride =  0u; // NOTE: for index buffers this needs to contain the index format
	u32          nSlot   =  0u;
};


class IRenderElement
{
public:
	IRenderElement() {}
	virtual ~IRenderElement() {};

	virtual CRenderChunk*      mfGetMatInfo() = 0;
	virtual TRenderChunkArray* mfGetMatInfoList() = 0;
	virtual i32                mfGetMatId() = 0;
	virtual void               mfReset() = 0;
	virtual bool               mfIsHWSkinned() = 0;
	virtual CRenderElement*      mfCopyConstruct(void) = 0;
	virtual void               mfCenter(Vec3& centr, CRenderObject* pObj, const SRenderingPassInfo& passInfo) = 0;
	virtual void               mfGetBBox(Vec3& vMins, Vec3& vMaxs) const = 0;

	virtual bool  mfUpdate(InputLayoutHandle eVertFormat, i32 Flags, bool bTessellation = false) = 0;

	virtual void  mfExport(struct SShaderSerializeContext& SC) = 0;
	virtual void  mfImport(struct SShaderSerializeContext& SC, u32& offset) = 0;


	//////////////////////////////////////////////////////////////////////////
	// ~Pipeline 2.0 methods.
	//////////////////////////////////////////////////////////////////////////

	virtual InputLayoutHandle GetVertexFormat() const = 0;

	//! Compile is called on a non mesh render elements, must be called only in rendering thread
	//! Returns false if compile failed, and render element must not be rendered
	virtual bool          Compile(CRenderObject* pObj, CRenderView *pRenderView, bool updateInstanceDataOnly) = 0;

	//! Custom Drawing for the non mesh render elements.
	//! Must be thread safe for the parallel recording
	virtual void          DrawToCommandList(CRenderObject* pObj, const struct SGraphicsPipelinePassContext& ctx, class CDeviceCommandList* commandList) = 0;

	//////////////////////////////////////////////////////////////////////////
	// ~Pipeline 2.0 methods.
	//////////////////////////////////////////////////////////////////////////

	virtual i32           Size() = 0;

	virtual void         Release(bool bForce = false) = 0;
	virtual void         GetMemoryUsage(IDrxSizer* pSizer) const = 0;
};

class CRenderElement : public IRenderElement
{
	static i32 s_nCounter;
public:
	static CRenderElement s_RootGlobal;
	static CRenderElement *s_pRootRelease[];
	CRenderElement*       m_NextGlobal;
	CRenderElement*       m_PrevGlobal;

	EDataType           m_Type;

	u32     m_nID;
	u16     m_Flags;
	u16     m_nFrameUpdated;

	enum { MAX_CUSTOM_TEX_BINDS_NUM = 2 };
	uk m_CustomData;
	i32   m_CustomTexBind[MAX_CUSTOM_TEX_BINDS_NUM];

	static DrxCriticalSection s_accessLock;

public:
	struct SGeometryInfo
	{
		u32        bonesRemapGUID               = 0u; // Input parameter to fetch correct skinning stream.

		i32           primitiveType                = 0; //!< \see eRenderPrimitiveType
		InputLayoutHandle eVertFormat              = EDefaultInputLayouts::Empty;

		i32         nFirstIndex                  = 0;
		i32         nNumIndices                  = 0;
		u32        nFirstVertex                 = 0u;
		u32        nNumVertices                 = 0u;

		u32        nNumVertexStreams            = 0u;

		SStreamInfo   indexStream;
		SStreamInfo   vertexStreams[VSF_NUM]; // contains only nNumVertexStreams elements

		uk         pTessellationAdjacencyBuffer = nullptr;
		uk         pSkinningExtraBonesBuffer    = nullptr;
		u32        nTessellationPatchIDOffset   = 0u;

		inline u32 CalcStreamMask()
		{
			u32 streamMask = 0;
			for (u32 s = 0; s < nNumVertexStreams; ++s)
				streamMask |= 1U << vertexStreams[s].nSlot;

			return streamMask;
		}

		inline u32 CalcLastStreamSlot()
		{
			u32 lastStreamSlot = 0;
			for (u32 s = 0; s < nNumVertexStreams; ++s)
				lastStreamSlot = std::max(lastStreamSlot, vertexStreams[s].nSlot);

			return lastStreamSlot;
		}
	};

public:
	CRenderElement(bool bGlobal);
	CRenderElement();
	virtual ~CRenderElement();

	inline void UnlinkGlobal()
	{
		if (!m_NextGlobal || !m_PrevGlobal)
			return;
		m_NextGlobal->m_PrevGlobal = m_PrevGlobal;
		m_PrevGlobal->m_NextGlobal = m_NextGlobal;
		m_NextGlobal = m_PrevGlobal = NULL;
	}
	inline void LinkGlobal(CRenderElement* Before)
	{
		if (m_NextGlobal || m_PrevGlobal)
			return;
		m_NextGlobal = Before->m_NextGlobal;
		Before->m_NextGlobal->m_PrevGlobal = this;
		Before->m_NextGlobal = this;
		m_PrevGlobal = Before;
	}

	tukk      mfTypeString();
	inline EDataType mfGetType() { return m_Type; }
	void             mfSetType(EDataType t) { m_Type = t; }

	inline u32 mfGetFlags(void)         { return m_Flags; }
	inline void   mfSetFlags(u32 fl)    { m_Flags = fl; }
	inline void   mfUpdateFlags(u32 fl) { m_Flags |= fl; }
	inline void   mfClearFlags(u32 fl)  { m_Flags &= ~fl; }
	inline bool   mfCheckUpdate(InputLayoutHandle eVertFormat, i32 Flags, u16 nFrame, bool bTessellation = false)
	{
		if (nFrame != m_nFrameUpdated || (m_Flags & (FCEF_DIRTY | FCEF_SKINNED | FCEF_UPDATEALWAYS)))
		{
			m_nFrameUpdated = nFrame;
			return mfUpdate(eVertFormat, Flags, bTessellation);
		}
		return true;
	}

	virtual CRenderChunk*      mfGetMatInfo();
	virtual TRenderChunkArray* mfGetMatInfoList();
	virtual i32                mfGetMatId();
	virtual void               mfReset();
	virtual bool               mfIsHWSkinned() { return false; }
	virtual CRenderElement*    mfCopyConstruct(void);
	virtual void               mfCenter(Vec3& centr, CRenderObject* pObj, const SRenderingPassInfo& passInfo);
	virtual void               mfGetBBox(Vec3& vMins, Vec3& vMaxs) const
	{
		vMins.Set(0, 0, 0);
		vMaxs.Set(0, 0, 0);
	}
	virtual void  mfGetPlane(Plane& pl);


	virtual uk mfGetPointer(ESrcPointer ePT, i32* Stride, EParamType Type, ESrcPointer Dst, i32 Flags);

	virtual bool  mfUpdate(InputLayoutHandle eVertFormat, i32 Flags, bool bTessellation = false) { return true; }

	virtual void  mfExport(struct SShaderSerializeContext& SC)                               { DrxFatalError("mfExport has not been implemented for this render element type"); }
	virtual void  mfImport(struct SShaderSerializeContext& SC, u32& offset)               { DrxFatalError("mfImport has not been implemented for this render element type"); }


	//////////////////////////////////////////////////////////////////////////
	// ~Pipeline 2.0 methods.
	//////////////////////////////////////////////////////////////////////////

	virtual InputLayoutHandle GetVertexFormat() const                                                    { return InputLayoutHandle::Unspecified; };
	virtual bool          GetGeometryInfo(SGeometryInfo& streams, bool bSupportTessellation = false) { return false; }

	//! Compile is called on a non mesh render elements, must be called only in rendering thread
	//! Returns false if compile failed, and render element must not be rendered
	virtual bool          Compile(CRenderObject* pObj, CRenderView *pRenderView, bool updateInstanceDataOnly)  { return false; };

	//! Custom Drawing for the non mesh render elements.
	//! Must be thread safe for the parallel recording
	virtual void          DrawToCommandList(CRenderObject* pObj, const struct SGraphicsPipelinePassContext& ctx, class CDeviceCommandList* commandList)  {};
	
	//////////////////////////////////////////////////////////////////////////
	// ~Pipeline 2.0 methods.
	//////////////////////////////////////////////////////////////////////////

	virtual i32           Size()                                                            { return 0; }

	virtual void         Release(bool bForce = false);
	virtual void         GetMemoryUsage(IDrxSizer* pSizer) const { /*nothing*/ }


	static void          ShutDown();
	static void          Tick();

	static void          Cleanup();
};

#include "CREMesh.h"
#include "CRESky.h"
#include "CREOcclusionQuery.h"
#include "CREFogVolume.h"
#include "CREWaterVolume.h"
#include "CREWaterOcean.h"
#include "CREGameEffect.h"
#include "CREBreakableGlass.h"
#include <drx3D/Eng3D/CREGeomCache.h>

#endif  // __RENDELEMENT_H__
