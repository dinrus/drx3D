// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   FlashUIEventSystem.h
//  Version:     v1.00
//  Created:     10/9/2010 by Paul Reindell.
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __FlashUIEventSystem_H__
#define __FlashUIEventSystem_H__

#include <drx3D/Sys/IFlashUI.h>
#include <drx3D/CoreX/Containers/DrxListenerSet.h>

class CFlashUIEventSystem
	: public IUIEventSystem
{
public:
	CFlashUIEventSystem(tukk sName, EEventSystemType eType) : m_sName(sName), m_eType(eType), m_listener(2) {};
	virtual ~CFlashUIEventSystem();

	virtual tukk                      GetName() const { return m_sName.c_str(); }
	virtual IUIEventSystem::EEventSystemType GetType() const { return m_eType; }

	virtual uint                             RegisterEvent(const SUIEventDesc& sEventDesc);

	virtual void                             RegisterListener(IUIEventListener* pListener, tukk name);
	virtual void                             UnregisterListener(IUIEventListener* pListener);

	virtual SUIArgumentsRet                  SendEvent(const SUIEvent& event);

	virtual const SUIEventDesc*              GetEventDesc(i32 index) const              { return index < m_eventDescriptions.size() ? m_eventDescriptions[index] : NULL; }
	virtual const SUIEventDesc*              GetEventDesc(tukk sEventName) const { return m_eventDescriptions(sEventName); }
	virtual i32                              GetEventCount()  const                     { return m_eventDescriptions.size(); }

	virtual uint                             GetEventId(tukk sEventName);

	virtual void                             GetMemoryUsage(IDrxSizer* s) const;

private:
	string           m_sName;
	EEventSystemType m_eType;
	TUIEventsLookup  m_eventDescriptions;

	typedef CListenerSet<IUIEventListener*> TEventListener;
	TEventListener m_listener;
};

typedef std::map<string, CFlashUIEventSystem*> TUIEventSystemMap;

struct CUIEventSystemIterator : public IUIEventSystemIterator
{
	CUIEventSystemIterator(TUIEventSystemMap* pMap);
	virtual void            AddRef();
	virtual void            Release();
	virtual IUIEventSystem* Next(string& sName);

private:
	i32                         m_iRefs;
	TUIEventSystemMap::iterator m_currIter;
	TUIEventSystemMap::iterator m_endIter;
};

#endif // #ifndef __FlashUIEventSystem_H__
