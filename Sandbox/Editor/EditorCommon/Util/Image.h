// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
////////////////////////////////////////////////////////////////////////////
//
//  DinrusPro 3D Engine Source File.
//  Copyright (C), DinrusPro 3D Studios, 2001.
// -------------------------------------------------------------------------
//  Created:     14/11/2001 by Timur.
//  Description: Generic image class
//
////////////////////////////////////////////////////////////////////////////
#include <drx3D/CoreX/Renderer/ITexture.h>
#include "Util/MemoryBlock.h"

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

#include <QImage>

class CXmlArchive;

/*!
 *  Templated image class.
 */

template<class T>
class TImage
{
public:
	TImage()
	{
		m_data = 0;
		m_width = 0;
		m_height = 0;
		m_bHasAlphaChannel = false;
		m_bIsLimitedHDR = false;
		m_bIsCubemap = false;
		m_bIsSRGB = true;
		m_nNumberOfMipmaps = 1;
		m_format = eTF_Unknown;
		m_strDccFilename = "";
	}
	virtual ~TImage() {}

	T&       ValueAt(i32 x, i32 y)       { return m_data[x + y * m_width]; }
	const T& ValueAt(i32 x, i32 y) const { return m_data[x + y * m_width]; }

	const T& ValueAtSafe(i32 x, i32 y) const
	{
		static T zero; 
		zero = 0;
		if (0 <= x && x < m_width && 0 <= y && y < m_height)
			return m_data[x + y * m_width];
		return zero;
	}

	T*           GetData() const            { return m_data; }
	i32          GetWidth() const           { return m_width; }
	i32          GetHeight() const          { return m_height; }

	bool         HasAlphaChannel() const    { return m_bHasAlphaChannel; }
	bool         IsLimitedHDR() const       { return m_bIsLimitedHDR; }
	bool         IsCubemap() const          { return m_bIsCubemap; }
	u32 GetNumberOfMipMaps() const { return m_nNumberOfMipmaps; }

	// Returns:
	//  size in bytes
	i32  GetSize() const { return m_width * m_height * sizeof(T); }

	bool IsValid() const { return m_data != 0; }

	void Attach(T* data, i32 width, i32 height)
	{
		assert(data);
		m_memory = new CMemoryBlock;
		m_memory->Attach(data, width * height * sizeof(T));
		m_data = data;
		m_width = width;
		m_height = height;
		m_strDccFilename = "";
	}
	void Attach(const TImage<T>& img)
	{
		assert(img.IsValid());
		m_memory = img.m_memory;
		m_data = (T*)m_memory->GetBuffer();
		m_width = img.m_width;
		m_height = img.m_height;
		m_strDccFilename = img.m_strDccFilename;
	}
	void Detach()
	{
		m_memory = 0;
		m_data = 0;
		m_width = 0;
		m_height = 0;
		m_strDccFilename = "";
	}

	bool Allocate(i32 width, i32 height)
	{
		if (width < 1)
			width = 1;
		if (height < 1)
			height = 1;

		// New memory block.
		m_memory = new CMemoryBlock;
		m_memory->Allocate(width * height * sizeof(T)); // +width for crash safety.
		m_data = (T*)m_memory->GetBuffer();
		m_width = width;
		m_height = height;
		if (!m_data)
			return false;
		return true;
	}

	void Release()
	{
		m_memory = 0;
		m_data = 0;
		m_width = 0;
		m_height = 0;
		m_strDccFilename = "";
	}

	// Copy operator.
	void Copy(const TImage<T>& img)
	{
		if (!img.IsValid())
			return;
		if (m_width != img.GetWidth() || m_height != img.GetHeight())
			Allocate(img.GetWidth(), img.GetHeight());
		*m_memory = *img.m_memory;
		m_data = (T*)m_memory->GetBuffer();
		m_strDccFilename = img.m_strDccFilename;
	}

	//////////////////////////////////////////////////////////////////////////
	void Clear()
	{
		Fill(0);
	}

	//////////////////////////////////////////////////////////////////////////
	void Fill(u8 c)
	{
		if (IsValid())
			memset(GetData(), c, GetSize());
	}

	//////////////////////////////////////////////////////////////////////////
	void GetSubImage(i32 x1, i32 y1, i32 width, i32 height, TImage<T>& img) const
	{
		i32 size = width * height;
		img.Allocate(width, height);
		for (i32 y = 0; y < height; y++)
		{
			for (i32 x = 0; x < width; x++)
			{
				img.ValueAt(x, y) = ValueAtSafe(x1 + x, y1 + y);
			}
		}
	}
	
	void SetSubImage(i32 x1, i32 y1, const TImage<T>& subImage, float heightOffset, float fClamp = -1.0f)
	{
		i32 width = subImage.GetWidth();
		i32 height = subImage.GetHeight();
		if (x1 < 0)
		{
			width = width + x1;
			x1 = 0;
		}
		if (y1 < 0)
		{
			height = height + y1;
			y1 = 0;
		}
		if (x1 + width > m_width)
			width = m_width - x1;
		if (y1 + height > m_height)
			height = m_height - y1;
		if (width <= 0 || height <= 0)
			return;

		if (fClamp < 0.0f)
		{
			for (i32 y = 0; y < height; y++)
			{
				for (i32 x = 0; x < width; x++)
				{
					ValueAt(x1 + x, y1 + y) = subImage.ValueAt(x, y) + heightOffset;
				}
			}
		}
		else
		{
			T TClamp = fClamp;
			for (i32 y = 0; y < height; y++)
			{
				for (i32 x = 0; x < width; x++)
				{
					ValueAt(x1 + x, y1 + y) = clamp_tpl(f32(subImage.ValueAt(x, y) + heightOffset), 0.0f, f32(TClamp));
				}
			}
		}
	}

