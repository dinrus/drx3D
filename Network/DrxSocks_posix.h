// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//////////////////////////////////////////////////////////////////////////
// NOTE: INTERNAL HEADER NOT FOR PUBLIC USE
// This header should only be include by DrxSocks.h
#if !defined(INCLUDED_FROM_DRX_SOCKS_H)
	#error "DRXTEK INTERNAL HEADER. Only include from DrxSocks.h."
#endif
//////////////////////////////////////////////////////////////////////////

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>

#if !DRX_PLATFORM_APPLE
	#include <arpa/nameser.h>
	#include <resolv.h>
#endif

#include <drx3D/CoreX/String/StringUtils.h>

#define SD_SEND SHUT_WR
#define SD_BOTH SHUT_RDWR

typedef sockaddr    DRXSOCKADDR;
typedef sockaddr_in DRXSOCKADDR_IN;
typedef socklen_t   DRXSOCKLEN_T;
typedef i32         DRXSOCKET;
typedef in_addr     DRXINADDR;
typedef in_addr_t   DRXINADDR_T;
typedef hostent     DRXHOSTENT;

// Type wrappers for sockets
typedef fd_set  DRXFD_SET;
typedef timeval DRXTIMEVAL;

#define DRX_INVALID_SOCKET DRXSOCKET(-1)
#define DRX_SOCKET_ERROR   -1

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
	return errno;
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

static i32 listen(DRXSOCKET s, i32 backlog)
{
	return ::listen(s, backlog);
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

static i32 closesocket(DRXSOCKET s)
{
	return ::close(s);
}

static i32 connect(DRXSOCKET s, const DRXSOCKADDR* addr, DRXSOCKLEN_T addrlen)
{
	return ::connect(s, addr, addrlen);
}

static DRXSOCKET accept(DRXSOCKET s, DRXSOCKADDR* addr, DRXSOCKLEN_T* addrlen)
{
	return ::accept(s, addr, addrlen);
}

static i32 send(DRXSOCKET s, tukk buf, i32 len, i32 flags)
{
	return ::send(s, buf, len, flags | MSG_NOSIGNAL);
}

static bool MakeSocketNonBlocking(DRXSOCKET sock)
{
	i32 nFlags = ::fcntl(sock, F_GETFL);
	nFlags |= O_NONBLOCK;
	if (::fcntl(sock, F_SETFL, nFlags) == -1)
	{
		return false;
	}
	return true;
}

static bool MakeSocketBlocking(DRXSOCKET sock)
{
	i32 nFlags = ::fcntl(sock, F_GETFL);
	nFlags &= ~O_NONBLOCK;
	if (::fcntl(sock, F_SETFL, nFlags) == -1)
	{
		return false;
	}
	return true;
}

static DRXHOSTENT* gethostbyname(tukk hostname)
{
	return ::gethostbyname(hostname);
}

static i32 gethostname(tuk name, i32 namelen)
{
	return ::gethostname(name, namelen);
}

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
		ret = inet_addr(hostnameOrIP);
	}
	return ret != INADDR_NONE ? ret : 0;
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

///////////////////////////////////c////////////////////////////////////////////

