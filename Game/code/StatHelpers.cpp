// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/CoreX/Game/IGameStatistics.h>
#include <drx3D/Game/StatHelpers.h>

#include <drx3D/Game/Game.h>
#include <drx3D/Game/Actor.h>
#include <drx3D/Game/GameRules.h>
#include <drx3D/CoreX/TypeInfo_impl.h>

#if 0	// MT : not imported from kiev
#include <drx3D/Game/ActorInventory.h>
#include <drx3D/Game/CharacterEquip.h>
#include <drx3D/Game/WeaponGameObject.h>
#include <drx3D/Game/Attachment.h>

bool CStatHelpers::SaveActorArmor( CActor* pActor )
{
	if ( IStatsTracker *tracker = pActor->GetStatsTracker() )
		if ( const CCharacterEquip *pEquip = pActor->GetCharacterEquip() )
		{
			XmlNodeRef resNode = GetISystem()->GetXmlUtils()->CreateStatsXmlNode(MULTIPLE_STAT_XML_TAG);
			for ( i32 i = 0; i < pEquip->GetItemCount(); ++i )
			{
				XmlNodeRef childNode = GetISystem()->CreateXmlNode("part");
				childNode->setAttr( "name", pEquip->GetItem( i ) );
				childNode->setAttr( "amount", 1 );

				resNode->addChild( childNode );
			}	

			tracker->StateValue( eSP_Armor, resNode );
			return true;
		}
		return false;
}

bool CStatHelpers::SaveActorStats( CActor* pActor, XmlNodeRef stats )
{
	if ( IStatsTracker *tracker = pActor->GetStatsTracker() )
	{
		tracker->StateValue(eSP_PlayerInfo, stats);
		return true;
	}
	return false;
}

bool CStatHelpers::SaveActorWeapons( CActor* pActor )
{
	if ( IStatsTracker *tracker = pActor->GetStatsTracker() )
		if ( const CActorInventory *pInv = pActor->GetActorInventory() )
		{
			XmlNodeRef resNode = GetISystem()->GetXmlUtils()->CreateStatsXmlNode(MULTIPLE_STAT_XML_TAG);
			for ( i32 i = 0; i < pInv->GetWeaponsCount() ; ++i )
			{
				XmlNodeRef childNode = GetISystem()->CreateXmlNode("weapon");
				childNode->setAttr( "name", pInv->GetWeapon( i )->GetName() );
				childNode->setAttr( "amount", 1 );

				resNode->addChild( childNode );
			}	

			tracker->StateValue( eSP_Weapons, resNode );
			return true;
		}
	return false;
}

bool CStatHelpers::SaveActorAttachments( CActor* pActor )
{
	if ( IStatsTracker *tracker = pActor->GetStatsTracker() )
		if ( const CActorInventory *pInv = pActor->GetActorInventory() )
		{
			XmlNodeRef resNode = GetISystem()->GetXmlUtils()->CreateStatsXmlNode(MULTIPLE_STAT_XML_TAG);
			for ( i32 i = 0; i < pInv->GetAttachmentsCount() ; ++i )
			{
				XmlNodeRef childNode = GetISystem()->CreateXmlNode("attachment");
				childNode->setAttr( "name", pInv->GetAttachment( i )->GetName() );
				childNode->setAttr( "amount", 1 );

				resNode->addChild( childNode );
			}	

			tracker->StateValue( eSP_Attachments, resNode );
			return true;
		}
	return false;
}

bool CStatHelpers::SaveActorAmmos( CActor* pActor )
{
	if ( IStatsTracker *tracker = pActor->GetStatsTracker() )
		if ( const CActorInventory *pInv = pActor->GetActorInventory() )
		{
			XmlNodeRef resNode = GetISystem()->GetXmlUtils()->CreateStatsXmlNode(MULTIPLE_STAT_XML_TAG);
			for ( i32 i = 0; i < pInv->GetAmmoTypesCount(); ++i )
				if ( IEntityClass* pEC = pInv->GetAmmoType( i ) )
				{
					XmlNodeRef childNode = GetISystem()->CreateXmlNode("ammo");
					childNode->setAttr( "name", pEC->GetName() );
					childNode->setAttr( "amount", pInv->GetAmmoCount( pEC ) );

					resNode->addChild( childNode );
				}	

			tracker->StateValue( eSP_Ammos, resNode );
			return true;
		}
	return false;
}
#endif

