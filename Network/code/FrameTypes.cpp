// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include <algorithm>
#include  <drx3D/Network/FrameTypes.h>
#include  <drx3D/Network/Config.h>
#include <drx3D/CoreX/Math/MTPseudoRandom.h>

u8 Frame_HeaderToID[256];
u8 Frame_IDToHeader[256];
u8 SeqBytes[256];
u8 UnseqBytes[256];

void InitFrameTypes()
{
	CMTRand_int32 r('C3rL');

	bool isDevmode = gEnv->pSystem->IsDevMode();

	memset(Frame_HeaderToID, 0, sizeof(Frame_HeaderToID));
	// use this block of code to 'allocate' any fixed packet headers you might need
	// i.e. we enforce that 'P' (0x50) correspond to a PingServer
	// it's not validated here, but everything that is fixed here should have an enum value
	// less than eH_FIRST_GENERAL_ENTRY
#ifdef __WITH_PB__
	Frame_HeaderToID[0xff] = eH_PunkBuster;
#endif
	Frame_HeaderToID[uchar('P')] = eH_PingServer;
	Frame_HeaderToID[uchar('?')] = eH_QueryLan;
	Frame_HeaderToID[uchar('C')] = eH_DrxLobby;
	uchar conn = '<';
	uchar discon = '>';
	if (isDevmode)
		std::swap(conn, discon);
	Frame_HeaderToID[uchar(conn)] = eH_ConnectionSetup;
	Frame_HeaderToID[uchar(discon)] = eH_Disconnect;
	Frame_HeaderToID[uchar('=')] = eH_DisconnectAck;

	// now we add the 'general entries' to the table
	// these are entries where we don't really care what byte signals their headers
	i32 ofs = 0;
	for (i32 i = eH_FIRST_GENERAL_ENTRY; i < eH_LAST_GENERAL_ENTRY; i++)
	{
		while (Frame_HeaderToID[ofs])
			ofs++;
		Frame_HeaderToID[ofs++] = i;
	}

	// perform a shuffle on general entries to get some obfuscation
	for (i32 i = 0; i < 512; i++)
	{
		i32 ofs1, ofs2;
		while (true)
		{
			ofs1 = ((r.GenerateUint32() + PROTOCOL_VERSION * 127) % 256) ^ (isDevmode << (r.GenerateUint32() % 8));
			ofs2 = (r.GenerateUint32() + 256 - eH_LAST_GENERAL_ENTRY) % 256;

			if (ofs1 == ofs2)
				continue;
			PREFAST_ASSUME(ofs >= 0 && ofs < DRX_ARRAY_COUNT(Frame_HeaderToID));
			if (Frame_HeaderToID[ofs1] && Frame_HeaderToID[ofs1] <= eH_FIRST_GENERAL_ENTRY)
				continue;
			if (Frame_HeaderToID[ofs2] && Frame_HeaderToID[ofs2] <= eH_FIRST_GENERAL_ENTRY)
				continue;
			break;
		}
		std::swap(Frame_HeaderToID[ofs1], Frame_HeaderToID[ofs2]);
	}

	for (i32 i = 0; i < 256; i++)
	{
		PREFAST_SUPPRESS_WARNING(6386)
		Frame_IDToHeader[Frame_HeaderToID[i]] = i;
	}

	for (i32 i = 0; i < 256; i++)
		SeqBytes[i] = i ^ 0xee;
	for (i32 i = 0; i < 512; i++)
	{
		u8 a, b;
		do
		{
			u32 rx = r.GenerateUint32();
			a = (rx >> 10) + PROTOCOL_VERSION;
			b = (rx + PROTOCOL_VERSION) >> 20;
		}
		while (a == b);
		std::swap(SeqBytes[a], SeqBytes[b]);
	}
	for (i32 i = 0; i < 256; i++)
		UnseqBytes[SeqBytes[i]] = i;
}
