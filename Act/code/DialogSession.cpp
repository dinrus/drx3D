// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   DialogSession.cpp
//  Version:     v1.00
//  Created:     07/07/2006 by AlexL
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: Dialog Session
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/DialogSession.h>
#include <drx3D/Act/DialogSystem.h>
#include <drx3D/Act/DialogScript.h>
#include <drx3D/Act/DialogActorContext.h>
#include <drx3D/Act/DialogCommon.h>
#include <drx3D/AI/ICommunicationUpr.h>

static const float END_GRACE_TIMEOUT = 60.0f;
static const CDialogSession::TActorFlags DEFAULT_ACTOR_FLAGS = CDialogSession::eDACF_Default; // CDialogSession::eDACF_NoAbortSound;

////////////////////////////////////////////////////////////////////////////
CDialogSession::CDialogSession(CDialogSystem* pDS, const CDialogScript* pScript, CDialogSystem::SessionID id)
{
	DiaLOG::Log(DiaLOG::eAlways, "CDialogSession::CDialogSession: this=%p SID=%d Script=%s ", this, id, pScript->GetID().c_str());
	assert(pDS != 0);
	assert(pScript != 0);
	m_pEntitySystem = gEnv->pEntitySystem;
	m_pDS = pDS;
	m_pScript = pScript;
	m_sessionID = id;
	m_curTime = 0.0f;
	m_nextTimeDelay = 0.0f;
	m_endGraceTimeOut = END_GRACE_TIMEOUT;
	m_curScriptLine = -1;
	m_nextScriptLine = -1;
	m_bPlaying = false;
	m_bValidated = false;
	m_bOK = false;
	m_bHaveSchedule = false;
	m_bAutoDelete = false;
	m_bReachedEnd = false;
	m_pendingActors = 0;
	m_playerAwareAngle = 0.0f;
	m_playerAwareDistance = 0.0f;
	m_playerAwareGraceTime = 3.0f;

	for (i32 i = 0; i < CDialogScript::MAX_ACTORS; ++i)
		m_actorFlags[i] = DEFAULT_ACTOR_FLAGS;
	m_aiBehaviourMode = eDIB_InterruptAlways;
	m_debugName.Format("SID=%d", m_sessionID, this);
	m_alertnessInterruptMode = Alert;
}

////////////////////////////////////////////////////////////////////////////
CDialogSession::~CDialogSession()
{
	m_actorContextMap.clear(); // not necessary, but be a bit more explicit
	DiaLOG::Log(DiaLOG::eAlways, "[DIALOG] CDialogSession::~CDialogSession: %s", GetDebugName());
}

////////////////////////////////////////////////////////////////////////////
void CDialogSession::Release()
{
	DoStop(); // force stopping [most important thing is un-precaching sounds]
	NotifyListeners(eDSE_SessionDeleted);
	delete this;
}

////////////////////////////////////////////////////////////////////////////
bool CDialogSession::AddListener(IDialogSessionListener* pListener)
{
	return stl::push_back_unique(m_listenerVec, pListener);
}

////////////////////////////////////////////////////////////////////////////
bool CDialogSession::RemoveListener(IDialogSessionListener* pListener)
{
	return stl::find_and_erase(m_listenerVec, pListener);
}

////////////////////////////////////////////////////////////////////////////
void CDialogSession::NotifyListeners(EDialogSessionEvent event)
{
	DiaLOG::Log(DiaLOG::eAlways, "[DIALOG} CDialogSession: %s Notifying listeners on Event %s", GetDebugName(), GetEventName(event));

	m_listenerVecTemp.reserve(m_listenerVec.size());
	m_listenerVecTemp.resize(0);
	m_listenerVecTemp.insert(m_listenerVecTemp.end(), m_listenerVec.begin(), m_listenerVec.end());

	// it's safe to remove oneself while being called back
	for (i32 i = (i32)m_listenerVecTemp.size() - 1; i >= 0; i--)
	{
		m_listenerVecTemp[i]->SessionEvent(this, event);
	}
}

