// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   FlashUIEventSystem.cpp
//  Version:     v1.00
//  Created:     10/9/2010 by Paul Reindell.
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/FlashUIEventSystem.h>
#include <drx3D/CoreX/Game/IGameFramework.h>
#include <drx3D/Act/FlashUI.h>

//------------------------------------------------------------------------------------
CFlashUIEventSystem::~CFlashUIEventSystem()
{
	for (TEventListener::Notifier notifier(m_listener); notifier.IsValid(); notifier.Next())
		notifier->OnEventSystemDestroyed(this);

	for (TUIEventsLookup::const_iterator iter = m_eventDescriptions.begin(); iter != m_eventDescriptions.end(); ++iter)
		delete *iter;
}

//------------------------------------------------------------------------------------
uint CFlashUIEventSystem::RegisterEvent(const SUIEventDesc& sEventDesc)
{
	m_eventDescriptions.push_back(new SUIEventDesc(sEventDesc));
	return (uint) m_eventDescriptions.size() - 1;
}

//------------------------------------------------------------------------------------
void CFlashUIEventSystem::RegisterListener(IUIEventListener* pListener, tukk name)
{
	const bool ok = m_listener.Add(pListener, name);
	DRX_ASSERT_MESSAGE(ok, "Listener already registered!");
}

//------------------------------------------------------------------------------------
void CFlashUIEventSystem::UnregisterListener(IUIEventListener* pListener)
{
	DRX_ASSERT_MESSAGE(m_listener.Contains(pListener), "Listener was never registered or already unregistered!");
	m_listener.Remove(pListener);
}

//------------------------------------------------------------------------------------
SUIArgumentsRet CFlashUIEventSystem::SendEvent(const SUIEvent& event)
{
	DRX_PROFILE_FUNCTION(PROFILE_ACTION);

	SUIArguments ret;
	if (gEnv->IsEditor())
	{
		if (GetType() == eEST_UI_TO_SYSTEM && !CFlashUI::CV_gfx_uievents_editorenabled)
		{
			const SUIEventDesc* pEventDesc = GetEventDesc(event.event);
			UIACTION_WARNING("%s (UI->System): Event sent \"%s\" BUT NOT DISPATCHED! (args: %s)", GetName(), pEventDesc ? pEventDesc->sDisplayName : "<UNDEFINED EVENT>", event.args.GetAsString());
			return ret;
		}
	}

#if defined (UIACTION_LOGGING)
	{
		tukk evtSysType = GetType() == eEST_UI_TO_SYSTEM ? "UI->System" : "System->UI";
		const SUIEventDesc* pEventDesc = GetEventDesc(event.event);
		UIACTION_LOG("%s (%s): Event sent \"%s\" (args: %s)", GetName(), evtSysType, pEventDesc ? pEventDesc->sDisplayName : "<UNDEFINED EVENT>", event.args.GetAsString());
	}
#endif

	for (TEventListener::Notifier notifier(m_listener); notifier.IsValid(); notifier.Next())
		ret.AddArguments(notifier->OnEvent(event));
	return ret;
}

//------------------------------------------------------------------------------------
uint CFlashUIEventSystem::GetEventId(tukk sEventName)
{
	uint res = 0;
	for (TUIEventsLookup::const_iterator iter = m_eventDescriptions.begin(); iter != m_eventDescriptions.end(); ++iter)
	{
		if (strcmp((*iter)->sName, sEventName) == 0)
			return res;
		++res;
	}
	return ~0;
}

//-------------------------------------------------------------------
void CFlashUIEventSystem::GetMemoryUsage(IDrxSizer* s) const
{
	SIZER_SUBCOMPONENT_NAME(s, "FlashUIEventSystem");
	//s->AddObject(this, sizeof(*this)); // collected by owner (AddContainer)
	s->AddContainer(m_eventDescriptions);
	s->AddObject(&m_listener, m_listener.MemSize());
}

//------------------------------------------------------------------------------------
CUIEventSystemIterator::CUIEventSystemIterator(TUIEventSystemMap* pMap)
{
	m_iRefs = 1;
	m_currIter = pMap->begin();
	m_endIter = pMap->end();
}

//------------------------------------------------------------------------------------
void CUIEventSystemIterator::AddRef()
{
	m_iRefs++;
}

//------------------------------------------------------------------------------------
void CUIEventSystemIterator::Release()
{
	if (--m_iRefs == 0)
		delete this;
}

//------------------------------------------------------------------------------------
IUIEventSystem* CUIEventSystemIterator::Next(string& sName)
{
	IUIEventSystem* pNext = NULL;
	if (m_currIter != m_endIter)
	{
		sName = m_currIter->first;
		pNext = m_currIter->second;
		++m_currIter;
	}
	return pNext;
}

//------------------------------------------------------------------------------------
