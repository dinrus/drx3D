// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef DDSIMAGE_H
#define DDSIMAGE_H

#include <drx3D/Eng3D/ImageExtensionHelper.h>
#include <drx3D/Render/Textures/CImage.h>
#include <drx3D/CoreX/Memory/IMemory.h>

/**
 * An ImageFile subclass for reading DDS files.
 */

namespace DDSSplitted
{
enum
{
	etexNumLastMips     = 3,
	etexLowerMipMaxSize = (1 << (etexNumLastMips + 2)), // + 2 means we drop all the mips that are less than 4x4(two mips: 2x2 and 1x1)
};

struct DDSDesc
{
	DDSDesc()
	{
		memset(this, 0, sizeof(*this));
	}

	tukk   pName;
	u32        nBaseOffset;

	u32        nWidth;
	u32        nHeight;
	u32        nDepth;
	u32        nSides;
	u32        nMips;
	u32        nMipsPersistent;
	ETEX_Format   eFormat;
	ETEX_TileMode eTileMode;
	u32        nFlags;
};

typedef DrxStackStringT<char, 192> TPath;

struct ChunkInfo
{
	TPath  fileName;
	u32 nOffsetInFile;
	u32 nSizeInFile;
	u32 nMipLevel;
	u32 nSideDelta;
};

struct RequestInfo
{
	string fileName;
	uk  pOut;
	u32 nOffs;
	u32 nRead;
};

struct FileWrapper
{
	explicit FileWrapper(CDrxFile& file) : m_bInMem(false), m_pFile(&file) {}
	FileWrapper(ukk p, size_t nLen) : m_bInMem(true), m_pRaw((tukk)p), m_nRawLen(nLen), m_nSeek(0) {}

	bool IsValid() const
	{
		return m_bInMem
		       ? m_pRaw != NULL
		       : m_pFile != NULL && m_pFile->GetHandle() != NULL;
	}

	size_t GetLength() const
	{
		return m_bInMem
		       ? m_nRawLen
		       : m_pFile->GetLength();
	}

	size_t ReadRaw(uk p, size_t amount)
	{
		if (m_bInMem)
		{
			amount = min(m_nRawLen - m_nSeek, amount);
			memcpy(p, m_pRaw + m_nSeek, amount);
			m_nSeek += amount;

			return amount;
		}
		else
		{
			return m_pFile->ReadRaw(p, amount);
		}
	}

	void Seek(size_t offs)
	{
		if (m_bInMem)
		{
			m_nSeek = min(offs, m_nRawLen);
		}
		else
		{
			m_pFile->Seek(offs, SEEK_SET);
		}
	}

	size_t Tell()
	{
		if (m_bInMem)
		{
			return m_nSeek;
		}
		else
		{
			return m_pFile->GetPosition();
		}
	}

	bool m_bInMem;
	union
	{
		CDrxFile* m_pFile;
		struct
		{
			tukk m_pRaw;
			size_t      m_nRawLen;
			size_t      m_nSeek;
		};
	};
};

typedef std::vector<ChunkInfo> Chunks;

TPath& MakeName(TPath& sOut, tukk sOriginalName, u32k nChunk, u32k nFlags);

size_t GetFilesToRead(ChunkInfo* pFiles, size_t nFilesCapacity, const DDSDesc& desc, u32 nStartMip, u32 nEndMip);

bool   SeekToAttachedImage(FileWrapper& file);

size_t LoadMipRequests(RequestInfo* pReqs, size_t nReqsCap, const DDSDesc& desc, byte* pBuffer, u32 nStartMip, u32 nEndMip);
size_t LoadMipsFromRequests(const RequestInfo* pReqs, size_t nReqs);
size_t LoadMips(byte* pBuffer, const DDSDesc& desc, u32 nStartMip, u32 nEndMip);

i32    GetNumLastMips(i32k nWidth, i32k nHeight, i32k nNumMips, i32k nSides, ETEX_Format eTF, u32k nFlags);
};

class CImageDDSFile : public CImageFile
{
	friend class CImageFile;  // For constructor
public:
	CImageDDSFile(const string& filename);
	CImageDDSFile(const string& filename, u32 nFlags);

	bool                         Stream(u32 nFlags, IImageFileStreamCallback* pStreamCallback);

	virtual DDSSplitted::DDSDesc mfGet_DDSDesc() const;

private: // ------------------------------------------------------------------------------
	/// Read the DDS file from the file.
	bool LoadFromFile(DDSSplitted::FileWrapper& file, u32 nFlags, DDSSplitted::RequestInfo* pConts, size_t& nConts, size_t nContsCap);

	/// Read the DDS file from the file.
	bool Load(const string& filename, u32 nFlags);

	void StreamAsyncOnComplete(IReadStream* pStream, unsigned nError);

	bool SetHeaderFromMemory(byte* pFileStart, byte* pFileAfterHeader, u32 nFlags);
	i32  AdjustHeader();

	bool PostLoad();

	void AdjustFirstFileName(u32& nFlags, tukk pFileName, DDSSplitted::TPath& adjustedFileName);

	CImageExtensionHelper::DDS_HEADER       m_DDSHeader;
	CImageExtensionHelper::DDS_HEADER_DXT10 m_DDSHeaderExtension;

	_smart_ptr<IMemoryBlock>                m_pFileMemory;
};

void WriteDDS(byte* dat, i32 wdt, i32 hgt, i32 dpth, i32 Size, tuk name, ETEX_Format eF, i32 NumMips, ETEX_Type eTT);

#endif
