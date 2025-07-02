// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//////////////////////////////////////////////////////////////////////////
// NOTE: INTERNAL HEADER NOT FOR PUBLIC USE
// This header should only be include by DrxSocks.h
#if !defined(INCLUDED_FROM_DRX_SOCKS_H)
	#error "DRXTEK INTERNAL HEADER. Only include from DrxSocks.h."
#endif
//////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
#include <drx3D/CoreX/Platform/DrxWindows.h>
#include <WinSock2.h>
#include <ws2tcpip.h>

#define DRX_INVALID_SOCKET INVALID_SOCKET
#define DRX_SOCKET_ERROR   SOCKET_ERROR

#include <string.h>

///////////////////////////////////////////////////////////////////////////////

// Type wrappers for sockets

typedef sockaddr      DRXSOCKADDR;
typedef sockaddr_in   DRXSOCKADDR_IN;
typedef socklen_t     DRXSOCKLEN_T;
typedef SOCKET        DRXSOCKET;
typedef in_addr       DRXINADDR;
typedef u64 DRXINADDR_T;
typedef hostent       DRXHOSTENT;

// Type wrappers for sockets
typedef fd_set  DRXFD_SET;
typedef timeval DRXTIMEVAL;

///////////////////////////////////////////////////////////////////////////////

namespace DrxSock
{
///////////////////////////////////////////////////////////////////////////////
// Forward declarations
static eDrxSockError TranslateLastSocketError();
static i32           TranslateToSocketError(eDrxSockError socketError);

static eDrxSockError TranslateInvalidSocket(DRXSOCKET s);
static eDrxSockError TranslateSocketError(i32 socketError);

///////////////////////////////////////////////////////////////////////////////

static i32 GetLastSocketError()
{
	return WSAGetLastError();
}

///////////////////////////////////////////////////////////////////////////////

static DRXSOCKET socket(i32 af, i32 type, i32 protocol)
{
	return ::socket(af, type, protocol);
}

///////////////////////////////////////////////////////////////////////////////

//! Create default INET socket.
static DRXSOCKET socketinet()
{
	return ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

///////////////////////////////////////////////////////////////////////////////

static i32 setsockopt(DRXSOCKET s, i32 level, i32 optname, tukk optval, DRXSOCKLEN_T optlen)
{
	return ::setsockopt(s, level, optname, optval, optlen);
}

///////////////////////////////////////////////////////////////////////////////

static i32 closesocket(DRXSOCKET s)
{
	return ::closesocket(s);
}

///////////////////////////////////////////////////////////////////////////////

static i32 shutdown(DRXSOCKET s, i32 how)
{
	return ::shutdown(s, how);
}

///////////////////////////////////////////////////////////////////////////////

static i32 getsockname(DRXSOCKET s, DRXSOCKADDR* addr, DRXSOCKLEN_T* addrlen)
{
	return ::getsockname(s, addr, addrlen);
}

///////////////////////////////////////////////////////////////////////////////

static i32 connect(DRXSOCKET s, const DRXSOCKADDR* addr, DRXSOCKLEN_T addrlen)
{
	return ::connect(s, addr, addrlen);
}

///////////////////////////////////////////////////////////////////////////////

static i32 listen(DRXSOCKET s, i32 backlog)
{
	return ::listen(s, backlog);
}

///////////////////////////////////////////////////////////////////////////////

static DRXSOCKET accept(DRXSOCKET s, DRXSOCKADDR* addr, DRXSOCKLEN_T* addrlen)
{
	return ::accept(s, addr, addrlen);
}

///////////////////////////////////////////////////////////////////////////////

static i32 send(DRXSOCKET s, tukk buf, i32 len, i32 flags)
{
	return ::send(s, buf, len, flags);
}

///////////////////////////////////////////////////////////////////////////////

static i32 recv(DRXSOCKET s, tuk buf, i32 len, i32 flags)
{
	return ::recv(s, buf, len, flags);
}

///////////////////////////////////////////////////////////////////////////////

static i32 recvfrom(DRXSOCKET s, tuk buf, i32 len, i32 flags, DRXSOCKADDR* addr, DRXSOCKLEN_T* addrLen)
{
	return ::recvfrom(s, buf, len, flags, addr, addrLen);
}

///////////////////////////////////////////////////////////////////////////////

static i32 sendto(DRXSOCKET s, tuk buf, i32 len, i32 flags, DRXSOCKADDR* addr, DRXSOCKLEN_T addrLen)
{
	return ::sendto(s, buf, len, flags, addr, addrLen);
}

///////////////////////////////////////////////////////////////////////////////

static i32 SetRecvTimeout(DRXSOCKET s, i32k seconds, i32k microseconds)
{
	struct timeval timeout;
	timeout.tv_sec = seconds;
	timeout.tv_usec = microseconds;
	return DrxSock::setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (tukk)&timeout, sizeof(timeout));
}

///////////////////////////////////////////////////////////////////////////////

static i32 SetSendTimeout(DRXSOCKET s, i32k seconds, i32k microseconds)
{
	struct timeval timeout;
	timeout.tv_sec = seconds;
	timeout.tv_usec = microseconds;
	return DrxSock::setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (tukk)&timeout, sizeof(timeout));
}

