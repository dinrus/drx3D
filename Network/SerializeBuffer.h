// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _SERIALIZEBUFFER_H_
#define _SERIALIZEBUFFER_H_

void inline SaveBuffer(uk pData, i32 nSize, uchar* pSerialBuffer, i32& nSaveBufferPos)
{
	if (pSerialBuffer)
	{
		// set the first 4 bytes of the buffer to the size of the buffer or to 0 if the data isn't available
		*(i32*)(pSerialBuffer + nSaveBufferPos) = pData ? nSize : 0;
	}

	nSaveBufferPos += sizeof(i32);

	if (pSerialBuffer)
	{
		if (nSize && pData)
			memcpy(pSerialBuffer + nSaveBufferPos, pData, nSize);
	}

	if (pData)
		nSaveBufferPos += nSize;
}

bool inline LoadBuffer(uk pData, u32 nMaxBytesToLoad, uchar* pSerialBuffer, i32& nSaveBufferPos)
{
	i32 nSize = 0;
	if (nMaxBytesToLoad < 4)
	{
		nSaveBufferPos += 4;
		return false;
	}

	nSize = *(i32*)(pSerialBuffer + nSaveBufferPos);
	nSaveBufferPos += 4;

	if ((u32)nSize > nMaxBytesToLoad)
		return false;

	if (!nSize)
		return true;

	assert(pData);

	if (nSize)
		memcpy(pData, pSerialBuffer + nSaveBufferPos, nSize);

	nSaveBufferPos += nSize;

	return true;
}

#endif // _SERIALIZEBUFFER_H_
