// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifdef INCLUDE_SCALEFORM_SDK
#pragma warning(push)
#pragma warning(disable : 6326)// Potential comparison of a constant with another constant
#pragma warning(disable : 6011)// Dereferencing NULL pointer
#include <drx3D/CoreX/Platform/DrxWindows.h>
#include <GRenderer.h> // includes <windows.h>
#pragma warning(pop)

#include <vector>
#include <drx3D/Sys/GMemorySTLAlloc.h>
#include <drx3D/CoreX/Renderer/IRenderer.h>

struct IScaleformRecording;
struct IScaleformPlayback;

class CScaleformRecording;
class CScaleformPlayback;

struct GRendererCommandBuffer;
#endif

class CCachedData;

//////////////////////////////////////////////////////////////////////
struct GRendererCommandBufferReadOnly
{
	friend struct IScaleformPlayback;
	friend class CScaleformPlayback;

	template<class T> friend void SF_Playback(T* pRenderer, GRendererCommandBufferReadOnly* pBuffer);

public:
	GRendererCommandBufferReadOnly()
		: m_curReadPos(0)
		, m_curWritePos(0)
		, m_rawData(nullptr)
		, m_rawLen(0)
	{}

	virtual ~GRendererCommandBufferReadOnly()
	{}

	size_t Size() const
	{
		return m_curWritePos;
	}

	size_t Capacity() const
	{
		return m_rawLen;
	}

	void Render(IScaleformPlayback* pRenderer)
	{
		gEnv->pRenderer->SF_Playback(pRenderer, this);
	}

protected:
	void Assign(u8* blob, size_t len)
	{
		m_rawData = blob;
		m_rawLen = len;
	}

private:
	template<class T> void Playback(T* pRenderer);

	void                   ResetReadPos()
	{
		m_curReadPos = 0;
	}

	size_t GetReadPos() const
	{
		assert(IsAligned(m_curReadPos));
		return m_curReadPos;
	}

	ukk GetReadPosPtr() const
	{
		assert(m_curReadPos < Size());
		assert(IsAligned(m_curReadPos));
		return Size() > 0 ? &m_rawData[m_curReadPos] : 0;
	}

	template<typename T>
	T Read()
	{
		const size_t readPos = m_curReadPos;
		const size_t cellSize = Align(sizeof(T));

		assert(IsAligned(readPos));
		assert(readPos + cellSize <= Size());

		T arg = *((T*)&m_rawData[readPos]);
		m_curReadPos += cellSize;

		return arg;
	}

	template<typename T>
	const T& ReadRef()
	{
		const size_t readPos = m_curReadPos;
		const size_t cellSize = Align(sizeof(T));

		assert(IsAligned(readPos));
		assert(readPos + cellSize <= Size());

		const T& arg = *((const T*)&m_rawData[readPos]);
		m_curReadPos += cellSize;

		return arg;
	}

	void Skip(const size_t size, const bool alignSize)
	{
		assert(IsAligned(m_curReadPos));
		const size_t cellSize = alignSize ? Align(size) : size;
		assert(m_curReadPos + cellSize <= Size());
		m_curReadPos += cellSize;
	}

	size_t Align(size_t pos) const
	{
		return (pos + 3) & ~3;
	}

	bool IsAligned(size_t pos) const
	{
		return (pos & 3) == 0;
	}

private:
	size_t         m_curReadPos;
	size_t         m_curWritePos;
	u8* m_rawData;
	size_t         m_rawLen;
};

//////////////////////////////////////////////////////////////////////
enum EGRendererCommandBufferCmds
{
	GRCBA_Nop,

	//////////////////////////////////////////////////////////////////////////

	//GRCBA_GetRenderCaps,
	//GRCBA_CreateTexture,
	//GRCBA_CreateTextureYUV,

	//GRCBA_CreateRenderTarget,
	//GRCBA_SetDisplayRenderTarget,
	//GRCBA_PushRenderTarget,
	GRCBA_PopRenderTarget,
	GRCBA_PushTempRenderTarget,

	GRCBA_BeginDisplay,
	GRCBA_EndDisplay,

	GRCBA_SetMatrix,
	GRCBA_SetUserMatrix,
	GRCBA_SetCxform,