////////////////////////////////////////////////////////////////////////////
bool CDialogSession::Play(i32 fromScriptLine)
{
	return InternalPlay(fromScriptLine, true);
}

////////////////////////////////////////////////////////////////////////////
bool CDialogSession::InternalPlay(i32 fromScriptLine, bool bNotify)
{
	if (!m_bValidated)
	{
		m_bOK = Validate();
	}

	if (!m_bOK)
	{
		GameWarning("[DIALOG] CDialogSession::Play: %s Parameters for Script '%s' not valid.", GetDebugName(), m_pScript->GetID().c_str());
		return false;
	}

	if (m_bPlaying)
		return false;

	// fromScriptLine == m_pScript->GetNumLines()    is a valid situation: sometimes the dialog reachs the last line, but the dialogactorcontext is not finished so the dialog is not "done" yet.
	// if a saveload happens in that situation and we return "false" here after loading the saveload, the dialog will never notify as "done".
	// when fromScriptLine == m_pScript->GetNumLines() and we call this function, the dialog will just gracefully finish in the next DoPlay() call.
	if (fromScriptLine < 0 || fromScriptLine > m_pScript->GetNumLines())
	{
		GameWarning("[DIALOG] CDialogSession::Play: %s FromScriptLine %d exceeds number of lines [0..%d] of Script '%s'.",
		            GetDebugName(), fromScriptLine, m_pScript->GetNumLines() - 1, m_pScript->GetID().c_str());
		return false;
	}

	m_pDS->AddSession(this);
	m_bPlaying = true;
	m_bReachedEnd = true;     // will be set to false by DoPlay
	m_bFirstUpdate = true;
	m_curTime = 0.0f;
	m_endGraceTimeOut = END_GRACE_TIMEOUT;
	m_curScriptLine = -1;
	m_nextScriptLine = fromScriptLine - 1;
	m_abortReason = eAR_None;
	m_pendingActors = 0;
	ScheduleNextLine(0.0f);

	if (bNotify)
		NotifyListeners(eDSE_SessionStart);

	//Restrict communication manager from using these actors
	{
		ICommunicationUpr* pCommunicationUpr = gEnv->pAISystem->GetCommunicationUpr();

		TIdToEntityMap::iterator iter = m_idToEntityMap.begin();
		while (iter != m_idToEntityMap.end())
		{
			EntityId actorId = (*iter).second;
			pCommunicationUpr->AddActorRestriction(actorId, true, true);
			++iter;
		}
	}

	{
		TActorContextMap::iterator iter = m_actorContextMap.begin();
		while (iter != m_actorContextMap.end())
		{
			(*iter).second->BeginSession();
			++iter;
		}
	}

	// AlexL 20/08/2007: don't update here
	// because other subsystems (FG) might process the BeginSession command
	// but get the abortion in the same frame.
	// to fix this, we delay the real playback of the first line to the next frame
	// so other subsystems get a chance to process it
	// DoPlay(0.0f);
	return true;
}

////////////////////////////////////////////////////////////////////////////
i32 CDialogSession::ScheduleNextLine(float dt)
{
	m_bHaveSchedule = true;
	m_nextTimeDelay = dt;
	++m_nextScriptLine;

	if (m_nextScriptLine < m_pScript->GetNumLines())
	{
		DiaLOG::Log(DiaLOG::eAlways, "[DIALOG] CDialogSession: %s Scheduling next line %d/%d [cur=%d] at %f",
		            GetDebugName(), m_nextScriptLine, m_pScript->GetNumLines() - 1, m_curScriptLine, m_curTime + m_nextTimeDelay);
	}
	else
	{
		DiaLOG::Log(DiaLOG::eAlways, "[DIALOG] CDialogSession: %s Scheduling END [cur=%d] at %f",
		            GetDebugName(), m_curScriptLine, m_curTime + m_nextTimeDelay);
	}
	return m_nextScriptLine;
}

