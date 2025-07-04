// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include <drx3D/Network/INetwork.h>
#include  <drx3D/Network/ContextViewStateUpr.h>
#include  <drx3D/Network/NetCVars.h>

CContextViewStateUpr::CContextViewStateUpr()
{
	m_remoteState = eCVS_Initial;
	m_localState = eCVS_Initial;
	m_remoteFinished = true;
	m_localStateState = eLSS_Set;
	m_locks = 0;
}

ILINE void CContextViewStateUpr::Dump(tukk s)
{
	//	NetQuickLog( false, 0.1f, "[cvsm] %s %p   local:%d remote:%d finished:%d statestate:%d locks:%d",
	//		s, this, m_localState, m_remoteState, m_remoteFinished, m_localStateState, m_locks );
}

void CContextViewStateUpr::DumpLockers(tukk name)
{
#if !DRXNETWORK_RELEASEBUILD
	if (!IsInState(eCVS_InGame))
		NetQuickLog(false, 0, "[cvsm] %s: debug code %d", name, GetStateDebugCode());
	for (std::map<string, i32>::iterator it = m_lockers.begin(); it != m_lockers.end(); ++it)
	{
		if (it->second)
			NetQuickLog(false, 0, "[cvsm] %s: locker %s has %d", name, it->first.c_str(), it->second);
	}
#endif
}

void CContextViewStateUpr::SetLocalState(EContextViewState state)
{
	Dump("Enter_SetLocalState");
	Verify(m_localStateState == eLSS_FinishedAcked, "Not in eLSS_FinishedAcked state calling SetLocalState");
	VerifyStateTransition(m_localState, state);
	m_localState = state;
	m_localStateState = eLSS_Set;

	EContextViewState temp;
	if (ShouldSendLocalState(temp, true))
		OnNeedToSendStateInformation(m_locks == 0);
	Dump("Exit_SetLocalState");
}

bool CContextViewStateUpr::ShouldSendLocalState(EContextViewState& state, bool ignoreLocks)
{
	Dump("ShouldSendLocalState");
	if (m_localStateState == eLSS_Set && (ignoreLocks || m_locks == 0))
	{
		state = m_localState;
		return true;
	}
	return false;
}

void CContextViewStateUpr::SendLocalState()
{
	Dump("Enter_SendLocalState");
	Verify(m_localStateState == eLSS_Set, "Not in state eLSS_Set calling SendLocalState");
	m_localStateState = eLSS_Sent;
	Dump("Exit_SendLocalState");
}

bool CContextViewStateUpr::AckLocalState(bool bAck)
{
	Dump("Enter_AckLocalState");
	Verify(m_localStateState == eLSS_Sent, "Not in state eLSS_Sent calling AckLocalState");
	bool ok = true;
	if (bAck)
	{
		m_localStateState = eLSS_Acked;
		ok = MaybeEnterState();
	}
	else
		m_localStateState = eLSS_Set;
	Dump("Exit_AckLocalState");
	return ok;
}

void CContextViewStateUpr::FinishLocalState()
{
	Dump("Enter_FinishLocalState");
	Verify(m_localStateState == eLSS_Acked, "Not in state eLSS_Acked calling AckLocalState");
	m_localStateState = eLSS_FinishedSet;

	if (ShouldSendFinishLocalState(true))
		OnNeedToSendStateInformation(m_locks == 0);
	Dump("Exit_FinishLocalState");
}

bool CContextViewStateUpr::ShouldSendFinishLocalState(bool ignoreLocks)
{
	Dump("ShouldSendFinishLocalState");
	return m_localStateState == eLSS_FinishedSet && (ignoreLocks || m_locks == 0);
}

void CContextViewStateUpr::SendFinishLocalState()
{
	Dump("Enter_SendFinishLocalState");
	Verify(m_localStateState == eLSS_FinishedSet, "Not in state eLSS_FinishedSet calling SendFinishLocalState");
	m_localStateState = eLSS_FinishedSent;
	Dump("Exit_SendFinishLocalState");
}

void CContextViewStateUpr::AckFinishLocalState(bool bAck)
{
	Dump("Enter_AckFinishLocalState");
	Verify(m_localStateState == eLSS_FinishedSent, "Not in state eLSS_FinishSent calling AckFinishLocalState");
	if (bAck)
	{
		m_localStateState = eLSS_FinishedAcked;
		MaybeExitState();
	}
	else
	{
		m_localStateState = eLSS_FinishedSet;
	}
	Dump("Exit_AckFinishLocalState");
}

bool CContextViewStateUpr::SetRemoteState(EContextViewState state)
{
	Dump("Enter_SetRemoteState");
	if (!m_remoteFinished)
	{
		NetWarning("SetRemoteState: Not finished remote state %d", m_remoteState);
		return false;
	}
	if (!ValidateStateTransition(m_remoteState, state))
	{
		NetWarning("SetRemoteState: Invalid state transition: %d -> %d", m_remoteState, state);
		return false;
	}
	m_remoteState = state;
	m_remoteFinished = false;
	Dump("Exit_SetRemoteState");
	return MaybeEnterState();
}

bool CContextViewStateUpr::FinishRemoteState()
{
	Dump("Enter_FinishRemoteState");
	if (m_remoteFinished)
		return false;
	m_remoteFinished = true;
	MaybeExitState();
	Dump("Exit_FinishRemoteState");
	return true;
}

void CContextViewStateUpr::Verify(bool condition, tukk message)
{
	NET_ASSERT(condition);
	if (!condition)
		OnViewStateDisconnect(message);
}

bool CContextViewStateUpr::ValidateStateTransition(EContextViewState from, EContextViewState to)
{
	if (to == eCVS_Initial)
		return true;
	return i32(to) == i32(from) + 1;
}

bool CContextViewStateUpr::MaybeEnterState()
{
	if (m_localState == m_remoteState)
		if (m_localStateState == eLSS_Acked && !m_remoteFinished)
			return EnterState(m_localState);
	return true;
}

void CContextViewStateUpr::MaybeExitState()
{
	if (m_localState == m_remoteState)
	{
		if (m_localStateState == eLSS_FinishedAcked && m_remoteFinished)
		{
			ExitState(m_localState);
			// if we exit a state we *MUST* start entering a new state
			Verify(m_localState != m_remoteState, "Invalid state from MaybeExitState");
		}
	}
}
