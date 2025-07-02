// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*=============================================================================
   DeviceUpr.cpp : Device manager

   Revision история:
* Created by Khonich Andrey

   =============================================================================*/

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/DriverD3D.h>

#include <drx3D/CoreX/Thread/IJobUpr.h>
#include <drx3D/CoreX/Thread/IJobUpr_JobDelegator.h>

#include <drx3D/Render/DeviceSubmissionQueue_D3D11.h>

#if defined(RENDERER_ENABLE_LEGACY_PIPELINE) || defined(DRX_RENDERER_DIRECT3D)

CSubmissionQueue_DX11::CSubmissionQueue_DX11()
#if DURANGO_ENABLE_ASYNC_DIPS
	: m_nAsyncDipJobCounter(0)
	#if DRX_PLATFORM_DURANGO
	, m_nPush(0)
	, m_nPull(0)
	#endif
#endif
{
#if DRX_PLATFORM_DURANGO && DURANGO_ENABLE_ASYNC_DIPS == 1
	memset(m_arrParams, 0, sizeof(m_arrParams));
#endif
}

CSubmissionQueue_DX11::~CSubmissionQueue_DX11()
{
#if DURANGO_ENABLE_ASYNC_DIPS
	#if !DRX_PLATFORM_DURANGO
		DrxAlignedDelete(s_dip_queue);
	#endif
#endif
}

void CSubmissionQueue_DX11::Init()
{
#if DRX_PLATFORM_DURANGO
	m_numInvalidDrawcalls = 0;
#endif

	ClearState();

#if DURANGO_ENABLE_ASYNC_DIPS
	#if !DRX_PLATFORM_DURANGO
		DrxAlignedDelete(s_dip_queue);
		s_dip_queue = DrxAlignedNew<TAsyncDIPQueue>();
	#endif
#endif
}

//=============================================================================

ILINE bool CSubmissionQueue_DX11::ValidateDrawcall()
{
	if (CHWShader_D3D::s_nActivationFailMask != 0)
	{
		++m_numInvalidDrawcalls;
		return false;
	}

	return true;
}

void CSubmissionQueue_DX11::Draw(u32 nVerticesCount, u32 nStartVertex)
{
	if (!ValidateDrawcall())
	{
		return;
	}

#if DURANGO_ENABLE_ASYNC_DIPS
	if (CRenderer::CV_r_durango_async_dips)
	{
		if (s_CommandBuffer.m_current_offset * 4 >= SCommandBuffer::COMMAND_BUFFER_SIZE * 3)
		{
			gcpRendD3D.WaitForAsynchronousDevice();
			s_CommandBuffer.m_current_offset = 0;
		}

		tuk start = &s_CommandBuffer.m_buffer[s_CommandBuffer.m_current_offset];
		CommitDeviceStatesAsync();

		SDraw* pJob = new(s_CommandBuffer.Allocate(sizeof(SDraw)))SDraw;
		pJob->nVerticesCount = nVerticesCount;
		pJob->nStartVertex = nStartVertex;

	#if DRX_PLATFORM_DURANGO
		SubmitAsyncDipJob(start, &s_CommandBuffer.m_buffer[s_CommandBuffer.m_current_offset], gcpRendD3D->GetAsynchronousDeviceState());
	#else
		TExecuteAsyncDIP dip;
		TExecuteAsyncDIPPacket packet(start, &s_CommandBuffer.m_buffer[s_CommandBuffer.m_current_offset], gcpRendD3D->GetAsynchronousDeviceState());
		s_dip_queue->AddPacket(packet, JobUpr::eHighPriority, &dip, false);
	#endif // DRX_PLATFORM_DURANGO

		++m_nAsyncDipJobCounter;
		if (m_nAsyncDipJobCounter >= CRenderer::CV_r_durango_async_dips_sync)
		{
			gcpRendD3D->WaitForAsynchronousDevice();
			m_nAsyncDipJobCounter = 0;
		}

		return;
	}
#endif

	CommitDeviceStates();
	SELECT_GPU(gcpRendD3D->m_nGPU);
	gcpRendD3D->GetDeviceContext_Unsynchronized().Draw(nVerticesCount, nStartVertex);
	SELECT_ALL_GPU();
	GetDeviceObjectFactory().SyncToGPU();
}

