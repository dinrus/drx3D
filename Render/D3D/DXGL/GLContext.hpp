// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   GLContext.hpp
//  Version:     v1.00
//  Created:     25/04/2013 by Valerio Guagliumi.
//  Описание: Declaration of the type CContext.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __GLCONTEXT__
#define __GLCONTEXT__

#include <drx3D/Render/GLCommon.hpp>
#include <drx3D/Render/GLFormat.hpp>
#include <drx3D/Render/GLState.hpp>
#include <drx3D/Render/GLResource.hpp>
#include <drx3D/Render/GLShader.hpp>
#include <drx3D/Render/GLView.hpp>

namespace NDrxOpenGL
{

	enum
	{
		// Slots are the virtual binding points accessible through the Direct3D interface
		MAX_STAGE_TEXTURE_SLOTS = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT,
		MAX_STAGE_SAMPLER_SLOTS = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT,
		MAX_STAGE_IMAGE_SLOTS = D3D11_PS_CS_UAV_REGISTER_COUNT,
		MAX_STAGE_STORAGE_BUFFER_SLOTS = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT,
		MAX_STAGE_CONSTANT_BUFFER_SLOTS = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT,

		MAX_STAGE_TEXTURE_UNITS = 64,
		MAX_STAGE_IMAGE_UNITS = 16,
		MAX_STAGE_STORAGE_BUFFER_UNITS = 64,
#if defined(DXGLES) && defined(DRX_PLATFORM_WINDOWS) 
		MAX_STAGE_UNIFORM_BUFFER_UNITS = 32,
#else
		MAX_STAGE_UNIFORM_BUFFER_UNITS = 16,
#endif

		MAX_TEXTURE_SLOTS = eST_NUM * MAX_STAGE_TEXTURE_SLOTS,
		MAX_SAMPLER_SLOTS = eST_NUM * MAX_STAGE_SAMPLER_SLOTS,
		MAX_IMAGE_SLOTS = eST_NUM * MAX_STAGE_IMAGE_SLOTS,
		MAX_STORAGE_BUFFER_SLOTS = eST_NUM * MAX_STAGE_STORAGE_BUFFER_SLOTS,
		MAX_CONSTANT_BUFFER_SLOTS = eST_NUM * MAX_STAGE_CONSTANT_BUFFER_SLOTS,

		// Units are the actual OpenGL binding points for resources - these are maximum numbers -
		// the actual supported counts are queried at runtime.
		MAX_TEXTURE_UNITS = eST_NUM * MAX_STAGE_TEXTURE_UNITS,
		MAX_IMAGE_UNITS = eST_NUM * MAX_STAGE_IMAGE_UNITS,
		MAX_STORAGE_BUFFER_UNITS = eST_NUM * MAX_STAGE_STORAGE_BUFFER_UNITS,
		MAX_UNIFORM_BUFFER_UNITS = eST_NUM * MAX_STAGE_UNIFORM_BUFFER_UNITS,

		MAX_RESOURCE_BINDINGS = 256,

		MAX_VERTEX_ATTRIBUTES = D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT,
#if DXGL_SUPPORT_VERTEX_ATTRIB_BINDING
		MAX_VERTEX_ATTRIB_BINDINGS = MAX_VERTEX_ATTRIBUTES,
#endif //DXGL_SUPPORT_VERTEX_ATTRIB_BINDING

#if DXGL_STREAMING_CONSTANT_BUFFERS
		MAX_PREVIOUS_FRAMES = 4,
#endif // DXGL_STREAMING_CONSTANT_BUFFERS
	};

	static_assert((u32)MAX_TEXTURE_SLOTS <= (u32)SUnitMap::MAX_TEXTURE_SLOT_IN_MAP, "Invalid texture slot limit!");
	static_assert((u32)MAX_SAMPLER_SLOTS <= (u32)SUnitMap::MAX_SAMPLER_SLOT_IN_MAP, "Invalid sampler slot slimit!");
	static_assert((u32)MAX_TEXTURE_UNITS <= (u32)SUnitMap::MAX_TEXTURE_UNIT_IN_MAP, "Invalid texture unit limit!");