////////////////////////////////////////////////////////////////////////////
void CDialogSession::DoPlay(float dt)
{
	if (m_bFirstUpdate == false)
		m_curTime += dt;
	else
		m_bFirstUpdate = false;

	// delayed aborting
	if (m_abortReason != eAR_None)
	{
		TActorContextMap::iterator iter = m_actorContextMap.begin();
		while (iter != m_actorContextMap.end())
		{
			CDialogActorContextPtr pContext = (*iter).second;
			if (pContext->IsStillPlaying())
			{
				m_endGraceTimeOut -= dt;
				if (m_endGraceTimeOut >= 0.0f)
					return;
				else
					break;
			}
			++iter;
		}
		DoStop();
		NotifyListeners(eDSE_Aborted);
		return;
	}

	bool bContinue = true;
	if (m_bHaveSchedule)
	{
		m_nextTimeDelay -= dt;
		if (m_nextTimeDelay <= 0.0f)
		{
			m_bHaveSchedule = false;
			m_curScriptLine = m_nextScriptLine;
			if (m_curScriptLine < m_pScript->GetNumLines())
			{
				const CDialogScript::SScriptLine* pLine = m_pScript->GetLine(m_curScriptLine);
				if (pLine)
				{
					m_bReachedEnd = false;
					DiaLOG::Log(DiaLOG::eAlways, "[DIALOG] CDialogSession: %s Playing new line %d: now=%f", GetDebugName(), m_curScriptLine, m_curTime);
					bool ok = PlayLine(pLine);
					bContinue = ok; // PlayLine can vote for immediate stop by returning false!
					NotifyListeners(eDSE_LineStarted);
				}
			}
			else
			{
				// we're past end
				m_bReachedEnd = true;
			}
		}
	}

	i32 nStillPlaying = 0;
	i32 nSoundWantContinue = 0;

	// update all contexts
	CDialogScript::SActorSet aborted = 0;
	{
		TActorContextMap::iterator iter = m_actorContextMap.begin();
		while (iter != m_actorContextMap.end())
		{
			CDialogActorContextPtr pContext = (*iter).second;
			pContext->Update(dt);
			if (pContext->IsAborted())
			{
				// First one which aborts, aborts the whole session
				if (m_abortReason == eAR_None)
					m_abortReason = pContext->GetAbortReason();
				aborted.SetActor(iter->first);
			}
			if (pContext->IsStillPlaying())
			{
				++nStillPlaying;
			}
			if (pContext->CheckActorFlags(CDialogSession::eDACF_NoAbortSound))
			{
				++nSoundWantContinue;
			}
			++iter;
		}
	}

	// when we've reached the end there could still be some contexts playing
	if (m_bReachedEnd)
	{
		if (nStillPlaying == 0)
		{
			// no context playing, do a real end now
			bContinue = false;
		}
		else
		{
			if (nStillPlaying != m_pendingActors)
			{
				m_pendingActors = nStillPlaying;
				DiaLOG::Log(DiaLOG::eAlways, "[DIALOG] CDialogSession: %s End of Script reached. Waiting for %d pending Actors.", GetDebugName(), nStillPlaying);
			}
			// wait for some grace time and stop if timed out
			m_endGraceTimeOut -= dt;
			bContinue = m_endGraceTimeOut >= 0.0f;
		}
	}

	if (aborted.NumActors() > 0)
	{
		if (nSoundWantContinue == 0 || nStillPlaying == 0)
		{
			DoStop();
			NotifyListeners(eDSE_Aborted);
		}
		else
		{
			DiaLOG::Log(DiaLOG::eAlways, "[DIALOG] CDialogSession: %s Delaying abortion.", GetDebugName());
		}
	}
	else if (bContinue == false)
	{
		DoStop();
		NotifyListeners(eDSE_EndOfDialog);
	}
}