void CSubmissionQueue_DX11::DrawInstanced(u32 nInstanceVerts, u32 nInstances, u32 nStartVertex, u32 nStartInstance)
{
	if (!ValidateDrawcall())
	{
		return;
	}

#if DURANGO_ENABLE_ASYNC_DIPS
	if (CRenderer::CV_r_durango_async_dips)
	{
		if (s_CommandBuffer.m_current_offset * 4 >= SCommandBuffer::COMMAND_BUFFER_SIZE * 3)
		{
			gcpRendD3D.WaitForAsynchronousDevice();
			s_CommandBuffer.m_current_offset = 0;
		}

		tuk start = &s_CommandBuffer.m_buffer[s_CommandBuffer.m_current_offset];
		CommitDeviceStatesAsync();

		SDrawInstanced* pJob = new(s_CommandBuffer.Allocate(sizeof(SDrawInstanced)))SDrawInstanced;
		pJob->nInstanceVerts = nInstanceVerts;
		pJob->nInstanceCount = nInstances;
		pJob->nStartVertex = nStartVertex;
		pJob->nStartInstance = nStartInstance;

	#if DRX_PLATFORM_DURANGO
		SubmitAsyncDipJob(start, &s_CommandBuffer.m_buffer[s_CommandBuffer.m_current_offset], gcpRendD3D->GetAsynchronousDeviceState());
	#else
		TExecuteAsyncDIP dip;
		TExecuteAsyncDIPPacket packet(start, &s_CommandBuffer.m_buffer[s_CommandBuffer.m_current_offset], gcpRendD3D->GetAsynchronousDeviceState());
		s_dip_queue->AddPacket(packet, JobUpr::eHighPriority, &dip, false);
	#endif // DRX_PLATFORM_DURANGO

		++m_nAsyncDipJobCounter;
		if (m_nAsyncDipJobCounter >= CRenderer::CV_r_durango_async_dips_sync)
		{
			gcpRendD3D->WaitForAsynchronousDevice();
			m_nAsyncDipJobCounter = 0;
		}

		return;
	}
#endif

	CommitDeviceStates();
	SELECT_GPU(gcpRendD3D->m_nGPU);
	gcpRendD3D->GetDeviceContext_Unsynchronized().DrawInstanced(nInstanceVerts, nInstances, nStartVertex, nStartInstance);
	SELECT_ALL_GPU();
	GetDeviceObjectFactory().SyncToGPU();
}

void CSubmissionQueue_DX11::DrawIndexedInstanced(u32 numIndices, u32 nInsts, u32 startIndex, u32 v0, u32 v1)
{
	if (!ValidateDrawcall())
	{
		return;
	}

#if DURANGO_ENABLE_ASYNC_DIPS
	if (CRenderer::CV_r_durango_async_dips)
	{
		if (s_CommandBuffer.m_current_offset * 4 >= SCommandBuffer::COMMAND_BUFFER_SIZE * 3)
		{
			gcpRendD3D.WaitForAsynchronousDevice();
			s_CommandBuffer.m_current_offset = 0;
		}

		tuk start = &s_CommandBuffer.m_buffer[s_CommandBuffer.m_current_offset];
		CommitDeviceStatesAsync();

		SDrawIndexedInstanced* pJob = new(s_CommandBuffer.Allocate(sizeof(SDrawIndexedInstanced)))SDrawIndexedInstanced;
		pJob->numIndices = numIndices;
		pJob->nInsts = nInsts;
		pJob->startIndex = startIndex;
		pJob->v0 = v0;
		pJob->v1 = v1;

	#if DRX_PLATFORM_DURANGO
		SubmitAsyncDipJob(start, &s_CommandBuffer.m_buffer[s_CommandBuffer.m_current_offset], gcpRendD3D->GetAsynchronousDeviceState());
	#else
		TExecuteAsyncDIP dip;
		TExecuteAsyncDIPPacket packet(start, &s_CommandBuffer.m_buffer[s_CommandBuffer.m_current_offset], gcpRendD3D->GetAsynchronousDeviceState());
		s_dip_queue->AddPacket(packet, JobUpr::eHighPriority, &dip, false);
	#endif // DRX_PLATFORM_DURANGO

		++m_nAsyncDipJobCounter;
		if (m_nAsyncDipJobCounter >= CRenderer::CV_r_durango_async_dips_sync)
		{
			gcpRendD3D->WaitForAsynchronousDevice();
			m_nAsyncDipJobCounter = 0;
		}

		return;
	}
#endif

	CommitDeviceStates();
	SELECT_GPU(gcpRendD3D->m_nGPU);
	gcpRendD3D->GetDeviceContext_Unsynchronized().DrawIndexedInstanced(numIndices, nInsts, startIndex, v0, v1);
	SELECT_ALL_GPU();
	GetDeviceObjectFactory().SyncToGPU();
}

