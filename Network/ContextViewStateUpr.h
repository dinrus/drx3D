// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CONTEXTVIEWSTATEMANAGER_H__
#define __CONTEXTVIEWSTATEMANAGER_H__

#include <drx3D/Network/Config.h>

class CContextViewStateUpr
{
public:
	virtual ~CContextViewStateUpr(){}
	CContextViewStateUpr();

	void LockStateChanges(tukk whom)
	{
#if !DRXNETWORK_RELEASEBUILD
		m_lockers[whom]++;
#endif
		m_locks++;
		NET_ASSERT(m_locks);
	}
	void UnlockStateChanges(tukk whom)
	{
#if !DRXNETWORK_RELEASEBUILD
		NET_ASSERT(m_lockers[whom]);
		m_lockers[whom]--;
#endif
		NET_ASSERT(m_locks);
		m_locks--;
		if (!m_locks && HasPendingStateChange(false))
			OnNeedToSendStateInformation(true);
	}

	bool IsInState(EContextViewState state) const
	{
		return
		  m_remoteState == state &&
		  m_remoteFinished == false &&
		  m_localState == state &&
		  (m_localStateState == eLSS_Acked || (m_localStateState == eLSS_FinishedSet && m_locks));
	}
	bool IsPastOrInState(EContextViewState state) const
	{
		if (m_remoteState < state)
			return false;
		if (m_localState == state)
			return m_localStateState >= eLSS_Acked;
		return m_localState > state;
	}
	bool IsBeforeState(EContextViewState state) const
	{
		return m_localState < state && m_remoteState < state;
	}

	// should be protected, but needed by some of the async callbacks in CNetContext
	void FinishLocalState();

	i32  GetStateDebugCode()
	{
		i32 local = 1 + i32(m_localState) - i32(eCVS_Initial);
		i32 lss = 1 + i32(m_localStateState) - i32(eLSS_Set);
		i32 remote = 1 + i32(m_remoteState) - i32(eCVS_Initial);
		return 1000 * remote + 100 * local + 10 * CLAMP(m_locks, 0, 9) + lss;
	}

	void GetMemoryStatistics(IDrxSizer* pSizer)
	{
		SIZER_COMPONENT_NAME(pSizer, "CContextViewStateUpr");

		pSizer->Add(*this);
	}

protected:
	void         SetLocalState(EContextViewState state);
	bool         ShouldSendLocalState(EContextViewState& state, bool ignoreLocks);
	void         SendLocalState();
	bool         AckLocalState(bool bAck);
	bool         ShouldSendFinishLocalState(bool ignoreLocks);
	void         SendFinishLocalState();
	void         AckFinishLocalState(bool bAck);

	bool         SetRemoteState(EContextViewState state);
	bool         FinishRemoteState();

	virtual bool EnterState(EContextViewState state) = 0;
	virtual void ExitState(EContextViewState state) = 0;
	virtual void OnNeedToSendStateInformation(bool urgent) = 0;
	virtual void OnViewStateDisconnect(tukk message) = 0;

	bool         HasPendingStateChange(bool ignoreLocks)
	{
		EContextViewState notUsed;
		return ShouldSendFinishLocalState(ignoreLocks) || ShouldSendLocalState(notUsed, ignoreLocks);
	}

	void DumpLockers(tukk name);

private:
	enum ELocalStateState
	{
		eLSS_Set,
		eLSS_Sent,
		eLSS_Acked,
		eLSS_FinishedSet,
		eLSS_FinishedSent,
		eLSS_FinishedAcked
	};

	EContextViewState m_localState;
	EContextViewState m_remoteState;
	bool              m_remoteFinished;
	ELocalStateState  m_localStateState;
	i32               m_locks;

	bool MaybeEnterState();
	void MaybeExitState();

	void Verify(bool condition, tukk message);
	void VerifyStateTransition(EContextViewState from, EContextViewState to) { Verify(ValidateStateTransition(from, to), "Invalid state transition"); }
	bool ValidateStateTransition(EContextViewState from, EContextViewState to);
	void Dump(tukk msg);
#if !DRXNETWORK_RELEASEBUILD
	std::map<string, i32> m_lockers;
#endif
};

class CChangeStateLock
{
public:
	CChangeStateLock() : m_pContext(0), m_name("BAD") {}
	CChangeStateLock(CContextViewStateUpr* pContext, tukk name) : m_pContext(pContext), m_name(name)
	{
		m_pContext->LockStateChanges(m_name);
	}
	CChangeStateLock(const CChangeStateLock& lk) : m_pContext(lk.m_pContext), m_name(lk.m_name)
	{
		if (m_pContext)
			m_pContext->LockStateChanges(m_name);
	}
	~CChangeStateLock()
	{
		if (m_pContext)
			m_pContext->UnlockStateChanges(m_name);
	}
	void Swap(CChangeStateLock& lk)
	{
		std::swap(m_pContext, lk.m_pContext);
		std::swap(m_name, lk.m_name);
	}
	CChangeStateLock& operator=(CChangeStateLock lk)
	{
		Swap(lk);
		return *this;
	}
	bool IsLocking() const
	{
		return m_pContext != 0;
	}

private:
	CContextViewStateUpr* m_pContext;
	string                    m_name;
};

#endif