	inline u32 TextureSlot(EShaderType eStage, u32 uIndex) { return uIndex + eStage * MAX_STAGE_TEXTURE_SLOTS; }
	inline u32 SamplerSlot(EShaderType eStage, u32 uIndex) { return uIndex + eStage * MAX_STAGE_SAMPLER_SLOTS; }
	inline u32 ImageSlot(EShaderType eStage, u32 uIndex) { return uIndex + eStage * MAX_STAGE_IMAGE_SLOTS; }
	inline u32 StorageBufferSlot(EShaderType eStage, u32 uIndex) { return uIndex + eStage * MAX_STAGE_STORAGE_BUFFER_SLOTS; }
	inline u32 ConstantBufferSlot(EShaderType eStage, u32 uIndex) { return uIndex + eStage * MAX_STAGE_CONSTANT_BUFFER_SLOTS; }

	struct SGlobalConfig
	{
#if DXGL_STREAMING_CONSTANT_BUFFERS
		static i32 iStreamingConstantBuffersMode;
		static i32 iStreamingConstantBuffersPersistentMap;
		static i32 iStreamingConstantBuffersGranularity;
		static i32 iStreamingConstantBuffersGrowth;
		static i32 iStreamingConstantBuffersMaxUnits;
#endif
		static i32 iMinFramePoolSize;
		static i32 iMaxFramePoolSize;
		static i32 iBufferUploadMode;
#if DXGL_ENABLE_SHADER_TRACING
		static i32 iShaderTracingMode;
		static i32 iShaderTracingHash;
		static i32 iVertexTracingID;
		static i32 iPixelTracingX;
		static i32 iPixelTracingY;
		static i32 iComputeTracingWorkGroupIDX;
		static i32 iComputeTracingWorkGroupIDY;
		static i32 iComputeTracingWorkGroupIDZ;
		static i32 iComputeTracingLocalInvocationIDX;
		static i32 iComputeTracingLocalInvocationIDY;
		static i32 iComputeTracingLocalInvocationIDZ;
#endif

		static void RegisterVariables();
	};

	// The reference values for the stencil operations
	struct SStencilRefCache
	{
		// Stencil reference value for front-facing polygons and non-polygons
		GLint m_iFrontFacesReference;

		// Stencil reference value for back-facing polygons
		GLint m_iBackFacesReference;
	};

	// Additional OpenGL internal state that is implicitly mapped to the depth stencil state
	struct SDepthStencilCache : SDepthStencilState
	{
		SStencilRefCache m_kStencilRef;
	};

	typedef SBlendState SBlendCache;

	// Additional OpenGL internal state that is implicitly mapped to the rasterizer state
	struct SRasterizerCache : SRasterizerState
	{
		bool m_bPolygonOffsetFillEnabled;
#if !DXGLES
		bool m_bPolygonOffsetLineEnabled;
#endif //!DXGLES
	};

	// Additional OpenGL internal state that is implicitly mapped to the texture unit state
	struct STextureUnitCache
	{
		CResourceName m_kTextureName;
		GLenum        m_eTextureTarget;
		GLuint        m_uSampler;
	};

	struct STextureSlot
	{
		SShaderTextureBasedView* m_pView;

		STextureSlot() :
			m_pView(NULL)
		{
		}

		bool operator==(const STextureSlot& kOther) const
		{
			return m_pView == kOther.m_pView;
		}

		bool operator!=(const STextureSlot& kOther) const
		{
			return !operator==(kOther);
		}
	};

	struct SSamplerSlot
	{
		SSamplerState* m_pSampler;

		SSamplerSlot(SSamplerState* pSampler = NULL)
			: m_pSampler(pSampler)
		{
		}

		bool operator==(const SSamplerSlot& kOther) const
		{
			return m_pSampler == kOther.m_pSampler;
		}

		bool operator!=(const SSamplerSlot& kOther) const
		{
			return !operator==(kOther);
		}
	};

	struct STextureUnitContext
	{
		STextureUnitCache      m_kCurrentUnitState;
		std::vector<STexture*> m_kModifiedTextures;
	};

#if DXGL_SUPPORT_SHADER_IMAGES