void CSubmissionQueue_DX11::DrawIndexed(u32 numIndices, u32 startIndex, u32 vbOffset)
{
	if (!ValidateDrawcall())
	{
		return;
	}

#if DURANGO_ENABLE_ASYNC_DIPS
	if (CRenderer::CV_r_durango_async_dips)
	{
		if (s_CommandBuffer.m_current_offset * 4 >= SCommandBuffer::COMMAND_BUFFER_SIZE * 3)
		{
			gcpRendD3D.WaitForAsynchronousDevice();
			s_CommandBuffer.m_current_offset = 0;
		}

		tuk start = &s_CommandBuffer.m_buffer[s_CommandBuffer.m_current_offset];
		CommitDeviceStatesAsync();

		SDrawIndexed* pJob = new(s_CommandBuffer.Allocate(sizeof(SDrawIndexed)))SDrawIndexed;
		pJob->NumIndices = numIndices;
		pJob->nIndexOffset = startIndex;
		pJob->nVertexOffset = vbOffset;

	#if DRX_PLATFORM_DURANGO
		SubmitAsyncDipJob(start, &s_CommandBuffer.m_buffer[s_CommandBuffer.m_current_offset], gcpRendD3D->GetAsynchronousDeviceState());
	#else
		TExecuteAsyncDIP dip;
		TExecuteAsyncDIPPacket packet(start, &s_CommandBuffer.m_buffer[s_CommandBuffer.m_current_offset], gcpRendD3D->GetAsynchronousDeviceState());
		s_dip_queue->AddPacket(packet, JobUpr::eHighPriority, &dip, false);
	#endif // DRX_PLATFORM_DURANGO

		++m_nAsyncDipJobCounter;
		if (m_nAsyncDipJobCounter >= CRenderer::CV_r_durango_async_dips_sync)
		{
			gcpRendD3D->WaitForAsynchronousDevice();
			m_nAsyncDipJobCounter = 0;
		}

		return;
	}
#endif

	CommitDeviceStates();
	SELECT_GPU(gcpRendD3D->m_nGPU);
	gcpRendD3D->GetDeviceContext_Unsynchronized().DrawIndexed(numIndices, startIndex, vbOffset);
	SELECT_ALL_GPU();
	GetDeviceObjectFactory().SyncToGPU();
}

void CSubmissionQueue_DX11::Dispatch(u32 dX, u32 dY, u32 dZ)
{
	if (!ValidateDrawcall())
	{
		return;
	}

	CommitDeviceStates();
	SELECT_GPU(gcpRendD3D->m_nGPU);
	gcpRendD3D->GetDeviceContext_Unsynchronized().Dispatch(dX, dY, dZ);
	SELECT_ALL_GPU();
	GetDeviceObjectFactory().SyncToGPU();
}

void CSubmissionQueue_DX11::DispatchIndirect(ID3D11Buffer* pBufferForArgs, u32 AlignedOffsetForArgs)
{
	if (!ValidateDrawcall())
	{
		return;
	}

	CommitDeviceStates();
	SELECT_GPU(gcpRendD3D->m_nGPU);
	gcpRendD3D->GetDeviceContext().DispatchIndirect(pBufferForArgs, AlignedOffsetForArgs);
	SELECT_ALL_GPU();
	GetDeviceObjectFactory().SyncToGPU();
}

void CSubmissionQueue_DX11::DrawIndexedInstancedIndirect(ID3D11Buffer* pBufferForArgs, u32 AlignedOffsetForArgs)
{
	if (!ValidateDrawcall())
	{
		return;
	}

	CommitDeviceStates();
	SELECT_GPU(gcpRendD3D->m_nGPU);
	gcpRendD3D->GetDeviceContext().DrawIndexedInstancedIndirect((ID3D11Buffer*)pBufferForArgs, AlignedOffsetForArgs);
	SELECT_ALL_GPU();
	GetDeviceObjectFactory().SyncToGPU();
}

#define USE_BIT_TEST 1