///////////////////////////////////////////////////////////////////////////////

//! \retval 1   Readable - use recv to get the data.
//! \retval 0   Timed out (exceeded passed timeout value).
//! \retval <0  An error occurred - see eDrxSocketError.
static i32 IsRecvPending(DRXSOCKET s, DRXTIMEVAL* timeout)
{
	DRXFD_SET emptySet;
	FD_ZERO(&emptySet);

	DRXFD_SET readSet;
	FD_ZERO(&readSet);
	FD_SET(s, &readSet);

	i32 ret = ::select(static_cast<i32>(s) + 1, &readSet, &emptySet, &emptySet, timeout);
	if (ret != SOCKET_ERROR)
	{
		ret = FD_ISSET(s, &readSet);
		if (ret != 0)
		{
			ret = 1;
		}
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////

//! \retval 1   Connected.
//! \retval 0   Timed out (exceeded passed timeout value).
//! \retval <0  An error occurred - see eDrxSocketError.
static i32 WaitForWritableSocket(DRXSOCKET s, DRXTIMEVAL* timeout)
{
	DRXFD_SET emptySet;
	FD_ZERO(&emptySet);

	DRXFD_SET writeSet;
	FD_ZERO(&writeSet);
	FD_SET(s, &writeSet);

	i32 ret = ::select(static_cast<i32>(s) + 1, &emptySet, &writeSet, &emptySet, timeout);

	if (ret != SOCKET_ERROR)
	{
		ret = FD_ISSET(s, &writeSet);
		if (ret != 0)
		{
			ret = 1;
		}
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////

//! IsRecvPending should be sufficient for most applications.
//! The correct value for the first parameter 'nfds' should be the highest
//! numbered socket (as passed to FD_SET) +1.
//! Only use this if you know what you're doing! Passing nfds as 0 on anything
//! other than WinSock is wrong!.
static i32 select(i32 nfds, DRXFD_SET* readfds, DRXFD_SET* writefds, DRXFD_SET* exceptfds, DRXTIMEVAL* timeout)
{
#if !defined(_RELEASE)
	if (nfds == 0)
	{
		DrxFatalError("DrxSock select detected first parameter is 0!  This *MUST* be fixed!");
	}
#endif

	return ::select(nfds, readfds, writefds, exceptfds, timeout);
}

///////////////////////////////////c////////////////////////////////////////////

static i32 bind(DRXSOCKET s, const DRXSOCKADDR* addr, DRXSOCKLEN_T addrlen)
{
	return ::bind(s, addr, addrlen);
}

///////////////////////////////////////////////////////////////////////////////

static bool MakeSocketNonBlocking(DRXSOCKET sock)
{
	u64 nTrue = 1;
	if (::ioctlsocket(sock, FIONBIO, &nTrue) == SOCKET_ERROR)
	{
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////

static bool MakeSocketBlocking(DRXSOCKET sock)
{
	u64 nTrue = 0;
	if (::ioctlsocket(sock, FIONBIO, &nTrue) == SOCKET_ERROR)
	{
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

static DRXHOSTENT* gethostbyname(tukk hostname)
{
	PREFAST_SUPPRESS_WARNING(4996);
	return ::gethostbyname(hostname);
}

///////////////////////////////////////////////////////////////////////////////

static i32 gethostname(tuk name, i32 namelen)
{
	return ::gethostname(name, namelen);
}

///////////////////////////////////////////////////////////////////////////////

static DRXINADDR_T GetAddrForHostOrIP(tukk hostnameOrIP, u32 timeoutMilliseconds = 0)
{
	u32 ret = INADDR_NONE;
	DRXHOSTENT* pHost = gethostbyname(hostnameOrIP);
	if (pHost)
	{
		ret = ((in_addr*)(pHost->h_addr))->s_addr;
	}
	else
	{
		PREFAST_SUPPRESS_WARNING(4996);
		ret = ::inet_addr(hostnameOrIP);
	}
	return ret != INADDR_NONE ? ret : 0;
}

///////////////////////////////////////////////////////////////////////////////

static DRXINADDR_T inet_addr(tukk cp)
{
	PREFAST_SUPPRESS_WARNING(4996);
	return ::inet_addr(cp);
}

///////////////////////////////////////////////////////////////////////////////

static tuk inet_ntoa(DRXINADDR in)
{
	PREFAST_SUPPRESS_WARNING(4996);
	return ::inet_ntoa(in);
}

///////////////////////////////////////////////////////////////////////////////

static u32 DNSLookup(tukk hostname, u32 timeoutMilliseconds = 200)
{
	DRXINADDR_T ret = DrxSock::GetAddrForHostOrIP(hostname, timeoutMilliseconds);

	if (ret == 0 || ret == ~0)
	{
		char host[256];

		size_t hostlen = strlen(hostname);
		size_t domainlen = sizeof(LOCAL_FALLBACK_DOMAIN);
		if (hostlen + domainlen > sizeof(host) - 1)
		{
			DrxWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, "hostname too long to fallback to local domain: '%s'", hostname);
			return 0;
		}

		drx_strcpy(host, hostname);
		drx_strcat(host, LOCAL_FALLBACK_DOMAIN);

		host[hostlen + domainlen - 1] = 0;

		ret = DrxSock::GetAddrForHostOrIP(host, timeoutMilliseconds);
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////

static eDrxSockError TranslateInvalidSocket(DRXSOCKET s)
{
	if (s == DRX_INVALID_SOCKET)
	{
		return TranslateLastSocketError();
	}
	return eCSE_NO_ERROR;
}

///////////////////////////////////////////////////////////////////////////////

static eDrxSockError TranslateSocketError(i32 socketError)
{
	if (socketError == DRX_SOCKET_ERROR)
	{
		return TranslateLastSocketError();
	}
	return eCSE_NO_ERROR;
}

///////////////////////////////////////////////////////////////////////////////

static eDrxSockError TranslateLastSocketError()
{
#define TRANSLATE(_from, _to) case (_from): \
  socketError = (_to); break;
	eDrxSockError socketError = eCSE_NO_ERROR;
	i32 error = GetLastSocketError();
	switch (error)
	{
		TRANSLATE(0, eCSE_NO_ERROR);

		TRANSLATE(WSAEACCES, eCSE_EACCES);
		TRANSLATE(WSAEADDRINUSE, eCSE_EADDRINUSE);
		TRANSLATE(WSAEADDRNOTAVAIL, eCSE_EADDRNOTAVAIL);
		TRANSLATE(WSAEAFNOSUPPORT, eCSE_EAFNOSUPPORT);
		TRANSLATE(WSAEALREADY, eCSE_EALREADY);
		TRANSLATE(WSAEBADF, eCSE_EBADF);
		TRANSLATE(WSAECONNABORTED, eCSE_ECONNABORTED);
		TRANSLATE(WSAECONNREFUSED, eCSE_ECONNREFUSED);
		TRANSLATE(WSAECONNRESET, eCSE_ECONNRESET);
		TRANSLATE(WSAEFAULT, eCSE_EFAULT);
		TRANSLATE(WSAEHOSTDOWN, eCSE_EHOSTDOWN);
		TRANSLATE(WSAEINPROGRESS, eCSE_EINPROGRESS);
		TRANSLATE(WSAEINTR, eCSE_EINTR);
		TRANSLATE(WSAEINVAL, eCSE_EINVAL);
		TRANSLATE(WSAEISCONN, eCSE_EISCONN);
		TRANSLATE(WSAEMFILE, eCSE_EMFILE);
		TRANSLATE(WSAEMSGSIZE, eCSE_EMSGSIZE);
		TRANSLATE(WSAENETUNREACH, eCSE_ENETUNREACH);
		TRANSLATE(WSAENOBUFS, eCSE_ENOBUFS);
		TRANSLATE(WSAENOPROTOOPT, eCSE_ENOPROTOOPT);
		TRANSLATE(WSAENOTCONN, eCSE_ENOTCONN);
		TRANSLATE(WSAEOPNOTSUPP, eCSE_EOPNOTSUPP);
		TRANSLATE(WSAEPROTONOSUPPORT, eCSE_EPROTONOSUPPORT);
		TRANSLATE(WSAETIMEDOUT, eCSE_ETIMEDOUT);
		TRANSLATE(WSAETOOMANYREFS, eCSE_ETOOMANYREFS);
		TRANSLATE(WSAEWOULDBLOCK, eCSE_EWOULDBLOCK);

		TRANSLATE(WSANOTINITIALISED, eCSE_ENOTINITIALISED);
	default:
		DRX_ASSERT_MESSAGE(false, string().Format("DrxSock could not translate OS error code %x, treating as miscellaneous", error));
		socketError = eCSE_MISC_ERROR;
		break;
	}
#undef TRANSLATE

	return socketError;
}

///////////////////////////////////////////////////////////////////////////////

static i32 TranslateToSocketError(eDrxSockError socketError)
{
#define TRANSLATE(_to, _from) case (_from): \
  error = (_to); break;                                            // reverse order of inputs (_to, _from) instead of (_from, _to) compared to TranslateLastSocketError
	i32 error = 0;
	switch (socketError)
	{
		TRANSLATE(0, eCSE_NO_ERROR);

		TRANSLATE(WSAEACCES, eCSE_EACCES);
		TRANSLATE(WSAEADDRINUSE, eCSE_EADDRINUSE);
		TRANSLATE(WSAEADDRNOTAVAIL, eCSE_EADDRNOTAVAIL);
		TRANSLATE(WSAEAFNOSUPPORT, eCSE_EAFNOSUPPORT);
		TRANSLATE(WSAEALREADY, eCSE_EALREADY);
		TRANSLATE(WSAEBADF, eCSE_EBADF);
		TRANSLATE(WSAECONNABORTED, eCSE_ECONNABORTED);
		TRANSLATE(WSAECONNREFUSED, eCSE_ECONNREFUSED);
		TRANSLATE(WSAECONNRESET, eCSE_ECONNRESET);
		TRANSLATE(WSAEFAULT, eCSE_EFAULT);
		TRANSLATE(WSAEHOSTDOWN, eCSE_EHOSTDOWN);
		TRANSLATE(WSAEINPROGRESS, eCSE_EINPROGRESS);
		TRANSLATE(WSAEINTR, eCSE_EINTR);
		TRANSLATE(WSAEINVAL, eCSE_EINVAL);
		TRANSLATE(WSAEISCONN, eCSE_EISCONN);
		TRANSLATE(WSAEMFILE, eCSE_EMFILE);
		TRANSLATE(WSAEMSGSIZE, eCSE_EMSGSIZE);
		TRANSLATE(WSAENETUNREACH, eCSE_ENETUNREACH);
		TRANSLATE(WSAENOBUFS, eCSE_ENOBUFS);
		TRANSLATE(WSAENOPROTOOPT, eCSE_ENOPROTOOPT);
		TRANSLATE(WSAENOTCONN, eCSE_ENOTCONN);
		TRANSLATE(WSAEOPNOTSUPP, eCSE_EOPNOTSUPP);
		TRANSLATE(WSAEPROTONOSUPPORT, eCSE_EPROTONOSUPPORT);
		TRANSLATE(WSAETIMEDOUT, eCSE_ETIMEDOUT);
		TRANSLATE(WSAETOOMANYREFS, eCSE_ETOOMANYREFS);
		TRANSLATE(WSAEWOULDBLOCK, eCSE_EWOULDBLOCK);

		TRANSLATE(WSANOTINITIALISED, eCSE_ENOTINITIALISED);
	default:
		DRX_ASSERT_MESSAGE(false, string().Format("DrxSock could not translate eDrxSockError error code %x, treating as miscellaneous", socketError));
		break;
	}
#undef TRANSLATE

	return error;
}

///////////////////////////////////////////////////////////////////////////////

}; // DrxSock