	struct SImageUnitCache
	{
		CResourceName                 m_kTextureName;
		SShaderImageViewConfiguration m_kConfiguration;
	};

#endif //DXGL_SUPPORT_SHADER_IMAGES

	// Additional OpenGL internal state that is implicitly mapped to the input assembler state
	struct SInputAssemblerCache
	{
		typedef u32 TAttributeBitMask;

		// Bit mask with 1 in the position of every vertex attribute enabled
		TAttributeBitMask m_uVertexAttribsEnabled;

		GLuint            m_auVertexAttribDivisors[MAX_VERTEX_ATTRIBUTES];

		struct SVertexAttribFormat
		{
			GLint     m_iSize;
			GLuint    m_uRelativeOffset;
			GLenum    m_eType;
			GLboolean m_bNormalized;
			GLboolean m_bInteger;

			bool operator==(const SVertexAttribFormat& kOther) const
			{
				return
					m_iSize == kOther.m_iSize &&
					m_uRelativeOffset == kOther.m_uRelativeOffset &&
					m_eType == kOther.m_eType &&
					m_bNormalized == kOther.m_bNormalized &&
					m_bInteger == kOther.m_bInteger;
			}

			bool operator!=(const SVertexAttribFormat& kOther) const
			{
				return !operator==(kOther);
			}
		};

		SVertexAttribFormat m_aVertexAttribFormats[MAX_VERTEX_ATTRIBUTES];

		GLuint              m_auVertexBindingIndices[MAX_VERTEX_ATTRIBUTES];

#if DXGL_SUPPORT_VERTEX_ATTRIB_BINDING
		// vertex buffer bindings (note that we don't cache buffer, offset and stride since those change for each call to glBindVertexBuffers
		GLuint m_auVertexBindingDivisors[MAX_VERTEX_ATTRIB_BINDINGS];

		// watermark so we can avoid sending redundant state
		GLint m_iLastNonZeroBindingSlot;
#endif //DXGL_SUPPORT_VERTEX_ATTRIB_BINDING
	};

	struct SInputAssemblerSlot
	{
		SBuffer* m_pVertexBuffer;
		u32   m_uStride;
		u32   m_uOffset;

		SInputAssemblerSlot()
			: m_pVertexBuffer(NULL)
			, m_uStride(0)
			, m_uOffset(0)
		{
		}
	};

	struct SColor
	{
		GLfloat m_afRGBA[4];

		inline bool operator==(const SColor& kOther) const
		{
			return memcmp(m_afRGBA, kOther.m_afRGBA, sizeof(m_afRGBA)) == 0;
		}

		inline bool operator!=(const SColor& kOther) const
		{
			return !operator==(kOther);
		}
	};

	struct SIndexedBufferBinding
	{
		CResourceName m_kName;
		SBufferRange  m_kRange;

		SIndexedBufferBinding(const CResourceName& kName = CResourceName(), SBufferRange kRange = SBufferRange())
			: m_kName(kName)
			, m_kRange(kRange)
		{
		}

		bool operator==(const SIndexedBufferBinding& kOther) const
		{
			return
				m_kName == kOther.m_kName &&
				m_kRange == kOther.m_kRange;
		}

		bool operator!=(const SIndexedBufferBinding& kOther) const
		{
			return !operator==(kOther);
		}
	};

	typedef SIndexedBufferBinding TIndexedBufferBinding;

#if DXGL_SUPPORT_VIEWPORT_ARRAY
	typedef GLdouble TDepthRangeValue;
	typedef GLfloat  TViewportValue;
#else
	typedef GLclampf TDepthRangeValue;
	typedef GLuint   TViewportValue;
#endif

	// The state that is not directly mapped to any of the DirectX 11 states
	struct SImplicitStateCache
	{
		// The name of the frame buffer currently to GL_DRAW_FRAMEBUFFER
		CResourceName m_kDrawFrameBuffer;

