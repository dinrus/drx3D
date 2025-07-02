// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------

	Plays Announcements based upon general game events

История:
- 17:11:2012		Created by Jim Bamford
*************************************************************************/

#ifndef __MISCANNOUNCER_H__
#define __MISCANNOUNCER_H__

#include <drx3D/Game/GameRulesModules/IGameRulesRevivedListener.h>
#include <drx3D/Game/GameRulesModules/IGameRulesRoundsListener.h>
#include <drx3D/Game/GameRulesTypes.h>
#include <drx3D/CoreX/Containers/DrxFixedArray.h>
#include <drx3D/Game/Audio/AudioSignalPlayer.h>
#include <drx3D/Act/IWeapon.h>
#include <drx3D/Act/IItemSystem.h>
#include <drx3D/CoreX/String/DrxFixedString.h>
#include <drx3D/Game/AudioTypes.h>

class CWeapon;

class CMiscAnnouncer : public SGameRulesListener, public IGameRulesRoundsListener, public IItemSystemListener, public IWeaponEventListener
{
public:
	CMiscAnnouncer();
	~CMiscAnnouncer();

	void Init();
	void Reset();

	void Update(const float dt);
	
	// SGameRulesListener
	virtual void GameOver(EGameOverType localWinner, bool isClientSpectator) {}
	virtual void EnteredGame() {}
	virtual void EndGameNear(EntityId id) {}
	virtual void ClientEnteredGame( EntityId clientId ) {}
	virtual void ClientDisconnect( EntityId clientId );
	virtual void OnActorDeath( CActor* pActor ) {}
	virtual void SvOnTimeLimitExpired() {}
	virtual void ClTeamScoreFeedback(i32 teamId, i32 prevScore, i32 newScore) {}
	// ~SGameRulesListener

	// IGameRulesRoundsListener
	virtual void OnRoundStart();
	virtual void OnRoundEnd() {}
	virtual void OnSuddenDeath() {}
	virtual void ClRoundsNetSerializeReadState(i32 newState, i32 curState) {}
	virtual void OnRoundAboutToStart() {} 
	// ~IGameRulesRoundsListener

	//IItemSystemListener
	virtual void OnSetActorItem(IActor *pActor, IItem *pItem );
	virtual void OnDropActorItem(IActor *pActor, IItem *pItem ) {}
	virtual void OnSetActorAccessory(IActor *pActor, IItem *pItem ) {}
	virtual void OnDropActorAccessory(IActor *pActor, IItem *pItem ){}
	//~IItemSystemListener

	// IWeaponEventListener
	virtual void OnShoot(IWeapon *pWeapon, EntityId shooterId, EntityId ammoId, IEntityClass* pAmmoType, const Vec3 &pos, const Vec3 &dir, const Vec3 &vel);
	virtual void OnStartFire(IWeapon *pWeapon, EntityId shooterId) {}
	virtual void OnStopFire(IWeapon *pWeapon, EntityId shooterId) {}
	virtual void OnStartReload(IWeapon *pWeapon, EntityId shooterId, IEntityClass* pAmmoType) {}
	virtual void OnEndReload(IWeapon *pWeapon, EntityId shooterId, IEntityClass* pAmmoType) {}
	virtual void OnOutOfAmmo(IWeapon *pWeapon, IEntityClass* pAmmoType) {}
	virtual void OnSetAmmoCount(IWeapon *pWeapon, EntityId shooterId) {}
	virtual void OnReadyToFire(IWeapon *pWeapon) {}
	virtual void OnPickedUp(IWeapon *pWeapon, EntityId actorId, bool destroyed) {}
	virtual void OnDropped(IWeapon *pWeapon, EntityId actorId) {}
	virtual void OnMelee(IWeapon* pWeapon, EntityId shooterId) {}
	virtual void OnStartTargetting(IWeapon *pWeapon) {}
	virtual void OnStopTargetting(IWeapon *pWeapon) {}
	virtual void OnSelected(IWeapon *pWeapon, bool selected) {}
	virtual void OnEndBurst(IWeapon *pWeapon, EntityId shooterId) {}
	virtual void OnFireModeChanged(IWeapon *pWeapon, i32 currentFireMode) {}
	virtual void OnWeaponRippedOff(CWeapon *pWeapon) {}
	//~IWeaponEventListener

	void WeaponFired();

protected:

	void InitXML(XmlNodeRef root);
	void SetNewWeaponListener(IWeapon* pWeapon, EntityId weaponId, EntityId actorId);
	void RemoveWeaponListener(EntityId weaponId);
	void RemoveAllWeaponListeners();
	void RemoveWeaponListenerForActor(EntityId actorId);

	bool AnnouncerRequired();

	typedef std::map<EntityId, EntityId> ActorWeaponListenerMap;
	ActorWeaponListenerMap m_actorWeaponListener;

	struct SOnWeaponFired
	{
		IEntityClass *m_weaponEntityClass;
		DrxFixedStringT<50> m_announcementName;
		EAnnouncementID m_announcementID;

		bool m_havePlayedFriendly;
		bool m_havePlayedEnemy;

		SOnWeaponFired() :
			m_weaponEntityClass(NULL),
			m_announcementID(0)
		{
			Reset();
		}

		SOnWeaponFired(IEntityClass *inWeaponEntityClass, tukk inAnnouncementName, EAnnouncementID inAnnouncementId) :
			m_weaponEntityClass(inWeaponEntityClass),
			m_announcementName(inAnnouncementName),
			m_announcementID(inAnnouncementId)
		{
			Reset();
		}

		void Reset()
		{
			m_havePlayedFriendly=false;
			m_havePlayedEnemy=false;
		}
	};

	typedef std::map<IEntityClass*, SOnWeaponFired> TWeaponFiredMap;
	TWeaponFiredMap m_weaponFiredMap;
};

#endif // __MISCANNOUNCER_H__
