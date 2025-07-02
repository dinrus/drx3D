// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  declaration of DinrusXNetwork IArithModel
   -------------------------------------------------------------------------
   История:
   - 26/07/2004   10:34 : Created by Craig Tiller
*************************************************************************/
#ifndef ARITHMODEL_H
#define ARITHMODEL_H

#include <drx3D/Network/Config.h>

#include <drx3D/Network/INetwork.h>

#if USE_ARITHSTREAM
	#include <drx3D/Network/ArithAlphabet.h>
	#include <drx3D/Network/CommStream.h>
	#include <drx3D/Network/Utils.h>
	#include <drx3D/Network/StringTable.h>
#endif

enum ETimeStream
{
	eTS_Network = 0,
	eTS_NetworkPing,
	eTS_NetworkPong,
	eTS_PongElapsed,
	eTS_Physics,
	eTS_RemoteTime,
	// must be last
	NUM_TIME_STREAMS
};

class CCheckForNullEntityEncoding
{
public:
	CCheckForNullEntityEncoding() { m_check++; }
	~CCheckForNullEntityEncoding() { m_check--; }

	static void BreakpointHereToCheck()
	{
	}

	static void EncodedNull()
	{
		if (m_check)
			BreakpointHereToCheck();
	}

private:
	static i32 m_check;
};

class CNoNullEntityIDSendable : public INetSendable
{
public:
	CNoNullEntityIDSendable(INetSendablePtr pSendable) : INetSendable(pSendable->GetFlags(), pSendable->GetReliability()), m_pBase(pSendable)
	{
		++g_objcnt.noNullEntID;
	}

	~CNoNullEntityIDSendable()
	{
		--g_objcnt.noNullEntID;
	}

#if ENABLE_PACKET_PREDICTION
	virtual SMessageTag GetMessageTag(INetSender* pSender, IMessageMapper* mapper)
	{
		return m_pBase->GetMessageTag(pSender, mapper);
	}
#endif

	virtual size_t             GetSize() { return sizeof(*this) + m_pBase->GetSize(); }
	virtual EMessageSendResult Send(INetSender* pSender)
	{
		CCheckForNullEntityEncoding check;
		return m_pBase->Send(pSender);
	}
	virtual void UpdateState(u32 nFromSeq, ENetSendableStateUpdate update)
	{
		m_pBase->UpdateState(nFromSeq, update);
	}
	virtual tukk GetDescription()
	{
		if (m_name.empty())
			m_name = string("No-null-entid ") + m_pBase->GetDescription();
		return m_name.c_str();
	}
	virtual void GetPositionInfo(SMessagePositionInfo& pos)
	{
		m_pBase->GetPositionInfo(pos);
	}

private:
	INetSendablePtr m_pBase;
	string          m_name;
};

inline INetSendablePtr CheckNullEntityWrites(INetSendablePtr pSendable)
{
	return new CNoNullEntityIDSendable(pSendable);
}

#if USE_ARITHSTREAM

class CNetContextState;

// implementation of IArithModel for the network engine
class CArithModel
{
private:
	struct STimeAdaption
	{
		STimeAdaption()
		{
			timeFraction32 = 0;
		}

		CTimeValue startTime;
		CTimeValue lastTime;

		struct STimeDelta
		{
			STimeDelta() { Reset(); }
			STimeDelta(const STimeDelta& other) : rate(other.rate), error(other.error) {}
			STimeDelta& operator=(const STimeDelta& other) { rate = other.rate; error = other.error; return *this; }
			u32 rate;
			u32 error;

			u16 Left() const
			{
				if (rate > error)
					return rate - error;
				else
					return 0;
			}
			u16 Right() const
			{
				u32 sum = u32(error) + rate;
				if (sum > 65535)
					return 65535;
				else
					return u16(sum);
			}
			void Update(u16 encodedDelta, bool hit);
			void Reset();
		};
		STimeDelta startDelta;
		STimeDelta frameDelta;

		u32     timeFraction32;
		bool       isInFrame;
	};

public:
	CArithModel();
	~CArithModel();
	CArithModel(const CArithModel&);
	CArithModel&      operator=(const CArithModel&);

	size_t            GetSize();

	void              SetTimeFraction(u32 timeFraction32);
	u32            GetTimeFraction() const;

	void              SetNetContextState(CNetContextState* pContext) { m_pNetContext = pContext; }
	CNetContextState* GetNetContextState()                           { return m_pNetContext; }

	void              WriteString(CCommOutputStream&, const string& szString);
	void              WriteChar(CCommOutputStream&, char c);
	bool              ReadString(CCommInputStream&, string& szString);
	char              ReadChar(CCommInputStream&);
	float             EstimateSizeOfString(const string& szString) const;
	float             EstimateSizeOfChar(char c) const;

	ILINE void        TableWriteString(CCommOutputStream& out, const string& szString)
	{
		m_stringTable.WriteString(out, szString, this);
	}
	ILINE void TableReadString(CCommInputStream& in, string& szString)
	{
		m_stringTable.ReadString(in, szString, this);
	}

	bool         IsTimeInFrame(ETimeStream time) const;
	int64        WriteGetTimeDelta(ETimeStream time, const CTimeValue& value, STimeAdaption::STimeDelta** pOutDeltaInfo = 0);
	CTimeValue   ReadTimeWithDelta(CCommInputStream& stm, ETimeStream time, int64 delta);

	void         WriteTime(CCommOutputStream&, ETimeStream stream, CTimeValue timeValue);
	CTimeValue   ReadTime(CCommInputStream&, ETimeStream stream);
	float        EstimateSizeOfTime(ETimeStream stream, CTimeValue timeValue) const;

	void         WriteNetId(CCommOutputStream&, SNetObjectID id);
	SNetObjectID ReadNetId(CCommInputStream&);

	void         GetMemoryStatistics(IDrxSizer* pSizer, bool countingThis = false);
	void         RecalculateProbabilities();

	void         SimplifyMemoryUsing(const CArithModel& mdl)
	{
		m_alphabet = mdl.m_alphabet;
		if (m_entityIDAlphabet.GetNumSymbols() == mdl.m_entityIDAlphabet.GetNumSymbols())
			m_entityIDAlphabet = mdl.m_entityIDAlphabet;
	}

	#if DEBUG_ENDPOINT_LOGIC
	void DumpCountsToFile(FILE* f) const { m_alphabet.DumpCountsToFile(f); }
	#endif
private:
	CNetContextState* m_pNetContext;

	typedef CArithLargeAlphabetOrder0<> TAlphabet;
	TAlphabet                   m_alphabet;
	CArithLargeAlphabetOrder0<> m_entityIDAlphabet;

	CStringTable                m_stringTable;

	// stores parameters for the time stream adaption

	u32        m_timeFraction32;
	STimeAdaption m_vTimeAdaption[NUM_TIME_STREAMS];
};

#endif

#endif
