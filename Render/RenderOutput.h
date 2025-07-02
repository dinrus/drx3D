// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/optional.h>

class CRenderDisplayContext;
class CTexture;
struct SDynTexture;

//////////////////////////////////////////////////////////////////////////
//! Render Output is a target for a Render View rendering.
class CRenderOutput
{
public:
	typedef _smart_ptr<CTexture> TexSmartPtr;

public:
	CRenderOutput() = delete;
	~CRenderOutput() = default;

	CRenderOutput(CRenderDisplayContext* pDisplayContext);
	CRenderOutput(CTexture* pTexture, u32 clearFlags, ColorF clearColor, float clearDepth);
	CRenderOutput(CTexture* pTexture, CTexture* pDepthTexture, u32 clearFlags, ColorF clearColor, float clearDepth);
	CRenderOutput(SDynTexture* pDynTexture, i32 outputWidth, i32 outputHeight, bool useTemporaryDepthBuffer, u32 clearFlags, ColorF clearColor, float clearDepth);

	void                   BeginRendering(CRenderView* pRenderView, stl::optional<u32> overrideClearFlags = stl::nullopt);
	void                   EndRendering(CRenderView* pRenderView);

	//! HDR and Z Depth render target
	ETEX_Format            GetColorFormat() const;
	ETEX_Format            GetDepthFormat() const;

	CTexture*              GetColorTarget() const;
	CTexture*              GetDepthTarget() const;
	bool                   RequiresTemporaryDepthBuffer() const { return m_bUseTempDepthBuffer; }

	//! Retrieve display context associated with this RenderOutput.
	CRenderDisplayContext* GetDisplayContext() const { return m_pDisplayContext; };

	void                   SetViewport(const SRenderViewport& viewport);
	const SRenderViewport& GetViewport() const { return m_viewport; }

	//! Get resolution of the output target surface
	//! Note that Viewport can be smaller then this.
	Vec2_tpl<uint32_t>     GetOutputResolution() const { return { m_OutputWidth, m_OutputHeight }; }
	Vec2_tpl<uint32_t>     GetDisplayResolution() const; /* TODO: inline */

	void                   InitializeOutputResolution(i32 outputWidth, i32 outputHeight);
	void                   ChangeOutputResolution(i32 outputWidth, i32 outputHeight);

	void                   ReleaseResources();

	void                   InitializeDisplayContext();
	void                   ReinspectDisplayContext();

	u32                 m_hasBeenCleared = 0;

	//! Debug name for this Render Output
	STxt            m_name;
	// Unique id to identify each output
	u32                 m_uniqueId = 0;

	// Denotes if pass-through is HDR content
	bool                   m_bHDRRendering = false;

	bool                   m_bRenderToSwapChain = false;

private:
	u32                 m_OutputWidth  = -1;
	u32                 m_OutputHeight = -1;

	CRenderDisplayContext* m_pDisplayContext = nullptr;
	SDynTexture*           m_pDynTexture = nullptr;

	TexSmartPtr            m_pColorTarget = nullptr;
	TexSmartPtr            m_pDepthTarget = nullptr;

	u32                 m_clearTargetFlag = 0;
	ColorF                 m_clearColor = {};
	float                  m_clearDepth = 0.0f;

	bool                   m_bUseTempDepthBuffer = false;

	SRenderViewport        m_viewport;

private:
	void AllocateColorTarget();
	void AllocateDepthTarget();
};
typedef std::shared_ptr<CRenderOutput> CRenderOutputPtr;
