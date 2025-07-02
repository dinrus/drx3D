// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//////////////////////////////////////////////////////////////////////////

#include <drx3D/LiveCreate/StdAfx.h>
#include <drx3D/Network/IServiceNetwork.h>
#include <drx3D/Network/IRemoteCommand.h>
#include <drx3D/LiveCreate/LiveCreateUpr.h>
#include <drx3D/LiveCreate/LiveCreateHost.h>
#include <drx3D/LiveCreate/LiveCreateNativeFile.h>

#ifndef NO_LIVECREATE

namespace LiveCreate
{

CLiveCreateFileReader::CLiveCreateFileReader()
	: m_handle(NULL)
{
}

CLiveCreateFileReader::~CLiveCreateFileReader()
{
	if (NULL != m_handle)
	{
		gEnv->pDrxPak->FClose(m_handle);
		m_handle = NULL;
	}
}

void CLiveCreateFileReader::Delete()
{
	delete this;
}

void CLiveCreateFileReader::Read(uk pData, u32k size)
{
	DWORD readBytes = 0;
	gEnv->pDrxPak->FRead((tuk)pData, size, m_handle);
}

void CLiveCreateFileReader::Skip(u32k size)
{
	gEnv->pDrxPak->FSeek(m_handle, size, SEEK_CUR);
}

void CLiveCreateFileReader::Read8(uk pData)
{
	Read(pData, sizeof(uint64));
	SwapEndian<uint64>(*(uint64*)pData, eLittleEndian);
}

void CLiveCreateFileReader::Read4(uk pData)
{
	Read(pData, sizeof(u32));
	SwapEndian<u32>(*(u32*)pData, eLittleEndian);
}

void CLiveCreateFileReader::Read2(uk pData)
{
	Read(pData, sizeof(u16));
	SwapEndian<u16>(*(u16*)pData, eLittleEndian);
}

void CLiveCreateFileReader::Read1(uk pData)
{
	Read(pData, sizeof(u8));
}

ukk CLiveCreateFileReader::GetPointer()
{
	return NULL;
}

CLiveCreateFileReader* CLiveCreateFileReader::CreateReader(tukk szNativePath)
{
	FILE* handle = gEnv->pDrxPak->FOpenRaw(szNativePath, "rb");
	if (NULL == handle)
	{
		return NULL;
	}

	CLiveCreateFileReader* ret = new CLiveCreateFileReader();
	ret->m_handle = handle;
	return ret;
}

}

#endif
