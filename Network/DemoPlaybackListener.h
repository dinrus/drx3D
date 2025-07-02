// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  Demo session playback.
   -------------------------------------------------------------------------
   История:
   - ??/2005 : Created by Craig Tiller
   - 04/2006 : Taken over by Jan Müller
*************************************************************************/

#ifndef __DEMOPLAYBACKLISTENER_H__
#define __DEMOPLAYBACKLISTENER_H__

#pragma once

#include <drx3D/Network/Config.h>
#if INCLUDE_DEMO_RECORDING

	#include <drx3D/Network/INetContextListener.h>
	#include <drx3D/Network/SimpleInputStream.h>

class CNetContext;
class CNetChannel;

class CDemoPlaybackListener : public INetContextListener
{
public:
	CDemoPlaybackListener(
	  CNetContext* pContext,
	  tukk filename,
	  CNetChannel* pClientChannel, CNetChannel* pServerChannel);
	~CDemoPlaybackListener();

	// INetContextListener
	virtual void   OnChannelEvent(CNetContextState* pState, INetChannel* pFrom, SNetChannelEvent* pEvent) {}
	virtual void   OnObjectEvent(CNetContextState* pState, SNetObjectEvent* pEvent);
	virtual string GetName();
	virtual void   PerformRegularCleanup() {; }
	virtual bool   IsDead()                { return m_bIsDead; }
	virtual void   Die();
	// ~INetContextListener

	virtual void GetMemoryStatistics(IDrxSizer* pSizer)
	{
		SIZER_COMPONENT_NAME(pSizer, "CDemoPlaybackListener");

		pSizer->Add(*this);
		pSizer->AddContainer(m_recIdToPlayId);
	}

	EntityId MapID(EntityId id)
	{
		// 0x10000 is null, but a different null to null
		id = stl::find_in_map(m_recIdToPlayId, id, 0);
		return id;
	}

private:
	std::unique_ptr<CSimpleInputStream> m_pInput;
	CNetContext*                      m_pContext;
	CNetChannel*                      m_pServerChannel;
	CNetChannel*                      m_pClientChannel;
	bool                              m_bInGame;
	bool                              m_bIsDead;

	struct LTStr
	{
		ILINE bool operator()(tukk s1, tukk s2) const
		{
			return strcmp(s1, s2) < 0;
		}
	};

	enum EInputResult
	{
		eIR_Ok,
		eIR_AbortRead,
		eIR_TryLater,
	};

	SStreamRecord                m_buffer;
	float                        m_startTime;
	CTimeValue                   m_initTime;

	std::map<EntityId, EntityId> m_recIdToPlayId;
	EntityId                     m_currentlyUpdating;
	EntityId                     m_currentlyBinding;
	bool                         m_currentlyBindingStatic;

	u8                        m_currentProfile;

	typedef EInputResult (CDemoPlaybackListener::*     InputHandler)();
	typedef std::map<tukk , InputHandler, LTStr> TInputHandlerMap;
	TInputHandlerMap*       m_pState;

	static TInputHandlerMap m_defaultHandlers;

	static void  InitHandlers();
	void         DoUpdate();

	EInputResult SkipLineWithWarning();
	EInputResult NetMessage();
	EInputResult BeginFrame();
	EInputResult BindObject();
	EInputResult UnbindObject();
	EInputResult UpdateObject();
	EInputResult BeginAspect();
	EInputResult FinishUpdateObject();
	EInputResult CppRMI();
	EInputResult ScriptRMI();
	EInputResult SetAspectProfile();

	EInputResult MessageHandler(EntityId rmiObj);

	void         CheckCurrentlyBinding();

	template<typename T>
	bool DecodeValue(tukk name, T& value, bool peek = false);

	void GC_Disconnect(EDisconnectionCause cause, string description);
};

#endif

#endif