i32 CStatHelpers::GetProfileId( i32 channelId )
{
	i32 profileId = 0;
	if(INetChannel* pCh = g_pGame->GetIGameFramework()->GetNetChannel(channelId))
		profileId = pCh->GetProfileId();

	return profileId;
}

IActor* CStatHelpers::GetProfileActor( i32 profileId )
{
	IActorIteratorPtr aIt = g_pGame->GetIGameFramework()->GetIActorSystem()->CreateActorIterator();
	while( IActor* pActor = aIt->Next() )
	{
		if ( pActor->IsPlayer() )
			if ( profileId == GetProfileId(pActor->GetChannelId()) )
				return pActor;
	}

	return 0;
}

i32 CStatHelpers::GetChannelId( i32 profileId )
{
	if ( IActor* pActor = GetProfileActor(profileId) )
		return pActor->GetChannelId();

	return 0;
}

EntityId CStatHelpers::GetEntityId( i32 profileId )
{
	if ( IActor* pActor = GetProfileActor(profileId) )
		return pActor->GetEntityId();

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// CPositionStats
//////////////////////////////////////////////////////////////////////////
CPositionStats::CPositionStats(const Vec3& pos, float elevation)
: m_pos(pos)
, m_elev(elevation)
{ }

CPositionStats::CPositionStats()
{
}

CPositionStats::CPositionStats(const CPositionStats *inStats)
{
	*this=*inStats;
}

CPositionStats &CPositionStats::operator=(const CPositionStats &inFrom)
{
	m_pos=inFrom.m_pos;
	m_elev=inFrom.m_elev;
	// do not copy CXMLSerializableBase values
	return *this;
}

//////////////////////////////////////////////////////////////////////////

XmlNodeRef CPositionStats::GetXML(IGameStatistics* pGS)
{
	XmlNodeRef node = pGS->CreateStatXMLNode();
	node->setAttr("x", m_pos.x);
	node->setAttr("y", m_pos.y);
	node->setAttr("z", m_pos.z);
	node->setAttr("g", m_elev);
	return node;
}

//////////////////////////////////////////////////////////////////////////

void CPositionStats::GetMemoryStatistics(IDrxSizer *pSizer) const
{
	pSizer->Add(*this);
}

//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// CLookDirStats
//////////////////////////////////////////////////////////////////////////

CLookDirStats::CLookDirStats(const Vec3& dir)
: m_dir(dir)
{ }

//////////////////////////////////////////////////////////////////////////

XmlNodeRef CLookDirStats::GetXML(IGameStatistics* pGS)
{
	XmlNodeRef node = pGS->CreateStatXMLNode();
	node->setAttr("x", m_dir.x);
	node->setAttr("y", m_dir.y);
	node->setAttr("z", m_dir.z);
	return node;
}

//////////////////////////////////////////////////////////////////////////

void CLookDirStats::GetMemoryStatistics(IDrxSizer *pSizer) const
{
	pSizer->Add(*this);
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// CShotFiredStats
//////////////////////////////////////////////////////////////////////////

CShotFiredStats::CShotFiredStats(EntityId projectileId, i32 ammo_left, tukk ammo_type)
: m_projectileId(projectileId), m_ammoLeft(ammo_left), m_ammoType(ammo_type)
{ }

//////////////////////////////////////////////////////////////////////////

XmlNodeRef CShotFiredStats::GetXML(IGameStatistics* pGS)
{
	XmlNodeRef node = pGS->CreateStatXMLNode();
	node->setAttr("projectile_id", m_projectileId);
	node->setAttr("ammo_left", m_ammoLeft);
	node->setAttr("ammo_type", m_ammoType.c_str());
	return node;
}

//////////////////////////////////////////////////////////////////////////

void CShotFiredStats::GetMemoryStatistics(IDrxSizer *pSizer) const
{
	pSizer->Add(*this);
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// CKillStats
//////////////////////////////////////////////////////////////////////////

CKillStats::CKillStats(EntityId projectileId, EntityId targetId, tukk hit_type, tukk weapon_class, tukk projectile_class)
: m_projectileId(projectileId), target_id(targetId), m_hitType(hit_type), m_weaponClass(weapon_class), m_projectileClass(projectile_class)
{ }

//////////////////////////////////////////////////////////////////////////

XmlNodeRef CKillStats::GetXML(IGameStatistics* pGS)
{
	XmlNodeRef node = pGS->CreateStatXMLNode();
	node->setAttr("projectile_id", m_projectileId);
	node->setAttr("target_id", target_id);
	node->setAttr("hit_type", m_hitType.c_str());
	node->setAttr("weapon_class", m_weaponClass.c_str());
	node->setAttr("projectile_class", m_projectileClass.c_str());
	return node;
}

//////////////////////////////////////////////////////////////////////////

void CKillStats::GetMemoryStatistics(IDrxSizer *pSizer) const
{
	pSizer->Add(*this);
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// CDeathStats
//////////////////////////////////////////////////////////////////////////

CDeathStats::CDeathStats(EntityId projectileId, EntityId killerId, tukk hit_type, tukk weapon_class, tukk projectile_class)
: m_projectileId(projectileId), m_killerId(killerId), m_hitType(hit_type), m_weaponClass(weapon_class), m_projectileClass(projectile_class)
{ }

//////////////////////////////////////////////////////////////////////////

XmlNodeRef CDeathStats::GetXML(IGameStatistics* pGS)
{
	XmlNodeRef node = pGS->CreateStatXMLNode();
	node->setAttr("projectile_id", m_projectileId);
	node->setAttr("killer_id", m_killerId);
	node->setAttr("hit_type", m_hitType.c_str());
	node->setAttr("weapon_class", m_weaponClass.c_str());
	node->setAttr("projectile_class", m_projectileClass.c_str());
	return node;
}

//////////////////////////////////////////////////////////////////////////

void CDeathStats::GetMemoryStatistics(IDrxSizer *pSizer) const
{
	pSizer->Add(*this);
}

//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// CHitStats
//////////////////////////////////////////////////////////////////////////

CHitStats::CHitStats(EntityId projectileId, EntityId shooterId, float damage, tukk hit_type, tukk weapon_class, tukk projectile_class, tukk hit_part)
: m_projectileId(projectileId), m_shooterId(shooterId), m_damage(damage), m_hitType(hit_type), m_weaponClass(weapon_class), m_projectileClass(projectile_class), m_hitPart(hit_part)
{ }

//////////////////////////////////////////////////////////////////////////

XmlNodeRef CHitStats::GetXML(IGameStatistics* pGS)
{
	XmlNodeRef node = pGS->CreateStatXMLNode();
	node->setAttr("projectile_id", m_projectileId);
	node->setAttr("shooter_id", m_shooterId);
	node->setAttr("damage", m_damage);
	node->setAttr("weapon_class", m_weaponClass.c_str());
	node->setAttr("projectile_class", m_projectileClass.c_str());
	node->setAttr("hit_type", m_hitType.c_str());
	node->setAttr("hit_part", m_hitPart.c_str());
	return node;
}

//////////////////////////////////////////////////////////////////////////

void CHitStats::GetMemoryStatistics(IDrxSizer *pSizer) const
{
	pSizer->Add(*this);
}

//////////////////////////////////////////////////////////////////////////

CXPIncEvent::CXPIncEvent(
	i32					inXPDelta,
	EXPReason	inReason) :
	m_delta(inXPDelta),
	m_reason(inReason)
{
}

// must match def of EXPReason
static tukk k_XPIncRsnsStrs[] =
{ 
	PlayerProgressionType(AUTOENUM_PARAM_1_AS_STRING_COMMA)
	EGRSTList(AUTOENUM_PARAM_1_AS_STRING_COMMA)
	XPIncReasons(AUTOENUM_PARAM_1_AS_STRING_COMMA)
};

XmlNodeRef CXPIncEvent::GetXML(IGameStatistics* pGS)
{
	XmlNodeRef node = pGS->CreateStatXMLNode();

	node->setAttr("delta", m_delta);

	COMPILE_TIME_ASSERT(k_XPRsn_Num==DRX_ARRAY_COUNT(k_XPIncRsnsStrs));

	EXPReason		reason=m_reason;

	if (reason<0 || reason>=DRX_ARRAY_COUNT(k_XPIncRsnsStrs))
	{
		reason=k_XPRsn_Unknown;
	}

	const char		*pStr=k_XPIncRsnsStrs[reason];

	// skip code prefixes from enums to make them more human readable
	if (strncmp(pStr,"EPP_",4)==0)
	{
		pStr+=4;
	}
	else if (strncmp(pStr,"EGRST_",6)==0)
	{
		pStr+=6;
	}
	else if (strncmp(pStr,"k_XPRsn_",8)==0)
	{
		pStr+=8;
	}
	
	node->setAttr("reason", pStr);

	return node;
}

void CXPIncEvent::GetMemoryStatistics(IDrxSizer *pSizer) const
{
	pSizer->Add(*this);
}

//////////////////////////////////////////////////////////////////////////

CScoreIncEvent::CScoreIncEvent(i32 score, EGameRulesScoreType type) :
m_score(score),
m_type(type)
{
}

static tukk k_ScoreIncTypeStrs[] =
{ 
	EGRSTList(AUTOENUM_PARAM_1_AS_STRING_COMMA)
};

XmlNodeRef CScoreIncEvent::GetXML(IGameStatistics* pGS)
{
	XmlNodeRef node = pGS->CreateStatXMLNode();

	node->setAttr("score", m_score);

	COMPILE_TIME_ASSERT(EGRST_Num==DRX_ARRAY_COUNT(k_ScoreIncTypeStrs));

	EGameRulesScoreType type=m_type;

	tukk pStr = "Unknown";
	if (m_type>=0 && m_type<DRX_ARRAY_COUNT(k_ScoreIncTypeStrs))
	{
		pStr = k_ScoreIncTypeStrs[m_type];
	}

	// skip code prefixes from enums to make them more human readable
	if (strncmp(pStr,"EGRST_",6)==0)
	{
		pStr+=6;
	}

	node->setAttr("type", pStr);

	return node;
}

void CScoreIncEvent::GetMemoryStatistics(IDrxSizer *pSizer) const
{
	pSizer->Add(*this);
}

//////////////////////////////////////////////////////////////////////////

CWeaponChangeEvent::CWeaponChangeEvent(tukk weapon_class, tukk bottom_attachment_class, tukk barrel_attachment_class, tukk scope_attachment_class)
: m_weaponClass(weapon_class), m_bottomAttachmentClass(bottom_attachment_class), m_barrelAttachmentClass(barrel_attachment_class), m_scopeAttachmentClass(scope_attachment_class)
{
}

XmlNodeRef CWeaponChangeEvent::GetXML(IGameStatistics* pGS)
{
	XmlNodeRef node = pGS->CreateStatXMLNode();
	node->setAttr("weapon_class", m_weaponClass.c_str());
	if (!m_bottomAttachmentClass.empty())
		node->setAttr("bottom_attachment_class", m_bottomAttachmentClass.c_str());
	if (!m_barrelAttachmentClass.empty())
		node->setAttr("barrel_attachment_class", m_barrelAttachmentClass.c_str());
	if (!m_scopeAttachmentClass.empty())
		node->setAttr("scope_attachment_class", m_scopeAttachmentClass.c_str());
	return node;
}

void CWeaponChangeEvent::GetMemoryStatistics(IDrxSizer* pSizer) const
{
	pSizer->Add(*this);
}

//////////////////////////////////////////////////////////////////////////

CFlashedEvent::CFlashedEvent(float flashDuration, EntityId shooterId)
: m_flashDuration(flashDuration), m_shooterId(shooterId)
{
}

XmlNodeRef CFlashedEvent::GetXML(IGameStatistics* pGS)
{
	XmlNodeRef node = pGS->CreateStatXMLNode();
	node->setAttr("flash_duration", m_flashDuration);
	node->setAttr("shooter_id", m_shooterId);
	return node;
}

void CFlashedEvent::GetMemoryStatistics(IDrxSizer* pSizer) const
{
	pSizer->Add(*this);
}

//////////////////////////////////////////////////////////////////////////

CTaggedEvent::CTaggedEvent(EntityId shooter, float time, CGameRules::ERadarTagReason reason)
: m_shooter(shooter)
, m_reason(reason)
{
	SetTime(CTimeValue(time));
}

static tukk k_RadarTagReasonStrs[] =
{ 
	ERTRList(AUTOENUM_PARAM_1_AS_STRING_COMMA)
};

XmlNodeRef CTaggedEvent::GetXML(IGameStatistics* pGS)
{
	COMPILE_TIME_ASSERT(CGameRules::eRTR_Last==DRX_ARRAY_COUNT(k_RadarTagReasonStrs));

	tukk sReason = "Unknown";
	if (m_reason>=0 && m_reason<CGameRules::eRTR_Last)
	{
		sReason = k_RadarTagReasonStrs[m_reason];
		// skip code prefixes from enums to make them more human readable
		assert(strncmp(sReason,"eRTR_",5)==0);
		sReason+=5;
	}

	XmlNodeRef node = pGS->CreateStatXMLNode();
	node->setAttr("shooter", m_shooter);
	node->setAttr("duration", GetTime().GetSeconds());
	node->setAttr("reason", sReason);
	return node;
}

void CTaggedEvent::GetMemoryStatistics(IDrxSizer* pSizer) const
{
	pSizer->Add(*this);
}

//////////////////////////////////////////////////////////////////////////

CExchangeItemEvent::CExchangeItemEvent(tukk old_item_class, tukk new_item_class)
: m_oldItemClass(old_item_class), m_newItemClass(new_item_class)
{
}

XmlNodeRef CExchangeItemEvent::GetXML(IGameStatistics* pGS)
{
	XmlNodeRef node = pGS->CreateStatXMLNode();
	node->setAttr("old_item_class", m_oldItemClass.c_str());
	node->setAttr("new_item_class", m_newItemClass.c_str());
	return node;
}

void CExchangeItemEvent::GetMemoryStatistics(IDrxSizer* pSizer) const
{
	pSizer->Add(*this);
}


//////////////////////////////////////////////////////////////////////////
CEnvironmentalWeaponEvent::CEnvironmentalWeaponEvent( EntityId weaponId, i16 action, i16 iParam )
: m_weaponId( weaponId )
, m_action( action )
, m_param( iParam )
{
}

XmlNodeRef CEnvironmentalWeaponEvent::GetXML(IGameStatistics* pGS)
{
	XmlNodeRef node = pGS->CreateStatXMLNode();
	node->setAttr("entity_id", m_weaponId );

	tukk actionName = NULL;

	switch( m_action )
	{
	case eEVE_Ripup:					actionName = "ripup";
														break;
	case eEVE_Pickup:					actionName = "pickup";
														break;
	case eEVE_Drop:						actionName = "drop";
														break;
	case eEVE_MeleeAttack:		actionName = "meleeAttack";
														break;
	case eEVE_MeleeKill:			actionName = "meleeKill";
														break;
	case eEVE_ThrowAttack:		actionName = "throwAttack";
														break;
	case eEVE_ThrowKill:			actionName = "throwKill";
														break;
	}

	node->setAttr("action", actionName );
	node->setAttr("param", m_param );
	return node;
}

void CEnvironmentalWeaponEvent::GetMemoryStatistics(IDrxSizer* pSizer) const
{
	pSizer->Add(*this);
}


//////////////////////////////////////////////////////////////////////////
CSpearStateEvent::CSpearStateEvent( u8 spearId, u8 state, u8 team1count, u8 team2count, u8 owningTeam, u8 capturingTeam )
	: m_spearId( spearId )
	, m_state( state )
	, m_team1count( team1count ) 
	, m_team2count( team2count ) 
	, m_owningTeam( owningTeam )
	, m_capturingTeam( capturingTeam )
{
}

XmlNodeRef CSpearStateEvent::GetXML(IGameStatistics* pGS)
{
	XmlNodeRef node = pGS->CreateStatXMLNode();

	tukk stateName = NULL;

	switch( m_state )
	{
	case eSSE_neutral:								stateName = "neutral";
		break;
	case eSSE_capturing_from_neutral:	stateName = "capturing_from_neutral";
		break;
	case eSSE_captured:								stateName = "captured";
		break;
	case eSSE_capturing_from_capture:	stateName = "capturing_from_capture";
		break;
	case eSSE_contested:							stateName = "contested";
		break;
	case eSSE_failed_capture:					stateName = "failed_capture";
		break;
	}

	node->setAttr( "state", stateName );
	node->setAttr( "spear_id", m_spearId );
	node->setAttr( "team1_count", m_team1count );
	node->setAttr( "team2_count", m_team2count );
	node->setAttr( "owning_team", m_owningTeam );
	node->setAttr( "capturing_team", m_capturingTeam );

	return node;
}

void CSpearStateEvent::GetMemoryStatistics(IDrxSizer* pSizer) const
{
	pSizer->Add(*this);
}
