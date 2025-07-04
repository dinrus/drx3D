// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#ifndef _RenderMesh_H_
#define _RenderMesh_H_

#include "VertexFormats.h"
#include <drx3D/Eng3D/IMaterial.h>
#include <drx3D/CoreX/Renderer/IShader.h>
#include <drx3D/CoreX/Renderer/IRenderer.h>  // PublicRenderPrimitiveType
#include <drx3D/CoreX/Math/Drx_Geo.h>
#include <drx3D/CoreX/Containers/DrxArray.h>
#include <drx3D/CoreX/Thread/IJobUpr.h>

class CMesh;
struct CRenderChunk;
class CRenderObject;
struct SSkinningData;
struct IMaterial;
struct IShader;
struct IIndexedMesh;
struct SMRendTexVert;
struct UCol;
struct GeomInfo;

struct TFace;
struct SMeshSubset;
struct SRenderingPassInfo;
struct SRenderObjectModifier;

//! Keep this in sync with BUFFER_USAGE hints DevBuffer.h.
enum ERenderMeshType
{
	eRMT_Immmutable = 0,
	eRMT_Static     = 1,
	eRMT_Dynamic    = 2,
	eRMT_Transient  = 3,
};

#define FSM_VERTEX_VELOCITY      1
#define FSM_NO_TANGENTS          2
#define FSM_CREATE_DEVICE_MESH   4
#define FSM_SETMESH_ASYNC        8
#define FSM_ENABLE_NORMALSTREAM  16
#define FSM_IGNORE_TEXELDENSITY  32
#define FSM_USE_COMPUTE_SKINNING 64

// Invalidate video buffer flags
#define FMINV_STREAM      1
#define FMINV_STREAM_MASK ((1 << VSF_NUM) - 1)
#define FMINV_INDICES     0x100
#define FMINV_ALL         -1

// Stream lock flags
#define  FSL_READ            0x01
#define  FSL_WRITE           0x02
#define  FSL_DYNAMIC         0x04
#define  FSL_DISCARD         0x08
#define  FSL_VIDEO           0x10
#define  FSL_SYSTEM          0x20
#define  FSL_INSTANCED       0x40
#define  FSL_NONSTALL_MAP    0x80    // Map must not stall for VB/IB locking
#define  FSL_VBIBPUSHDOWN    0x100   // Push down from vram on demand if target architecture supports it, used internally
#define  FSL_DIRECT          0x200   // Access VRAM directly if target architecture supports it, used internally
#define  FSL_LOCKED          0x400   // Internal use
#define  FSL_SYSTEM_CREATE   (FSL_WRITE | FSL_DISCARD | FSL_SYSTEM)
#define  FSL_SYSTEM_UPDATE   (FSL_WRITE | FSL_SYSTEM)
#define  FSL_VIDEO_CREATE    (FSL_WRITE | FSL_DISCARD | FSL_VIDEO)
#define  FSL_VIDEO_UPDATE    (FSL_WRITE | FSL_VIDEO)

#define FSL_ASYNC_DEFER_COPY (1u << 1)
#define FSL_FREE_AFTER_ASYNC (2u << 1)

struct IRenderMesh
{
	enum EMemoryUsageArgument
	{
		MEM_USAGE_COMBINED,
		MEM_USAGE_ONLY_SYSTEM,
		MEM_USAGE_ONLY_VIDEO,
		MEM_USAGE_ONLY_STREAMS,
	};

	//! Render mesh initialization parameters, that can be used to create RenderMesh from row pointers.
	struct SInitParamerers
	{
		InputLayoutHandle             eVertexFormat;
		ERenderMeshType           eType;

		uk                     pVertBuffer;
		i32                       nVertexCount;
		SPipTangents*             pTangents;
		SPipNormal*               pNormals;
		vtx_idx*                  pIndices;
		i32                       nIndexCount;
		PublicRenderPrimitiveType nPrimetiveType;
		i32                       nRenderChunkCount;
		i32                       nClientTextureBindID;
		bool                      bOnlyVideoBuffer;
		bool                      bPrecache;
		bool                      bLockForThreadAccess;

		SInitParamerers() : eVertexFormat(EDefaultInputLayouts::P3F_C4B_T2F), eType(eRMT_Static), pVertBuffer(0), nVertexCount(0), pTangents(0), pNormals(0), pIndices(0), nIndexCount(0),
			nPrimetiveType(prtTriangleList), nRenderChunkCount(0), nClientTextureBindID(0), bOnlyVideoBuffer(false), bPrecache(true), bLockForThreadAccess(false) {}
	};

	struct ThreadAccessLock
	{
		ThreadAccessLock(IRenderMesh* pRM)
			: m_pRM(pRM)
		{
			m_pRM->LockForThreadAccess();
		}