	GRCBA_PushBlendMode,
	GRCBA_PopBlendMode,

	GRCBA_SetPerspective3D,
	GRCBA_SetView3D,
	GRCBA_SetWorld3D,
	GRCBA_SetWorld3D_NullPtr,

	GRCBA_SetVertexData,
	GRCBA_SetIndexData,
	//GRCBA_ReleaseCachedData,

	GRCBA_DrawIndexedTriList,
	GRCBA_DrawLineStrip,

	GRCBA_LineStyleDisable,
	GRCBA_LineStyleColor,

	GRCBA_FillStyleDisable,
	GRCBA_FillStyleColor,
	GRCBA_FillStyleBitmap,
	GRCBA_FillStyleGouraud,

	GRCBA_DrawBitmaps,

	GRCBA_BeginSubmitMask,
	GRCBA_EndSubmitMask,
	GRCBA_DisableMask,

	//GRCBA_CheckFilterSupport,
	GRCBA_DrawBlurRect,
	GRCBA_DrawColorMatrixRect,

	//GRCBA_GetRenderStats,
	//GRCBA_GetStats,
	//GRCBA_ReleaseResources
};

#ifdef INCLUDE_SCALEFORM_SDK

//////////////////////////////////////////////////////////////////////
struct IScaleformRecording : public GRenderer
{
	// GRenderer interface
public:
	virtual bool           GetRenderCaps(RenderCaps* pCaps) override = 0;
	virtual GTexture*      CreateTexture() override = 0;
	virtual GTexture*      CreateTextureYUV() override = 0;

	virtual GRenderTarget* CreateRenderTarget() override = 0;
	virtual void           SetDisplayRenderTarget(GRenderTarget* pRT, bool setState = 1) override = 0;
	virtual void           PushRenderTarget(const GRectF& frameRect, GRenderTarget* pRT) override = 0;
	virtual void           PopRenderTarget() override = 0;
	virtual GTexture*      PushTempRenderTarget(const GRectF& frameRect, UInt targetW, UInt targetH, bool wantStencil = 0) override = 0;

	virtual void           BeginDisplay(GColor backgroundColor, const GViewport& viewport, Float x0, Float x1, Float y0, Float y1) override = 0;
	virtual void           EndDisplay() override = 0;

	virtual void           SetMatrix(const GMatrix2D& m) override = 0;
	virtual void           SetUserMatrix(const GMatrix2D& m) override = 0;
	virtual void           SetCxform(const Cxform& cx) override = 0;

	virtual void           PushBlendMode(BlendType mode) override = 0;
	virtual void           PopBlendMode() override = 0;

	virtual void           SetPerspective3D(const GMatrix3D& projMatIn) override = 0;
	virtual void           SetView3D(const GMatrix3D& viewMatIn) override = 0;
	virtual void           SetWorld3D(const GMatrix3D* pWorldMatIn) override = 0;

	virtual void           SetVertexData(ukk pVertices, i32 numVertices, VertexFormat vf, CacheProvider* pCache = 0) override = 0;
	virtual void           SetIndexData(ukk pIndices, i32 numIndices, IndexFormat idxf, CacheProvider* pCache = 0) override = 0;
	virtual void           ReleaseCachedData(CachedData* pData, CachedDataType type) override = 0;

	virtual void           DrawIndexedTriList(i32 baseVertexIndex, i32 minVertexIndex, i32 numVertices, i32 startIndex, i32 triangleCount) override = 0;
	virtual void           DrawLineStrip(i32 baseVertexIndex, i32 lineCount) override = 0;

	virtual void           LineStyleDisable() override = 0;
	virtual void           LineStyleColor(GColor color) override = 0;

	virtual void           FillStyleDisable() override = 0;
	virtual void           FillStyleColor(GColor color) override = 0;
	virtual void           FillStyleBitmap(const FillTexture* pFill) override = 0;
	virtual void           FillStyleGouraud(GouraudFillType fillType, const FillTexture* pTexture0 = 0, const FillTexture* pTexture1 = 0, const FillTexture* pTexture2 = 0) override = 0;

