// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// Includes
#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/D3DREBreakableGlassBuffer.h>

#include <drx3D/Render/DriverD3D.h>
#include <drx3D/CoreX/Renderer/VertexFormats.h>

// Shared buffer-type constants
#define LOOSE_MEM_SIZE            3

#define MAX_NUM_LOOSE_PLANE_VERTS (GLASSCFG_MAX_NUM_PLANE_VERTS >> LOOSE_MEM_SIZE)
#define MAX_NUM_LOOSE_CRACK_VERTS (GLASSCFG_MAX_NUM_CRACK_VERTS >> LOOSE_MEM_SIZE)
#define MAX_NUM_LOOSE_PLANE_INDS  (GLASSCFG_MAX_NUM_PLANE_INDS >> LOOSE_MEM_SIZE)
#define MAX_NUM_LOOSE_CRACK_INDS  (GLASSCFG_MAX_NUM_CRACK_INDS >> LOOSE_MEM_SIZE)

// Unique buffer-type constants
struct SVertexConst
{
	SVertexConst(u32k numVerts, u32k numInds)
		: maxNumVerts(numVerts)
		, maxNumInds(numInds)
	{
	}

	u32 maxNumVerts;
	u32 maxNumInds;
};

const SVertexConst s_bufferConsts[CREBreakableGlassBuffer::EBufferType_Num] =
{
	// Plane buffer vertices
	SVertexConst(GLASSCFG_MAX_NUM_PLANE_VERTS,                          GLASSCFG_MAX_NUM_PLANE_INDS),

	// Crack buffer vertices
	SVertexConst(GLASSCFG_MAX_NUM_CRACK_VERTS,                          GLASSCFG_MAX_NUM_CRACK_INDS),

	// Frag buffer vertices
	SVertexConst(MAX_NUM_LOOSE_PLANE_VERTS + MAX_NUM_LOOSE_CRACK_VERTS,
	             MAX_NUM_LOOSE_PLANE_INDS + MAX_NUM_LOOSE_CRACK_INDS)
};

// Singleton
CREBreakableGlassBuffer* CREBreakableGlassBuffer::s_pInstance = NULL;

//--------------------------------------------------------------------------------------------------
// Name: Constructor
// Desc: Pre-allocated the geometry buffers
//--------------------------------------------------------------------------------------------------
CREBreakableGlassBuffer::CREBreakableGlassBuffer()
{
	InitialiseBuffers();
}

//--------------------------------------------------------------------------------------------------
// Name: Destructor
// Desc: Releases all geometry buffers
//--------------------------------------------------------------------------------------------------
CREBreakableGlassBuffer::~CREBreakableGlassBuffer()
{
	ReleaseBuffers();
}

//--------------------------------------------------------------------------------------------------
// Name: RT_Instance
// Desc: Returns (and creates) the static instance
//--------------------------------------------------------------------------------------------------
CREBreakableGlassBuffer& CREBreakableGlassBuffer::RT_Instance()
{
	ASSERT_IS_RENDER_THREAD(gRenDev->m_pRT);

	if (!s_pInstance)
	{
		s_pInstance = new CREBreakableGlassBuffer();
	}

	return *s_pInstance;
}

//--------------------------------------------------------------------------------------------------
// Name: RT_ReleaseInstance
// Desc: Releases the static instance
//--------------------------------------------------------------------------------------------------
void CREBreakableGlassBuffer::RT_ReleaseInstance()
{
	ASSERT_IS_RENDER_THREAD(gRenDev->m_pRT);
	SAFE_DELETE(s_pInstance);
}

