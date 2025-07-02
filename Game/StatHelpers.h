// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef		__STATHELPERS_H__
#define		__STATHELPERS_H__

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/CoreX/Game/IGameStatistics.h>
#include <drx3D/Game/CircularStatsStorage.h>
#include <drx3D/Game/GameRulesTypes.h>
#include <drx3D/Game/GameRules.h>

#define USE_STATS_CIRCULAR_BUFFER			1

#if USE_STATS_CIRCULAR_BUFFER
#define	SERIALIZABLE_STATS_BASE			CCircularBufferTimelineEntry
#else
#define SERIALIZABLE_STATS_BASE			CXMLSerializableBase
#endif

struct IStatsTracker;
struct IActor;
class CActor;

class CStatHelpers
{
public:
	static bool SaveActorArmor( CActor* pActor );
	static bool SaveActorStats( CActor* pActor, XmlNodeRef stats );
	static bool SaveActorWeapons( CActor* pActor );
	static bool SaveActorAttachments( CActor* pActor );
	static bool SaveActorAmmos( CActor* pActor );
	static i32 GetProfileId( i32 channelId );
	static i32 GetChannelId( i32 profileId );
	static EntityId GetEntityId( i32 profileId );

private:
	static IActor* GetProfileActor( i32 profileId );
};

//////////////////////////////////////////////////////////////////////////

class CPositionStats : public SERIALIZABLE_STATS_BASE
{
public:

	CPositionStats();
	CPositionStats(const Vec3& pos, float elevation);
	CPositionStats(const CPositionStats *inCopy);
	CPositionStats &operator=(const CPositionStats &inFrom);
	virtual XmlNodeRef GetXML(IGameStatistics* pGS);
	virtual void GetMemoryStatistics(IDrxSizer* pSizer) const;

	Vec3 m_pos;
	float m_elev;
};

//////////////////////////////////////////////////////////////////////////

class CLookDirStats : public SERIALIZABLE_STATS_BASE
{
public:

	CLookDirStats(const Vec3& dir);
	virtual XmlNodeRef GetXML(IGameStatistics* pGS);
	virtual void GetMemoryStatistics(IDrxSizer* pSizer) const;

	Vec3 m_dir;
};

//////////////////////////////////////////////////////////////////////////

class CShotFiredStats : public SERIALIZABLE_STATS_BASE
{
public:

	CShotFiredStats(EntityId projectileId, i32 ammo_left, tukk ammo_type);
	virtual XmlNodeRef GetXML(IGameStatistics* pGS);
	virtual void GetMemoryStatistics(IDrxSizer* pSizer) const;

	EntityId m_projectileId;
	i32 m_ammoLeft;
	CDrxName m_ammoType;
};

//////////////////////////////////////////////////////////////////////////

class CKillStats : public SERIALIZABLE_STATS_BASE
{
public:

	CKillStats(EntityId projectileId, EntityId targetId, tukk hit_type, tukk  weapon_class, tukk projectile_class);
	virtual XmlNodeRef GetXML(IGameStatistics* pGS);
	virtual void GetMemoryStatistics(IDrxSizer* pSizer) const;

	EntityId m_projectileId;
	EntityId target_id;
	CDrxName m_hitType;
	CDrxName m_weaponClass;
	CDrxName m_projectileClass;
};

//////////////////////////////////////////////////////////////////////////

class CDeathStats : public SERIALIZABLE_STATS_BASE
{
public:

	CDeathStats(EntityId projectileId, EntityId killerId, tukk hit_type, tukk weapon_class, tukk projectile_class);
	virtual XmlNodeRef GetXML(IGameStatistics* pGS);
	virtual void GetMemoryStatistics(IDrxSizer* pSizer) const;

	EntityId m_projectileId;
	EntityId m_killerId;
	CDrxName m_hitType;
	CDrxName m_weaponClass;
	CDrxName m_projectileClass;
};

//////////////////////////////////////////////////////////////////////////

class CHitStats : public SERIALIZABLE_STATS_BASE
{
public:
	CHitStats(EntityId projectileId, EntityId shooterId, float damage, tukk hit_type, tukk weapon_class, tukk projectile_class, tukk hit_part);
	virtual XmlNodeRef GetXML(IGameStatistics* pGS);
	virtual void GetMemoryStatistics(IDrxSizer* pSizer) const;

	EntityId m_projectileId;
	EntityId m_shooterId;
	float m_damage;
	CDrxName m_hitType;
	CDrxName m_weaponClass;
	CDrxName m_projectileClass;
	CDrxName m_hitPart;
};