#if !DEVICE_MANAGER_IMMEDIATE_STATE_WRITE
void CSubmissionQueue_DX11::BindConstantBuffers(SHADER_TYPE type, CDrxDeviceContextWrapper& rDeviceContext)
{
	#if USE_BIT_TEST
	using namespace DevManUtil;
	while (m_CB[type].dirty)
	{
		const unsigned lsb = m_CB[type].dirty & ~(m_CB[type].dirty - 1);
		const unsigned lbit = bsr(lsb);
		const unsigned hbit = bsf(~(m_CB[type].dirty) & ~(lsb - 1));
		const unsigned base = lbit;
		const unsigned count = hbit - lbit;
		#if DEVICE_MANAGER_USE_TYPE_DELEGATES
		rDeviceContext.TSSetConstantBuffers(type, base, count, &m_CB[type].buffers[base]);
		#else
		switch (type)
		{
		case TYPE_VS:
			rDeviceContext.VSSetConstantBuffers(base, count, &m_CB[TYPE_VS].buffers[base]);
			break;
		case TYPE_PS:
			rDeviceContext.PSSetConstantBuffers(base, count, &m_CB[TYPE_PS].buffers[base]);
			break;
		case TYPE_GS:
			rDeviceContext.GSSetConstantBuffers(base, count, &m_CB[TYPE_GS].buffers[base]);
			break;
		case TYPE_DS:
			rDeviceContext.DSSetConstantBuffers(base, count, &m_CB[TYPE_DS].buffers[base]);
			break;
		case TYPE_HS:
			rDeviceContext.HSSetConstantBuffers(base, count, &m_CB[TYPE_HS].buffers[base]);
			break;
		case TYPE_CS:
			rDeviceContext.CSSetConstantBuffers(base, count, &m_CB[TYPE_CS].buffers[base]);
			break;
		}
		#endif
		;
		const signed mask = iszero(32 - (signed)hbit) - 1;
		m_CB[type].dirty &= (~((1 << hbit) - (1 << lbit)) & mask);
	}
	#else
	for (size_t i = 0; i < 32 && m_CB[type].dirty; ++i)
	{
		if (m_CB[type].dirty & (1 << i))
		{
			const unsigned base = i;
			const unsigned count = 1;
			#if DEVICE_MANAGER_USE_TYPE_DELEGATES
			rDeviceContext.TSSetConstantBuffers(type, base, count, &m_CB[type].buffers[base]);
			#else
			switch (type)
			{
			case TYPE_VS:
				rDeviceContext.VSSetConstantBuffers(base, count, &m_CB[TYPE_VS].buffers[base]);
				break;
			case TYPE_PS:
				rDeviceContext.PSSetConstantBuffers(base, count, &m_CB[TYPE_PS].buffers[base]);
				break;
			case TYPE_GS:
				rDeviceContext.GSSetConstantBuffers(base, count, &m_CB[TYPE_GS].buffers[base]);
				break;
			case TYPE_DS:
				rDeviceContext.DSSetConstantBuffers(base, count, &m_CB[TYPE_DS].buffers[base]);
				break;
			case TYPE_HS:
				rDeviceContext.HSSetConstantBuffers(base, count, &m_CB[TYPE_HS].buffers[base]);
				break;
			case TYPE_CS:
				rDeviceContext.CSSetConstantBuffers(base, count, &m_CB[TYPE_CS].buffers[base]);
				break;
			}
			#endif
			;
			m_CB[type].dirty &= ~(1 << i);
		}
	}
	#endif
}

