#include <drx/Core/Core.h>
#include <drx3D/CoreX/BaseTypes.h>
#include <drx3D/Sys/ZLibCompressor.h>

void MD5Init (SMD5Context *context);
void MD5Update (SMD5Context *context, u8k *input, u32 inputLen);
void MD5Final (u8 *digest, SMD5Context *context);
