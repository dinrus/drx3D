// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __DRXSOCKETERROR_H__
#define __DRXSOCKETERROR_H__

#pragma once

enum ESocketError
{
	eSE_Ok = 0,
	eSE_Cancelled,
	eSE_FragmentationOccured,
	eSE_BufferTooSmall,
	eSE_ZeroLengthPacket,
	eSE_ConnectInProgress,
	eSE_AlreadyConnected,
	eSE_MiscNonFatalError,

	eSE_Unrecoverable = 0x8000,
	eSE_UnreachableAddress,

	eSE_SocketClosed,
	eSE_ConnectionRefused,
	eSE_ConnectionReset,
	eSE_ConnectionTimedout,

	eSE_MiscFatalError
};

#endif
