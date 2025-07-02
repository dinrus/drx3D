// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _CRE_BREAKABLE_GLASS_BUFFER_
#define _CRE_BREAKABLE_GLASS_BUFFER_
#pragma once

#include <drx3D/CoreX/Renderer/RendElements/CREBreakableGlassConfig.h>

//==================================================================================================
// Name: CREBreakableGlassBuffer
// Desc: Breakable glass geometry buffer management
// Author: Chris Bunner
//==================================================================================================
class CREBreakableGlassBuffer
{
public:
	CREBreakableGlassBuffer();
	~CREBreakableGlassBuffer();

	// Geometry buffer types
	enum EBufferType
	{
		EBufferType_Plane = 0,
		EBufferType_Crack,
		EBufferType_Frag,
		EBufferType_Num
	};

	// Singleton access from Render Thread
	static CREBreakableGlassBuffer& RT_Instance();
	static void                     RT_ReleaseInstance();

	// Buffer allocation
	u32 RT_AcquireBuffer(const bool isFrag);
	bool   RT_IsBufferValid(u32k id, const EBufferType buffType) const;
	bool   RT_ClearBuffer(u32k id, const EBufferType buffType);

	// Buffer updates
	bool RT_UpdateVertexBuffer(u32k id, const EBufferType buffType, SVF_P3F_C4B_T2F* pVertData, u32k vertCount);
	bool RT_UpdateIndexBuffer(u32k id, const EBufferType buffType, u16* pIndData, u32k indCount);
	bool RT_UpdateTangentBuffer(u32k id, const EBufferType buffType, SPipTangents* pTanData, u32k tanCount);

	// Buffer drawing
	bool RT_DrawBuffer(u32k id, const EBufferType buffType);

	// Buffer state
	static u32k NoBuffer = 0;
	static const buffer_handle_t InvalidStreamHdl = ~0u;
	static u32k NumBufferSlots = GLASSCFG_MAX_NUM_ACTIVE_GLASS;

private:
	// Internal allocation
	void InitialiseBuffers();
	void ReleaseBuffers();

	// Internal drawing
	void DrawBuffer(u32k cyclicId, const EBufferType buffType, u32k indCount);

	// Common buffer state/data
	struct SBuffer
	{
		SBuffer()
			: verticesHdl(InvalidStreamHdl)
			, indicesHdl(InvalidStreamHdl)
			, tangentStreamHdl(InvalidStreamHdl)
			, lastIndCount(0)
		{
		}

		// Geometry buffers
		buffer_handle_t verticesHdl;
		buffer_handle_t indicesHdl;
		buffer_handle_t tangentStreamHdl;

		// State
		u32 lastIndCount;
	};

	// Buffer data
	ILINE bool IsVBufferValid(const SBuffer& buffer) const
	{
		return buffer.verticesHdl != InvalidStreamHdl && buffer.indicesHdl != InvalidStreamHdl && buffer.tangentStreamHdl != InvalidStreamHdl;
	}

	SBuffer m_buffer[EBufferType_Num][NumBufferSlots];
	u32  m_nextId[EBufferType_Num];

	// Singleton instance
	static CREBreakableGlassBuffer* s_pInstance;
};

#endif // _CRE_BREAKABLE_GLASS_BUFFER_
