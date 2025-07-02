// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/CompositeDatagramSocket.h>

CCompositeDatagramSocket::CCompositeDatagramSocket()
{
}

CCompositeDatagramSocket::~CCompositeDatagramSocket()
{
}

void CCompositeDatagramSocket::AddChild(IDatagramSocketPtr child)
{
	stl::push_back_unique(m_children, child);
}

void CCompositeDatagramSocket::GetSocketAddresses(TNetAddressVec& addrs)
{
	for (ChildVecIter iter = m_children.begin(); iter != m_children.end(); ++iter)
		(*iter)->GetSocketAddresses(addrs);
}

ESocketError CCompositeDatagramSocket::Send(u8k* pBuffer, size_t nLength, const TNetAddress& to)
{
	ESocketError err = eSE_Ok;
	for (ChildVecIter iter = m_children.begin(); iter != m_children.end(); ++iter)
	{
		ESocketError childErr = (*iter)->Send(pBuffer, nLength, to);
		if (childErr > err)
			err = childErr;
	}
	return err;
}

ESocketError CCompositeDatagramSocket::SendVoice(u8k* pBuffer, size_t nLength, const TNetAddress& to)
{
	ESocketError err = eSE_Ok;
	for (ChildVecIter iter = m_children.begin(); iter != m_children.end(); ++iter)
	{
		ESocketError childErr = (*iter)->SendVoice(pBuffer, nLength, to);
		if (childErr > err)
			err = childErr;
	}
	return err;
}

void CCompositeDatagramSocket::RegisterListener(IDatagramListener* pListener)
{
	for (ChildVecIter iter = m_children.begin(); iter != m_children.end(); ++iter)
	{
		(*iter)->RegisterListener(pListener);
	}
}

void CCompositeDatagramSocket::UnregisterListener(IDatagramListener* pListener)
{
	for (ChildVecIter iter = m_children.begin(); iter != m_children.end(); ++iter)
	{
		(*iter)->UnregisterListener(pListener);
	}
}

void CCompositeDatagramSocket::Die()
{
	for (ChildVecIter iter = m_children.begin(); iter != m_children.end(); ++iter)
		(*iter)->Die();
}

bool CCompositeDatagramSocket::IsDead()
{
	for (ChildVecIter iter = m_children.begin(); iter != m_children.end(); ++iter)
		if ((*iter)->IsDead())
			return true;
	return false;
}

DRXSOCKET CCompositeDatagramSocket::GetSysSocket()
{
	for (ChildVecIter iter = m_children.begin(); iter != m_children.end(); ++iter)
		if ((*iter)->GetSysSocket() != DRX_INVALID_SOCKET)
			return (*iter)->GetSysSocket();
	return DRX_INVALID_SOCKET;
}

void CCompositeDatagramSocket::RegisterBackoffAddress(const TNetAddress& addr)
{
	for (ChildVecIter iter = m_children.begin(); iter != m_children.end(); ++iter)
		(*iter)->RegisterBackoffAddress(addr);
}

void CCompositeDatagramSocket::UnregisterBackoffAddress(const TNetAddress& addr)
{
	for (ChildVecIter iter = m_children.begin(); iter != m_children.end(); ++iter)
		(*iter)->UnregisterBackoffAddress(addr);
}
