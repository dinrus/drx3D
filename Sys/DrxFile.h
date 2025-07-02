// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

#include <drx3D/Sys/ISystem.h>
#include <drx3D/Sys/IDrxPak.h>
#include <drx3D/Sys/IConsole.h>
#include <drx3D/CoreX/String/Path.h>
#include <drx3D/CoreX/String/StringUtils.h>

//////////////////////////////////////////////////////////////////////////
// Defines for DinrusX filetypes extensions.
//////////////////////////////////////////////////////////////////////////
#define DRX_GEOMETRY_FILE_EXT                "cgf"
#define DRX_SKEL_FILE_EXT                    "chr"     //will be a SKEL soon
#define DRX_SKIN_FILE_EXT                    "skin"
#define DRX_CHARACTER_ANIMATION_FILE_EXT     "caf"
#define DRX_CHARACTER_DEFINITION_FILE_EXT    "cdf"
#define DRX_CHARACTER_LIST_FILE_EXT          "cid"
#define DRX_ANIM_GEOMETRY_FILE_EXT           "cga"
#define DRX_MATERIAL_FILE_EXT                "mtl"
#define DRX_ANIM_GEOMETRY_ANIMATION_FILE_EXT "anm"
#define DRX_COMPILED_FILE_EXT                "(c)"
#define DRX_BINARY_XML_FILE_EXT              "binxml"
#define DRX_XML_FILE_EXT                     "xml"
#define DRX_CHARACTER_PARAM_FILE_EXT         "chrparams"
#define DRX_GEOM_CACHE_FILE_EXT              "cax"
//////////////////////////////////////////////////////////////////////////
#define DRXFILE_MAX_PATH                     260
//////////////////////////////////////////////////////////////////////////

inline tukk DrxGetExt(tukk filepath)
{
	tukk str = filepath;
	size_t len = strlen(filepath);
	for (tukk p = str + len - 1; p >= str; --p)
	{
		switch (*p)
		{
		case ':':
		case '/':
		case '\\':
			// we've reached a path separator - it means there's no extension in this name
			return "";
		case '.':
			// there's an extension in this file name
			return p + 1;
		}
	}
	return "";
}

//! Check if specified file name is a character file.
inline bool IsCharacterFile(tukk filename)
{
	tukk ext = DrxGetExt(filename);
	if (stricmp(ext, DRX_SKEL_FILE_EXT) == 0 || stricmp(ext, DRX_SKIN_FILE_EXT) == 0 || stricmp(ext, DRX_CHARACTER_DEFINITION_FILE_EXT) == 0 || stricmp(ext, DRX_ANIM_GEOMETRY_FILE_EXT) == 0)
	{
		return true;
	}
	else
		return false;
}

//! Check if specified file name is a static geometry file.
inline bool IsStatObjFile(tukk filename)
{
	tukk ext = DrxGetExt(filename);
	if (stricmp(ext, DRX_GEOMETRY_FILE_EXT) == 0)
	{
		return true;
	}
	else
		return false;
}

struct IDrxPak;

//! Wrapper on file system.
class CDrxFile
{
public:
	CDrxFile();
	CDrxFile(IDrxPak* pIPak);    //!< Allow an alternative IDrxPak interface.
	CDrxFile(tukk filename, tukk mode, i32 nOpenFlagsEx = 0);
	~CDrxFile();

	bool Open(tukk filename, tukk mode, i32 nOpenFlagsEx = 0);
	void Close();

	//! Writes data in a file to the current file position.
	size_t Write(ukk lpBuf, size_t nSize);

	//! Reads data from a file at the current file position.
	size_t ReadRaw(uk lpBuf, size_t nSize);

	//! Template version, for automatic size support.
	template<class T>
	inline size_t ReadTypeRaw(T* pDest, size_t nCount = 1)
	{
		return ReadRaw(pDest, sizeof(T) * nCount);
	}

	//! Automatic endian-swapping version.
	template<class T>
	inline size_t ReadType(T* pDest, size_t nCount = 1)
	{
		size_t nRead = ReadRaw(pDest, sizeof(T) * nCount);
		SwapEndian(pDest, nCount);
		return nRead;
	}

	//! Template version, for automatic size support.
	template<typename T>
	inline size_t WriteType(const T* pData, const size_t nCount = 1)
	{
		return Write(pData, sizeof(T) * nCount);
	}

	//! Retrieves the length of the file.
	size_t GetLength();

	//! Moves the current file pointer to the specified position.
	size_t Seek(size_t seek, i32 mode);

	//! Moves the current file pointer at the beginning of the file.
	void SeekToBegin();

	//! Moves the current file pointer at the end of the file.
	size_t SeekToEnd();

	//! Retrieves the current file pointer.
	size_t GetPosition();

	//! Tests for end-of-file on a selected file.
	bool IsEof();

	//! Flushes any data yet to be written.
	void Flush();

	//! Gets a handle to a pack object.
	FILE* GetHandle() const { return m_file; };

	//! Retrieves the filename of the selected file.
	tukk GetFilename() const { return m_filename; };

	//! Retrieves the filename after adjustment to the real relative to engine root path.
	//! Example:
	//! Original filename "textures/red.dds" adjusted filename will look like "game/textures/red.dds"
	//! \return Adjusted filename, this is a pointer to a static string, copy return value if you want to keep it.
	tukk GetAdjustedFilename() const;

	//! Checks if file is opened from pak file.
	bool IsInPak() const;

	//! Gets path of archive this file is in.
	tukk GetPakPath() const;

private:
	char     m_filename[DRXFILE_MAX_PATH];
	FILE*    m_file;
	IDrxPak* m_pIPak;
};

