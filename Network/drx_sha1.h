// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __DRX_SHA1_H__
#define __DRX_SHA1_H__

namespace DrxSHA1
{
void sha1Calc(tukk str, u32* digest);
void sha1Calc(u8k* str, uint64 length, u32* digest);
};

#endif // __DRX_SHA1_H__