		// The name of the frame buffer currently to GL_READ_FRAMEBUFFER
		CResourceName m_kReadFrameBuffer;

#if !DXGLES
		// The enable state of GL_FRAMEBUFFER_SRGB
		bool m_bFrameBufferSRGBEnabled;
#endif //!DXGLES

#if DXGL_SUPPORT_MULTISAMPLED_TEXTURES
		bool       m_bSampleMaskEnabled;
		GLbitfield m_aSampleMask;
#endif //DXGL_SUPPORT_MULTISAMPLED_TEXTURES

		SColor m_akBlendColor;

		// Viewport xy ranges
		TViewportValue m_akViewportData[DXGL_NUM_SUPPORTED_VIEWPORTS * 4];

		// Viewport depth ranges
		TDepthRangeValue m_akDepthRangeData[DXGL_NUM_SUPPORTED_VIEWPORTS * 2];

#if DXGL_SUPPORT_TESSELLATION
		GLint m_iNumPatchControlPoints;
#endif //DXGL_SUPPORT_TESSELLATION

		// The buffer currently bound to each buffer target
		CResourceName m_akBuffersBound[eBB_NUM];

		// The buffer range currently bound to each indexed buffer
#if DXGL_SUPPORT_SHADER_STORAGE_BLOCKS
		TIndexedBufferBinding m_akStorageBuffersBound[MAX_STORAGE_BUFFER_UNITS];
#endif //DXGL_SUPPORT_SHADER_STORAGE_BLOCKS
		TIndexedBufferBinding m_akUniformBuffersBound[MAX_UNIFORM_BUFFER_UNITS];

		// Pixelstore values
		GLint m_iUnpackRowLength;
		GLint m_iUnpackImageHeight;
		GLint m_iUnpackAlignment;
		GLint m_iPackRowLength;
#if !DXGLES
		GLint m_iPackImageHeight;
#endif //!DXGLES
		GLint m_iPackAlignment;
	};

	// Stores the current state of an OpenGL so that the device can lazily update
	// states without the overhead of calling glGet* functions.
	struct SStateCache : SImplicitStateCache
	{
		SBlendCache          m_kBlend;
		SDepthStencilCache   m_kDepthStencil;
		SRasterizerCache     m_kRasterizer;
		SStencilRefCache     m_kStencilRef;
		STextureUnitCache    m_akTextureUnits[MAX_TEXTURE_UNITS];
#if DXGL_SUPPORT_SHADER_IMAGES
		SImageUnitCache      m_akImageUnits[MAX_IMAGE_UNITS];
#endif //DXGL_SUPPORT_SHADER_IMAGES
		SInputAssemblerCache m_kInputAssembler;
	};

#if DXGL_ENABLE_SHADER_TRACING

	struct SVertexShaderTraceHeader
	{
		u32 m_uVertexID;
	};

	struct SFragmentShaderTraceHeader
	{
		float m_afFragmentCoordX;
		float m_afFragmentCoordY;
	};

	struct SComputeShaderTraceHeader
	{
		u32 m_uGlobalInvocationIDX;
		u32 m_uGlobalInvocationIDY;
		u32 m_uGlobalInvocationIDZ;
	};

	struct SVertexShaderTraceInfo
	{
		SVertexShaderTraceHeader m_kHeader;
		u32                   m_uVertexIndex;
	};

	struct SFragmentShaderTraceInfo
	{
		SFragmentShaderTraceHeader m_kHeader;
		u32                     m_uFragmentCoordX;
		u32                     m_uFragmentCoordY;
	};

	struct SComputeShaderTraceInfo
	{
		SComputeShaderTraceHeader     m_kHeader;
		struct ID { u32 x, y, z; } m_kWorkGroupID, m_kLocalInvocationID;
	};

#endif //DXGL_ENABLE_SHADER_TRACING

#if DXGL_STREAMING_CONSTANT_BUFFERS

	struct SConstantBufferSlot
	{
		SBuffer*     m_pBuffer;
		SBufferRange m_kRange;

		SConstantBufferSlot() : m_pBuffer(NULL) {}
	};

	typedef SConstantBufferSlot TConstantBufferSlot;

	struct SContextFrame
	{
		GLsync          m_pEndFence;
		u32          m_uRefCount;
		SContextFrame** m_pFreeHead;
		SContextFrame*  m_pNext;

