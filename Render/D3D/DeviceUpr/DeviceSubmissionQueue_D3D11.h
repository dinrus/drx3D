// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#define DEVICE_MANAGER_USE_TYPE_DELEGATES 1

#if DRX_PLATFORM_ORBIS
	#define DEVICE_MANAGER_IMMEDIATE_STATE_WRITE 1
#else
	#define DEVICE_MANAGER_IMMEDIATE_STATE_WRITE 0
#endif

class CDeviceTexture;
class CDeviceVertexBuffer;
class CDeviceIndexBuffer;
class CDrxDeviceContextWrapper;
class CConstantBuffer;

//===============================================================================================================

class CSubmissionQueue_DX11
{
	friend class CDrxDX12DeviceContext;

public:
	enum SHADER_TYPE
	{
		TYPE_VS = 0,
		TYPE_PS,
		TYPE_GS,
		TYPE_DS,
		TYPE_HS,
		TYPE_CS,
		MAX_TYPE,
	};

	static_assert(
		i32(eHWSC_Vertex  ) == i32(TYPE_VS) &&
		i32(eHWSC_Pixel   ) == i32(TYPE_PS) &&
		i32(eHWSC_Geometry) == i32(TYPE_GS) &&
		i32(eHWSC_Domain  ) == i32(TYPE_DS) &&
		i32(eHWSC_Hull    ) == i32(TYPE_HS) &&
		i32(eHWSC_Compute ) == i32(TYPE_CS),
		"SHADER_TYPE enumeration should match EHWShaderClass for performance reasons");

	enum
	{
		MAX_BOUND_CB       = 16,
		MAX_BOUND_VBS      = 16,
		MAX_BOUND_SRVS     = 128,
		MAX_BOUND_UAVS     = 64,
		MAX_BOUND_SAMPLERS = 16,
		MAX_SRV_DIRTY      = MAX_BOUND_SRVS / 32,
		MAX_UAV_DIRTY      = MAX_BOUND_UAVS / 32,
		SRV_DIRTY_SHIFT    = 5,
		SRV_DIRTY_MASK     = 31,
		UAV_DIRTY_SHIFT    = 5,
		UAV_DIRTY_MASK     = 31
	};

private:
	friend class CD3DRenderer;

private:

#if !DEVICE_MANAGER_IMMEDIATE_STATE_WRITE
	// Constant Buffers
	struct
	{
		D3DBuffer* buffers[MAX_BOUND_CB];
		D3DBuffer* buffers1[MAX_BOUND_CB];
		u32     offsets[MAX_BOUND_CB];
		u32     sizes[MAX_BOUND_CB];
		u32     dirty;
		u32     dirty1;
	} m_CB[MAX_TYPE];

	// Shader Resource Views
	struct
	{
		D3DShaderResource* views[MAX_BOUND_SRVS];
		u32             dirty[MAX_SRV_DIRTY];
	} m_SRV[MAX_TYPE];

	// Unordered Access Views
	struct
	{
		D3DUAV* views[MAX_BOUND_UAVS];
		u32  counts[MAX_BOUND_UAVS];
		u32  dirty[MAX_UAV_DIRTY];
	} m_UAV[MAX_TYPE];

	// SamplerStates
	struct
	{
		D3DSamplerState* samplers[MAX_BOUND_SAMPLERS];
		u32           dirty;
	} m_Samplers[MAX_TYPE];

	// VertexBuffers
	struct
	{
		D3DBuffer* buffers[MAX_BOUND_VBS];
		u32     offsets[MAX_BOUND_VBS];
		u32     strides[MAX_BOUND_VBS];
		u32     dirty;
	} m_VBs;

	// IndexBuffer
	struct
	{
		D3DBuffer* buffer;
		u32     offset;
		i32        format;
		u32     dirty;
	} m_IB;

	struct
	{
		D3DVertexDeclaration* decl;
		bool                  dirty;
	} m_VertexDecl;

	struct
	{
		D3D11_PRIMITIVE_TOPOLOGY topology;
		bool                     dirty;
	} m_Topology;

	struct
	{
		ID3D11DepthStencilState* dss;
		u32                   stencilref;
		bool                     dirty;
	} m_DepthStencilState;

	struct
	{
		ID3D11BlendState* pBlendState;
		float             BlendFactor[4];
		u32            SampleMask;
		bool              dirty;
	} m_BlendState;

	struct
	{
		ID3D11RasterizerState* pRasterizerState;
		bool                   dirty;
	} m_RasterState;

	struct
	{
		ID3D11Resource* shader;
		bool            dirty;
	} m_Shaders[MAX_TYPE];
#endif

	u32 m_numInvalidDrawcalls;

#if DURANGO_ENABLE_ASYNC_DIPS
	// counter to cap the number of device calls which are executed as a AsyncDIP Job
	u32 m_nAsyncDipJobCounter;

	void BindConstantBuffersAsync(SHADER_TYPE type);
	void BindOffsetConstantBuffersAsync(SHADER_TYPE type);
	void BindSRVsAsync(SHADER_TYPE type);
	void BindUAVsAsync(SHADER_TYPE type);
	void BindSamplersAsync(SHADER_TYPE type);
	void BindIAAsync();
	void BindShaderAsync(SHADER_TYPE type);
	void BindStateAsync();
	void CommitDeviceStatesAsync();
#endif