		~ThreadAccessLock()
		{
			m_pRM->UnLockForThreadAccess();
		}

	private:
		ThreadAccessLock(const ThreadAccessLock&);
		ThreadAccessLock& operator=(const ThreadAccessLock&);

	private:
		IRenderMesh* m_pRM;
	};

	// <interfuscator:shuffle>
	virtual ~IRenderMesh(){}

	//////////////////////////////////////////////////////////////////////////
	// Reference Counting.
	virtual void AddRef() = 0;
	virtual i32  Release() = 0;
	//////////////////////////////////////////////////////////////////////////

	//! Prevent rendering if video memory could not been allocated for it.
	virtual bool CanUpdate() = 0;
	virtual bool CanRender() = 0;

	//! Returns type name given to the render mesh on creation time.
	virtual tukk GetTypeName() = 0;

	//! Returns the name of the source given to the render mesh on creation time.
	virtual tukk     GetSourceName() const = 0;

	virtual i32             GetIndicesCount() = 0;
	virtual i32             GetVerticesCount() = 0;
	virtual InputLayoutHandle   GetVertexFormat() = 0;
	virtual ERenderMeshType GetMeshType() = 0;
	virtual float           GetGeometricMeanFaceArea() const = 0;

	virtual void            SetSkinned(bool bSkinned = true) = 0;
	virtual uint            GetSkinningWeightCount() const = 0;

	//! Create render buffers from render mesh. Returns the final size of the render mesh or ~0U on failure.
	virtual size_t SetMesh(CMesh& mesh, i32 nSecColorsSetOffset, u32 flags, const Vec3* pPosOffset, bool requiresLock) = 0;
	virtual void   CopyTo(IRenderMesh* pDst, i32 nAppendVtx = 0, bool bDynamic = false, bool fullCopy = true) = 0;
	virtual void   SetSkinningDataVegetation(struct SMeshBoneMapping_uint8* pBoneMapping) = 0;
	virtual void   SetSkinningDataCharacter(CMesh& mesh, u32 flags, struct SMeshBoneMapping_u16* pBoneMapping, struct SMeshBoneMapping_u16* pExtraBoneMapping) = 0;

	//! Creates an indexed mesh from this render mesh (accepts an optional pointer to an IIndexedMesh object that should be used).
	virtual IIndexedMesh* GetIndexedMesh(IIndexedMesh* pIdxMesh = 0) = 0;
	virtual i32           GetRenderChunksCount(IMaterial* pMat, i32& nRenderTrisCount) = 0;

	virtual IRenderMesh*  GenerateMorphWeights() = 0;
	virtual IRenderMesh*  GetMorphBuddy() = 0;
	virtual void          SetMorphBuddy(IRenderMesh* pMorph) = 0;

	virtual bool          UpdateVertices(ukk pVertBuffer, i32 nVertCount, i32 nOffset, i32 nStream, u32 copyFlags, bool requiresLock = true) = 0;
	virtual bool          UpdateIndices(const vtx_idx* pNewInds, i32 nInds, i32 nOffsInd, u32 copyFlags, bool requiresLock = true) = 0;
	virtual void          SetCustomTexID(i32 nCustomTID) = 0;
	virtual void          SetChunk(i32 nIndex, CRenderChunk& chunk) = 0;
	virtual void          SetChunk(IMaterial* pNewMat, i32 nFirstVertId, i32 nVertCount, i32 nFirstIndexId, i32 nIndexCount, float texelAreaDensity, i32 nMatID = 0) = 0;

	//! Assign array of render chunks.
	//! Initializes render element for each render chunk.
	virtual void                                 SetRenderChunks(CRenderChunk* pChunksArray, i32 nCount, bool bSubObjectChunks) = 0;

	virtual void                                 GenerateQTangents() = 0;
	virtual void                                 CreateChunksSkinned() = 0;
	virtual void                                 NextDrawSkinned() = 0;
	virtual IRenderMesh*                         GetVertexContainer() = 0;
	virtual void                                 SetVertexContainer(IRenderMesh* pBuf) = 0;
	virtual TRenderChunkArray&                   GetChunks() = 0;
	virtual TRenderChunkArray&                   GetChunksSkinned() = 0;
	virtual TRenderChunkArray&                   GetChunksSubObjects() = 0;
	virtual void                                 SetBBox(const Vec3& vBoxMin, const Vec3& vBoxMax) = 0;
	virtual void                                 GetBBox(Vec3& vBoxMin, Vec3& vBoxMax) = 0;
	virtual void                                 UpdateBBoxFromMesh() = 0;
	virtual u32*                              GetPhysVertexMap() = 0;
	virtual bool                                 IsEmpty() = 0;

