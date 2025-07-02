// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _GIMAGEINFO_XRENDER_H_
#define _GIMAGEINFO_XRENDER_H_

#pragma once

#ifdef INCLUDE_SCALEFORM_SDK

	#pragma warning(push)
	#pragma warning(disable : 6326)// Potential comparison of a constant with another constant
	#pragma warning(disable : 6011)// Dereferencing NULL pointer
	#include <drx3D/CoreX/Platform/DrxWindows.h>
	#include <GImageInfo.h> // includes <windwows.h>
	#pragma warning(pop)

struct IFlashLoadMovieImage;

class DrxGImageInfoBase : public GImageInfoBase
{
public:
	DrxGImageInfoBase() {}
};

class GImageInfoXRender : public DrxGImageInfoBase
{
	// GImageInfoBase interface
public:
	virtual UInt      GetWidth() const;
	virtual UInt      GetHeight() const;
	#if defined(GFX_AMP_SERVER)
	virtual UPInt     GetBytes() const;
	virtual UPInt     GetExternalBytes() const;
	virtual bool      IsExternal() const;

	virtual UInt32    GetImageId() const;
	virtual GImage*   GetImage() const;
	#endif
	virtual GTexture* GetTexture(GRenderer* pRenderer);

public:
	GImageInfoXRender(GImage* pImage);
	virtual ~GImageInfoXRender();

private:
	u32         m_width;
	u32         m_height;
	GPtr<GImage>   m_pImg;
	GPtr<GTexture> m_pTex;
};

class GImageInfoFileXRender : public DrxGImageInfoBase
{
	// GImageInfoBase interface
public:
	virtual UInt      GetWidth() const;
	virtual UInt      GetHeight() const;
	#if defined(GFX_AMP_SERVER)
	virtual UPInt     GetBytes() const;
	virtual UPInt     GetExternalBytes() const;
	virtual bool      IsExternal() const;

	virtual UInt32    GetImageId() const;
	virtual GImage*   GetImage() const;
	#endif
	virtual GTexture* GetTexture(GRenderer* pRenderer);

public:
	GImageInfoFileXRender(tukk pImgFilePath, u32 targetWidth, u32 targetHeight);
	virtual ~GImageInfoFileXRender();

private:
	u32         m_targetWidth;
	u32         m_targetHeight;
	string         m_imgFilePath;
	GPtr<GTexture> m_pTex;
};

class GImageInfoTextureXRender : public DrxGImageInfoBase
{
	// GImageInfoBase interface
public:
	virtual UInt      GetWidth() const;
	virtual UInt      GetHeight() const;
	#if defined(GFX_AMP_SERVER)
	virtual UPInt     GetBytes() const;
	virtual UPInt     GetExternalBytes() const;
	virtual bool      IsExternal() const;

	virtual UInt32    GetImageId() const;
	virtual GImage*   GetImage() const;
	#endif
	virtual GTexture* GetTexture(GRenderer* pRenderer);

public:
	GImageInfoTextureXRender(ITexture* pTexture);
	GImageInfoTextureXRender(ITexture* pTexture, i32 width, i32 height);
	virtual ~GImageInfoTextureXRender();

private:
	u32         m_width;
	u32         m_height;
	i32            m_texId;
	GPtr<GTexture> m_pTex;
};

class GImageInfoILMISrcXRender : public DrxGImageInfoBase
{
	// GImageInfoBase interface
public:
	virtual UInt      GetWidth() const;
	virtual UInt      GetHeight() const;
	#if defined(GFX_AMP_SERVER)
	virtual UPInt     GetBytes() const;
	virtual UPInt     GetExternalBytes() const;
	virtual bool      IsExternal() const;

	virtual UInt32    GetImageId() const;
	virtual GImage*   GetImage() const;
	#endif
	virtual GTexture* GetTexture(GRenderer* pRenderer);

public:
	GImageInfoILMISrcXRender(IFlashLoadMovieImage* pSrc);
	virtual ~GImageInfoILMISrcXRender();

private:
	u32                m_width;
	u32                m_height;
	IFlashLoadMovieImage* m_pSrc;
	GPtr<GTexture>        m_pTex;
};

#endif // #ifdef INCLUDE_SCALEFORM_SDK

#endif // #ifndef _GIMAGEINFO_XRENDER_H_
