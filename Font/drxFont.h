// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef DRXFONT_DRXFONT_H
#define DRXFONT_DRXFONT_H

#if !defined(USE_NULLFONT_ALWAYS)

	#include <map>

class CFFont;

class CDrxFont : public IDrxFont
{
	friend class CFFont;

public:
	CDrxFont(ISystem* pSystem);
	virtual ~CDrxFont();

	virtual void    Release();
	virtual IFFont* NewFont(tukk pFontName);
	virtual IFFont* GetFont(tukk pFontName) const;
	virtual void    SetRendererProperties(IRenderer* pRenderer);
	virtual void    GetMemoryUsage(IDrxSizer* pSizer) const;
	virtual string  GetLoadedFontNames() const;

public:
	void  UnregisterFont(tukk pFontName);
	bool  RndPropIsRGBA() const          { return m_rndPropIsRGBA; }
	float RndPropHalfTexelOffset() const { return m_rndPropHalfTexelOffset; }

private:
	typedef std::map<string, CFFont*> FontMap;
	typedef FontMap::iterator         FontMapItor;
	typedef FontMap::const_iterator   FontMapConstItor;

private:
	FontMap  m_fonts;
	ISystem* m_pSystem;
	bool     m_rndPropIsRGBA;
	float    m_rndPropHalfTexelOffset;
};

#endif

#endif // DRXFONT_DRXFONT_H