void CSubmissionQueue_DX11::BindOffsetConstantBuffers(SHADER_TYPE type, CDrxDeviceContextWrapper& rDeviceContext)
{
	#if (DRX_RENDERER_DIRECT3D >= 111)
		#if USE_BIT_TEST
	using namespace DevManUtil;
	while (m_CB[type].dirty1)
	{
		const unsigned lsb = m_CB[type].dirty1 & ~(m_CB[type].dirty1 - 1);
		const unsigned lbit = bsr(lsb);
		const unsigned hbit = bsf(~(m_CB[type].dirty1) & ~(lsb - 1));
		const unsigned base = lbit;
		const unsigned count = hbit - lbit;
		#if DEVICE_MANAGER_USE_TYPE_DELEGATES
		rDeviceContext.TSSetConstantBuffers1(type, base, count, &m_CB[type].buffers1[base], &m_CB[type].offsets[base], &m_CB[type].sizes[base]);
		#else
		switch (type)
		{
		case TYPE_VS:
			rDeviceContext.VSSetConstantBuffers1(base, count, &m_CB[TYPE_VS].buffers1[base], &m_CB[TYPE_VS].offsets[base], &m_CB[TYPE_VS].sizes[base]);
			break;
		case TYPE_PS:
			rDeviceContext.PSSetConstantBuffers1(base, count, &m_CB[TYPE_PS].buffers1[base], &m_CB[TYPE_PS].offsets[base], &m_CB[TYPE_PS].sizes[base]);
			break;
		case TYPE_GS:
			rDeviceContext.GSSetConstantBuffers1(base, count, &m_CB[TYPE_GS].buffers1[base], &m_CB[TYPE_GS].offsets[base], &m_CB[TYPE_GS].sizes[base]);
			break;
		case TYPE_DS:
			rDeviceContext.DSSetConstantBuffers1(base, count, &m_CB[TYPE_DS].buffers1[base], &m_CB[TYPE_DS].offsets[base], &m_CB[TYPE_DS].sizes[base]);
			break;
		case TYPE_HS:
			rDeviceContext.HSSetConstantBuffers1(base, count, &m_CB[TYPE_HS].buffers1[base], &m_CB[TYPE_HS].offsets[base], &m_CB[TYPE_HS].sizes[base]);
			break;
		case TYPE_CS:
			rDeviceContext.CSSetConstantBuffers1(base, count, &m_CB[TYPE_CS].buffers1[base], &m_CB[TYPE_CS].offsets[base], &m_CB[TYPE_CS].sizes[base]);
			break;
		}
		#endif
		;
		const signed mask = iszero(32 - (signed)hbit) - 1;
		m_CB[type].dirty1 &= (~((1 << hbit) - (1 << lbit)) & mask);
	}
		#else
	for (size_t i = 0; i < 32 && m_CB[type].dirty1; ++i)
	{
		if (m_CB[type].dirty1 & (1 << i))
		{
			const unsigned base = i;
			const unsigned count = 1;
			#if DEVICE_MANAGER_USE_TYPE_DELEGATES
			rDeviceContext.TSSetConstantBuffers1(type, base, count, &m_CB[type].buffers1[base], &m_CB[type].offsets[base], &m_CB[type].sizes[base]);
			#else
			switch (type)
			{
			case TYPE_VS:
				rDeviceContext.VSSetConstantBuffers1(base, count, &m_CB[TYPE_VS].buffers1[base], &m_CB[TYPE_VS].offsets[base], &m_CB[TYPE_VS].sizes[base]);
				break;
			case TYPE_PS:
				rDeviceContext.PSSetConstantBuffers1(base, count, &m_CB[TYPE_PS].buffers1[base], &m_CB[TYPE_PS].offsets[base], &m_CB[TYPE_PS].sizes[base]);
				break;
			case TYPE_GS:
				rDeviceContext.GSSetConstantBuffers1(base, count, &m_CB[TYPE_GS].buffers1[base], &m_CB[TYPE_GS].offsets[base], &m_CB[TYPE_GS].sizes[base]);
				break;
			case TYPE_DS:
				rDeviceContext.DSSetConstantBuffers1(base, count, &m_CB[TYPE_DS].buffers1[base], &m_CB[TYPE_DS].offsets[base], &m_CB[TYPE_DS].sizes[base]);
				break;
			case TYPE_HS:
				rDeviceContext.HSSetConstantBuffers1(base, count, &m_CB[TYPE_HS].buffers1[base], &m_CB[TYPE_HS].offsets[base], &m_CB[TYPE_HS].sizes[base]);
				break;
			case TYPE_CS:
				rDeviceContext.CSSetConstantBuffers1(base, count, &m_CB[TYPE_CS].buffers1[base], &m_CB[TYPE_CS].offsets[base], &m_CB[TYPE_CS].sizes[base]);
				break;
			}
			#endif
			;
			m_CB[type].dirty1 &= ~(1 << i);
		}
	}
		#endif
	#endif
}

void CSubmissionQueue_DX11::BindSamplers(SHADER_TYPE type, CDrxDeviceContextWrapper& rDeviceContext)
{
	#if USE_BIT_TEST
	while (m_Samplers[type].dirty)
	{
		using namespace DevManUtil;
		const unsigned lsb = m_Samplers[type].dirty & ~(m_Samplers[type].dirty - 1);
		const unsigned lbit = bsr(lsb);
		const unsigned hbit = bsf(~(m_Samplers[type].dirty) & ~(lsb - 1));
		const unsigned base = lbit;
		const unsigned count = hbit - lbit;
		#if DEVICE_MANAGER_USE_TYPE_DELEGATES
		rDeviceContext.TSSetSamplers(type, base, count, &m_Samplers[type].samplers[base]);
		#else
		switch (type)
		{
		case TYPE_VS:
			rDeviceContext.VSSetSamplers(base, count, &m_Samplers[TYPE_VS].samplers[base]);
			break;
		case TYPE_PS:
			rDeviceContext.PSSetSamplers(base, count, &m_Samplers[TYPE_PS].samplers[base]);
			break;
		case TYPE_GS:
			rDeviceContext.GSSetSamplers(base, count, &m_Samplers[TYPE_GS].samplers[base]);
			break;
		case TYPE_DS:
			rDeviceContext.DSSetSamplers(base, count, &m_Samplers[TYPE_DS].samplers[base]);
			break;
		case TYPE_HS:
			rDeviceContext.HSSetSamplers(base, count, &m_Samplers[TYPE_HS].samplers[base]);
			break;
		case TYPE_CS:
			rDeviceContext.CSSetSamplers(base, count, &m_Samplers[TYPE_CS].samplers[base]);
			break;
		}
		#endif
		;
		const signed mask = iszero(32 - (signed)hbit) - 1;
		m_Samplers[type].dirty &= (~((1 << hbit) - (1 << lbit)) & mask);
	}
	#else
	for (size_t i = 0; i < 32 && m_Samplers[type].dirty; ++i)
	{
		if (m_Samplers[type].dirty & (1 << i))
		{
			const unsigned base = i;
			const unsigned count = 1;
			#if DEVICE_MANAGER_USE_TYPE_DELEGATES
			rDeviceContext.TSSetSamplers(type, base, count, &m_Samplers[type].samplers[base]);
			#else
			switch (type)
			{
			case TYPE_VS:
				rDeviceContext.VSSetSamplers(base, count, &m_Samplers[TYPE_VS].samplers[base]);
				break;
			case TYPE_PS:
				rDeviceContext.PSSetSamplers(base, count, &m_Samplers[TYPE_PS].samplers[base]);
				break;
			case TYPE_GS:
				rDeviceContext.GSSetSamplers(base, count, &m_Samplers[TYPE_GS].samplers[base]);
				break;
			case TYPE_DS:
				rDeviceContext.DSSetSamplers(base, count, &m_Samplers[TYPE_DS].samplers[base]);
				break;
			case TYPE_HS:
				rDeviceContext.HSSetSamplers(base, count, &m_Samplers[TYPE_HS].samplers[base]);
				break;
			case TYPE_CS:
				rDeviceContext.CSSetSamplers(base, count, &m_Samplers[TYPE_CS].samplers[base]);
				break;
			}
			#endif
			;
			m_Samplers[type].dirty &= ~(1 << i);
		}
	}
	#endif
}