	void SetSubImage(i32 x1, i32 y1, const TImage<T>& subImage)
	{
		i32 width = subImage.GetWidth();
		i32 height = subImage.GetHeight();
		if (x1 < 0)
		{
			width = width + x1;
			x1 = 0;
		}
		if (y1 < 0)
		{
			height = height + y1;
			y1 = 0;
		}
		if (x1 + width > m_width)
			width = m_width - x1;
		if (y1 + height > m_height)
			height = m_height - y1;
		if (width <= 0 || height <= 0)
			return;

		for (i32 y = 0; y < height; y++)
		{
			for (i32 x = 0; x < width; x++)
			{
				ValueAt(x1 + x, y1 + y) = subImage.ValueAt(x, y);
			}
		}
	}

	//! Compress image to memory block.
	void Compress(CMemoryBlock& mem) const
	{
		assert(IsValid());
		m_memory->Compress(mem);
	}

	//! Uncompress image from memory block.
	bool Uncompress(const CMemoryBlock& mem)
	{
		assert(IsValid());
		// New memory block.
		_smart_ptr<CMemoryBlock> temp = new CMemoryBlock;
		mem.Uncompress(*temp);
		bool bValid = (GetSize() == m_memory->GetSize())
		              || ((GetSize() + m_width * sizeof(T)) == m_memory->GetSize());
		if (bValid)
		{
			m_memory = temp;
			m_data = (T*)m_memory->GetBuffer();
		}
		return bValid;
		//assert( GetSize() == m_memory.GetSize() );
	}

	void           SetHasAlphaChannel(bool bHasAlphaChannel)      { m_bHasAlphaChannel = bHasAlphaChannel; }
	void           SetIsLimitedHDR(bool bIsLimitedHDR)            { m_bIsLimitedHDR = bIsLimitedHDR; }
	void           SetIsCubemap(bool bIsCubemap)                  { m_bIsCubemap = bIsCubemap; }
	void           SetNumberOfMipMaps(u32 nNumberOfMips) { m_nNumberOfMipmaps = nNumberOfMips; }

	void           SetFormatDescription(const string& str) { m_formatDescription = str; };
	const string& GetFormatDescription() const             { return m_formatDescription; };

	void           SetFormat(ETEX_Format format)            { m_format = format; }
	ETEX_Format    GetFormat() const                        { return m_format; }

	void           SetSRGB(bool bEnable)                    { m_bIsSRGB = bEnable; }
	bool           GetSRGB() const                          { return m_bIsSRGB; }

	void           SetDccFilename(const string& str)       { m_strDccFilename = str; };
	const string& GetDccFilename() const                   { return m_strDccFilename; }

	// RotateOrt() — orthonormal image rotation
	// nRot is type of rotation:
	// 1 — 90 grads, 2 - 180 grads, 3 - 270 grads, other value - no rotation
	void RotateOrt(const TImage<T>& img, i32 nRot)
	{
		if (!img.IsValid())
			return;

		i32 width;
		i32 height;

		if (nRot == 1 || nRot == 3)
		{
			width = img.GetHeight();
			height = img.GetWidth();
		}
		else
		{
			width = img.GetWidth();
			height = img.GetHeight();
		}

		if (m_width != width || m_height != height)
			Allocate(width, height);

		for (i32 y = 0; y < m_height; y++)
			for (i32 x = 0; x < m_width; x++)
			{
				if (nRot == 1)
					ValueAt(x, y) = img.ValueAt(m_height - y - 1, x);
				else if (nRot == 2)
					ValueAt(x, y) = img.ValueAt(m_width - x - 1, m_height - y - 1);
				else if (nRot == 3)
					ValueAt(x, y) = img.ValueAt(y, m_width - x - 1);
				else
					ValueAt(x, y) = img.ValueAt(x, y);
			}
	}

private:
	// Restrict use of copy constructor.
	TImage(const TImage<T>& img);
	TImage<T>& operator=(const TImage<T>& img);

	//! Memory holding image data.
	_smart_ptr<CMemoryBlock> m_memory;

	T*                       m_data;
	i32                      m_width;
	i32                      m_height;
	bool                     m_bHasAlphaChannel;
	bool                     m_bIsLimitedHDR;
	bool                     m_bIsCubemap;
	bool                     m_bIsSRGB;
	u32             m_nNumberOfMipmaps;
	string                  m_formatDescription;
	string                  m_strDccFilename;
	ETEX_Format              m_format;
};

//////////////////////////////////////////////////////////////////////////
class EDITOR_COMMON_API CImageEx : public TImage<u32>
{
public:
	CImageEx() : TImage() { m_bGetHistogramEqualization = false; };

	bool LoadGrayscale16Tiff(const string& file);
	bool SaveGrayscale16Tiff(const string& file);

	void SwapRedAndBlue();
	void ReverseUpDown();
	void FillAlpha(u8 value = 0xff);

	// request histogram equalization for HDRs
	void SetHistogramEqualization(bool bHistogramEqualization) { m_bGetHistogramEqualization = bHistogramEqualization; }
	bool GetHistogramEqualization()                            { return m_bGetHistogramEqualization; }

	QImage ToQImage() const;

private:
	bool m_bGetHistogramEqualization;
};

//////////////////////////////////////////////////////////////////////////
// Define types of most commonly used images.
//////////////////////////////////////////////////////////////////////////
typedef TImage<float>          CFloatImage;
typedef TImage<u8>  CByteImage;
typedef TImage<unsigned short> CWordImage;

