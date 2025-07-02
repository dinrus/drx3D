// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _GTEXTURE_XRENDER_H_
#define _GTEXTURE_XRENDER_H_

#pragma once

#ifdef INCLUDE_SCALEFORM_SDK

#include <drx3D/CoreX/Platform/platform.h>
#include <drx3D/CoreX/Renderer/IRenderer.h>
#include <drx3D/CoreX/Renderer/IShader.h>

#pragma warning(push)
#pragma warning(disable : 6326)   // Potential comparison of a constant with another constant
#include <drx3D/CoreX/Platform/DrxWindows.h>
#include <GRenderer.h>            // includes <windows.h>
#pragma warning(pop)

#include <drx3D/CoreX/Project/ProjectDefines.h>
#include "GImageInfo.h"

class GRendererXRender;
class GImageBase;
class ITexture;

class GTextureXRenderBase:public GTexture
{
	// GTexture interface
public:
	virtual GRenderer* GetRenderer() const;

	virtual Handle GetUserData() const       { return m_userData; }
	virtual void   SetUserData(Handle hData) { m_userData = hData; }

	virtual void AddChangeHandler(ChangeHandler* pHandler)    {}
	virtual void RemoveChangeHandler(ChangeHandler* pHandler) {}

	// GTextureXRenderBase interface
public:
	virtual bool InitTextureFromFile(tukk pFilename) = 0;
	virtual bool InitTextureFromTexId(i32 texid)            = 0;

public:
	GTextureXRenderBase(GRenderer* pRenderer, i32 texID = -1, ETEX_Format fmt = eTF_Unknown, bool bIsTemp = false);
	virtual ~GTextureXRenderBase();

	i32 GetID() const { return m_texID; } // GetID() non-virtual / m_texID member of base for performance reasons to allow inlining of GetID() calls
	i32* GetIDPtr() { return &m_texID; }

	ETEX_Format GetFmt() const { return m_fmt; }
	bool        IsYUV() const  { return m_fmt == eTF_YUV || m_fmt == eTF_YUVA; }

public:
#if defined(ENABLE_FLASH_INFO)
	static size_t GetTextureMemoryUsed()  { return (size_t) ms_textureMemoryUsed; }
	static u32 GetNumTextures()        { return (u32) ms_numTexturesCreated; }
	static u32 GetNumTexturesTotal()   { return (u32) ms_numTexturesCreatedTotal; }
	static u32 GetNumTexturesTempRT()  { return (u32) ms_numTexturesCreatedTemp; }
	static i32  GetFontCacheTextureID() { return ms_fontCacheTextureID; }
#endif

protected:
#if defined(ENABLE_FLASH_INFO)
	static  i32 ms_textureMemoryUsed;
	static  i32 ms_numTexturesCreated;
	static  i32 ms_numTexturesCreatedTotal;
	static  i32 ms_numTexturesCreatedTemp;
	static  i32 ms_fontCacheTextureID;
#endif

protected:
	i32 m_texID;
	ETEX_Format m_fmt;
	Handle m_userData;
	GRenderer* m_pRenderer;
#if defined(ENABLE_FLASH_INFO)
	bool m_isTemp;
#endif
};

class GTextureXRender:public GTextureXRenderBase
{
	// GTexture interface
public:
	virtual bool IsDataValid() const { return m_texID > 0; }

	virtual bool InitTexture(GImageBase* pIm, UInt usage = Usage_Wrap);
	virtual bool InitDynamicTexture(i32 width, i32 height, GImage::ImageFormat format, i32 mipmaps, UInt usage);

	virtual void Update(i32 level, i32 n, const UpdateRect* pRects, const GImageBase* pIm);

	virtual i32  Map(i32 level, i32 n, MapRect* maps, i32 flags = 0);
	virtual bool Unmap(i32 level, i32 n, MapRect* maps, i32 flags = 0);

