// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id: DummyPlayer.h$
$DateTime$
Описание: A dummy player used to simulate a client player for profiling purposes

-------------------------------------------------------------------------
История:
- 01/07/2010 11:15:00: Created by Martin Sherburn

*************************************************************************/

#ifndef __DUMMYPLAYER_H__
#define __DUMMYPLAYER_H__

#if (USE_DEDICATED_INPUT)

#include <drx3D/Game/Player.h>
#include <drx3D/Game/AIDemoInput.h>

class CDummyPlayer : public CPlayer
{
public:
	CDummyPlayer();
	virtual ~CDummyPlayer();

	virtual bool IsPlayer() const { return true; }
	
	virtual bool Init( IGameObject * pGameObject );
	virtual void Update(SEntityUpdateContext& ctx, i32 updateSlot);

	EDefaultableBool GetFire();
	void SetFire(EDefaultableBool value);
	EDefaultableBool GetMove();
	void SetMove(EDefaultableBool value);

	void GetMemoryUsage(IDrxSizer *pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
		CPlayer::GetInternalMemoryUsage(pSizer); // collect memory of parent class
	}

protected:
	virtual void OnChangeTeam();

private:
};

#endif //USE_DEDICATED_INPUT

#endif //!__DUMMYPLAYER_H__