static i32 bind(DRXSOCKET s, const DRXSOCKADDR* addr, DRXSOCKLEN_T addrlen)
{
	return ::bind(s, addr, addrlen);
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

	i32 ret = ::select(s + 1, &readSet, &emptySet, &emptySet, timeout);

	if (ret >= 0)
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

	i32 ret = ::select(s + 1, &emptySet, &writeSet, &emptySet, timeout);

	if (ret >= 0)
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

static DRXINADDR_T inet_addr(tukk cp)
{
	return ::inet_addr(cp);
}

///////////////////////////////////////////////////////////////////////////////

static tuk inet_ntoa(DRXINADDR in)
{
	return ::inet_ntoa(in);
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
  socketError = (_to);  break;
	eDrxSockError socketError = eCSE_NO_ERROR;
	i32 error = GetLastSocketError();
	switch (error)
	{
		TRANSLATE(0, eCSE_NO_ERROR);
		TRANSLATE(EACCES, eCSE_EACCES);
		TRANSLATE(EADDRINUSE, eCSE_EADDRINUSE);
		TRANSLATE(EADDRNOTAVAIL, eCSE_EADDRNOTAVAIL);
		TRANSLATE(EAFNOSUPPORT, eCSE_EAFNOSUPPORT);
		TRANSLATE(EALREADY, eCSE_EALREADY);
		TRANSLATE(EBADF, eCSE_EBADF);
		TRANSLATE(ECONNABORTED, eCSE_ECONNABORTED);
		TRANSLATE(ECONNREFUSED, eCSE_ECONNREFUSED);
		TRANSLATE(ECONNRESET, eCSE_ECONNRESET);
		TRANSLATE(EFAULT, eCSE_EFAULT);
		TRANSLATE(EHOSTDOWN, eCSE_EHOSTDOWN);
		TRANSLATE(EINPROGRESS, eCSE_EINPROGRESS);
		TRANSLATE(EINTR, eCSE_EINTR);
		TRANSLATE(EINVAL, eCSE_EINVAL);
		TRANSLATE(EISCONN, eCSE_EISCONN);
		TRANSLATE(EMFILE, eCSE_EMFILE);
		TRANSLATE(EMSGSIZE, eCSE_EMSGSIZE);
		TRANSLATE(ENETUNREACH, eCSE_ENETUNREACH);
		TRANSLATE(ENOBUFS, eCSE_ENOBUFS);
		TRANSLATE(ENOPROTOOPT, eCSE_ENOPROTOOPT);
		TRANSLATE(ENOTCONN, eCSE_ENOTCONN);
		TRANSLATE(EOPNOTSUPP, eCSE_EOPNOTSUPP);
		TRANSLATE(EPROTONOSUPPORT, eCSE_EPROTONOSUPPORT);
		TRANSLATE(ETIMEDOUT, eCSE_ETIMEDOUT);
		TRANSLATE(ETOOMANYREFS, eCSE_ETOOMANYREFS);
		TRANSLATE(EWOULDBLOCK, eCSE_EWOULDBLOCK);
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
	// reverse order of inputs (_to, _from) instead of (_from, _to) compared to TranslateLastSocketError
#define TRANSLATE(_to, _from) case (_from): \
  error = (_to); break;
	i32 error = 0;
	switch (socketError)
	{
		TRANSLATE(0, eCSE_NO_ERROR);
		TRANSLATE(EACCES, eCSE_EACCES);
		TRANSLATE(EADDRINUSE, eCSE_EADDRINUSE);
		TRANSLATE(EADDRNOTAVAIL, eCSE_EADDRNOTAVAIL);
		TRANSLATE(EAFNOSUPPORT, eCSE_EAFNOSUPPORT);
		TRANSLATE(EALREADY, eCSE_EALREADY);
		TRANSLATE(EBADF, eCSE_EBADF);
		TRANSLATE(ECONNABORTED, eCSE_ECONNABORTED);
		TRANSLATE(ECONNREFUSED, eCSE_ECONNREFUSED);
		TRANSLATE(ECONNRESET, eCSE_ECONNRESET);
		TRANSLATE(EFAULT, eCSE_EFAULT);
		TRANSLATE(EHOSTDOWN, eCSE_EHOSTDOWN);
		TRANSLATE(EINPROGRESS, eCSE_EINPROGRESS);
		TRANSLATE(EINTR, eCSE_EINTR);
		TRANSLATE(EINVAL, eCSE_EINVAL);
		TRANSLATE(EISCONN, eCSE_EISCONN);
		TRANSLATE(EMFILE, eCSE_EMFILE);
		TRANSLATE(EMSGSIZE, eCSE_EMSGSIZE);
		TRANSLATE(ENETUNREACH, eCSE_ENETUNREACH);
		TRANSLATE(ENOBUFS, eCSE_ENOBUFS);
		TRANSLATE(ENOPROTOOPT, eCSE_ENOPROTOOPT);
		TRANSLATE(ENOTCONN, eCSE_ENOTCONN);
		TRANSLATE(EOPNOTSUPP, eCSE_EOPNOTSUPP);
		TRANSLATE(EPROTONOSUPPORT, eCSE_EPROTONOSUPPORT);
		TRANSLATE(ETIMEDOUT, eCSE_ETIMEDOUT);
		TRANSLATE(ETOOMANYREFS, eCSE_ETOOMANYREFS);
		TRANSLATE(EWOULDBLOCK, eCSE_EWOULDBLOCK);
	default:
		DRX_ASSERT_MESSAGE(false, string().Format("DrxSock could not translate eDrxSockError error code %x, treating as miscellaneous", socketError));
		break;
	}
#undef TRANSLATE

	return error;
}

} // DrxSock
