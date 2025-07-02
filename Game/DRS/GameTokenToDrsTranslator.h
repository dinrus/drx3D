// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/************************************************************************

   a simple bridge between game tokens and DRS data.
   it will send an DRS signal every time a (bool) gametoken changes its value

   /************************************************************************/

#pragma once

#include <drx3D/CoreX/Game/IGameTokens.h>

class CGameTokenSignalCreator:public IGameTokenEventListener
{
public:
	CGameTokenSignalCreator(DRS::IResponseActor* pSenderForTokenSignals);
	~CGameTokenSignalCreator();

	//////////////////////////////////////////////////////////
	// IGameTokenEventListener implementation
	virtual void OnGameTokenEvent(EGameTokenEvent event, IGameToken* pGameToken);
	virtual void GetMemoryUsage(class IDrxSizer* pSizer) const;
	//////////////////////////////////////////////////////////

private:
	DRS::IResponseActor* m_pSenderForTokenSignals;
};