////////////////////////////////////////////////////////////////////////////
bool CDialogSession::PlayLine(const CDialogScript::SScriptLine* pLine)
{
	bool bOk = false;
	IEntity* pActorEntity = GetActorEntity(pLine->m_actor);
	if (pActorEntity)
	{
		CDialogActorContextPtr pContext = GetContext(pLine->m_actor);
		if (pContext)
		{
			bOk = pContext->PlayLine(pLine);
		}
		else
		{
			assert(pContext != 0);
			GameWarning("[DIALOG] CDialogSession::PlayLine: %s [Script=%s] No Context for Actor %d",
			            GetDebugName(), m_pScript->GetID().c_str(), pLine->m_actor);
		}
	}

	return bOk;
}

////////////////////////////////////////////////////////////////////////////
IEntity* CDialogSession::GetActorEntity(CDialogScript::TActorID actorID) const
{
	EntityId id = stl::find_in_map(this->m_idToEntityMap, actorID, 0);
	if (id == 0)
		return 0;
	return m_pEntitySystem->GetEntity(id);
}

////////////////////////////////////////////////////////////////////////////
EntityId CDialogSession::GetActorEntityId(CDialogScript::TActorID actorID) const
{
	return stl::find_in_map(this->m_idToEntityMap, actorID, 0);
}

////////////////////////////////////////////////////////////////////////////
IEntityAudioComponent* CDialogSession::GetEntityAudioProxy(IEntity* pEntity) const
{
	if (!pEntity)
		return 0;

	return pEntity->GetOrCreateComponent<IEntityAudioComponent>();
}

////////////////////////////////////////////////////////////////////////////
bool CDialogSession::Stop()
{
	bool bStopped = DoStop();
	if (bStopped)
		NotifyListeners(eDSE_UserStopped);

	return bStopped;
}

////////////////////////////////////////////////////////////////////////////
CDialogSession::EAbortReason CDialogSession::GetAbortReasonForActor(CDialogScript::TActorID actorID) const
{
	const CDialogActorContextPtr pContext = GetContext(actorID);
	if (pContext)
	{
		return pContext->GetAbortReason();
	}
	return eAR_None;
}

