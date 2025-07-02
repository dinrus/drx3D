// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id: GameRealtimeRemoveUpdate.h,v 1.1 23/09/2009 Johnmichael Quinlan
Описание:  This is the header file for the game module specific Realtime remote update. 
							The purpose of this module is to allow data update to happen remotely inside 
							the game layer so that you can, for example, edit the terrain and see 
							the changes in the console.
-------------------------------------------------------------------------
История:
- 23/09/2009   10:45: Created by Johnmichael Quinlan
*************************************************************************/

#ifndef GameRealtimeRemoteUpdate_h__
#define GameRealtimeRemoteUpdate_h__

#pragma once

#include <DrxLiveCreate/IRealtimeRemoteUpdate.h>

class CGameRealtimeRemoteUpdateListener : public IRealtimeUpdateGameHandler
{
protected:
	CGameRealtimeRemoteUpdateListener();
	virtual ~CGameRealtimeRemoteUpdateListener();

public:
	static CGameRealtimeRemoteUpdateListener& GetGameRealtimeRemoteUpdateListener();
	bool Enable(bool boEnable);

	bool Update();
protected:
	
	virtual bool UpdateGameData(XmlNodeRef oXmlNode, u8 * auchBinaryData);
	virtual void UpdateCamera(XmlNodeRef oXmlNode);
	virtual void CloseLevel();
	virtual void CameraSync();
	virtual void EndCameraSync();
private:
	bool m_bCameraSync;
	i32  m_nPreviousFlyMode;
	Vec3 m_Position;
	Vec3 m_ViewDirection;
	Vec3 m_headPositionDelta;


	XmlNodeRef	m_CameraUpdateInfo;
};

#endif //GameRealtimeRemoteUpdate_h__