void CSubmissionQueue_DX11::BindSRVs(SHADER_TYPE type, CDrxDeviceContextWrapper& rDeviceContext)
{
	for (size_t j = 0; j < MAX_SRV_DIRTY; ++j)
	{
	#if USE_BIT_TEST
		using namespace DevManUtil;
		while (m_SRV[type].dirty[j])
		{
			const unsigned lsb = m_SRV[type].dirty[j] & ~(m_SRV[type].dirty[j] - 1);
			const unsigned lbit = bsr(lsb);
			const unsigned hbit = bsf(~(m_SRV[type].dirty[j]) & ~(lsb - 1));
			const unsigned base = j * 32 + lbit;
			const unsigned count = hbit - lbit;
			#if DEVICE_MANAGER_USE_TYPE_DELEGATES
			rDeviceContext.TSSetShaderResources(type, base, count, &m_SRV[type].views[base]);
			#else
			switch (type)
			{
			case TYPE_VS:
				rDeviceContext.VSSetShaderResources(base, count, &m_SRV[TYPE_VS].views[base]);
				break;
			case TYPE_PS:
				rDeviceContext.PSSetShaderResources(base, count, &m_SRV[TYPE_PS].views[base]);
				break;
			case TYPE_GS:
				rDeviceContext.GSSetShaderResources(base, count, &m_SRV[TYPE_GS].views[base]);
				break;
			case TYPE_DS:
				rDeviceContext.DSSetShaderResources(base, count, &m_SRV[TYPE_DS].views[base]);
				break;
			case TYPE_HS:
				rDeviceContext.HSSetShaderResources(base, count, &m_SRV[TYPE_HS].views[base]);
				break;
			case TYPE_CS:
				rDeviceContext.CSSetShaderResources(base, count, &m_SRV[TYPE_CS].views[base]);
				break;
			}
			#endif
			const signed mask = iszero(32 - (signed)hbit) - 1;
			m_SRV[type].dirty[j] &= (~((1 << hbit) - (1 << lbit)) & mask);
		}
	#else
		for (size_t i = 0; i < 32 && m_SRV[type].dirty[j]; ++i)
		{
			if (m_SRV[type].dirty[j] & (1 << i))
			{
				const unsigned base = j * 32 + i;
				const unsigned count = 1;
				#if DEVICE_MANAGER_USE_TYPE_DELEGATES
				rDeviceContext.TSSetShaderResources(type, base, count, &m_SRV[type].views[base]);
				#else
				switch (type)
				{
				case TYPE_VS:
					rDeviceContext.VSSetShaderResources(base, count, &m_SRV[TYPE_VS].views[base]);
					break;
				case TYPE_PS:
					rDeviceContext.PSSetShaderResources(base, count, &m_SRV[TYPE_PS].views[base]);
					break;
				case TYPE_GS:
					rDeviceContext.GSSetShaderResources(base, count, &m_SRV[TYPE_GS].views[base]);
					break;
				case TYPE_DS:
					rDeviceContext.DSSetShaderResources(base, count, &m_SRV[TYPE_DS].views[base]);
					break;
				case TYPE_HS:
					rDeviceContext.HSSetShaderResources(base, count, &m_SRV[TYPE_HS].views[base]);
					break;
				case TYPE_CS:
					rDeviceContext.CSSetShaderResources(base, count, &m_SRV[TYPE_CS].views[base]);
					break;
				}
				#endif
				m_SRV[type].dirty[j] &= ~(1 << i);
			}
		}
	#endif
	}
}

