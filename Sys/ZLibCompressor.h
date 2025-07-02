// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/******************************************************************************
** ZLibCompressor.h
** 23/6/10
******************************************************************************/

#ifndef __ZLIBCOMPRESSOR_H__
#define __ZLIBCOMPRESSOR_H__

#include <drx3D/Sys/IZLibCompressor.h>

class CZLibCompressor : public IZLibCompressor
{
protected:
	virtual ~CZLibCompressor();

public:
	virtual IZLibDeflateStream* CreateDeflateStream(i32 inLevel, EZLibMethod inMethod, i32 inWindowBits, i32 inMemLevel, EZLibStrategy inStrategy, EZLibFlush inFlushMethod);
	virtual void                Release();

	virtual void                MD5Init(SMD5Context* pIOCtx);
	virtual void                MD5Update(SMD5Context* pIOCtx, u8k* pInBuff, u32 len);
	virtual void                MD5Final(SMD5Context* pIOCtx, u8 outDigest[16]);
};

#endif // __ZLIBCOMPRESSOR_H__