		SContextFrame(SContextFrame** pFreeHead)
			: m_pEndFence(NULL)
			, m_uRefCount(0)
			, m_pFreeHead(pFreeHead)
			, m_pNext(nullptr)
		{
		}

		~SContextFrame()
		{
			m_pNext = *m_pFreeHead;
			*m_pFreeHead = this;
		}

		void AddRef()
		{
			++m_uRefCount;
		}

		void Release()
		{
			if (--m_uRefCount == 0)
				this->~SContextFrame();
		}
	};

	struct SStreamingBuffer
	{
		CResourceName m_kName;
		u32        m_uSectionCapacity;
		u32        m_uRequestedSectionSize;
		u32        m_uNextPosition;
		u32        m_uEndPosition;
#if DXGL_SUPPORT_BUFFER_STORAGE
		GLvoid*       m_pMappedData;
#endif

		SStreamingBuffer()
			: m_uSectionCapacity(0)
			, m_uRequestedSectionSize(0)
			, m_uNextPosition(0)
			, m_uEndPosition(0)
#if DXGL_SUPPORT_BUFFER_STORAGE
			, m_pMappedData(NULL)
#endif
		{
		}

		~SStreamingBuffer()
		{
			if (m_kName.IsValid())
			{
				GLuint uName(m_kName.GetName());
				glDeleteBuffers(1, &uName);
			}
		}
	};

	struct SStreamingBufferContext
	{
		SContextFramePtr            m_spCurrentFrame;
		SContextFramePtr            m_aspPreviousFrames[MAX_PREVIOUS_FRAMES];
		SContextFrame*              m_pFreeFramesHead;
		std::vector<SContextFrame*> m_kFramePools;
		u32                      m_uPreviousFramesBegin, m_uPreviousFramesNum;
#if DXGL_SUPPORT_BUFFER_STORAGE
		bool                        m_bFlushNeeded;
#endif

		SStreamingBuffer m_akStreamingBuffers[MAX_UNIFORM_BUFFER_UNITS];
		u32           m_uNumStreamingBuffersUnits;

		SStreamingBufferContext();
		~SStreamingBufferContext();

		void SwitchFrame(CDevice* pDevice);
		void UpdateStreamingSizes(CDevice* pDevice);

		void UploadAndBindUniformData(CContext* pContext, const SConstantBufferSlot& kSlot, u32 uUnit);
		void FlushUniformData();
	};

#else

	typedef TIndexedBufferBinding TConstantBufferSlot;

#endif

	class CContext : public SList::TListEntry
	{
#if DXGL_STREAMING_CONSTANT_BUFFERS
		friend struct SStreamingBufferContext;
#endif
	public:
		CContext(
			CDevice* pDevice,
			const TRenderingContext& kRenderingContext,
			const TWindowContext& kDefaultWindowContext,
			u32 uIndex);
		~CContext();

		bool Initialize();

		// Context management
		u32                   IncReservationCount() { return ++m_uReservationCount; }
		u32                   DecReservationCount() { return --m_uReservationCount; }
		const TRenderingContext& GetRenderingContext() { return m_kRenderingContext; }
		const TWindowContext&    GetWindowContext() { return m_kWindowContext; }
		CDevice*                 GetDevice() { return m_pDevice; }
		u32                   GetIndex() { return m_uIndex; }
		CContext*                GetReservedContext() { return m_pReservedContext; }
		void                     SetReservedContext(CContext* pReservedContext) { m_pReservedContext = pReservedContext; }
		void                     SetWindowContext(const TWindowContext& kWindowContext);