	// GTextureXRenderBase interface
public:
	virtual bool InitTextureFromFile(tukk pFilename);
	virtual bool InitTextureFromTexId(i32 texid);

public:
	GTextureXRender(GRenderer* pRenderer, i32 texID = -1, ETEX_Format fmt = eTF_Unknown);
	virtual ~GTextureXRender();

private:
	bool InitTextureInternal(const GImageBase* pIm);
	bool InitTextureInternal(ETEX_Format texFmt, i32 width, i32 height, i32 pitch, u8* pData);
	void SwapRB(u8* pImageData, u32 width, u32 height, u32 pitch) const;
#if defined(NEED_ENDIAN_SWAP)
	void SwapEndian(u8* pImageData, u32 width, u32 height, u32 pitch) const;
#endif
};

class GTextureXRenderTempRT:public GTextureXRender
{
	// GTexture interface overrides
public:
	virtual Handle GetUserData() const       { return (Handle)this; }
	virtual void   SetUserData(Handle hData) { assert(false); }

public:
	GTextureXRenderTempRT(GRenderer* pRenderer, i32 texID = -1, ETEX_Format fmt = eTF_Unknown);
	virtual ~GTextureXRenderTempRT();
};

class GTextureXRenderTempRTLockless:public GTextureXRenderBase
{
	// GTexture interface
public:
	virtual Handle GetUserData() const       { return (Handle)m_pRT; }
	virtual void   SetUserData(Handle hData) { assert(false); }

	virtual bool IsDataValid() const { return true; }

	virtual bool InitTexture(GImageBase* pIm, UInt usage = Usage_Wrap)                                          { assert(false); return false; }
	virtual bool InitDynamicTexture(i32 width, i32 height, GImage::ImageFormat format, i32 mipmaps, UInt usage) { assert(false); return false; }

	virtual void Update(i32 level, i32 n, const UpdateRect* pRects, const GImageBase* pIm) { assert(false); }

	virtual i32  Map(i32 level, i32 n, MapRect* maps, i32 flags = 0)   { assert(false); return -1; }
	virtual bool Unmap(i32 level, i32 n, MapRect* maps, i32 flags = 0) { assert(false); return false; }

	// GTextureXRenderBase interface
public:
	virtual bool InitTextureFromFile(tukk pFilename) { assert(false); return false; }
	virtual bool InitTextureFromTexId(i32 texid)            { assert(false); return false; }

public:
	GTextureXRenderTempRTLockless(GRenderer* pRenderer);
	virtual ~GTextureXRenderTempRTLockless();

	void SetRT(GTexture* pRT) {	m_pRT = pRT; }
	GTexture* GetRT() { return m_pRT; }

private:
	GPtr<GTexture> m_pRT;
};

class GTextureXRenderYUV:public GTextureXRenderBase
{
	// GTexture interface
public:
	virtual bool IsDataValid() const { return m_numIDs > 0; }

	virtual bool InitTexture(GImageBase* pIm, UInt usage = Usage_Wrap);
	virtual bool InitDynamicTexture(i32 width, i32 height, GImage::ImageFormat format, i32 mipmaps, UInt usage);

	virtual void Update(i32 level, i32 n, const UpdateRect* pRects, const GImageBase* pIm);

	virtual i32  Map(i32 level, i32 n, MapRect* maps, i32 flags = 0);
	virtual bool Unmap(i32 level, i32 n, MapRect* maps, i32 flags = 0);

	// GTextureXRenderBase interface
public:
	virtual bool InitTextureFromFile(tukk pFilename);
	virtual bool InitTextureFromTexId(i32 texid);

public:
	GTextureXRenderYUV(GRenderer* pRenderer);
	virtual ~GTextureXRenderYUV();

	i32k* GetIDs() const    { return m_texIDs; }
	i32        GetNumIDs() const { return m_numIDs; }

	u32 Res(u32 i, u32 val) const { return i == 0 || i == 3 ? val : val >> 1; }

	bool IsStereoContent() const       { return m_isStereoContent; }
	void SetStereoContent(bool enable) { m_isStereoContent = enable; }

private:
	void Clear();

private:
	i32  m_texIDs[4];
	i32  m_numIDs;
	u32 m_width;
	u32 m_height;
	bool m_isStereoContent;
};

#endif // #ifdef INCLUDE_SCALEFORM_SDK

#endif // #ifndef _GTEXTURE_XRENDER_H_