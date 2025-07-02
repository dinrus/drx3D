// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/DrxVariant.h>
#include <drx3D/Network/DrxSocks.h>
#include <drx3D/CoreX/Lobby/CommonIDrxLobby.h>

#if !DRX_PLATFORM_APPLE && !DRX_PLATFORM_ORBIS && defined(__GNUC__)
    #ifdef _SS_MAXSIZE
    #undef _SS_MAXSIZE
    #endif
	#define _SS_MAXSIZE 256
#endif

#if DRX_PLATFORM_DURANGO
	#define USE_SOCKADDR_STORAGE_ADDR 1
#else
	#define USE_SOCKADDR_STORAGE_ADDR 0
#endif

typedef u16               TLocalNetAddress;

typedef DrxFixedStringT<128> TAddressString;

#if USE_SOCKADDR_STORAGE_ADDR
struct SSockAddrStorageAddr
{
	ILINE SSockAddrStorageAddr() { memset(&addr, 0, sizeof(addr)); }
	ILINE SSockAddrStorageAddr(sockaddr_storage addr) { this->addr = addr; }
	sockaddr_storage addr;
	ILINE bool operator<(const SSockAddrStorageAddr& rhs) const
	{
		if (addr.ss_family == rhs.addr.ss_family)
		{
			switch (addr.ss_family)
			{
			case AF_INET6:
				{
					sockaddr_in6* pLHS = (sockaddr_in6*)&addr;
					sockaddr_in6* pRHS = (sockaddr_in6*)&rhs.addr;
					i32 addrcmp = memcmp(&pLHS->sin6_addr, &pRHS->sin6_addr, sizeof(pLHS->sin6_addr));

					return (addrcmp < 0) || ((addrcmp == 0) && (pLHS->sin6_port < pRHS->sin6_port));
				}

			default:
				DrxFatalError("SSockAddrStorageAddr < Unknown address family %u", addr.ss_family);
				return false;
			}
		}
		else
		{
			DrxFatalError("SSockAddrStorageAddr < Mismatched address families %u %u", addr.ss_family, rhs.addr.ss_family);
			return false;
		}
	}
	ILINE bool operator==(const SSockAddrStorageAddr& rhs) const
	{
		if (addr.ss_family == rhs.addr.ss_family)
		{
			switch (addr.ss_family)
			{
			case AF_INET6:
				{
					sockaddr_in6* pLHS = (sockaddr_in6*)&addr;
					sockaddr_in6* pRHS = (sockaddr_in6*)&rhs.addr;
					i32 addrcmp = memcmp(&pLHS->sin6_addr, &pRHS->sin6_addr, sizeof(pLHS->sin6_addr));
					return (addrcmp == 0) && (pLHS->sin6_port == pRHS->sin6_port);
				}

			default:
				DrxFatalError("SSockAddrStorageAddr == Unknown address family %u", addr.ss_family);
				return false;
			}
		}
		else
		{
			DrxFatalError("SSockAddrStorageAddr == Mismatched address families %u %u", addr.ss_family, rhs.addr.ss_family);
			return false;
		}
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const {}
};
#endif

struct LobbyIdAddr
{
	ILINE LobbyIdAddr(uint64 id) { this->id = id; }
	uint64 id;
	ILINE bool operator<(const LobbyIdAddr& rhs) const
	{
		return id < rhs.id;
	}

	ILINE bool operator==(const LobbyIdAddr& rhs) const
	{
		return id == rhs.id;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const {}
};

struct SIPv4Addr
{
	ILINE SIPv4Addr() : addr(0), port(0), lobbyService(eCLS_NumServices) {}
	ILINE SIPv4Addr(u32 addr, u16 port) { this->addr = addr; this->port = port; this->lobbyService = eCLS_NumServices; }
	ILINE SIPv4Addr(u32 addr, u16 port, EDrxLobbyService lobbyService) { this->addr = addr; this->port = port; this->lobbyService = lobbyService; }

	u32 addr;
	u16 port;

	// Can be used to specify which Lobby Socket Service to use to send lobby packets with if it differs from the current service.
	u8 lobbyService;

	ILINE bool operator<(const SIPv4Addr& rhs) const
	{
		return addr < rhs.addr || (addr == rhs.addr && port < rhs.port);
	}

	ILINE bool operator==(const SIPv4Addr& rhs) const
	{
		return addr == rhs.addr && port == rhs.port;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const {}
};

struct SNullAddr
{
	ILINE bool operator<(const SNullAddr& rhs) const
	{
		return false;
	}

	ILINE bool operator==(const SNullAddr& rhs) const
	{
		return true;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const {}
};

typedef DrxVariant<
    SNullAddr, TLocalNetAddress, SIPv4Addr, LobbyIdAddr
#if USE_SOCKADDR_STORAGE_ADDR
    , SSockAddrStorageAddr
#endif
    > TNetAddress;

typedef DynArray<TNetAddress>                            TNetAddressVec;