void CSubmissionQueue_DX11::BindUAVs(SHADER_TYPE type, CDrxDeviceContextWrapper& rDeviceContext)
{
	for (size_t j = 0; j < MAX_UAV_DIRTY; ++j)
	{
	#if USE_BIT_TEST
		using namespace DevManUtil;
		while (m_UAV[type].dirty[j])
		{
			const unsigned lsb = m_UAV[type].dirty[j] & ~(m_UAV[type].dirty[j] - 1);
			const unsigned lbit = bsr(lsb);
			const unsigned hbit = bsf(~(m_UAV[type].dirty[j]) & ~(lsb - 1));
			const unsigned base = j * 32 + lbit;
			const unsigned count = hbit - lbit;
			switch (type)
			{
			case TYPE_VS:
				assert(0 && "NOT IMPLEMENTED ON D3D11.0");
				break;
			case TYPE_PS:
				rDeviceContext.OMSetRenderTargetsAndUnorderedAccessViews(D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, NULL, NULL, base, count, &m_UAV[TYPE_PS].views[base], &m_UAV[TYPE_PS].counts[base]);
				break;
			case TYPE_GS:
				assert(0 && "NOT IMPLEMENTED ON D3D11.0");
				break;
			case TYPE_DS:
				assert(0 && "NOT IMPLEMENTED ON D3D11.0");
				break;
			case TYPE_HS:
				assert(0 && "NOT IMPLEMENTED ON D3D11.0");
				break;
			case TYPE_CS:
				rDeviceContext.CSSetUnorderedAccessViews(base, count, &m_UAV[TYPE_CS].views[base], &m_UAV[TYPE_CS].counts[base]);
				break;
			}
			const signed mask = iszero(32 - (signed)hbit) - 1;
			m_UAV[type].dirty[j] &= (~((1 << hbit) - (1 << lbit)) & mask);
		}
	#else
		for (size_t i = 0; i < 32 && m_UAV[type].dirty[j]; ++i)
		{
			if (m_UAV[type].dirty[j] & (1 << i))
			{
				const unsigned base = j * 32 + i;
				const unsigned count = 1;
				switch (type)
				{
				case TYPE_VS:
					assert(0 && "NOT IMPLEMENTED ON D3D11.0");
					break;
				case TYPE_PS:
					rDeviceContext.OMSetRenderTargetsAndUnorderedAccessViews(D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, NULL, NULL, base, count, &m_UAV[TYPE_PS].views[base], &m_UAV[TYPE_PS].counts[base]);
					break;
				case TYPE_GS:
					assert(0 && "NOT IMPLEMENTED ON D3D11.0");
					break;
				case TYPE_DS:
					assert(0 && "NOT IMPLEMENTED ON D3D11.0");
					break;
				case TYPE_HS:
					assert(0 && "NOT IMPLEMENTED ON D3D11.0");
					break;
				case TYPE_CS:
					rDeviceContext.CSSetUnorderedAccessViews(base, count, &m_UAV[TYPE_CS].views[base], &m_UAV[TYPE_CS].counts[base]);
					break;
				}
				m_UAV[type].dirty[j] &= ~(1 << i);
			}
		}
	#endif
	}
}

void CSubmissionQueue_DX11::BindIA(CDrxDeviceContextWrapper& rDeviceContext)
{
	#if USE_BIT_TEST
	using namespace DevManUtil;
	while (m_VBs.dirty)
	{
		const unsigned lsb = m_VBs.dirty & ~(m_VBs.dirty - 1);
		const unsigned lbit = bsr(lsb);
		const unsigned hbit = bsf(~(m_VBs.dirty) & ~(lsb - 1));
		const unsigned base = lbit;
		const unsigned count = hbit - lbit;
		const signed mask = iszero(32 - (signed)hbit) - 1;
		rDeviceContext.IASetVertexBuffers(base, count, &m_VBs.buffers[base], &m_VBs.strides[base], &m_VBs.offsets[base]);
		m_VBs.dirty &= (~((1 << hbit) - (1 << lbit)) & mask);
	}
	#else
	for (size_t i = 0; i < 32 && m_VBs.dirty; ++i)
	{
		if (m_VBs.dirty & (1 << i))
		{
			const unsigned base = i;
			const unsigned count = 1;
			rDeviceContext.IASetVertexBuffers(base, count, &m_VBs.buffers[base], &m_VBs.strides[base], &m_VBs.offsets[base]);
			m_VBs.dirty &= ~(1 << i);
		}
	}
	#endif

	if (m_IB.dirty)
	{
		rDeviceContext.IASetIndexBuffer(m_IB.buffer, (DXGI_FORMAT)m_IB.format, m_IB.offset);
		m_IB.dirty = 0;
	}
	if (m_VertexDecl.dirty)
	{
		rDeviceContext.IASetInputLayout(m_VertexDecl.decl);
		m_VertexDecl.dirty = false;
	}
	if (m_Topology.dirty)
	{
		rDeviceContext.IASetPrimitiveTopology(m_Topology.topology);
		m_Topology.dirty = false;
	}
}