		// Explicit state
		bool SetBlendState(const SBlendState& kState);
		bool SetRasterizerState(const SRasterizerState& kState);
		bool SetDepthStencilState(const SDepthStencilState& kDepthStencilState, GLint iStencilRef);
		void SetBlendColor(GLfloat afRed, GLfloat fGreen, GLfloat fBlue, GLfloat fAlpha);
		void SetSampleMask(GLbitfield aSampleMask);
		void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY eTopology);
		void SetViewports(u32 uNumViewports, const D3D11_VIEWPORT* pViewports);
		void SetScissorRects(u32 uNumRects, const D3D11_RECT* pRects);
		void SetRenderTargets(u32 uNumRTViews, SOutputMergerView** apRenderTargetViews, SOutputMergerView* pDepthStencilView);
		void SetShader(SShader* pShader, u32 uStage);
		void SetInputLayout(SInputLayout* pInputLayout);
		void SetVertexBuffer(u32 uSlot, SBuffer* pVertexBuffer, u32 uStride, u32 uOffset);
		void SetIndexBuffer(SBuffer* pIndexBuffer, GLenum eIndexType, GLuint uIndexStride, GLuint uOffset);
		void SetShaderResourceView(SShaderView* pShaderResourceView, u32 uStage, u32 uIndex);
		void SetUnorderedAccessView(SShaderView* pUnorderedResourceView, u32 uStage, u32 uIndex);
		void SetSampler(SSamplerState* pState, u32 uStage, u32 uIndex);
		void SetConstantBuffer(SBuffer* pConstantBuffer, SBufferRange kRange, u32 uStage, u32 uIndex);

		// Implicit state
		void SetShaderTexture(SShaderTextureBasedView* pView, u32 uStage, u32 uIndex);
#if DXGL_SUPPORT_SHADER_IMAGES
		void SetShaderImage(SShaderImageView* pView, u32 uStage, u32 uIndex);
#endif
#if DXGL_SUPPORT_SHADER_STORAGE_BLOCKS
		void   SetShaderBuffer(SShaderBufferView* pView, u32 uStage, u32 uIndex);
#endif
		void   SetUnpackRowLength(GLint iValue);
		void   SetUnpackImageHeight(GLint iValue);
		void   SetUnpackAlignment(GLint iValue);
		void   SetPackRowLength(GLint iValue);
		void   SetPackImageHeight(GLint iValue);
		void   SetPackAlignment(GLint iValue);
		GLenum BindBuffer(const CResourceName& kBufferName, EBufferBinding eBinding);
		GLenum BindBuffer(SBuffer* pBuffer, EBufferBinding eBinding);

		// Commands
		void ClearRenderTarget(SOutputMergerView* pRenderTargetView, const float afColor[4]);
		void ClearDepthStencil(SOutputMergerView* pDepthStencilView, bool bClearDepth, bool bClearStencil, float fDepthValue, u8 uStencilValue);
		void DrawIndexed(u32 uIndexCount, u32 uStartIndexLocation, u32 uBaseVertexLocation);
		void Draw(u32 uVertexCount, u32 uStartVertexLocation);
		void DrawIndexedInstanced(u32 uIndexCountPerInstance, u32 uInstanceCount, u32 uStartIndexLocation, u32 uBaseVertexLocation, u32 uStartInstanceLocation);
		void DrawInstanced(u32 uVertexCountPerInstance, u32 uInstanceCount, u32 uStartVertexLocation, u32 uStartInstanceLocation);
#if DXGL_SUPPORT_COMPUTE
		void Dispatch(u32 uGroupX, u32 uGroupY, u32 uGroupZ);
		void DispatchIndirect(u32 uIndirectOffset);
#endif
		void Flush();

		// Cached objects
		_smart_ptr<SFrameBuffer> AllocateFrameBuffer(const SFrameBufferConfiguration& kConfiguration);
		void                     RemoveFrameBuffer(SFrameBuffer* pFrameBuffer, SOutputMergerView* pInvalidView);
		_smart_ptr<SPipeline>    AllocatePipeline(const SPipelineConfiguration& kConfiguration);
		void                     RemovePipeline(SPipeline* pPipeline, SShader* pInvalidShader);
		_smart_ptr<SUnitMap>     AllocateUnitMap(_smart_ptr<SUnitMap> spConfiguration);

