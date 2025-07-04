// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Network/Config.h>

#if INTERNET_SIMULATOR

	#include <drx3D/Network/IDatagramSocket.h>
	#include <drx3D/Network/NetTimer.h>

class CInternetSimulatorSocket : public IDatagramSocket
{
public:
	CInternetSimulatorSocket(IDatagramSocketPtr pChild);
	~CInternetSimulatorSocket();

	// IDatagramSocket
	virtual void         RegisterListener(IDatagramListener* pListener) override;
	virtual void         UnregisterListener(IDatagramListener* pListener) override;
	virtual void         GetSocketAddresses(TNetAddressVec& addrs) override;
	virtual ESocketError Send(u8k* pBuffer, size_t nLength, const TNetAddress& to) override;
	virtual ESocketError SendVoice(u8k* pBuffer, size_t nLength, const TNetAddress& to) override;
	virtual void         Die() override;
	virtual bool         IsDead() override;
	virtual DRXSOCKET    GetSysSocket() override;
	virtual void         RegisterBackoffAddress(const TNetAddress& addr) override   { m_pChild->RegisterBackoffAddress(addr); }
	virtual void         UnregisterBackoffAddress(const TNetAddress& addr) override { m_pChild->UnregisterBackoffAddress(addr); }
	// ~IDatagramSocket

	enum EProfile
	{
		PROFILE_PERFECT       = -1,
		PROFILE_FATAL         = -2,
		PROFILE_RANDOM        = -3,

		DEFAULT_PROFILE_GREAT = 0,
		DEFAULT_PROFILE_GOOD,
		DEFAULT_PROFILE_TYPICAL,
		DEFAULT_PROFILE_IFFY,
		DEFAULT_PROFILE_BAD,
		DEFAULT_PROFILE_AWFUL,

		DEFAULT_PROFILE_MAX
	};

	static void LoadXMLProfiles(tukk pFileName);
	static void SetProfile(EProfile profile);

private:

	enum EPayloadType
	{
		ePT_Data,
		ePT_Voice
	};

	struct SPendingSend
	{
		EPayloadType                         eType;
		size_t                               nLength;
		u8*                               pBuffer;
		TNetAddress                          to;
		_smart_ptr<CInternetSimulatorSocket> pThis;
	};

	struct SProfileEntry
	{
		float fLossMin;
		float fLossMax;
		float fLagMin;
		float fLagMax;
	};

	IDatagramSocketPtr    m_pChild;

	static SProfileEntry* sm_pProfiles;
	static u32         sm_nProfiles;

	static SProfileEntry  sm_DefaultProfiles[DEFAULT_PROFILE_MAX];

	static void           SimulatorUpdate(NetTimerId, uk , CTimeValue);
};

#endif
