// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef BASE64_H
#define BASE64_H

namespace Base64
{
static u8 indexTable[65] = \
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"        \
  "abcdefghijklmnopqrstuvwxyz"        \
  "0123456789+/";

static u8 invertTable[256];
static bool invertTableBuilt = false;

static void buildinverttable(void)
{
	u32 u, v;
	for (u = 0; u < 256; ++u)
	{
		invertTable[u] = '\0';
		for (v = 0; v < 64; ++v)
		{
			if (indexTable[v] == u)
			{
				invertTable[u] = v;
				break;
			}
		}
	}
}

static u32 encode_base64(tuk out, tukk __restrict const in, u32k size, bool terminate /*= true*/)
{
	u8 buf[3];
	u32 u;
	tukk const start = out;

	// Начально разложенные байты.

	i32k remainder = size % 3;
	u32k initial = size - remainder;
	for (u = 0; u < initial; u += 3)
	{
		for (u32 v = 0; v < 3; ++v)
			buf[v] = in[u + v];

		*(out++) = indexTable[(buf[0] & 0xfe) >> 2];
		*(out++) = indexTable[((buf[0] & 0x03) << 4) | ((buf[1] & 0xf0) >> 4)];
		*(out++) = indexTable[((buf[1] & 0x0f) << 2) | ((buf[2] & 0xc0) >> 6)];
		*(out++) = indexTable[buf[2] & 0x3f];
	}

	// Остаток неразложенных (unaligned) байтов.

	for (i32 v = 0; v < remainder; ++v)
		buf[v] = in[u + v];

	if (remainder == 2)
	{
		*(out++) = indexTable[(buf[0] & 0xfe) >> 2];
		*(out++) = indexTable[((buf[0] & 0x03) << 4) | ((buf[1] & 0xf0) >> 4)];
		*(out++) = indexTable[((buf[1] & 0x0f) << 2)];
		*(out++) = '=';
	}
	else if (remainder == 1)
	{
		*(out++) = indexTable[(buf[0] & 0xfe) >> 2];
		*(out++) = indexTable[(buf[0] & 0x03) << 4];
		*(out++) = '=';
		*(out++) = '=';
	}

	if (terminate) *(out++) = '\0';

	return (u32)(out - start);
}

static u32 decode_base64(tuk out, tukk __restrict const in, u32k size, bool terminate /*= true*/)
{
	u8 buf[4];
	u32 u;
	tukk const start = out;

	if (!invertTableBuilt)
	{
		invertTableBuilt = true;
		buildinverttable();
	}

	assert(size % 4 == 0 && "Expected padding on Base64 encoded string.");

	for (u = 0; u < size; u += 4)
	{
		for (u32 v = 0; v < 4; ++v)
			buf[v] = in[u + v];

		*(out++) = (invertTable[buf[0]] << 2) | ((invertTable[buf[1]] & 0x30) >> 4);
		if (buf[1] == '=' || buf[2] == '=')
		{
			break;
		}
		*(out++) = ((invertTable[buf[1]] & 0x0f) << 4) | ((invertTable[buf[2]] & 0x3c) >> 2);
		if (buf[2] == '=' || buf[3] == '=')
		{
			break;
		}
		*(out++) = ((invertTable[buf[2]] & 0x03) << 6) | (invertTable[buf[3]] & 0x3f);
	}

	if (terminate) *(out++) = '\0';

	return (u32)(out - start);
}

static u32 encodedsize_base64(u32k size)
{
	return (size + 2 - ((size + 2) % 3)) * 4 / 3;
}

static u32 decodedsize_base64(u32k size)
{
#define PADDINGP2(offset, align) \
  ((align) + (((offset) - 1) & ~((align) - 1))) - (offset)

	u32 nSize = size * 3 / 4 + PADDINGP2(size * 3 / 4, 4);

#undef PADDINGP2

	return nSize;
}
};

#endif