	virtual u8*                               GetPosPtrNoCache(i32& nStride, u32 nFlags, i32 nOffset = 0) = 0;
	virtual u8*                               GetPosPtr(i32& nStride, u32 nFlags, i32 nOffset = 0) = 0;
	virtual u8*                               GetColorPtr(i32& nStride, u32 nFlags, i32 nOffset = 0) = 0;
	virtual u8*                               GetNormPtr(i32& nStride, u32 nFlags, i32 nOffset = 0) = 0;
	virtual u8*                               GetUVPtrNoCache(i32& nStride, u32 nFlags, i32 nOffset = 0) = 0;
	virtual u8*                               GetUVPtr(i32& nStride, u32 nFlags, i32 nOffset = 0) = 0;

	virtual u8*                               GetTangentPtr(i32& nStride, u32 nFlags, i32 nOffset = 0) = 0;
	virtual u8*                               GetQTangentPtr(i32& nStride, u32 nFlags, i32 nOffset = 0) = 0;

	virtual u8*                               GetHWSkinPtr(i32& nStride, u32 nFlags, i32 nOffset = 0, bool remapped = false) = 0;
	virtual u8*                               GetVelocityPtr(i32& nStride, u32 nFlags, i32 nOffset = 0) = 0;

	virtual void                                 UnlockStream(i32 nStream) = 0;
	virtual void                                 UnlockIndexStream() = 0;

	virtual vtx_idx*                             GetIndexPtr(u32 nFlags, i32 nOffset = 0) = 0;
	virtual const PodArray<std::pair<i32, i32>>* GetTrisForPosition(const Vec3& vPos, IMaterial* pMaterial) = 0;

	virtual float                                GetExtent(EGeomForm eForm) = 0;
	virtual void                                 GetRandomPoints(Array<PosNorm> points, CRndGen& seed, EGeomForm eForm, SSkinningData const* pSkinning = NULL) = 0;

	virtual void                                 Render(CRenderObject* pObj, const SRenderingPassInfo& passInfo) = 0;
	virtual void                                 AddRenderElements(IMaterial* pIMatInfo, CRenderObject* pObj, const SRenderingPassInfo& passInfo, i32 nSortId = EFSLIST_GENERAL, i32 nAW = 1) = 0;
	virtual void                                 AddRE(IMaterial* pMaterial, CRenderObject* pObj, IShader* pEf, const SRenderingPassInfo& passInfo, i32 nList, i32 nAW) = 0;
	virtual void                                 SetREUserData(float* pfCustomData, float fFogScale = 0, float fAlpha = 1) = 0;

	//! Debug draw this render mesh.
	virtual void DebugDraw(const struct SGeometryDebugDrawInfo& info, u32 nVisibleChunksMask = ~0) = 0;

	//! Returns mesh memory usage and add it to the DrxSizer (if not NULL).
	//! \param pSizer Sizer interface, can be NULL if caller only want to calculate size.
	//! \param nType see EMemoryUsageArgument.
	virtual size_t GetMemoryUsage(IDrxSizer* pSizer, EMemoryUsageArgument nType) const = 0;
	virtual void   GetMemoryUsage(IDrxSizer* pSizer) const = 0;

	//! Get allocated only in video memory or only in system memory.
	virtual i32   GetAllocatedBytes(bool bVideoMem) const = 0;
	virtual float GetAverageTrisNumPerChunk(IMaterial* pMat) = 0;
	virtual i32   GetTextureMemoryUsage(const IMaterial* pMaterial, IDrxSizer* pSizer = NULL, bool bStreamedIn = true) const = 0;
	virtual void  KeepSysMesh(bool keep) = 0;                                                                                     // HACK: temp workaround for GDC-888
	virtual void  UnKeepSysMesh() = 0;
	virtual void  SetMeshLod(i32 nLod) = 0;

	virtual void  LockForThreadAccess() = 0;
	virtual void  UnLockForThreadAccess() = 0;

	//! Sets the async update state - will sync before rendering to this.
	virtual  i32* SetAsyncUpdateState(void) = 0;
	virtual void          CreateRemappedBoneIndicesPair(const DynArray<JointIdType>& arrRemapTable, const uint pairGuid, ukk tag) = 0;
	virtual void          ReleaseRemappedBoneIndicesPair(const uint pairGuid) = 0;

	virtual void          OffsetPosition(const Vec3& delta) = 0;

	virtual bool          RayIntersectMesh(const Ray& ray, Vec3& hitpos, Vec3& p0, Vec3& p1, Vec3& p2, Vec2& uv0, Vec2& uv1, Vec2& uv2) = 0;

	// </interfuscator:shuffle>
};

#endif                                                                                                                          // _RenderMesh_H_
