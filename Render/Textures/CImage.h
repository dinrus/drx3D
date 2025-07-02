// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef CIMAGE_H
#define CIMAGE_H

#include <drx3D/CoreX/Renderer/IImage.h>

#define SH_LITTLE_ENDIAN

// The mask for extracting just R/G/B from an ulong or SRGBPixel
#ifdef SH_BIG_ENDIAN
	#define RGB_MASK 0xffffff00
#else
	#define RGB_MASK 0x00ffffff
#endif

/**
 * An RGB pixel.
 */
struct SRGBPixel
{
	u8 blue, green, red, alpha;
	SRGBPixel()  /* : red(0), green(0), blue(0), alpha(255) {} */
	{ *(u32*)this = (u32)~RGB_MASK; }
	SRGBPixel(i32 r, i32 g, i32 b) : red(r), green(g), blue(b), alpha(255) {}
	//bool eq (const SRGBPixel& p) const { return ((*(u32 *)this) & RGB_MASK) == ((*(u32 *)&p) & RGB_MASK); }
};

class CImageFile;
namespace DDSSplitted {
struct DDSDesc;
}

struct IImageFileStreamCallback
{
	virtual void OnImageFileStreamComplete(CImageFile* pImFile) = 0;

protected:
	virtual ~IImageFileStreamCallback() {}
};

struct SImageFileStreamState
{
	enum
	{
		MaxStreams = 64
	};

	struct Request
	{
		uk  pOut;
		size_t nOffs;
		size_t nSize;
	};

	void RaiseComplete(CImageFile* pFile)
	{
		if (m_pCallback)
		{
			m_pCallback->OnImageFileStreamComplete(pFile);
			m_pCallback = NULL;
		}
	}

	 i32              m_nPending;
	u32                    m_nFlags;
	IImageFileStreamCallback* m_pCallback;
	IReadStreamPtr            m_pStreams[MaxStreams];
	Request                   m_requests[MaxStreams];
};

class CImageFile : public IImageFile, public IStreamCallback
{
private:
	CImageFile(const CImageFile&);
	CImageFile& operator=(const CImageFile&);

private:
	 i32 m_nRefCount;

protected:
	i32    m_Width;  // Width of image.
	i32    m_Height; // Height of image.
	i32    m_Depth;  // Depth of image.
	i32    m_Sides;  // Depth of image.

	i32    m_ImgSize;

	i32    m_NumMips;
	i32    m_NumPersistantMips;
	i32    m_Flags;       // e.g. FIM_GREYSCALE|FIM_ALPHA
	i32    m_nStartSeek;
	float  m_fAvgBrightness;
	ColorF m_cMinColor;
	ColorF m_cMaxColor;

	union // The image data.
	{
		byte*      m_pByteImage[6];
		SRGBPixel* m_pPixImage[6];
	};

	EImFileError           m_eError;   // Last error code.
	string                 m_FileName; // file name

	ETEX_Format            m_eFormat;
	ETEX_TileMode          m_eTileMode;

	SImageFileStreamState* m_pStreamState;

protected:
	CImageFile(const string& filename);

	void mfSet_error(const EImFileError error, tukk detail = NULL);
	void mfSet_dimensions(i32k w, i32k h);
public:
	virtual ~CImageFile();

	virtual i32 AddRef()
	{
		return DrxInterlockedIncrement(&m_nRefCount);
	}

	virtual i32 Release()
	{
		i32 nRef = DrxInterlockedDecrement(&m_nRefCount);
		if (nRef == 0)
		{
			delete this;
		}
		else if (nRef < 0)
		{
			assert(0);
			DrxFatalError("Deleting Reference Counted Object Twice");
		}
		return nRef;
	}

	const string&                mfGet_filename() const { return m_FileName; }

	i32                          mfGet_width() const    { return m_Width; }
	i32                          mfGet_height() const   { return m_Height; }
	i32                          mfGet_depth() const    { return m_Depth; }
	i32                          mfGet_NumSides() const { return m_Sides; }

	EImFileError                 mfGet_error() const    { return m_eError; }

	byte*                        mfGet_image(i32k nSide);
	void                         mfFree_image(i32k nSide);
	bool                         mfIs_image(i32k nSide) const              { return m_pByteImage[nSide] != NULL; }

	i32                          mfGet_StartSeek() const                        { return m_nStartSeek; }

	void                         mfSet_ImageSize(i32 Size)                      { m_ImgSize = Size; }
	i32                          mfGet_ImageSize() const                        { return m_ImgSize; }

	ETEX_Format                  mfGetFormat() const                            { return m_eFormat; }
	ETEX_TileMode                mfGetTileMode() const                          { return m_eTileMode; }

	void                         mfSet_numMips(i32k num)                   { m_NumMips = num; }
	i32                          mfGet_numMips() const                          { return m_NumMips; }

	void                         mfSet_numPersistantMips(i32k num)         { m_NumPersistantMips = num; }
	i32                          mfGet_numPersistantMips() const                { return m_NumPersistantMips; }

	void                         mfSet_avgBrightness(const float avgBrightness) { m_fAvgBrightness = avgBrightness; }
	float                        mfGet_avgBrightness() const                    { return m_fAvgBrightness; }

	void                         mfSet_minColor(const ColorF& minColor)         { m_cMinColor = minColor; }
	const ColorF&                mfGet_minColor() const                         { return m_cMinColor; }

	void                         mfSet_maxColor(const ColorF& maxColor)         { m_cMaxColor = maxColor; }
	const ColorF&                mfGet_maxColor() const                         { return m_cMaxColor; }

	void                         mfSet_Flags(i32k Flags)                   { m_Flags |= Flags; }
	i32                          mfGet_Flags() const                            { return m_Flags; }

	void                         mfAbortStreaming();

	virtual DDSSplitted::DDSDesc mfGet_DDSDesc() const = 0;

public:
	enum class EResourceCompilerResult
	{
		//! Texture was already compiled, no need to run RC
		AlreadyCompiled,
		//! Texture was queued for compilation, will not be available immediately
		Queued,
		//! Texture was compiled immediately, and can be loaded right away.
		Available,
		//! Texture compilation failed
		Failed,
		//! RC invoke was disabled, or image format was not supported
		Skipped,
	};

	static _smart_ptr<CImageFile>  mfLoad_file(const string& filename, u32k nFlags);
	static _smart_ptr<CImageFile>  mfStream_File(const string& filename, u32k nFlags, IImageFileStreamCallback* pCallback);
	static EResourceCompilerResult mfInvokeRC(const string& fileToLoad, const string& filename, tuk extOut, size_t extOutCapacity, bool immediate);
};

#endif