		// Copying
		void BlitFrameBuffer(SFrameBufferObject& kSrcFBO, SFrameBufferObject& kDstFBO, GLenum eSrcColorBuffer, GLenum eDstColorBuffer, GLint iSrcXMin, GLint iSrcYMin, GLint iSrcXMax, GLint iSrcYMax, GLint iDstXMin, GLint iDstYMin, GLint iDstXMax, GLint iDstYMax, GLbitfield uMask, GLenum eFilter);
		bool BlitOutputMergerView(SOutputMergerView* pSrcView, SOutputMergerView* pDstView, GLint iSrcXMin, GLint iSrcYMin, GLint iSrcXMax, GLint iSrcYMax, GLint iDstXMin, GLint iDstYMin, GLint iDstXMax, GLint iDstYMax, GLbitfield uMask, GLenum eFilter);
		void ReadbackFrameBufferAttachment(SFrameBufferObject& kFBO, GLenum eColorBuffer, GLint iXMin, GLint iYMin, GLsizei iWidth, GLint iHeight, GLenum eBaseFormat, GLenum eDataType, GLvoid* pvData);
		bool ReadBackOutputMergerView(SOutputMergerView* pView, GLint iXMin, GLint iYMin, GLsizei iWidth, GLint iHeight, GLvoid* pvData);

#if DXGL_ENABLE_SHADER_TRACING
		void TogglePixelTracing(bool bEnable, u32 uShaderHash, u32 uPixelX = 0, u32 uPixelY = 0);
		void ToggleVertexTracing(bool bEnable, u32 uShaderHash, u32 uVertexID = 0);
		void ToggleComputeTracing(bool bEnable, u32 uShaderHash, u32 uWorkGroupIDX, u32 uWorkGroupIDY = 0, u32 uWorkGroupIDZ = 0, u32 uLocalInvocationIDX = 0, u32 uLocalInvocationIDY = 0, u32 uLocalInvocationIDZ = 0);
#endif

#if DXGL_TRACE_CALLS
		void CallTraceWrite(tukk szTrace);
		void CallTraceFlush();
#endif

#if !DXGL_SUPPORT_COPY_IMAGE
		const CResourceName& GetCopyPixelBuffer() { return m_kCopyPixelBuffer; }
#endif

	private:
		bool GetBlendCache(SBlendCache& kCache);
		bool GetDepthStencilCache(SDepthStencilCache& kDepthStencilCache);
		bool GetRasterizerCache(SRasterizerCache& kCache);
		bool GetTextureUnitCache(u32 uUnit, STextureUnitCache& kCache);
#if DXGL_SUPPORT_SHADER_IMAGES
		bool GetImageUnitCache(u32 uUnit, SImageUnitCache& kCache);
#endif
		bool GetInputAssemblerCache(SInputAssemblerCache& kCache);
		bool GetImplicitStateCache(SImplicitStateCache& kCache);

		bool InitializePipeline(SPipeline* pPipeline);

		void BindUniformBuffer(const TIndexedBufferBinding& kBufferRange, u32 uUnit);
#if DXGL_SUPPORT_SHADER_STORAGE_BLOCKS
		void BindStorageBuffer(const TIndexedBufferBinding& kBufferRange, u32 uUnit);
#endif
#if DXGL_SUPPORT_SHADER_IMAGES
		void BindImage(const CResourceName& kName, SShaderImageViewConfiguration kConfiguration, u32 uUnit);
#endif

		void BindDrawFrameBuffer(const CResourceName& kName);
		void BindReadFrameBuffer(const CResourceName& kName);
		void SetNumPatchControlPoints(GLint uNumControlPoints);
#if !DXGL_SUPPORT_DRAW_WITH_BASE_VERTEX
		void SetVertexOffset(u32 uVertexOffset);
#endif

		void FlushDrawState();
#if DXGL_SUPPORT_VERTEX_ATTRIB_BINDING
		void FlushInputAssemblerStateVab();
#endif
		void FlushInputAssemblerState();
		void FlushFrameBufferState();
		void FlushPipelineState();
#if DXGL_SUPPORT_COMPUTE
		void FlushDispatchState();
#endif
		void FlushTextureUnits();
		void FlushUniformBufferUnits();
#if DXGL_SUPPORT_SHADER_STORAGE_BLOCKS
		void FlushStorageBufferUnits();
#endif
#if DXGL_SUPPORT_SHADER_IMAGES
		void FlushImageUnits();
#endif
		void FlushResourceUnits();
#if DXGL_ENABLE_SHADER_TRACING
		void FlushShaderTracingState();
#endif

