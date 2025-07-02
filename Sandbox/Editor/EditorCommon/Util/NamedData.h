// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
////////////////////////////////////////////////////////////////////////////
//
//  DinrusPro 3D Engine Source File.
//  Copyright (C), DinrusPro 3D Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   NamedData.h
//  Created:     30/10/2001 by Timur.
//  Description: Collection of Named data blocks
//
////////////////////////////////////////////////////////////////////////////
#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>
#include "Util/MemoryBlock.h"

class CPakFile;

class EDITOR_COMMON_API CNamedData : public CObject
{
	DECLARE_SERIAL(CNamedData)
public:
	CNamedData();
	virtual ~CNamedData();
	void          AddDataBlock(const string& blockName, uk pData, i32 nSize, bool bCompress = true);
	void          AddDataBlock(const string& blockName, CMemoryBlock& block);
	//! Returns uncompressed block data.
	bool          GetDataBlock(const string& blockName, uk & pData, i32& nSize);
	//! Returns raw data block in original form (Compressed or Uncompressed).
	CMemoryBlock* GetDataBlock(const string& blockName, bool& bCompressed);

	void          Clear();
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCurveObject)
public:
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

	//! Save named data to pak file.
	void Save(CPakFile& pakFile);

	//! Save named data files.
	void Save(const string& levelPath);

	//! Load named data.
	bool Load(const string& levelPath);

private:
	struct DataBlock
	{
		string      blockName;
		CMemoryBlock data;
		CMemoryBlock compressedData;
		//! This block is compressed.
		bool         bCompressed;
		bool         bFastCompression;
	};
	typedef std::map<string, DataBlock*, stl::less_stricmp<string>> TBlocks;
	TBlocks m_blocks;
};