////////////////////////////////////////////////////////////////////////////
bool CDialogSession::DoStop()
{
	DiaLOG::Log(DiaLOG::eAlways, "[DIALOG] CDialogSession::DoStop: %s", GetDebugName());

	if (!m_bPlaying)
		return false;

	{
		TActorContextMap::iterator iter = m_actorContextMap.begin();
		while (iter != m_actorContextMap.end())
		{
			(*iter).second->EndSession();
			++iter;
		}
	}
	//Remove actor restrictions from this sessions on the communication manager.
	{
		ICommunicationUpr* pCommunicationUpr = gEnv->pAISystem->GetCommunicationUpr();

		TIdToEntityMap::iterator iter = m_idToEntityMap.begin();
		while (iter != m_idToEntityMap.end())
		{
			EntityId actorId = (*iter).second;
			pCommunicationUpr->RemoveActorRestriction(actorId, true, true);
			++iter;
		}
	}
	m_pDS->RemoveSession(this);
	m_bPlaying = false;

	if (m_bAutoDelete)
	{
		m_bAutoDelete = false;
		m_pDS->DeleteSession(m_sessionID);
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////
void CDialogSession::SetAutoDelete(bool bAutoDelete)
{
	m_bAutoDelete = bAutoDelete;
}

////////////////////////////////////////////////////////////////////////////
bool CDialogSession::Update(float dt)
{
	if (!m_bPlaying)
		return false;

	if (dt > 0.0f)
	{
		DoPlay(dt);
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////
bool CDialogSession::SetActor(CDialogScript::TActorID actorID, EntityId entityId)
{
	// Setting an actor invalidates Session
	m_bValidated = false;

	if (actorID >= CDialogScript::MAX_ACTORS)
		return false;

	m_idToEntityMap[actorID] = entityId;
	if (entityId != 0)
	{
		m_actorSet.SetActor(actorID);
		m_actorContextMap[actorID] = new CDialogActorContext(this, actorID);
	}
	else
	{
		m_actorSet.ResetActor(actorID);
		m_actorContextMap.erase(actorID);
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////
CDialogSession::CDialogActorContextPtr CDialogSession::GetContext(CDialogScript::TActorID actorID) const
{
	CDialogActorContextPtr pContext = stl::find_in_map(m_actorContextMap, actorID, 0);
	return pContext;
}
////////////////////////////////////////////////////////////////////////////

CDialogSession::CDialogActorContextPtr CDialogSession::GetContext(CDialogSystem::ActorContextID contextID) const
{
	for (TActorContextMap::const_iterator it = m_actorContextMap.begin(); it != m_actorContextMap.end(); ++it)
	{
		if (it->second->GetContextID() == contextID)
		{
			return it->second;
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////
bool CDialogSession::Validate()
{
	m_bOK = m_actorSet.Satisfies(m_pScript->GetRequiredActorSet());
	m_bValidated = true;
	return m_bOK;
}

////////////////////////////////////////////////////////////////////////////
tukk CDialogSession::GetEventName(EDialogSessionEvent event)
{
	static tuk names[] = {
		"eDSE_SessionStart",
		"eDSE_EndOfDialog",
		"eDSE_UserStopped",
		"eDSE_Aborted",
		"eDSE_SessionDeleted",
		"eDSE_LineStarted"
	};
	static i32k numNames = sizeof(names) / sizeof(*names);
	i32 index = (i32) event;
	if (index < 0 || index >= numNames)
	{
		return "eDSE_UNKNOWN!";
	}
	return names[index];
}

template<typename T>
void SerializeArray(TSerialize ser, tukk szName, T* vec, u32 count)
{
	ser.BeginGroup(szName);
	if (ser.IsWriting())
	{
		ser.Value("count", count);
		for (u32 i = 0; i < count; ++i)
		{
			ser.BeginGroup("i");
			ser.Value("Value", vec[i]);
			ser.EndGroup();
		}
	}
	else
	{
		u32 storedCount = 0;
		ser.Value("count", storedCount);
		u32 toFill = storedCount < count ? storedCount : count;
		u32 i = 0;
		while (toFill--)
		{
			ser.BeginGroup("i");
			ser.Value("Value", vec[i++]);
			ser.EndGroup();
		}
	}
	ser.EndGroup();
}

////////////////////////////////////////////////////////////////////////////
void CDialogSession::Serialize(TSerialize ser)
{
	//IEntitySystem*           m_pEntitySystem;     ctor
	//CDialogSystem*           m_pDS;               ctor
	//const CDialogScript*     m_pScript;           ctor
	//TIdToEntityMap           m_idToEntityMap;     SetActor
	//TListenerVec             m_listenerVec;       cleared
	//TActorContextMap         m_actorContextMap;   SetActor
	//CDialogSystem::SessionID m_sessionID;         ctor
	//string                   m_debugName;         ctor
	//CDialogScript::SActorSet m_actorSet;          ctor(=0) then SetActor

	//float                    m_curTime;           ctor 0
	//float                    m_nextTimeDelay;     ctor 0
	//i32                      m_curScriptLine;     ctor -1
	//i32                      m_nextScriptLine;    ctor -1

	//u32             m_bPlaying     : 1;  ctor false
	//u32             m_bValidated   : 1;  ctor false
	//u32             m_bOK          : 1;  ctor false
	//u32             m_bHaveSchedule: 1;  ctor false
	bool playing = m_bPlaying;
	ser.Value("m_bPlaying", playing);
	ser.Value("m_curScriptLine", m_curScriptLine);
	ser.Value("m_playerAwareAngle", m_playerAwareAngle);
	ser.Value("m_playerAwareDistance", m_playerAwareDistance);
	ser.Value("m_playerAwareGraceTime", m_playerAwareGraceTime);
	ser.Value("m_aiBehaviourMode", m_aiBehaviourMode);
	ser.Value("m_alertnessInterruptMode", *(alias_cast<i32*>(&m_alertnessInterruptMode)));

	// IdToEntityMap
	if (ser.IsWriting())
	{
		ser.Value("m_idToEntityMap", m_idToEntityMap);
	}
	else
	{
		TIdToEntityMap idmap;
		ser.Value("m_idToEntityMap", idmap);
		DiaLOG::Log(DiaLOG::eAlways, "CDialogSession::Serialize: %s was playing=%d --> ID2EntityMap", GetDebugName(), playing);
		TIdToEntityMap::iterator iter = idmap.begin();
		TIdToEntityMap::iterator end = idmap.end();
		while (iter != end)
		{
			DiaLOG::Log(DiaLOG::eAlways, "ID %d -> Entity %d", iter->first, iter->second);
			++iter;
		}
		m_idToEntityMap = idmap;
	}

	// Serialize actor flags
	if (ser.IsReading())
	{
		for (i32 i = 0; i < CDialogScript::MAX_ACTORS; ++i)
			m_actorFlags[i] = DEFAULT_ACTOR_FLAGS;
	}

	if (!m_idToEntityMap.empty())
	{
		static const size_t maxCount = sizeof(m_actorFlags) / sizeof(*m_actorFlags);
		assert(m_idToEntityMap.size() <= maxCount);
		u32k count = m_idToEntityMap.size() < maxCount ? m_idToEntityMap.size() : maxCount;
		SerializeArray(ser, "m_actorFlags", m_actorFlags, count);
	}
}

////////////////////////////////////////////////////////////////////////////
bool CDialogSession::RestoreAndPlay()
{
	// create actor contexts
	TIdToEntityMap::iterator iter = m_idToEntityMap.begin();
	TIdToEntityMap::iterator end = m_idToEntityMap.end();
	for (; iter != end; ++iter)
	{
		const CDialogScript::TActorID& actorID = iter->first;
		if (actorID >= CDialogScript::MAX_ACTORS)
			continue;
		const EntityId& entityId = iter->second;
		if (entityId != 0)
		{
			m_actorSet.SetActor(actorID);
			m_actorContextMap[actorID] = new CDialogActorContext(this, actorID);
		}
		else
		{
			m_actorSet.ResetActor(actorID);
			m_actorContextMap.erase(actorID);
		}
	}
	return InternalPlay(m_curScriptLine, false);
}

////////////////////////////////////////////////////////////////////////////
CDialogScript::TActorID CDialogSession::GetActorIdForEntity(EntityId entityId) const
{
	TIdToEntityMap::const_iterator iter = m_idToEntityMap.begin();
	TIdToEntityMap::const_iterator end = m_idToEntityMap.end();
	while (iter != end)
	{
		if (iter->second == entityId)
			return iter->first;
		++iter;
	}
	return CDialogScript::NO_ACTOR_ID;
}

////////////////////////////////////////////////////////////////////////////
CDialogSession::TActorFlags CDialogSession::GetActorFlags(CDialogScript::TActorID actorID) const
{
	if (actorID >= CDialogScript::MAX_ACTORS)
		return 0;
	return m_actorFlags[actorID];
}

////////////////////////////////////////////////////////////////////////////
void CDialogSession::SetActorFlags(CDialogScript::TActorID actorID, CDialogSession::TActorFlags inFlags)
{
	if (actorID >= CDialogScript::MAX_ACTORS)
		return;
	m_actorFlags[actorID] = inFlags;
}

void CDialogSession::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(m_idToEntityMap);
	pSizer->AddObject(m_listenerVec);
	pSizer->AddObject(m_actorContextMap);
}