	virtual void           DrawBitmaps(BitmapDesc* pBitmapList, i32 listSize, i32 startIndex, i32 count, const GTexture* pTi, const GMatrix2D& m, CacheProvider* pCache = 0) override = 0;

	virtual void           BeginSubmitMask(SubmitMaskMode maskMode) override = 0;
	virtual void           EndSubmitMask() override = 0;
	virtual void           DisableMask() override = 0;

	virtual UInt           CheckFilterSupport(const BlurFilterParams& params) override = 0;
	virtual void           DrawBlurRect(GTexture* pSrcIn, const GRectF& inSrcRect, const GRectF& inDestRect, const BlurFilterParams& params, bool isLast = false) override = 0;
	virtual void           DrawColorMatrixRect(GTexture* pSrcIn, const GRectF& inSrcRect, const GRectF& inDestRect, const Float* pMatrix, bool isLast = false) override = 0;

	virtual void           GetRenderStats(Stats* pStats, bool resetStats = false) override = 0;
	virtual void           GetStats(GStatBag* pBag, bool reset = true) override = 0;
	virtual void           ReleaseResources() override = 0;

	// IScaleformRenderer interface
public:
	virtual IScaleformPlayback*    GetPlayback() const = 0;

	virtual void                   SetClearFlags(u32 clearFlags, ColorF clearColor = Clr_Transparent) = 0;
	virtual void                   SetCompositingDepth(float depth) = 0;

	virtual void                   SetStereoMode(bool stereo, bool isLeft) = 0;
	virtual void                   StereoEnforceFixedProjectionDepth(bool enforce) = 0;
	virtual void                   StereoSetCustomMaxParallax(float maxParallax) = 0;

	virtual void                   AvoidStencilClear(bool avoid) = 0;
	virtual void                   EnableMaskedRendering(bool enable) = 0;
	virtual void                   ExtendCanvasToViewport(bool extend) = 0;

	virtual void                   SetThreadIDs(u32 mainThreadID, u32 renderThreadID) = 0;
	virtual void                   SetRecordingCommandBuffer(GRendererCommandBuffer* pCmdBuf) = 0;
	virtual bool                   IsMainThread() const = 0;
	virtual bool                   IsRenderThread() const = 0;

	virtual void                   GetMemoryUsage(IDrxSizer* pSizer) const = 0;

	virtual std::vector<ITexture*> GetTempRenderTargets() const = 0;
};
#endif

//////////////////////////////////////////////////////////////////////
struct IScaleformPlayback
{
	enum BlendType
	{
		Blend_None       = 0,
		Blend_Normal     = 1,
		Blend_Layer      = 2,
		Blend_Multiply   = 3,
		Blend_Screen     = 4,
		Blend_Lighten    = 5,
		Blend_Darken     = 6,
		Blend_Difference = 7,
		Blend_Add        = 8,
		Blend_Subtract   = 9,
		Blend_Invert     = 10,
		Blend_Alpha      = 11,
		Blend_Erase      = 12,
		Blend_Overlay    = 13,
		Blend_HardLight  = 14
	};

	enum VertexFormat
	{
		Vertex_None      = 0,
		Vertex_XY16i     = 1,
		Vertex_XY32f     = 2, // Unsupported
		Vertex_XY16iC32  = 3,
		Vertex_XY16iCF32 = 4,
		Vertex_Glyph     = 5, // Custom value
		Vertex_Num
	};

	enum IndexFormat
	{
		Index_None = 0,
		Index_16   = 1,
		Index_32   = 2
	};

	enum GouraudFillType
	{
		GFill_Color,
		GFill_1Texture,
		GFill_1TextureColor,
		GFill_2Texture,
		GFill_2TextureColor,
		GFill_3Texture
	};

	enum SubmitMaskMode
	{
		Mask_Clear,
		Mask_Increment,
		Mask_Decrement
	};

	struct BlurFilterParams
	{
		u32 Mode;
		float  BlurX, BlurY;
		u32 Passes;
		Vec2   Offset;
		ColorB Color, Color2;
		float  Strength;
		float  cxform[4][2];
	};

	struct Matrix23
	{
		float                  m00, m01, m02;
		float                  m10, m11, m12;

