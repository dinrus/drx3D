// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
////////////////////////////////////////////////////////////////////////////
//
//  DinrusPro 3D Engine Source File.
//  Copyright (C), DinrusPro 3D Studios, 2002.
// -------------------------------------------------------------------------
//  Created:     10/10/2002 by Timur.
//  Description: Memory block helper used with ZLib
//
////////////////////////////////////////////////////////////////////////////
#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>
#include <drx3D/CoreX/smartptr.h>

class EDITOR_COMMON_API CMemoryBlock : public _i_reference_target_t
{
public:
	CMemoryBlock();
	CMemoryBlock(const CMemoryBlock& mem);
	~CMemoryBlock();

	CMemoryBlock& operator=(const CMemoryBlock& mem);

	//! Allocate or reallocate memory for this block.
	//! @param size Amount of memory in bytes to allocate.
	//! @return true if the allocation succeeded.
	bool Allocate(i32 size, i32 uncompressedSize = 0);

	//! Frees memory allocated in this block.
	void Free();

	//! Attach memory buffer to this block.
	void Attach(uk buffer, i32 size, i32 uncompressedSize = 0);
	//! Detach memory buffer that was previously attached.
	void Detach();

	//! Returns amount of allocated memory in this block.
	i32 GetSize() const { return m_size; }

	//! Returns amount of allocated memory in this block.
	i32   GetUncompressedSize() const { return m_uncompressedSize; }

	uk GetBuffer() const           { return m_buffer; };

	//! Copy memory range to memory block.
	void Copy(uk src, i32 size);

	//! Compress this memory block to specified memory block.
	//! @param toBlock target memory block where compressed result will be stored.
	void Compress(CMemoryBlock& toBlock) const;

	//! Uncompress this memory block to specified memory block.
	//! @param toBlock target memory block where compressed result will be stored.
	void Uncompress(CMemoryBlock& toBlock) const;

	//! Serialize memory block to archive.
	void Serialize(CArchive& ar);

	//! Is MemoryBlock is empty.
	bool IsEmpty() const { return m_buffer == 0; }

private:
	uk m_buffer;
	i32   m_size;
	//! If not 0, memory block is compressed.
	i32   m_uncompressedSize;
	//! True if memory block owns its memory.
	bool  m_owns;
};