#define IfPak(PakFunc, stdfunc, args) (m_pIPak ? m_pIPak->PakFunc args : stdfunc args)

//! CDrxFile implementation.
inline CDrxFile::CDrxFile()
{
	m_file = 0;
	m_pIPak = gEnv ? gEnv->pDrxPak : NULL;
}

inline CDrxFile::CDrxFile(IDrxPak* pIPak)
{
	m_file = 0;
	m_pIPak = pIPak;
}

//////////////////////////////////////////////////////////////////////////
inline CDrxFile::CDrxFile(tukk filename, tukk mode, i32 nOpenFlagsEx)
{
	m_file = 0;
	m_pIPak = gEnv ? gEnv->pDrxPak : NULL;
	Open(filename, mode, nOpenFlagsEx);
}

//////////////////////////////////////////////////////////////////////////
inline CDrxFile::~CDrxFile()
{
	Close();
}

//////////////////////////////////////////////////////////////////////////
//! \note For nOpenFlagsEx see IDrxPak::EFOpenFlags.
inline bool CDrxFile::Open(tukk filename, tukk mode, i32 nOpenFlagsEx)
{
	char tempfilename[DRXFILE_MAX_PATH] = "";
	drx_strcpy(tempfilename, filename);

#if !defined (_RELEASE)
	if (gEnv && gEnv->IsEditor())
	{
		ICVar* const pCvar = gEnv->pConsole->GetCVar("ed_lowercasepaths");
		if (pCvar)
		{
			i32k lowercasePaths = pCvar->GetIVal();
			if (lowercasePaths)
			{
				string lowerString = tempfilename;
				lowerString.MakeLower();
				drx_strcpy(tempfilename, lowerString.c_str());
			}
		}
	}
#endif
	if (m_file)
	{
		Close();
	}
	drx_strcpy(m_filename, tempfilename);

	m_file = m_pIPak ? m_pIPak->FOpen(tempfilename, mode, nOpenFlagsEx) : fopen(tempfilename, mode);
	return m_file != NULL;
}

//////////////////////////////////////////////////////////////////////////
inline void CDrxFile::Close()
{
	if (m_file)
	{
		IfPak(FClose, fclose, (m_file));
		m_file = 0;
		m_filename[0] = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
inline size_t CDrxFile::Write(ukk lpBuf, size_t nSize)
{
	assert(m_file);
	return IfPak(FWrite, fwrite, (lpBuf, 1, nSize, m_file));
}

//////////////////////////////////////////////////////////////////////////
inline size_t CDrxFile::ReadRaw(uk lpBuf, size_t nSize)
{
	assert(m_file);
	return IfPak(FReadRaw, fread, (lpBuf, 1, nSize, m_file));
}

//////////////////////////////////////////////////////////////////////////
inline size_t CDrxFile::GetLength()
{
	assert(m_file);
	if (m_pIPak)
		return m_pIPak->FGetSize(m_file);
	long curr = ftell(m_file);
	if (fseek(m_file, 0, SEEK_END) != 0)
		return 0;
	long size = ftell(m_file);
	if (fseek(m_file, curr, SEEK_SET) != 0)
		return 0;
	return size;
}

#if DRX_PLATFORM_WINDOWS && DRX_PLATFORM_64BIT
	#pragma warning( push )               //AMD Port
	#pragma warning( disable : 4267 )
#endif

//////////////////////////////////////////////////////////////////////////
inline size_t CDrxFile::Seek(size_t seek, i32 mode)
{
	assert(m_file);
	return IfPak(FSeek, fseek, (m_file, long(seek), mode));
}

#if DRX_PLATFORM_WINDOWS && DRX_PLATFORM_64BIT
	#pragma warning( pop )                //AMD Port
#endif

//////////////////////////////////////////////////////////////////////////
inline void CDrxFile::SeekToBegin()
{
	Seek(0, SEEK_SET);
}

//////////////////////////////////////////////////////////////////////////
inline size_t CDrxFile::SeekToEnd()
{
	return Seek(0, SEEK_END);
}

//////////////////////////////////////////////////////////////////////////
inline size_t CDrxFile::GetPosition()
{
	assert(m_file);
	return IfPak(FTell, ftell, (m_file));
}

//////////////////////////////////////////////////////////////////////////
inline bool CDrxFile::IsEof()
{
	assert(m_file);
	return IfPak(FEof, feof, (m_file)) != 0;
}

//////////////////////////////////////////////////////////////////////////
inline void CDrxFile::Flush()
{
	assert(m_file);
	IfPak(FFlush, fflush, (m_file));
}

//////////////////////////////////////////////////////////////////////////
inline bool CDrxFile::IsInPak() const
{
	if (m_file && m_pIPak)
		return m_pIPak->GetFileArchivePath(m_file) != NULL;
	return false;
}

//////////////////////////////////////////////////////////////////////////
inline tukk CDrxFile::GetPakPath() const
{
	if (m_file && m_pIPak)
	{
		tukk sPath = m_pIPak->GetFileArchivePath(m_file);
		if (sPath != NULL)
			return sPath;
	}
	return "";
}

//////////////////////////////////////////////////////////////////////////
inline tukk CDrxFile::GetAdjustedFilename() const
{
	static char szAdjustedFile[IDrxPak::g_nMaxPath];
	assert(m_pIPak);
	if (!m_pIPak)
		return "";

	//! Gets mod path to file.
	tukk gameUrl = m_pIPak->AdjustFileName(m_filename, szAdjustedFile, 0);

	//! Returns standard path otherwise.
	if (gameUrl != &szAdjustedFile[0])
		drx_strcpy(szAdjustedFile, gameUrl);
	return szAdjustedFile;
}

//! \endcond