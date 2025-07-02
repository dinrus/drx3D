// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#if RENDERER_SUPPORT_SCALEFORM
#include <drx3D/Render/DrxNameR.h>
#include <drx3D/Render/ScaleformPlayback.h>

#include <forward_list>

#include <drx3D/Render/D3D/GraphicsPipeline/PrimitiveRenderPass.h>

class CD3D9Renderer;
class CShader;

//////////////////////////////////////////////////////////////////////////
struct SSF_ResourcesD3D
{
	CDrxNameTSCRC m_shTech_SolidColor;
	CDrxNameTSCRC m_shTech_GlyphMultiplyTexture;
	CDrxNameTSCRC m_shTech_GlyphTexture;
	CDrxNameTSCRC m_shTech_GlyphAlphaTexture;
	CDrxNameTSCRC m_shTech_GlyphMultiplyTextureYUV;
	CDrxNameTSCRC m_shTech_GlyphTextureYUV;
	CDrxNameTSCRC m_shTech_GlyphMultiplyTextureYUVA;
	CDrxNameTSCRC m_shTech_GlyphTextureYUVA;
	CDrxNameTSCRC m_shTech_CxformMultiplyTexture;
	CDrxNameTSCRC m_shTech_CxformTexture;
	CDrxNameTSCRC m_shTech_CxformGouraudMultiplyNoAddAlpha;
	CDrxNameTSCRC m_shTech_CxformGouraudNoAddAlpha;
	CDrxNameTSCRC m_shTech_CxformGouraudMultiply;
	CDrxNameTSCRC m_shTech_CxformGouraud;
	CDrxNameTSCRC m_shTech_CxformGouraudMultiplyTexture;
	CDrxNameTSCRC m_shTech_CxformGouraudTexture;
	CDrxNameTSCRC m_shTech_CxformMultiply2Texture;
	CDrxNameTSCRC m_shTech_Cxform2Texture;
	CDrxNameTSCRC m_shTech_GlyphTextureMat;
	CDrxNameTSCRC m_shTech_GlyphTextureMatMul;
	CDrxNameTSCRC m_shTech_BlurFilter_Box1;
	CDrxNameTSCRC m_shTech_BlurFilter_Box2;
	CDrxNameTSCRC m_shTech_BlurFilterMul_Box1;
	CDrxNameTSCRC m_shTech_BlurFilterMul_Box2;
	CDrxNameTSCRC m_shTech_InnerShadow_Box2;
	CDrxNameTSCRC m_shTech_InnerShadowHighlight_Box2;
	CDrxNameTSCRC m_shTech_InnerShadowMul_Box2;
	CDrxNameTSCRC m_shTech_InnerShadowMulHighlight_Box2;
	CDrxNameTSCRC m_shTech_InnerShadowKnockout_Box2;
	CDrxNameTSCRC m_shTech_InnerShadowHighlightKnockout_Box2;
	CDrxNameTSCRC m_shTech_InnerShadowMulKnockout_Box2;
	CDrxNameTSCRC m_shTech_InnerShadowMulHighlightKnockout_Box2;
	CDrxNameTSCRC m_shTech_Shadow_Box2;
	CDrxNameTSCRC m_shTech_ShadowHighlight_Box2;
	CDrxNameTSCRC m_shTech_ShadowMul_Box2;
	CDrxNameTSCRC m_shTech_ShadowMulHighlight_Box2;
	CDrxNameTSCRC m_shTech_ShadowKnockout_Box2;
	CDrxNameTSCRC m_shTech_ShadowHighlightKnockout_Box2;
	CDrxNameTSCRC m_shTech_ShadowMulKnockout_Box2;
	CDrxNameTSCRC m_shTech_ShadowMulHighlightKnockout_Box2;
	CDrxNameTSCRC m_shTech_Shadowonly_Box2;
	CDrxNameTSCRC m_shTech_ShadowonlyHighlight_Box2;
	CDrxNameTSCRC m_shTech_ShadowonlyMul_Box2;
	CDrxNameTSCRC m_shTech_ShadowonlyMulHighlight_Box2;
	CDrxNameTSCRC* m_FilterTechnique[SSF_GlobalDrawParams::BlurCount];
	CDrxNameTSCRC* m_FillTechnique[SSF_GlobalDrawParams::FillCount][2];

	CShader* m_pShader;

	InputLayoutHandle m_vertexDecls[IScaleformPlayback::Vertex_Num];
	D3DVertexDeclaration* m_pVertexDecls[IScaleformPlayback::Vertex_Num];

	SamplerStateHandle samplerStateHandles[8];
	std::vector<CTexture*> m_renderTargets;
	std::vector<CTexture*> m_depthTargets;

	SSF_ResourcesD3D(CD3D9Renderer* pRenderer);
	~SSF_ResourcesD3D();

	CShader*  GetShader(CD3D9Renderer* pRenderer);
	CTexture* GetColorSurface(CD3D9Renderer* pRenderer, i32 nWidth, i32 nHeight, ETEX_Format eFormat, i32 nMaxWidth = 1 << 30, i32 nMaxHeight = 1 << 30);
	CTexture* GetStencilSurface(CD3D9Renderer* pRenderer, i32 nWidth, i32 nHeight, ETEX_Format eFormat);

	void FreeColorSurfaces();
	void FreeStencilSurfaces();

	struct CRenderPrimitiveHeap
	{
		typedef std::forward_list<CRenderPrimitive> CompiledRPList;

		CompiledRPList m_freeList;
		std::unordered_map<i32, CompiledRPList> m_useList;
		DrxCriticalSectionNonRecursive m_lock;

		CRenderPrimitive* GetUsablePrimitive(i32 key);
		void FreeUsedPrimitives(i32 key);
		void Clear();
	}
	m_PrimitiveHeap;

	struct STransientConstantBufferHeap
	{
		typedef std::forward_list<CConstantBufferPtr> TransientCBList;

		TransientCBList m_freeList;
		TransientCBList m_useList;
		DrxCriticalSectionNonRecursive m_lock;

		CConstantBuffer* GetUsableConstantBuffer();
		void FreeUsedConstantBuffers();

		~STransientConstantBufferHeap();
	}
	m_CBHeap;
};

#endif