		static const Matrix23& Identity() { static Matrix23 id = { 1.f, 0.f, 0.f, 0.f, 1.f, 0.f }; return id; }
	};

	struct Viewport
	{
		i32 Left, Top;
		i32 Width, Height;

		Viewport() {}
		Viewport(i32 l, i32 t, i32 w, i32 h) { Left = l; Top = t; Width = w; Height = h; }
	};

	struct RectF
	{
		float Left, Top;
		float Right, Bottom;

		float Width() const  { return Right - Left; }
		float Height() const { return Bottom - Top; }

		RectF() {}
		RectF(float l, float t, float r, float b) { Left = l; Top = t; Right = r; Bottom = b; }
	};

	struct BitmapDesc
	{
		RectF  Coords;
		RectF  TextureCoords;
		u32 Color;
	};

	enum BitmapWrapMode
	{
		Wrap_Repeat,
		Wrap_Clamp
	};

	enum BitmapSampleMode
	{
		Sample_Point,
		Sample_Linear
	};

	struct FillTexture
	{
		ITexture*        pTexture;
		Matrix23         TextureMatrix;
		BitmapWrapMode   WrapMode;
		BitmapSampleMode SampleMode;
	};

	enum DeviceDataType
	{
		DevDT_Vertex     = 1,
		DevDT_Index      = 2,
		DevDT_BitmapList = 3
	};

	struct DeviceData
	{
		DeviceDataType Type;
		u32         NumElements;
		u32         StrideSize;

		union
		{
			struct
			{
				IScaleformPlayback::VertexFormat VertexFormat;
				InputLayoutHandle::ValueType    eVertexFormat;
			};

			struct
			{
				IScaleformPlayback::IndexFormat IndexFormat;
				//	RenderIndexType eIndexFormat;
			};
		};

		ukk OriginalDataPtr;
		uintptr_t   DeviceDataHandle;
	};

	class Stats
	{
	public:
		u32 Triangles;
		u32 Lines;
		u32 Primitives;
		u32 Masks;
		u32 Filters;

		Stats() { Clear(); }
		void Clear() { Triangles = 0; Lines = 0; Primitives = 0; Masks = 0; Filters = 0; }
	};

	enum FilterModes
	{
		Filter_Blur         = 1,
		Filter_Shadow       = 2,
		Filter_Highlight    = 4,

		Filter_Knockout     = 0x100,
		Filter_Inner        = 0x200,
		Filter_HideObject   = 0x400,

		Filter_UserModes    = 0x0ffff,
		Filter_SkipLastPass = 0x10000,
		Filter_LastPassOnly = 0x20000,
	};

	// GRenderer interface
public:
	virtual ITexture* CreateRenderTarget() = 0;
	virtual void  SetDisplayRenderTarget(ITexture* pRT, bool setState = 1) = 0;
	virtual void  PushRenderTarget(const RectF& frameRect, ITexture* pRT) = 0;
	virtual void  PopRenderTarget() = 0;
	virtual i32 PushTempRenderTarget(const RectF& frameRect, u32 targetW, u32 targetH, bool wantClear = false, bool wantStencil = false) = 0;

	virtual void  BeginDisplay(ColorF backgroundColor, const Viewport& viewport, bool bScissor, const Viewport& scissor, const RectF& x0x1y0y1) = 0;
	virtual void  EndDisplay() = 0;

	virtual void  SetMatrix(const Matrix23& m) = 0;
	virtual void  SetUserMatrix(const Matrix23& m) = 0;
	virtual void  SetCxform(const ColorF& cx0, const ColorF& cx1) = 0;

	virtual void  PushBlendMode(BlendType mode) = 0;
	virtual void  PopBlendMode() = 0;

	virtual void  SetPerspective3D(const Matrix44& projMatIn) = 0;
	virtual void  SetView3D(const Matrix44& viewMatIn) = 0;
	virtual void  SetWorld3D(const Matrix44f* pWorldMatIn) = 0;

	virtual void  SetVertexData(const DeviceData* pVertices) = 0;
	virtual void  SetIndexData(const DeviceData* pIndices) = 0;

