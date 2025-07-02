// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// ------------------------------------------------------------------------
//  Имя файла:   LZ4Decompressor.h
//  Created:     5/9/2012 by Axel Gneiting
//  Описание: lz4 hc decompress wrapper
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#ifndef __LZ4DECOMPRESSOR_H__
#define __LZ4DECOMPRESSOR_H__

#include <drx3D/Sys/ILZ4Decompressor.h>

class CLZ4Decompressor : public ILZ4Decompressor
{
public:
	virtual bool DecompressData(tukk pIn, tuk pOut, const uint outputSize) const;
	virtual void Release();

private:
	virtual ~CLZ4Decompressor() {}
};

#endif // __LZ4DECOMPRESSOR_H__