		void SwitchFrame();

#if DXGL_ENABLE_SHADER_TRACING
		void PrepareTraceHeader(u32 uFirstVertex, u32 uFirstIndex);
		void BeginTrace(u32 uFirstVertex, u32 uFirstIndex);
		void EndTrace();
#endif

	private:
		u32              m_uIndex;
		CContext*           m_pReservedContext;
		CDevice*            m_pDevice;
		TRenderingContext   m_kRenderingContext;
		TWindowContext      m_kWindowContext;
		u32              m_uReservationCount;

		SStateCache         m_kStateCache;
		GLenum              m_ePrimitiveTopologyMode;
		GLenum              m_eIndexType;
		GLuint              m_uIndexStride;
		GLuint              m_uIndexOffset;
		SPipelinePtr        m_spPipeline;
		SFrameBufferPtr     m_spFrameBuffer;
		GLuint              m_uGlobalVAO;
		STextureUnitContext m_kTextureUnitContext;

		// State that is only synchronized during draw calls
		SFrameBufferConfiguration m_kFrameBufferConfig;
		SPipelineConfiguration    m_kPipelineConfiguration;
		SInputLayout*             m_pInputLayout;
		SInputAssemblerSlot       m_akInputAssemblerSlots[MAX_VERTEX_ATTRIBUTES];
		STextureSlot              m_akTextureSlots[MAX_TEXTURE_SLOTS];
		SSamplerSlot              m_akSamplerSlots[MAX_SAMPLER_SLOTS];
#if DXGL_SUPPORT_SHADER_IMAGES
		SImageUnitCache           m_akImageSlots[MAX_IMAGE_SLOTS];
#endif
#if DXGL_SUPPORT_SHADER_STORAGE_BLOCKS
		TIndexedBufferBinding   m_akStorageBufferSlots[MAX_STORAGE_BUFFER_SLOTS];
#endif
		TConstantBufferSlot     m_akConstantBufferSlots[MAX_CONSTANT_BUFFER_SLOTS];
		SUnitMap*               m_apResourceUnitMaps[eRUT_NUM];
#if DXGL_STREAMING_CONSTANT_BUFFERS
		SStreamingBufferContext m_kStreamingBuffers;
#endif
#if !DXGL_SUPPORT_DRAW_WITH_BASE_VERTEX
		u32 m_uVertexOffset;
#endif

		// Flags that tell which parts of the state need to be synchronized during the next draw call
		bool m_bFrameBufferStateDirty;
		bool m_bPipelineDirty;
		bool m_bInputLayoutDirty;
		bool m_bInputAssemblerSlotsDirty;
		bool m_abResourceUnitsDirty[eRUT_NUM];

#if DXGL_ENABLE_SHADER_TRACING
		CResourceName         m_kShaderTracingBuffer;
		TIndexedBufferBinding m_kReplacedStorageBuffer;
		EShaderType           m_eStageTracing;
		u32                m_uShaderTraceHash;
		u32                m_uShaderTraceCount;
		union
		{
			SVertexShaderTraceInfo   m_kVertex;
			SFragmentShaderTraceInfo m_kFragment;
			SComputeShaderTraceInfo  m_kCompute;
		} m_kStageTracingInfo;
#endif //DXGL_ENABLE_SHADER_TRACING

#if DXGL_TRACE_CALLS
		STraceFile m_kCallTrace;
#endif

#if !DXGL_SUPPORT_COPY_IMAGE
		CResourceName              m_kCopyPixelBuffer;
#endif
		SPipelineCompilationBuffer m_kPipelineCompilationBuffer;

		// Hash maps of persistent frame buffers, pipelines and sampler unit maps that can be re-used every time
		// a compatible configuration is requested.
		struct SFrameBufferCache* m_pFrameBufferCache;
		struct SPipelineCache*    m_pPipelineCache;
		struct SUnitMapCache*     m_pUnitMapCache;
	};

}

#endif
