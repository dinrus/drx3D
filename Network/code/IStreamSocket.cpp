// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/Network.h>
#include  <drx3D/Network/TCPStreamSocket.h>

struct SCreateStreamSocketVisitor
{
	IStreamSocketPtr pStreamSocket;

	template<class T>
	void operator()(const T& type)
	{
		Visit(type);
	}

	template<typename T>
	ILINE void Visit(const T&)
	{
		NetWarning("unsupported address type for CreateStreamSocket");
	}

	void Visit(const TNetAddress& var)
	{
		VisitVariant(var);
	}

	void Visit(const SIPv4Addr&)
	{
		CTCPStreamSocket* pSocket = new CTCPStreamSocket();
		if (!pSocket->Init())
		{
			delete pSocket;
			pSocket = NULL;
		}
		pStreamSocket = pSocket;
	}

private:
	template<size_t I = 0>
	void VisitVariant(const TNetAddress& var)
	{
		if (var.index() == I)
		{
			Visit(stl::get<I>(var));
		}
		else
		{
			VisitVariant<I + 1>(var);
		}
	}
};
template<>
void SCreateStreamSocketVisitor::VisitVariant<stl::variant_size<TNetAddress>::value>(const TNetAddress& var)
{
	DRX_ASSERT_MESSAGE(false, "Invalid variant index.");
}

IStreamSocketPtr CreateStreamSocket(const TNetAddress& addr)
{
	SCreateStreamSocketVisitor visitor;
	stl::visit(visitor, addr);
	return visitor.pStreamSocket;
}
