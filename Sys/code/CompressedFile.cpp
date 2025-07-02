// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/System.h>
#include <drx/Core/lib/z/zlib.h>

bool CSystem::CompressDataBlock(ukk input, size_t inputSize, uk output, size_t& outputSize, i32 level)
{
	uLongf destLen = outputSize;
	Bytef* dest = static_cast<Bytef*>(output);
	uLong sourceLen = inputSize;
	const Bytef* source = static_cast<const Bytef*>(input);
	bool ok = Z_OK == compress2(dest, &destLen, source, sourceLen, level);
	outputSize = destLen;
	return ok;
}

bool CSystem::DecompressDataBlock(ukk input, size_t inputSize, uk output, size_t& outputSize)
{
	uLongf destLen = outputSize;
	Bytef* dest = static_cast<Bytef*>(output);
	uLong sourceLen = inputSize;
	const Bytef* source = static_cast<const Bytef*>(input);
	bool ok = Z_OK == uncompress(dest, &destLen, source, sourceLen);
	outputSize = destLen;
	return ok;
}
