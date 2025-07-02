// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/UpdateAspectDataContext.h>
#include  <drx3D/Network/NetContext.h>

CUpdateAspectDataContext::CUpdateAspectDataContext(CNetContext* pNetContext, SContextObject& obj, SContextObjectEx& objx, CTimeValue localPhysicsTime)
	: m_pNetContext(pNetContext)
	, m_obj(obj)
	, m_objx(objx)
	, m_ref(&obj, &objx)
	, m_localPhysicsTime(localPhysicsTime)
	, m_takenAspects(0)
	, m_fetchAspects(0)
{
	m_allowedAspects = 0;
	if (obj.bOwned)
		m_allowedAspects = objx.nAspectsEnabled;
	else if (obj.bControlled)
		m_allowedAspects = (m_pNetContext->DelegatableAspects() & objx.delegatableMask) & objx.nAspectsEnabled;
}

void CUpdateAspectDataContext::RequestFetchAspects(const NetworkAspectType fetchAspects)
{
	PrefetchLine(m_objx.vAspectData, 0);
	PrefetchLine(m_objx.vAspectData, 124);  //128 offset would potentially be off of the end of the array
	m_fetchAspects = fetchAspects & m_allowedAspects;

	const NetworkAspectType nAspectsEnabled = m_objx.nAspectsEnabled;
	const NetworkAspectType nOriginalFetchAspects = m_fetchAspects;
	const NetworkAspectType nOriginalAllowedAspects = m_allowedAspects;
	const NetworkAspectType nCombinedOriginalAspects = nOriginalAllowedAspects & nOriginalFetchAspects;
	NetworkAspectType nRunningShift = 1;

	for (NetworkAspectID i = 0; i < NumAspects; i++, nRunningShift = nRunningShift << 1)
	{
		const NetworkAspectType aspectBit = nRunningShift;

		bool enabled = (nAspectsEnabled & aspectBit) != 0;
		TMemHdl& rOldHdl = m_oldHdls[i];
		rOldHdl = CMementoMemoryUpr::InvalidHdl;
		if (enabled)
		{
			if (m_objx.vAspectData[i] == CMementoMemoryUpr::InvalidHdl)
			{
				// object is enabled, but we don't have data for it
				// so we'd better fetch it
				m_fetchAspects |= aspectBit;
			}
			else if (aspectBit & nCombinedOriginalAspects)
			{
				rOldHdl = m_objx.vAspectData[i];
				m_objx.vAspectData[i] = CMementoMemoryUpr::InvalidHdl;
			}
		}
		else if (m_objx.vAspectData[i] != CMementoMemoryUpr::InvalidHdl)
		{
			// object is disabled, but we have data for it...
			// drop that data
			rOldHdl = m_objx.vAspectData[i];
			m_objx.vAspectData[i] = CMementoMemoryUpr::InvalidHdl;
			m_fetchAspects &= ~aspectBit;
		}
	}
}

void CUpdateAspectDataContext::FetchDataFromGame(NetworkAspectID i)
{
	const NetworkAspectType aspectBit = 1 << i;

	CMementoStreamAllocator streamAllocatorForNewState(&MMM());
	// assume the size of data taken will be the same as last time, or 8 bytes if we've never fetched anything
	size_t sizeHint = 8;
	if (m_oldHdls[i] != CMementoMemoryUpr::InvalidHdl)
		sizeHint = MMM().GetHdlSize(m_oldHdls[i]);
	CByteOutputStream stm(&streamAllocatorForNewState, sizeHint);
	NET_ASSERT(m_objx.vAspectData[i] == CMementoMemoryUpr::InvalidHdl);
#if LOG_BUFFER_UPDATES
	//	if (CNetCVars::Get().LogBufferUpdates)
	//		NetLog("[buf] pull %s:%s", id.GetText(), m_pNetContext->GetAspectName(i));
#endif
	// timestamped aspects need the timestamp as the first part of the stored buffer
	if (m_pNetContext->TimestampedAspects() & aspectBit)
	{
		stm.PutTyped<CTimeValue>() = m_localPhysicsTime;
	}
	CCompressionUpr& cman = CNetwork::Get()->GetCompressionUpr();
	u8 readProfile = (m_obj.vAspectProfiles[i] >= MaxProfilesPerAspect ? 0 : m_obj.vAspectProfiles[i]);
	ChunkID readChunkID = GetChunkIDFromObject(m_ref, i);
	// actually fetch the data from game
	bool gameReadResult = cman.GameContextRead(m_pNetContext->GetGameContext(), &stm, m_obj.userID, aspectBit, readProfile, readChunkID);
	if (gameReadResult && TakeChange(i, aspectBit, stm, streamAllocatorForNewState))
	{
		m_objx.vAspectData[i] = streamAllocatorForNewState.GetHdl();
		MMM().ResizeHdl(m_objx.vAspectData[i], stm.GetSize());
		m_objx.vAspectDataVersion[i]++;
		m_takenAspects |= aspectBit;
	}
	else
	{
		MMM().FreeHdl(streamAllocatorForNewState.GetHdl());
		m_objx.vAspectData[i] = m_oldHdls[i];
		m_oldHdls[i] = CMementoMemoryUpr::InvalidHdl;
	}
}

CUpdateAspectDataContext::~CUpdateAspectDataContext()
{
	for (i32 i = 0; i < NumAspects; i++)
	{
		if (m_oldHdls[i] != CMementoMemoryUpr::InvalidHdl)
		{
			MMM().FreeHdl(m_oldHdls[i]);
		}
	}
}

bool CUpdateAspectDataContext::TakeChange(NetworkAspectID i, NetworkAspectType aspectBit, CByteOutputStream& stm, CMementoStreamAllocator& streamAllocatorForNewState)
{
	bool take = true;
	if (m_oldHdls[i] != CMementoMemoryUpr::InvalidHdl)
	{
		const size_t StreamSize = stm.GetSize();
		const size_t HandleSize = MMM().GetHdlSize(m_oldHdls[i]);

		//We obtain pOld and pNew here to prefetch. The (vast) majority of calls end up using them
		//	so this is worth it to mitigate L2 cache miss cost.
		tukk pOld = (tukk)MMM().PinHdl(m_oldHdls[i]);
		PrefetchLine(pOld, 0);
		tukk pNew = (tukk)MMM().PinHdl(streamAllocatorForNewState.GetHdl());
		PrefetchLine(pNew, 0);

		if (StreamSize == HandleSize)
		{
			i32 ofs = 0;
			if (m_pNetContext->TimestampedAspects() & aspectBit)
				ofs += sizeof(CTimeValue);
			if (StreamSize == ofs)
				take = false;
			else
			{
				i32 cmpSz = StreamSize - ofs;
				if (cmpSz > 0 && 0 == memcmp(pOld + ofs, pNew + ofs, cmpSz))
					take = false;
				else if (!cmpSz)
					take = false;
			}
		}
	}
	return take;
}
