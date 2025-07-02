// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/Config.h>
#include  <drx3D/Network/IDatagramSocket.h>
#include  <drx3D/Network/LocalDatagramSocket.h>
#include  <drx3D/Network/UDPDatagramSocket.h>
#include  <drx3D/Network/CompositeDatagramSocket.h>
#include  <drx3D/Network/InternetSimulatorSocket.h>

class CNullSocket : public IDatagramSocket
{
public:
	virtual void         RegisterListener(IDatagramListener* pListener) override                         {}
	virtual void         UnregisterListener(IDatagramListener* pListener) override                       {}
	virtual void         GetSocketAddresses(TNetAddressVec& addrs) override                              { addrs.push_back(TNetAddress(SNullAddr())); }
	virtual ESocketError Send(u8k* pBuffer, size_t nLength, const TNetAddress& to) override      { return eSE_MiscFatalError; }
	virtual ESocketError SendVoice(u8k* pBuffer, size_t nLength, const TNetAddress& to) override { return eSE_MiscFatalError; }
	virtual void         Die() override                                                                  {}
	virtual bool         IsDead() override                                                               { return true; }
	virtual void         RegisterBackoffAddress(const TNetAddress& addr) override                               {}
	virtual void         UnregisterBackoffAddress(const TNetAddress& addr) override                             {}
};

void CDatagramSocket::RegisterListener(IDatagramListener* pListener)
{
	m_listeners.Add(pListener, "");
}

void CDatagramSocket::UnregisterListener(IDatagramListener* pListener)
{
	m_listeners.Remove(pListener);
}

void CDatagramSocket::OnPacket(const TNetAddress& addr, u8k* pData, u32 length)
{
	for (TListeners::Notifier notifier(m_listeners); notifier.IsValid(); notifier.Next())
	{
		notifier->OnPacket(addr, pData, length);
	}
}

void CDatagramSocket::OnError(const TNetAddress& addr, ESocketError error)
{
	for (TListeners::Notifier notifier(m_listeners); notifier.IsValid(); notifier.Next())
	{
		notifier->OnError(addr, error);
	}
}

struct SOpenSocketVisitor
{
private:
	u32 flags;

	template<size_t I = 0>
	IDatagramSocketPtr CreateFromVariant(const TNetAddress& var)
	{
		if (var.index() == I)
		{
			return Create(stl::get<I>(var));
		}
		else
		{
			return CreateFromVariant<I + 1>(var);
		}
	}

public:
	SOpenSocketVisitor(u32 _flags)
		: flags(_flags)
	{}

	template<class T>
	IDatagramSocketPtr CreateUDP(const T& addr)
	{
		CUDPDatagramSocket* pSocket = new CUDPDatagramSocket();
		if (!pSocket->Init(addr, flags))
		{
			delete pSocket;
			return NULL;
		}
		return pSocket;
	}

	IDatagramSocketPtr Create(const TNetAddress& var)
	{
		return CreateFromVariant(var);
	}

	IDatagramSocketPtr Create(TLocalNetAddress addr)
	{
		CLocalDatagramSocket* pSocket = new CLocalDatagramSocket();
		if (!pSocket->Init(addr))
		{
			delete pSocket;
			return NULL;
		}
		return pSocket;
	}

	IDatagramSocketPtr Create(const LobbyIdAddr& addr)
	{
		return new CNullSocket;
	}

#if USE_SOCKADDR_STORAGE_ADDR
	IDatagramSocketPtr Create(const SSockAddrStorageAddr& addr)
	{
		return new CNullSocket;
	}
#endif

	IDatagramSocketPtr Create(const SIPv4Addr& addr)
	{
		IDatagramSocketPtr pLocal = Create(addr.port);
		if (!pLocal)
			return NULL;
		IDatagramSocketPtr pNormal = CreateUDP(addr);
		if (!pNormal)
			return NULL;

		CCompositeDatagramSocket* pSocket = new CCompositeDatagramSocket();
		pSocket->AddChild(pLocal);
		pSocket->AddChild(pNormal);
		return pSocket;
	}

	IDatagramSocketPtr Create(SNullAddr)
	{
		return new CNullSocket;
	}

	template<class T>
	IDatagramSocketPtr operator()(const T& type)
	{
		return Create(type);
	}
};
template<>
IDatagramSocketPtr SOpenSocketVisitor::CreateFromVariant<stl::variant_size<TNetAddress>::value>(const TNetAddress& var)
{
	DRX_ASSERT_MESSAGE(false, "Invalid variant index.");
	return nullptr;
}

IDatagramSocketPtr OpenSocket(const TNetAddress& addr, u32 flags)
{
	SOpenSocketVisitor v(flags);
	IDatagramSocketPtr pSocket = stl::visit(v, addr);
#if INTERNET_SIMULATOR
	if (pSocket && stl::get_if<SIPv4Addr>(&addr))
		pSocket = new CInternetSimulatorSocket(pSocket);
#endif

	return pSocket;
}
