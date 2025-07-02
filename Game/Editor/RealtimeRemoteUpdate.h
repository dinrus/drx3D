// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id: RealtimeRemoveUpdate.h,v 1.1 2009/01/03 10:45:15 PauloZaffari Exp wwwrun $
$DateTime$
Описание:  This is the header file for the module Realtime remote 
update. The purpose of this module is to allow data update to happen 
remotely so that you can, for example, edit the terrain and see the changes
in the console.
-------------------------------------------------------------------------
История:
- 03:01:2009   10:45: Created by Paulo Zaffari
*************************************************************************/

#ifndef RealtimeRemoteUpdate_h__
#define RealtimeRemoteUpdate_h__

#pragma once

#include <drx3D/Network/INotificationNetwork.h>
#include <DrxLiveCreate/IRealtimeRemoteUpdate.h>

class CRealtimeRemoteUpdateListener:public INotificationNetworkListener,public IRealtimeRemoteUpdate
{
	//////////////////////////////////////////////////////////////////////////
	// Types & typedefs.
	public:
	protected:
	private:
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Methods
	public:
		// From IRealtimeRemoteUpdate
		bool	Enable(bool boEnable=true);
		bool	IsEnabled();

		// From INotificationNetworkListener and from IRealtimeRemoteUpdate
		void	OnNotificationNetworkReceive(ukk pBuffer, size_t length);

		// Singleton management
		static CRealtimeRemoteUpdateListener&	GetRealtimeRemoteUpdateListener();
protected:
		CRealtimeRemoteUpdateListener();
		virtual ~CRealtimeRemoteUpdateListener();

		void	LoadArchetypes(XmlNodeRef &root);
		void  LoadEntities( XmlNodeRef &root );
		void  LoadTimeOfDay( XmlNodeRef &root );
		void  LoadMaterials( XmlNodeRef &root );

		void  LoadConsoleVariables(XmlNodeRef &root );

		void  LoadParticles(XmlNodeRef &root );

		void  LoadTerrainLayer(XmlNodeRef &root, u8*	uchData);
	private:
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Data
	public:
	protected:
		// As currently you can't query if the currently listener is registered
		// the control has to be done here.
		bool m_boIsEnabled;

		i32  m_nPreviousFlyMode;
		bool m_boIsSyncingCamera;
	private:
	//////////////////////////////////////////////////////////////////////////
};

#endif // RealtimeRemoteUpdate_h__
