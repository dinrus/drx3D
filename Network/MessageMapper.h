// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  Message definition to id management
   -------------------------------------------------------------------------
   История:
   - 26/07/2004   : Created by Craig Tiller
*************************************************************************/
#ifndef MESSAGE_MAPPER_H
#define MESSAGE_MAPPER_H

#include <drx3D/Network/INetwork.h>
#include <drx3D/CoreX/Containers/VectorMap.h>

//! this class maps SNetMessageDef pointers to message-id's and back again
class CMessageMapper
#if ENABLE_PACKET_PREDICTION
	: public IMessageMapper
#endif
{
public:
	CMessageMapper();
	virtual ~CMessageMapper() {};

	void                  Reset(size_t nReservedMessages, const SNetProtocolDef* pProtocols, size_t nProtocols);

	virtual u32        GetMsgId(const SNetMessageDef*) const;
	const SNetMessageDef* GetDispatchInfo(u32 id, size_t* pnSubProtocol);
	u32                GetNumberOfMsgIds() const;

	bool                  LookupMessage(tukk name, SNetMessageDef const** pDef, size_t* pnSubProtocol);

	void                  GetMemoryStatistics(IDrxSizer* pSizer)
	{
		SIZER_COMPONENT_NAME(pSizer, "CMessageMapper");

		pSizer->Add(*this);

		pSizer->AddContainer(m_messages);
		pSizer->AddContainer(m_IdMap);
		pSizer->AddContainer(m_DefMap);
		pSizer->AddContainer(m_nameToDef);
	}

private:
	struct SSinkMessages
	{
		u32                nStart;
		u32                nEnd;
		const SNetMessageDef* pFirst;
		const SNetMessageDef* pLast;
	};
	typedef std::vector<SSinkMessages, stl::STLGlobalAllocator<SSinkMessages>> TMsgVec;
	TMsgVec m_messages;

	typedef VectorMap<const SNetMessageDef*, u32, std::less<const SNetMessageDef*>, stl::STLGlobalAllocator<std::pair<const SNetMessageDef*, u32>>> TIdMap;
	typedef std::vector<const SNetMessageDef*, stl::STLGlobalAllocator<const SNetMessageDef*>>                                                            TDefMap;
	TIdMap  m_IdMap;
	TDefMap m_DefMap;
	u32  m_DefMapBias;

	struct LTStr
	{
		bool operator()(tukk a, tukk b) const
		{
			return strcmp(a, b) < 0;
		}
	};
	std::map<tukk , const SNetMessageDef*, LTStr, stl::STLGlobalAllocator<std::pair<tukk const, const SNetMessageDef*>>> m_nameToDef;
};

#endif