//////////////////////////////////////////////////////////////////////////

class CXPIncEvent : public SERIALIZABLE_STATS_BASE
{
public:
	CXPIncEvent(i32 inDelta, EXPReason inReason);
	virtual XmlNodeRef GetXML(IGameStatistics* pGS);
	virtual void GetMemoryStatistics(IDrxSizer* pSizer) const;

	i32					m_delta;
	EXPReason	m_reason;
};

//////////////////////////////////////////////////////////////////////////

class CScoreIncEvent : public SERIALIZABLE_STATS_BASE
{
public:
	CScoreIncEvent(i32 score, EGameRulesScoreType type);
	virtual XmlNodeRef GetXML(IGameStatistics* pGS);
	virtual void GetMemoryStatistics(IDrxSizer* pSizer) const;

	i32	m_score;
	EGameRulesScoreType	m_type;
};

//////////////////////////////////////////////////////////////////////////

class CWeaponChangeEvent : public SERIALIZABLE_STATS_BASE
{
public:
	CWeaponChangeEvent(tukk weapon_class, tukk bottom_attachment_class = "", tukk barrel_attachment_class = "", tukk scope_attachment_class = "");
	virtual XmlNodeRef GetXML(IGameStatistics* pGS);
	virtual void GetMemoryStatistics(IDrxSizer* pSizer) const;

	CDrxName m_weaponClass;
	CDrxName m_bottomAttachmentClass;
	CDrxName m_barrelAttachmentClass;
	CDrxName m_scopeAttachmentClass;
};

//////////////////////////////////////////////////////////////////////////

class CFlashedEvent : public SERIALIZABLE_STATS_BASE
{
public:
	CFlashedEvent(float flashDuration, EntityId shooterId);
	virtual XmlNodeRef GetXML(IGameStatistics* pGS);
	virtual void GetMemoryStatistics(IDrxSizer* pSizer) const;

	float	m_flashDuration;
	EntityId	m_shooterId;
};

//////////////////////////////////////////////////////////////////////////

class CTaggedEvent : public SERIALIZABLE_STATS_BASE
{
public:
	CTaggedEvent(EntityId shooter, float time, CGameRules::ERadarTagReason reason);
	virtual XmlNodeRef GetXML(IGameStatistics* pGS);
	virtual void GetMemoryStatistics(IDrxSizer* pSizer) const;

	EntityId m_shooter;
	CGameRules::ERadarTagReason m_reason;
};

//////////////////////////////////////////////////////////////////////////

class CExchangeItemEvent : public SERIALIZABLE_STATS_BASE
{
public:
	CExchangeItemEvent(tukk old_item_class, tukk new_item_class);
	virtual XmlNodeRef GetXML(IGameStatistics* pGS);
	virtual void GetMemoryStatistics(IDrxSizer* pSizer) const;

	CDrxName m_oldItemClass;
	CDrxName m_newItemClass;
};

///////////////////////////////////////////////////////////////////////////
enum 
{
	eEVE_Ripup,
	eEVE_Pickup,
	eEVE_Drop,
	eEVE_MeleeAttack,
	eEVE_MeleeKill,
	eEVE_ThrowAttack,
	eEVE_ThrowKill,
};


class CEnvironmentalWeaponEvent : public SERIALIZABLE_STATS_BASE
{
public:
	CEnvironmentalWeaponEvent( EntityId weaponId, i16 action, i16 iParam);
	virtual XmlNodeRef GetXML( IGameStatistics* pGS );
	virtual void GetMemoryStatistics( IDrxSizer* pSizer ) const;


	EntityId	m_weaponId;
	i16			m_action;
	i16			m_param;
};


///////////////////////////////////////////////////////////////////////////
enum
{
	eSSE_neutral,
	eSSE_capturing_from_neutral,
	eSSE_captured,
	eSSE_capturing_from_capture,
	eSSE_contested,
	eSSE_failed_capture,
};

class CSpearStateEvent : public SERIALIZABLE_STATS_BASE
{
// owning team currently owns this spear and is scoring points on it
// capturing team is the only team with players round this spear and is currently moving it into their control

public:
	CSpearStateEvent( u8 spearId, u8 state, u8 team1count, u8 team2count, u8 owningTeam, u8 capturingTeam );
	virtual XmlNodeRef GetXML( IGameStatistics* pGS );
	virtual void GetMemoryStatistics( IDrxSizer* pSizer ) const;

	u8 m_spearId;
	u8 m_state;
	u8 m_team1count; 
	u8 m_team2count; 
	u8 m_owningTeam; 
	u8 m_capturingTeam;
};


#endif