//--------------------------------------------------------------------------------------------------
// Name: InitialiseBuffers
// Desc: Buffer creation called from render thread
//--------------------------------------------------------------------------------------------------
void CREBreakableGlassBuffer::InitialiseBuffers()
{
	for (uint i = 0; i < EBufferType_Num; ++i)
	{
		const SVertexConst& bufferConsts = s_bufferConsts[i];

		for (uint j = 0; j < NumBufferSlots; ++j)
		{
			SBuffer& buffer = m_buffer[i][j];

			// Allocate buffers
			buffer.verticesHdl = gRenDev->m_DevBufMan.Create(BBT_VERTEX_BUFFER, BU_DYNAMIC, bufferConsts.maxNumVerts * sizeof(SVF_P3F_C4B_T2F));
			buffer.indicesHdl = gRenDev->m_DevBufMan.Create(BBT_INDEX_BUFFER, BU_DYNAMIC, bufferConsts.maxNumInds * sizeof(u16));
			buffer.tangentStreamHdl = gRenDev->m_DevBufMan.Create(BBT_VERTEX_BUFFER, BU_DYNAMIC, bufferConsts.maxNumVerts * sizeof(SPipTangents));

			// Default state
			buffer.lastIndCount = 0;
		}

		// Reset ID counter
		m_nextId[i] = NumBufferSlots;
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: ReleaseBuffers
// Desc: Buffer release called from render thread
//--------------------------------------------------------------------------------------------------
void CREBreakableGlassBuffer::ReleaseBuffers()
{
	for (uint i = 0; i < EBufferType_Num; ++i)
	{
		for (uint j = 0; j < NumBufferSlots; ++j)
		{
			SBuffer& buffer = m_buffer[i][j];

			// Release buffers
			if (buffer.verticesHdl != InvalidStreamHdl)
			{
				gRenDev->m_DevBufMan.Destroy(buffer.verticesHdl);
				buffer.verticesHdl = InvalidStreamHdl;
			}

			if (buffer.indicesHdl != InvalidStreamHdl)
			{
				gRenDev->m_DevBufMan.Destroy(buffer.indicesHdl);
				buffer.indicesHdl = InvalidStreamHdl;
			}

			if (buffer.tangentStreamHdl != InvalidStreamHdl)
			{
				gRenDev->m_DevBufMan.Destroy(buffer.tangentStreamHdl);
				buffer.tangentStreamHdl = InvalidStreamHdl;
			}

			// Zero state
			buffer.lastIndCount = 0;
		}
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: RT_AcquireBuffer
// Desc: Supplies an id for the next buffer slot
//--------------------------------------------------------------------------------------------------
u32 CREBreakableGlassBuffer::RT_AcquireBuffer(const bool isFrag)
{
	const EBufferType buffType = isFrag ? EBufferType_Frag : EBufferType_Plane;
	u32& nextId = m_nextId[buffType];

	// Reset data	for new user
	++nextId;
	u32k id = nextId;
	u32k cyclicId = id % NumBufferSlots;

	SBuffer& buffer = m_buffer[buffType][cyclicId];
	buffer.lastIndCount = 0;

	// Crack buffers are auto-managed alongside Plane buffers
	if (buffType == EBufferType_Plane)
	{
		SBuffer& crackBuffer = m_buffer[EBufferType_Crack][cyclicId];
		crackBuffer.lastIndCount = 0;
		m_nextId[EBufferType_Crack] = id;
	}

	// Return id
	return id;
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: RT_IsBufferValid
// Desc: Cyclic buffer ids only valid for last "NumBufferSlots" requests
//--------------------------------------------------------------------------------------------------
bool CREBreakableGlassBuffer::RT_IsBufferValid(u32k id, const EBufferType buffType) const
{
	u32k nextId = m_nextId[buffType];
	return (id != NoBuffer) && (id > nextId - NumBufferSlots) && (id <= nextId);
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: RT_ClearBuffer
// Desc: Sets the last index count to zero, ensuring the buffer will no longer be drawn
//--------------------------------------------------------------------------------------------------
bool CREBreakableGlassBuffer::RT_ClearBuffer(u32k id, const EBufferType buffType)
{
	u32k cyclicId = id % NumBufferSlots;
	SBuffer& buffer = m_buffer[buffType][cyclicId];
	bool success = false;

	if (buffer.lastIndCount && RT_IsBufferValid(id, buffType))
	{
		buffer.lastIndCount = 0;

		// Crack buffers are auto-managed alongside Plane buffers
		if (buffType == EBufferType_Plane)
		{
			SBuffer& crackBuffer = m_buffer[EBufferType_Crack][cyclicId];
			crackBuffer.lastIndCount = 0;
		}

		success = true;
	}

	return success;
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: RT_UpdateVertexBuffer
// Desc: Attempts to update an vertex buffer
//--------------------------------------------------------------------------------------------------
bool CREBreakableGlassBuffer::RT_UpdateVertexBuffer(u32k id, const EBufferType buffType, SVF_P3F_C4B_T2F* pVertData, u32k vertCount)
{
	u32k cyclicId = id % NumBufferSlots;
	const SBuffer& buffer = m_buffer[buffType][cyclicId];
	const SVertexConst& bufferConsts = s_bufferConsts[buffType];

	bool success = false;

	// Update vertex data
	u32k vertexCount = min(vertCount, bufferConsts.maxNumVerts);

	if (vertexCount > 0 && buffer.verticesHdl && pVertData && RT_IsBufferValid(id, buffType))
	{
		const size_t devHdl = buffer.verticesHdl;
		DRX_ASSERT_MESSAGE(devHdl >= 0, "Updating an invalid VBuffer.");

		u32k stride = sizeof(SVF_P3F_C4B_T2F);
		gRenDev->m_DevBufMan.UpdateBuffer(devHdl, pVertData, vertexCount * stride);

		success = true;
	}

	return success;
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: RT_UpdateIndexBuffer
// Desc: Attempts to update an index buffer
//--------------------------------------------------------------------------------------------------
bool CREBreakableGlassBuffer::RT_UpdateIndexBuffer(u32k id, const EBufferType buffType, u16* pIndData, u32k indCount)
{
	u32k cyclicId = id % NumBufferSlots;
	SBuffer& buffer = m_buffer[buffType][cyclicId];
	const SVertexConst& bufferConsts = s_bufferConsts[buffType];

	bool success = false;

	// Update index data
	u32k indexCount = min(indCount, bufferConsts.maxNumInds);

	if (indexCount > 0 && buffer.indicesHdl && pIndData && RT_IsBufferValid(id, buffType))
	{
		const size_t devHdl = buffer.indicesHdl;
		DRX_ASSERT_MESSAGE(devHdl >= 0, "Updating an invalid VBuffer.");

		u32k stride = sizeof(u16);
		gRenDev->m_DevBufMan.UpdateBuffer(devHdl, pIndData, indexCount * stride);

		success = true;
	}

	// Update index count
	buffer.lastIndCount = success ? indexCount : 0;

	return success;
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: RT_UpdateTangentBuffer
// Desc: Attempts to update a tangent buffer
//--------------------------------------------------------------------------------------------------
bool CREBreakableGlassBuffer::RT_UpdateTangentBuffer(u32k id, const EBufferType buffType, SPipTangents* pTanData, u32k tanCount)
{
	u32k cyclicId = id % NumBufferSlots;
	const SBuffer& buffer = m_buffer[buffType][cyclicId];
	const SVertexConst& bufferConsts = s_bufferConsts[buffType];

	bool success = false;

	// Update tangent data
	u32k tangentCount = min(tanCount, bufferConsts.maxNumVerts);

	if (tangentCount > 0 && buffer.tangentStreamHdl != InvalidStreamHdl && pTanData && RT_IsBufferValid(id, buffType))
	{
		u32k stride = sizeof(SPipTangents);
		gRenDev->m_DevBufMan.UpdateBuffer(buffer.tangentStreamHdl, pTanData, tangentCount * stride);

		success = true;
	}

	return success;
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: RT_DrawBuffer
// Desc: Draws a buffer slot
//--------------------------------------------------------------------------------------------------
bool CREBreakableGlassBuffer::RT_DrawBuffer(u32k id, const EBufferType buffType)
{
	u32k cyclicId = id % NumBufferSlots;
	const SBuffer& buffer = m_buffer[buffType][cyclicId];
	const SVertexConst& bufferConsts = s_bufferConsts[buffType];

	bool success = false;

	if (buffer.lastIndCount && RT_IsBufferValid(id, buffType))
	{
		u32k count = buffer.lastIndCount;

		if (count >= 3)
		{
			DrawBuffer(cyclicId, buffType, count);
			success = true;
		}
	}

	return success;
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: DrawBuffer
// Desc: Draws a buffer slot
//--------------------------------------------------------------------------------------------------
void CREBreakableGlassBuffer::DrawBuffer(u32k cyclicId, const EBufferType buffType, u32k indCount)
{
	DRX_ASSERT_MESSAGE(indCount % 3 == 0, "Invalid number of glass indices");

	ASSERT_LEGACY_PIPELINE
		/*

	const SBuffer& buffer = m_buffer[buffType][cyclicId];
	const SVertexConst& bufferConsts = s_bufferConsts[buffType];

	if (IsVBufferValid(buffer))
	{
		CD3D9Renderer* pRenderer = gcpRendD3D;
		buffer_size_t vbOffset = 0, tbOffset = 0, ibOffset = 0;

		// Get buffers
		D3DVertexBuffer* pVBuffer = pRenderer->m_DevBufMan.GetD3DVB(buffer.verticesHdl, &vbOffset);
		D3DVertexBuffer* pTBuffer = pRenderer->m_DevBufMan.GetD3DVB(buffer.tangentStreamHdl, &tbOffset);
		D3DIndexBuffer*  pIBuffer = pRenderer->m_DevBufMan.GetD3DIB(buffer.indicesHdl, &ibOffset);

		if (pVBuffer && pTBuffer && pIBuffer)
		{
			i32k coreVStream = 0;
			i32k tanVStream = 1;

			// Bind data
			pRenderer->FX_SetVertexDeclaration(1 << VSF_TANGENTS, EDefaultInputLayouts::P3F_C4B_T2F);
			pRenderer->FX_SetVStream(coreVStream, pVBuffer, vbOffset, sizeof(SVF_P3F_C4B_T2F));
			pRenderer->FX_SetVStream(tanVStream, pTBuffer, tbOffset, sizeof(SPipTangents));
			pRenderer->FX_SetIStream(pIBuffer, ibOffset, Index16);

			// Draw
			pRenderer->FX_Commit();
			pRenderer->FX_DrawIndexedPrimitive(eptTriangleList, 0, 0, bufferConsts.maxNumVerts, 0, indCount);
		}
	}
	*/
}//-------------------------------------------------------------------------------------------------
