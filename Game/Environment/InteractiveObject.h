// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Crysis2 interactive object, for playing co-operative animations with player

-------------------------------------------------------------------------
История:
- 10:12:2009: Created by Benito G.R.

*************************************************************************/

#pragma once

#ifndef __DRXSIS2_INTERACTIVE_OBJECT_H__
#define __DRXSIS2_INTERACTIVE_OBJECT_H__

#include <drx3D/Act/IGameObject.h>
#include <drx3D/Game/InteractiveObjectEnums.h>
#include <drx3D/Game/ItemString.h>

#include <IDrxMannequinDefs.h>

#define INTERACTIVE_OBJECT_EX_ANIMATIONS_DEBUG	0

class CInteractiveObjectEx : public CGameObjectExtensionHelper<CInteractiveObjectEx, IGameObjectExtension>
{

private:

	enum EState
	{
		eState_NotUsed = 0,
		eState_InUse,
		eState_Used, //Has been used at least one time but can still be used
		eState_Done
	};

	struct SInteractionDataSet
	{
		SInteractionDataSet();
		~SInteractionDataSet(){};

		QuatT			m_alignmentHelperLocation;
		float			m_interactionRadius;
		float			m_interactionAngle;
		TagID			m_targetTagID;
		ItemString m_helperName;
	};

public:
	CInteractiveObjectEx();
	virtual ~CInteractiveObjectEx();

	// IGameObjectExtension
	virtual bool Init(IGameObject *pGameObject);
	virtual void InitClient(i32 channelId);
	virtual void PostInit(IGameObject *pGameObject);
	virtual void PostInitClient(i32 channelId);
	virtual bool ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params );
	virtual void PostReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params ) {}
	virtual bool GetEntityPoolSignature( TSerialize signature );
	virtual void Release();
	virtual void FullSerialize( TSerialize ser );
	virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags );
	virtual void PostSerialize();
	virtual void SerializeSpawnInfo( TSerialize ser );
	virtual ISerializableInfoPtr GetSpawnInfo();
	virtual void Update( SEntityUpdateContext &ctx, i32 updateSlot);
	virtual void PostUpdate(float frameTime );
	virtual void PostRemoteSpawn();
	virtual void HandleEvent( const SGameObjectEvent &goEvent);
	virtual void ProcessEvent(SEntityEvent &entityEvent);
	virtual void SetChannelId(u16 id);
	virtual void SetAuthority(bool auth);
	virtual void GetMemoryUsage(IDrxSizer *pSizer) const;
	//~IGameObjectExtension

	//Script callbacks
	i32 CanUse(EntityId entityId) const;
	virtual void Use(EntityId entityId);	
	virtual void StopUse(EntityId entityId);
	virtual void AbortUse();
	//~Script callbacks

	void StartUseSpecific(EntityId entityId, i32 interactionIndex);
	void OnExploded(const Vec3& explosionSource);

private:

	bool Reset();

	void StartUse(EntityId entityId);
	void ForceInstantStandaloneUse(i32k interactionIndex);
	void Done(EntityId entityId);
	void CalculateHelperLocation(tukk helperName, QuatT& helper) const;
	void Physicalize(pe_type physicsType, bool forcePhysicalization = false);
	void ParseAllInteractions(const SmartScriptTable& interactionProperties, std::vector<tuk>& interactionNames); 
	void InitAllInteractionsFromMannequin();
	i32  CalculateSelectedInteractionIndex( const EntityId entityId ) const;

	// returns -1 if not interaction constraints satsified, else returns index of first interaction the user can perform
	i32  CanUserPerformAnyInteraction(EntityId userId) const;
	bool InteractionConstraintsSatisfied(const IEntity* pUserEntity, const SInteractionDataSet& interactionDataSet) const; 
	bool CanStillBeUsed();
	void HighlightObject(bool highlight);

	void ForceSkeletonUpdate( bool on );

#ifndef _RELEASE
	void DebugRender() const; 
#endif //#ifndef _RELEASE

protected:
	bool IsValidName(tukk name) const;

	u32 GetCrcForName(tukk name) const;

private:
	typedef std::vector<SInteractionDataSet> SInteractionDataSetArray;
	SInteractionDataSetArray m_interactionDataSets;
	SInteractionDataSet m_loadedInteractionData;

	EState	m_state;
	i32		m_physicalizationAfterAnimation;
	i32		m_activeInteractionIndex; // Index into m_InteractionDataSets to indicate active interaction. 

	TagID m_interactionTagID;
	TagID m_stateTagID;

	u32	m_currentLoadedCharacterCrc;
	bool		m_bHighlighted;
	bool		m_bRemoveDecalsOnUse;
	bool		m_bStartInteractionOnExplosion;
};


class CDeployableBarrier : public CInteractiveObjectEx
{
};


#endif //__DRXSIS2_INTERACTIVE_OBJECT_H__
