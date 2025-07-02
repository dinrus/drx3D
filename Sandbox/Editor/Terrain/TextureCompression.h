// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#if !defined(AFX_TEXTURECOMPRESSION_H__B2702EC6_F5D8_4BB3_B2EE_A2F66C128380__INCLUDED_)
#define AFX_TEXTURECOMPRESSION_H__B2702EC6_F5D8_4BB3_B2EE_A2F66C128380__INCLUDED_

#if _MSC_VER > 1000
	#pragma once
#endif // _MSC_VER > 1000

#include "Util/DrxMemFile.h"        // CDrxMemFile

class CTextureCompression
{
public:
	// constructor
	// Arguments:
	//   bHighQuality - true:slower but better quality, false=use hardware instead
	CTextureCompression(const bool bHighQuality);
	// destructor
	virtual ~CTextureCompression();

	void WriteDDSToFile(CFile& toFile, u8* dat, i32 w, i32 h, i32 Size,
	                    ETEX_Format eSrcF, ETEX_Format eDstF, i32 NumMips, const bool bHeader, const bool bDither);

private: // ------------------------------------------------------------------

	static void SaveCompressedMipmapLevel(ukk data, size_t size, uk userData);

	bool                      m_bHighQuality; // true:slower but better quality, false=use hardware instead
	static CFile*             m_pFile;
	static DrxCriticalSection m_sFileLock;
};

#endif // !defined(AFX_TEXTURECOMPRESSION_H__B2702EC6_F5D8_4BB3_B2EE_A2F66C128380__INCLUDED_)