	virtual void  DrawIndexedTriList(i32 baseVertexIndex, i32 minVertexIndex, i32 numVertices, i32 startIndex, i32 triangleCount) = 0;
	virtual void  DrawLineStrip(i32 baseVertexIndex, i32 lineCount) = 0;

	virtual void  LineStyleDisable() = 0;
	virtual void  LineStyleColor(ColorF color) = 0;

	virtual void  FillStyleDisable() = 0;
	virtual void  FillStyleColor(ColorF color) = 0;
	virtual void  FillStyleBitmap(const FillTexture& Fill) = 0;
	virtual void  FillStyleGouraud(GouraudFillType fillType, const FillTexture& Texture0, const FillTexture& Texture1, const FillTexture& Texture2) = 0;

	virtual void  DrawBitmaps(const DeviceData* pBitmaps, i32 startIndex, i32 count, ITexture* pTi, const Matrix23& m) = 0;

	virtual void  BeginSubmitMask(SubmitMaskMode maskMode) = 0;
	virtual void  EndSubmitMask() = 0;
	virtual void  DisableMask() = 0;

	virtual void  DrawBlurRect(ITexture* pSrcIn, const RectF& inSrcRect, const RectF& inDestRect, const BlurFilterParams& params, bool isLast = false) = 0;
	virtual void  DrawColorMatrixRect(ITexture* pSrcIn, const RectF& inSrcRect, const RectF& inDestRect, const float* pMatrix, bool isLast = false) = 0;

	virtual void  ReleaseResources() = 0;

public:
	virtual DeviceData* CreateDeviceData(ukk pVertices, i32 numVertices, VertexFormat vf, bool bTemp = false) = 0;
	virtual DeviceData* CreateDeviceData(ukk pIndices, i32 numIndices, IndexFormat idxf, bool bTemp = false) = 0;
	virtual DeviceData* CreateDeviceData(const BitmapDesc* pBitmapList, i32 numBitmaps, bool bTemp = false) = 0;
	virtual void        ReleaseDeviceData(DeviceData* pData) = 0;

	// IScaleformRenderer interface
public:
	virtual void                   SetClearFlags(u32 clearFlags, ColorF clearColor = Clr_Transparent) = 0;
	virtual void                   SetCompositingDepth(float depth) = 0;

	virtual void                   SetStereoMode(bool stereo, bool isLeft) = 0;
	virtual void                   StereoEnforceFixedProjectionDepth(bool enforce) = 0;
	virtual void                   StereoSetCustomMaxParallax(float maxParallax) = 0;

	virtual void                   AvoidStencilClear(bool avoid) = 0;
	virtual void                   EnableMaskedRendering(bool enable) = 0;
	virtual void                   ExtendCanvasToViewport(bool extend) = 0;

	virtual void                   SetThreadIDs(u32 mainThreadID, u32 renderThreadID) = 0;
	virtual bool                   IsMainThread() const = 0;
	virtual bool                   IsRenderThread() const = 0;

	virtual void                   GetMemoryUsage(IDrxSizer* pSizer) const = 0;

	virtual std::vector<ITexture*> GetTempRenderTargets() const = 0;
};

//////////////////////////////////////////////////////////////////////
class CCachedData
{
public:
	void AddRef()
	{
		DrxInterlockedIncrement(&m_refCnt);
	}

	void Release()
	{
		long refCnt = DrxInterlockedDecrement(&m_refCnt);
		if (!refCnt)
			delete this;
	}

	const IScaleformPlayback::DeviceData* GetPtr()
	{
		//assert(m_lock == 1);
		return m_pData;
	}

	void Lock()
	{
		LockInternal(1);
	}

	void Unlock()
	{
		m_lock = 0;
	}

protected:
	void LockInternal(const LONG lockVal)
	{
		while (true)
		{
			if (0 == DrxInterlockedCompareExchange(&m_lock, lockVal, 0))
				break;
		}
	}

protected:
	CCachedData(const IScaleformPlayback::DeviceData* pData)
		: m_refCnt(1)
		, m_lock(0)
		, m_pData(pData)
	{}

	virtual ~CCachedData()
	{}

protected:
	 i32                          m_refCnt;
	 LONG                         m_lock;
	const IScaleformPlayback::DeviceData* m_pData;
};
