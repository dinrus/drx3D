// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __GAME_MOVING_PLATFORM_MGR_H__
#define __GAME_MOVING_PLATFORM_MGR_H__ 

#include <drx3D/Game/GameRules.h>

class CMovingPlatformMgr
{
	struct SContact
	{
		SContact()
			: fTimeSinceFirstContact(0.f)
			, fTimeSinceLastContact(0.f)
		{}
		ILINE void Update(const float dt) { fTimeSinceFirstContact += dt; fTimeSinceLastContact += dt; }
		ILINE void Refresh() { fTimeSinceLastContact = 0.f; }
		float fTimeSinceFirstContact;
		float fTimeSinceLastContact;
	};

public:
	CMovingPlatformMgr();
	~CMovingPlatformMgr();

	i32 OnCollisionLogged( const EventPhys* pEvent );
	i32 OnDeletedLogged( const EventPhys* pEvent );
	void Update( const float dt );
	void Reset();

	/* Static Physics Event Receivers */
	static i32 StaticOnCollision( const EventPhys * pEvent );
	static i32 StaticOnDeleted( const EventPhys * pEvent );

protected:
	typedef std::map<EntityId, SContact> TContactList;
	TContactList m_contacts;
};

#endif
