// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
Описание: 
Listener interface for player updates.
-------------------------------------------------------------------------
История:
- 2:12:2009	Created by Adam Rutkowski
*************************************************************************/
#ifndef __PLAYERUPDATELISTENER_H__
#define __PLAYERUPDATELISTENER_H__

struct IPlayerUpdateListener
{
	virtual ~IPlayerUpdateListener(){}
	virtual void Update(float fFrameTime) = 0;
};

#endif //__PLAYERUPDATELISTENER_H__