	void BindConstantBuffers(SHADER_TYPE type, CDrxDeviceContextWrapper& rDeviceContext);
	void BindOffsetConstantBuffers(SHADER_TYPE type, CDrxDeviceContextWrapper& rDeviceContext);
	void BindSRVs(SHADER_TYPE type, CDrxDeviceContextWrapper& rDeviceContext);
	void BindUAVs(SHADER_TYPE type, CDrxDeviceContextWrapper& rDeviceContext);
	void BindSamplers(SHADER_TYPE type, CDrxDeviceContextWrapper& rDeviceContext);
	void BindShader(SHADER_TYPE type, CDrxDeviceContextWrapper& rDeviceContext);
	void BindState(CDrxDeviceContextWrapper& rDeviceContext);
	void BindIA(CDrxDeviceContextWrapper& rDeviceContext);

	bool ValidateDrawcall();

public:
	CSubmissionQueue_DX11();
	~CSubmissionQueue_DX11();

	void           Init();
	void           ClearState();
	void           RT_Tick();

	inline void    BindConstantBuffer(SHADER_TYPE type, const CConstantBuffer* Buffer, u32 slot);
	inline void    BindConstantBuffer(SHADER_TYPE type, const CConstantBuffer* Buffer, u32 slot, u32 offset, u32 size);
	inline void    BindCBs(SHADER_TYPE type, const CConstantBuffer* const* Buffer, u32 start_slot, u32 count);
	inline void    BindConstantBuffer(SHADER_TYPE type, D3DBuffer* Buffer, u32 slot);
	inline void    BindConstantBuffer(SHADER_TYPE type, D3DBuffer* Buffer, u32 slot, u32 offset, u32 size);
	inline void    BindCBs(SHADER_TYPE type, D3DBuffer* const* Buffer, u32 start_slot, u32 count);
	inline void    BindSRV(SHADER_TYPE type, D3DShaderResource* SRV, u32 slot);
	inline void    BindSRV(SHADER_TYPE type, D3DShaderResource* const* SRV, u32 start_slot, u32 count);
	inline void    BindUAV(SHADER_TYPE type, D3DUAV* UAV, u32 counts, u32 slot);
	inline void    BindUAV(SHADER_TYPE type, D3DUAV* const* UAV, u32k* counts, u32 start_slot, u32 count);
	inline void    BindSampler(SHADER_TYPE type, D3DSamplerState* Sampler, u32 slot);
	inline void    BindSampler(SHADER_TYPE type, D3DSamplerState* const* Samplers, u32 start_slot, u32 count);
	inline void    BindVB(D3DBuffer* VB, u32 slot, u32 offset, u32 stride);
	inline void    BindVB(u32 start, u32 count, D3DBuffer* const* Buffers, u32k* offset, u32k* stride);
	inline void    BindIB(D3DBuffer* Buffer, u32 offset, DXGI_FORMAT format);
	inline void    BindVtxDecl(D3DVertexDeclaration* decl);
	inline void    BindTopology(D3D11_PRIMITIVE_TOPOLOGY top);
	inline void    BindShader(SHADER_TYPE type, ID3D11Resource* shader);
	inline void    SetDepthStencilState(ID3D11DepthStencilState* dss, u32 stencilref);
	inline void    SetBlendState(ID3D11BlendState* pBlendState, float* BlendFactor, u32 SampleMask);
	inline void    SetRasterState(ID3D11RasterizerState* pRasterizerState);

	void           CommitDeviceStates();
	void           Draw(u32 nVerticesCount, u32 nStartVertex);
	void           DrawInstanced(u32 nInstanceVerts, u32 nInsts, u32 nStartVertex, u32 nStartInstance);
	void           DrawIndexed(u32, u32, u32);
	void           DrawIndexedInstanced(u32 numIndices, u32 nInsts, u32 startIndex, u32 v0, u32 v1);
	void           DrawIndexedInstancedIndirect(ID3D11Buffer*, u32);
	void           Dispatch(u32, u32, u32);
	void           DispatchIndirect(ID3D11Buffer*, u32);

	u32 GetNumInvalidDrawcalls()
	{
		return m_numInvalidDrawcalls;
	}

#if (DRX_RENDERER_DIRECT3D >= 110) && (DRX_RENDERER_DIRECT3D < 120) && DRX_PLATFORM_DURANGO
private:
	#if DURANGO_ENABLE_ASYNC_DIPS
	// fields for the AsyncDIP Job queue
	enum { nQueueSize = 2048 }; // size of the queue to store the job parameters

	// submit new AsyncDIP packages to the queue
	void SubmitAsyncDipJob(tuk start, tuk end,  i32* nSyncState);

	// struct to define the job parameters
	struct SParam
	{
		tuk         start;
		tuk         end;
		 i32* nSyncState;
	};

	DRX_ALIGN(64) u32 m_nPush;
	DRX_ALIGN(64) u32 m_nPull;
	DRX_ALIGN(64) SParam m_arrParams[nQueueSize];
public:

	// execute async dip jobs (should be called by the job system during idle time)
	void ExecuteAsyncDIP();
	#endif // DURANGO_ENABLE_ASYNC_DIPS
#endif   // DRX_PLATFORM_DURANGO
};

#include <drx3D/Render/DeviceSubmissionQueue_D3D11.inl"

#if (DRX_RENDERER_DIRECT3D >= 110) && (DRX_RENDERER_DIRECT3D < 120) && DRX_PLATFORM_DURANGO
	#include <drx3D/Render/DeviceSubmissionQueue_D3D11_Durango.inl"
#endif
