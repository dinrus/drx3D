// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __ILOADINGMESSAGEPROVIDER_H__
#define __ILOADINGMESSAGEPROVIDER_H__

class CLoadingMessageProviderListNode;

class ILoadingMessageProvider
{
	public:
	virtual	~ILoadingMessageProvider(){}
	ILoadingMessageProvider(CLoadingMessageProviderListNode * node);
	virtual i32 GetNumMessagesProvided() const = 0;
	virtual string GetMessageNum(i32 n) const = 0;
};

class CLoadingMessageProviderListNode
{
	private:
	const ILoadingMessageProvider * m_messageProvider;
	static CLoadingMessageProviderListNode * s_first;
	static CLoadingMessageProviderListNode * s_last;
	CLoadingMessageProviderListNode * m_next;
	CLoadingMessageProviderListNode * m_prev;

	public:
	void Init(const ILoadingMessageProvider * messageProvider);
	~CLoadingMessageProviderListNode();
	static string GetRandomMessage();
	static void ListAll();
};

#endif // __ILOADINGMESSAGEPROVIDER_H__