void CSubmissionQueue_DX11::BindState(CDrxDeviceContextWrapper& rDeviceContext)
{
	if (m_RasterState.dirty)
	{
		rDeviceContext.RSSetState(m_RasterState.pRasterizerState);
		m_RasterState.dirty = false;
	}
	if (m_BlendState.dirty)
	{
		rDeviceContext.OMSetBlendState(m_BlendState.pBlendState, m_BlendState.BlendFactor, m_BlendState.SampleMask);
		m_BlendState.dirty = false;
	}
	if (m_DepthStencilState.dirty)
	{
		rDeviceContext.OMSetDepthStencilState(m_DepthStencilState.dss, m_DepthStencilState.stencilref);
		m_DepthStencilState.dirty = false;
	}
}

void CSubmissionQueue_DX11::BindShader(SHADER_TYPE type, CDrxDeviceContextWrapper& rDeviceContext)
{
	if (m_Shaders[type].dirty)
	{
		#if DEVICE_MANAGER_USE_TYPE_DELEGATES
		rDeviceContext.TSSetShader(type, (ID3D11DeviceChild*)m_Shaders[type].shader, NULL, 0);
		#else
		switch (type)
		{
		case TYPE_VS:
			rDeviceContext.VSSetShader((D3DVertexShader*)m_Shaders[TYPE_VS].shader, NULL, 0);
			break;
		case TYPE_PS:
			rDeviceContext.PSSetShader((D3DPixelShader*)m_Shaders[TYPE_PS].shader, NULL, 0);
			break;
		case TYPE_HS:
			rDeviceContext.HSSetShader((ID3D11HullShader*)m_Shaders[TYPE_HS].shader, NULL, 0);
			break;
		case TYPE_GS:
			rDeviceContext.GSSetShader((ID3D11GeometryShader*)m_Shaders[TYPE_GS].shader, NULL, 0);
			break;
		case TYPE_DS:
			rDeviceContext.DSSetShader((ID3D11DomainShader*)m_Shaders[TYPE_DS].shader, NULL, 0);
			break;
		case TYPE_CS:
			rDeviceContext.CSSetShader((ID3D11ComputeShader*)m_Shaders[TYPE_CS].shader, NULL, 0);
			break;
		}
		#endif
		m_Shaders[type].dirty = false;
	}
}

void CSubmissionQueue_DX11::CommitDeviceStates()
{
	DRX_PROFILE_FUNCTION(PROFILE_RENDERER);
	DETAILED_PROFILE_MARKER("CommitDeviceStates");

	#if DURANGO_ENABLE_ASYNC_DIPS
	gcpRendD3D->WaitForAsynchronousDevice();
	#endif
	#if defined(BUFFER_ENABLE_DIRECT_ACCESS) && (DRX_RENDERER_DIRECT3D >= 110) && (DRX_RENDERER_DIRECT3D < 120)
		{
		DETAILED_PROFILE_MARKER("InvalidateBuffers");
		u32k threadid = gRenDev->GetRenderThreadID();
		CDeviceObjectFactory::BufferInvalidationsT& invalidated = GetDeviceObjectFactory().m_buffer_invalidations[threadid];

		for (size_t i = 0, end = invalidated.size(); i < end; ++i)
			CDeviceObjectFactory::InvalidateGpuCache(
			  invalidated[i].buffer,
			  invalidated[i].base_ptr,
			  invalidated[i].size,
			  invalidated[i].offset);

		invalidated.clear();
	}
	#endif

	{
		DETAILED_PROFILE_MARKER("BindDeviceResources");
		CDrxDeviceContextWrapper& rDeviceContext = gcpRendD3D->GetDeviceContext_Unsynchronized();
		BindIA(rDeviceContext);
		BindState(rDeviceContext);
		for (signed i = 0; i < MAX_TYPE; ++i)
		{
			BindShader               (static_cast<SHADER_TYPE>(i), rDeviceContext);
			BindConstantBuffers      (static_cast<SHADER_TYPE>(i), rDeviceContext);
			BindOffsetConstantBuffers(static_cast<SHADER_TYPE>(i), rDeviceContext);
			BindSRVs                 (static_cast<SHADER_TYPE>(i), rDeviceContext);
			BindUAVs                 (static_cast<SHADER_TYPE>(i), rDeviceContext);
			BindSamplers             (static_cast<SHADER_TYPE>(i), rDeviceContext);
		}
	}
}
#endif
#endif
