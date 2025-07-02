// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef DRXFONT_FFONT_H
#define DRXFONT_FFONT_H

#pragma once

#if !defined(USE_NULLFONT_ALWAYS)

	#include <vector>
	#include <drx3D/CoreX/Math/Drx_Math.h>
	#include <drx3D/CoreX/Math/Drx_Color.h>
	#include <drx3D/CoreX/String/DrxString.h>

struct ISystem;
class CDrxFont;
class CFontTexture;

class CFFont : public IFFont
{
public:
	struct SRenderingPass
	{
		ColorB m_color;
		Vec2   m_posOffset;
		i32    m_blendSrc;
		i32    m_blendDest;

		SRenderingPass()
			: m_color(255, 255, 255, 255)
			, m_posOffset(0, 0)
			, m_blendSrc(GS_BLSRC_SRCALPHA)
			, m_blendDest(GS_BLDST_ONEMINUSSRCALPHA)
		{
		}

		void GetMemoryUsage(IDrxSizer* pSizer) const {}
	};

	struct SEffect
	{
		string                      m_name;
		std::vector<SRenderingPass> m_passes;

		SEffect(tukk name)
			: m_name(name)
		{
			assert(name);
		}

		SRenderingPass* AddPass()
		{
			m_passes.push_back(SRenderingPass());
			return &m_passes[m_passes.size() - 1];
		}

		void ClearPasses()
		{
			m_passes.resize(0);
		}

		void GetMemoryUsage(IDrxSizer* pSizer) const
		{
			pSizer->AddObject(m_name);
			pSizer->AddObject(m_passes);
		}
	};

	typedef std::vector<SEffect> Effects;
	typedef Effects::iterator    EffectsIt;

public:
	virtual void         Release();
	virtual bool         Load(tukk pFontFilePath, u32 width, u32 height, u32 flags);
	virtual bool         Load(tukk pXMLFile);
	virtual void         Free();
	virtual void         DrawString(float x, float y, tukk pStr, const bool asciiMultiLine, const STextDrawContext& ctx);
	virtual void         DrawString(float x, float y, float z, tukk pStr, const bool asciiMultiLine, const STextDrawContext& ctx);
	virtual Vec2         GetTextSize(tukk pStr, const bool asciiMultiLine, const STextDrawContext& ctx);
	virtual size_t       GetTextLength(tukk pStr, const bool asciiMultiLine) const;
	virtual void         WrapText(string& result, float maxWidth, tukk pStr, const STextDrawContext& ctx);
	virtual void         GetMemoryUsage(IDrxSizer* pSizer) const;
	virtual void         GetGradientTextureCoord(float& minU, float& minV, float& maxU, float& maxV) const;
	virtual u32 GetEffectId(tukk pEffectName) const;

public:
	virtual void RenderCallback(float x, float y, float z, tukk pStr, const bool asciiMultiLine, const STextDrawContext& ctx, IRenderAuxGeom *pAux);

public:
	CFFont(ISystem* pSystem, CDrxFont* pDrxFont, tukk pFontName);

	bool          InitTexture();
	bool          InitCache();

	CFontTexture* GetFontTexture() const { return m_pFontTexture; }
	const string& GetName() const        { return m_name; }

	SEffect*      AddEffect(tukk pEffectName);
	SEffect*      GetDefaultEffect();

private:
	virtual ~CFFont();

	void Prepare(tukk pStr, bool updateTexture);
	Vec2 GetTextSizeUInternal(tukk pStr, const bool asciiMultiLine, const STextDrawContext& ctx);

private:
	string           m_name;
	string           m_curPath;

	CFontTexture*    m_pFontTexture;

	size_t           m_fontBufferSize;
	u8*   m_pFontBuffer;

	i32              m_texID;

	ISystem*         m_pSystem;
	CDrxFont*        m_pDrxFont;

	bool             m_fontTexDirty;

	Effects          m_effects;

};

#endif

#endif // DRXFONT_FFONT_H
