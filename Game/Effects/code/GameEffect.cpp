// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// Game effect - Ideal for handling a specific visual game feature.

// Includes
#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/Effects/GameEffect.h>
#include <drx3D/Game/Effects/GameEffectsSystem.h>
#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Game/Actor.h>

//--------------------------------------------------------------------------------------------------
// Name: CGameEffect
// Desc: Constructor
//--------------------------------------------------------------------------------------------------
CGameEffect::CGameEffect()
{
	m_prev	= NULL;
	m_next	= NULL;
	m_flags = 0;
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: ~CGameEffect
// Desc: Destructor
//--------------------------------------------------------------------------------------------------
CGameEffect::~CGameEffect()
{
#if DEBUG_GAME_FX_SYSTEM
	// Output message if effect hasn't been released before being deleted
	const bool bEffectIsReleased = (m_flags & GAME_EFFECT_RELEASED) ||			// -> Needs to be released before deleted
																 !(m_flags & GAME_EFFECT_INITIALISED) ||	// -> Except when not initialised
																 (gEnv->IsEditor());											// -> Or the editor (memory safely released by editor)
	if(!bEffectIsReleased)
	{
		string dbgMessage = m_debugName + " being destroyed without being released first";
		FX_ASSERT_MESSAGE(bEffectIsReleased,dbgMessage.c_str());
	}
#endif

	if(CGameEffectsSystem::Exists())
	{
		GAME_FX_SYSTEM.UnRegisterEffect(this);	// -> Effect should have been released and been unregistered, but to avoid
																						//    crashes call unregister here too
	}

	// Unregister as ViewSystem listener (for cut-scenes, ...)
	if (g_pGame)
	{
		IViewSystem *pViewSystem = g_pGame->GetIGameFramework()->GetIViewSystem();
		if (pViewSystem)
		{
			pViewSystem->RemoveListener(this);
		}
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: Initialise
// Desc: Initializes game effect
//--------------------------------------------------------------------------------------------------
void CGameEffect::Initialise(const SGameEffectParams* gameEffectParams)
{
#if DEBUG_GAME_FX_SYSTEM
	m_debugName = GetName(); // Store name so it can be accessed in destructor and debugging
#endif

	if(!IsFlagSet(GAME_EFFECT_INITIALISED))
	{
		SGameEffectParams params;
		if(gameEffectParams)
		{
			params = *gameEffectParams;
		}

		SetFlag(GAME_EFFECT_AUTO_UPDATES_WHEN_ACTIVE,params.autoUpdatesWhenActive);
		SetFlag(GAME_EFFECT_AUTO_UPDATES_WHEN_NOT_ACTIVE,params.autoUpdatesWhenNotActive);
		SetFlag(GAME_EFFECT_AUTO_RELEASE,params.autoRelease);
		SetFlag(GAME_EFFECT_AUTO_DELETE,params.autoDelete);

		GAME_FX_SYSTEM.RegisterEffect(this);

		SetFlag(GAME_EFFECT_INITIALISED,true);
		SetFlag(GAME_EFFECT_RELEASED,false);
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: Release
// Desc: Releases game effect
//--------------------------------------------------------------------------------------------------
void CGameEffect::Release()
{
	SetFlag(GAME_EFFECT_RELEASING, true);
	if(IsFlagSet(GAME_EFFECT_ACTIVE))
	{
		SetActive(false);
	}
	GAME_FX_SYSTEM.UnRegisterEffect(this);
	SetFlag(GAME_EFFECT_INITIALISED,false);
	SetFlag(GAME_EFFECT_RELEASING, false);
	SetFlag(GAME_EFFECT_RELEASED,true);
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: SetActive
// Desc: Sets active status
//--------------------------------------------------------------------------------------------------
void CGameEffect::SetActive(bool isActive)
{
	FX_ASSERT_MESSAGE(IsFlagSet(GAME_EFFECT_INITIALISED),"Effect changing active status without being initialised first");
	FX_ASSERT_MESSAGE((IsFlagSet(GAME_EFFECT_RELEASED)==false),"Effect changing active status after being released");

	SetFlag(GAME_EFFECT_ACTIVE,isActive);
	GAME_FX_SYSTEM.RegisterEffect(this); // Re-register effect with game effects system
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: Update
// Desc: Updates game effect
//--------------------------------------------------------------------------------------------------
void CGameEffect::Update(float frameTime)
{
	FX_ASSERT_MESSAGE(IsFlagSet(GAME_EFFECT_INITIALISED),"Effect being updated without being initialised first");
	FX_ASSERT_MESSAGE((IsFlagSet(GAME_EFFECT_RELEASED)==false),"Effect being updated after being released");
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: Next
// Desc: Gets next effect
//--------------------------------------------------------------------------------------------------
IGameEffect* CGameEffect::Next() const
{
	return m_next;
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: Prev
// Desc: Gets previous effect
//--------------------------------------------------------------------------------------------------
IGameEffect* CGameEffect::Prev() const
{
	return m_prev;
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: SetNext
// Desc: Sets the next effect
//--------------------------------------------------------------------------------------------------
void CGameEffect::SetNext(IGameEffect* newNext)
{
	m_next = newNext;
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: SetPrev
// Desc: Sets the previous effect
//--------------------------------------------------------------------------------------------------
void CGameEffect::SetPrev(IGameEffect* newPrev)
{
	m_prev = newPrev;
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: SetFlag
// Desc: Sets a game effect flag
//--------------------------------------------------------------------------------------------------
void CGameEffect::SetFlag(u32 flag,bool state)
{
	SET_FLAG(m_flags,flag,state);
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: IsFlagSet
// Desc: Checks a game effect flag
//--------------------------------------------------------------------------------------------------
bool CGameEffect::IsFlagSet(u32 flag) const
{
	return IS_FLAG_SET(m_flags,flag);
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: GetFlags
// Desc: Gets effect's flags
//--------------------------------------------------------------------------------------------------
u32 CGameEffect::GetFlags() const 
{ 
	return m_flags; 
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: SetFlags
// Desc: Sets effect's flags
//--------------------------------------------------------------------------------------------------
void CGameEffect::SetFlags(u32 flags)
{
	m_flags = flags;
}//-------------------------------------------------------------------------------------------------

bool CGameEffect::OnCameraChange(const SCameraParams& cameraParams)
{
	IViewSystem *pViewSystem = g_pGame->GetIGameFramework()->GetIViewSystem();
	if (pViewSystem)
	{
		const bool bClientActorView = pViewSystem->IsClientActorViewActive();
		SetFlag(GAME_EFFECT_ACTIVE, bClientActorView);
		if (!bClientActorView)
			ResetRenderParameters();
	}

	return true;
}

//--------------------------------------------------------------------------------------------------
// Name: SpawnParticlesOnSkeleton
// Desc: Spawn particles on Skeleton
//--------------------------------------------------------------------------------------------------
void CGameEffect::SpawnParticlesOnSkeleton(IEntity* pEntity, IParticleEmitter* pParticleEmitter, u32 numParticles,float maxHeightScale) const
{
	if((pEntity) && (numParticles>0) && (pParticleEmitter) && (maxHeightScale>0.0f))
	{
		ICharacterInstance* pCharacter = pEntity->GetCharacter(0);
		if(pCharacter)
		{
			IDefaultSkeleton& rIDefaultSkeleton = pCharacter->GetIDefaultSkeleton();
			ISkeletonPose* pPose = pCharacter->GetISkeletonPose();
			if(pPose)
			{
				Vec3 animPos;
				Quat animRot;

				IActor* pActor = gEnv->pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId());
				if(pActor) // First try to get animation data
				{
					QuatT animLoc = pActor->GetAnimatedCharacter()->GetAnimLocation();
					animPos = animLoc.t;
					animRot = animLoc.q;
				}
				else // If no actor, then use entity data
				{
					animPos = pEntity->GetWorldPos();
					animRot = pEntity->GetWorldRotation();
				}

				animRot.Invert();

				AABB bbox;
				pEntity->GetLocalBounds(bbox);
				float bbHeight = bbox.max.z - bbox.min.z;
				// Avoid division by 0
				if(bbHeight == 0)
				{
					bbHeight = 0.0001f;
				}

				u32k numJoints = rIDefaultSkeleton.GetJointCount();

				for (u32 i = 0; i < numParticles; ++i)
				{
					i32 id = drx_random(0U, numJoints - 1);
					i32 parentId = rIDefaultSkeleton.GetJointParentIDByID(id);

					if(parentId>0)
					{
						QuatT boneQuat = pPose->GetAbsJointByID(id);
						QuatT parentBoneQuat= pPose->GetAbsJointByID(parentId);
						float lerpScale = drx_random(0.0f, 1.0f);

						QuatTS loc(IDENTITY);
						loc.t = LERP(boneQuat.t,parentBoneQuat.t,lerpScale);

						float heightScale = ((loc.t.z - bbox.min.z) / bbHeight);
						if(heightScale < maxHeightScale)
						{
							loc.t = loc.t * animRot;
							loc.t = loc.t + animPos;

							pParticleEmitter->EmitParticle(NULL, NULL, &loc);
						}
					}
				}
			}
		}
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: SetMaterialOnEntity
// Desc: Sets material on entity
//--------------------------------------------------------------------------------------------------
void CGameEffect::SetMaterialOnEntity(IMaterial* pMaterial,EntityId entityId,PodArray<i32>*	pBodyAttachmentIndexArray)
{
	if(pMaterial && pBodyAttachmentIndexArray)
	{
		IEntity* pEntity = gEnv->pEntitySystem->GetEntity(entityId);
		if(pEntity)
		{
			SEntitySlotInfo slotInfo;
			bool gotSlotInfo = pEntity->GetSlotInfo(0, slotInfo);
			if(gotSlotInfo && slotInfo.pCharacter)
			{
				IAttachmentUpr* pAttachmentUpr = slotInfo.pCharacter->GetIAttachmentUpr();
				if(pAttachmentUpr)
				{
					i32 attachmentCount = pBodyAttachmentIndexArray->size();
					for(i32 i=0; i<attachmentCount; i++)
					{
						IAttachment* attachment = pAttachmentUpr->GetInterfaceByIndex((*pBodyAttachmentIndexArray)[i]);

						if(attachment)
						{
							IAttachmentObject* attachmentObj = attachment->GetIAttachmentObject();
							if(attachmentObj)
							{
								attachmentObj->SetReplacementMaterial(pMaterial);
							}
						}
					}
				}
			}
		}
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: SetMaterialOnEntity
// Desc: Sets material on entity
//--------------------------------------------------------------------------------------------------
void CGameEffect::SetMaterialOnEntity(IMaterial* pMaterial,EntityId entityId,PodArray<ItemString>*	bodyAttachmentNameArray)
{
	if(pMaterial && bodyAttachmentNameArray)
	{
		IEntity* pEntity = gEnv->pEntitySystem->GetEntity(entityId);
		if(pEntity)
		{
			SEntitySlotInfo slotInfo;
			bool gotSlotInfo = pEntity->GetSlotInfo(0, slotInfo);
			if(gotSlotInfo && slotInfo.pCharacter)
			{
				IAttachmentUpr* pAttachmentUpr = slotInfo.pCharacter->GetIAttachmentUpr();
				if(pAttachmentUpr)
				{
					i32 attachmentCount = bodyAttachmentNameArray->size();
					for(i32 i=0; i<attachmentCount; i++)
					{
						IAttachment* attachment = pAttachmentUpr->GetInterfaceByName(((*bodyAttachmentNameArray)[i]).c_str());

						if(attachment)
						{
							IAttachmentObject* attachmentObj = attachment->GetIAttachmentObject();
							if(attachmentObj)
							{
								attachmentObj->SetReplacementMaterial(pMaterial);
							}
						}
					}
				}
			}
		}
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: ReadAttachmentNames
// Desc: Reads attachment names
//--------------------------------------------------------------------------------------------------
void CGameEffect::ReadAttachmentNames(const IItemParamsNode* pParamNode,PodArray<ItemString>* pAttachmentNameArray)
{
	if(pParamNode && pAttachmentNameArray)
	{
		i32 attachmentCount = pParamNode->GetChildCount();
		pAttachmentNameArray->resize(attachmentCount);
		for(i32 i=0; i<attachmentCount; i++)
		{
			const IItemParamsNode* attachmentNode = pParamNode->GetChild(i);
			if(attachmentNode)
			{
				(*pAttachmentNameArray)[i] = attachmentNode->GetAttribute("name");
			}
		}
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: InitAttachmentIndexArray
// Desc: Initialises attachment index array
//--------------------------------------------------------------------------------------------------
void CGameEffect::InitAttachmentIndexArray(PodArray<i32> *attachmentIndexArray,PodArray<ItemString>* pAttachmentNameArray,EntityId entityId) const 
{
	if(attachmentIndexArray && pAttachmentNameArray && (entityId!=0))
	{
		if(attachmentIndexArray->size() == 0)
		{
			// Store attachment index array
			i32k attachmentNameCount = pAttachmentNameArray->size();

			SEntitySlotInfo slotInfo;
			IEntity* pEntity = gEnv->pEntitySystem->GetEntity(entityId);
			if(pEntity)
			{
				bool gotEntitySlotInfo = pEntity->GetSlotInfo(0, slotInfo);
				if(gotEntitySlotInfo)
				{
					attachmentIndexArray->reserve(attachmentNameCount);

					if(slotInfo.pCharacter)
					{
						IAttachmentUpr* attachmentUpr = slotInfo.pCharacter->GetIAttachmentUpr();
						if(attachmentUpr)
						{
							i32k attachmentCount = attachmentUpr->GetAttachmentCount();

							for(i32 n=0; n<attachmentNameCount; n++)
							{
								for(i32 a=0; a<attachmentCount; a++)
								{
									IAttachment* attachment = attachmentUpr->GetInterfaceByIndex(a);
									if(attachment)
									{
										tukk currentAttachmentName = attachment->GetName();
										if(strcmp(currentAttachmentName,(*pAttachmentNameArray)[n].c_str())==0)
										{
											attachmentIndexArray->push_back(a);
											break;
										}
									}
								}
							}
						}
					}
				}
			}	
		}
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: LoadMaterial
// Desc: Loads and calls AddRef on material
//--------------------------------------------------------------------------------------------------
IMaterial* CGameEffect::LoadMaterial(tukk pMaterialName) 
{
	IMaterial* pMaterial = NULL;
	I3DEngine* p3DEngine = gEnv->p3DEngine;
	if(pMaterialName && p3DEngine)
	{
		IMaterialUpr* pMaterialUpr = p3DEngine->GetMaterialUpr();
		if(pMaterialUpr)
		{
			pMaterial = pMaterialUpr->LoadMaterial(pMaterialName);
			if(pMaterial)
			{
				pMaterial->AddRef();
			}
		}
	}
	return pMaterial;
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: LoadParticleEffect
// Desc: Loads and calls AddRef on a particle effect
//--------------------------------------------------------------------------------------------------
IParticleEffect* CGameEffect::LoadParticleEffect(tukk pParticleEffectName)
{
	IParticleEffect* pParticleEffect = NULL;
	IParticleUpr* pParticleUpr =  gEnv->pParticleUpr;

	if(pParticleEffectName && pParticleUpr)
	{
		pParticleEffect = gEnv->pParticleUpr->FindEffect(pParticleEffectName);
		if(pParticleEffect)
		{
			pParticleEffect->AddRef();
		}
	}

	return pParticleEffect;
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: IsEntity3rdPerson
// Desc: Returns 3rd person state
//--------------------------------------------------------------------------------------------------
bool CGameEffect::IsEntity3rdPerson(EntityId entityId)
{
	bool bIs3rdPerson = true;

	IGame* pGame = gEnv->pGame;
	if(pGame)
	{
		IGameFramework* pGameFramework = pGame->GetIGameFramework();
		if(pGameFramework)
		{
			EntityId clientEntityId = pGameFramework->GetClientActorId();
			if(clientEntityId == entityId)
			{
				bIs3rdPerson = pGameFramework->GetClientActor()->IsThirdPerson();
			}
		}
	}

	return bIs3rdPerson;
}//-------------------------------------------------------------------------------------------------
