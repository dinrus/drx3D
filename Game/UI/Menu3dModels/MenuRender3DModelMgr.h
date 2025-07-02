// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __MENURENDER3DMODELMGR_H__
#define __MENURENDER3DMODELMGR_H__

// Includes
#include <drx3D/CoreX/Game/IGameFramework.h>
#include <drx3D/CoreX/Containers/DrxFixedArray.h>

// Forward declares
struct CEntityAttachment;
struct IAttachment;

// Defines
#ifndef _RELEASE
	#define DEBUG_MENU_RENDER_3D_MODEL_MGR	1
#else
	#define DEBUG_MENU_RENDER_3D_MODEL_MGR	0
#endif

//==================================================================================================
// Name: CMenuRender3DModelMgr
// Desc: Rendering of 3d models in menus
// Created by: Tim Furnish
// Refactored by: James Chilvers
//==================================================================================================
class CMenuRender3DModelMgr : public IGameFrameworkListener
{
public:

	// Public data structs and enums
	enum
	{
		kAddedModelIndex_MaxEntities = 20,
		kAddedModelIndex_Invalid = 255
	};

	typedef u8 TAddedModelIndex;

	struct SModelParams
	{
		SModelParams()
		{
			pName = NULL;
			pFilename = NULL;
			pMaterialOverrideFilename = NULL;
			rot = ZERO;
			secondRot = ZERO;
			continuousRot = ZERO;
			posOffset = ZERO;
			posSecondOffset = ZERO;
			silhouetteColor = ZERO;
			userRotScale = 0.0f;
			scale = 0.9f;
			secondScale = 0.9f;
			memset(screenRect,0,sizeof(screenRect));
		}

		Ang3				rot;
		Ang3				secondRot;
		Ang3				continuousRot;
		Vec3				posOffset;
		Vec3				posSecondOffset;
		Vec3				silhouetteColor;
		float				screenRect[4];
		float				userRotScale;
		float				scale;
		float				secondScale;
		tukk pName;
		tukk pFilename;
		tukk pMaterialOverrideFilename;
	};

	struct SModelData
	{
		SModelData()
		{
			rot=ZERO;
			secondRot=ZERO;
			posOffset=ZERO;
			posSecondOffset=ZERO;
			silhouetteColor=ZERO;
			animSpeed=0.0f;
			userRotScale=0.0f;
			scale=0.9f;
			secondScale=0.9f;
			memset(screenRect,0,sizeof(screenRect));
		}
		DrxFixedStringT<256>	filename;
		DrxFixedStringT<256>	materialOverrideFilename;
		DrxFixedStringT<128>	animName;
		DrxFixedStringT<64>		name;
		DrxFixedStringT<64>		attachmentParent;
		DrxFixedStringT<64>		attachmentLocation;
		Ang3									rot;
		Ang3									secondRot;
		Vec3									posOffset;
		Vec3									posSecondOffset;
		Vec3									silhouetteColor;
		float									screenRect[4];
		float									animSpeed;
		float									userRotScale;
		float									scale;
		float									secondScale;
	};

	struct SLightData
	{
		friend class CMenuRender3DModelMgr;

		SLightData()
		{
			pos=ZERO;
			color=ZERO;
			specular=0.0f;
			radius=400.0f; // Default to something sensible
			pLightSource=NULL;
		}

		Vec3						pos;
		Vec3						color;
		float						specular;
		float						radius;

	private:
		ILightSource*		pLightSource;
	};

	struct SSceneSettings
	{
		SSceneSettings()
		{
			ambientLight = Vec4(0.0f,0.0f,0.0f,0.0f);
			fadeInSpeed = 0.0f;
			fovScale = 0.0f;
			flashEdgeFadeScale = 0.2f;
		}

		PodArray<SLightData>	lights;
		Vec4		ambientLight;
		float		fadeInSpeed;
		float		fovScale;
		float		flashEdgeFadeScale;
	};

	// Creation/deletion
	// For each menu, create a new CMenuRender3DModelMgr, and then call the static release function
	// when finished with it
	CMenuRender3DModelMgr();
	static void Release(bool bImmediateDelete = false);
	ILINE static CMenuRender3DModelMgr* GetInstance() { return s_pInstance; }

	ILINE static bool IsMenu3dModelEngineSupportActive()
	{
		// If there is a character manager, then either we are in-game or we are in the front end
		// with menu 3d model engine support, both case support for rendering 3d models in menus.
		return (gEnv->pCharacterUpr) ? true : false;
	}

	// Scene
	void SetSceneSettings(const SSceneSettings& sceneSettings);

	// Add models using model data
	void AddModelData(const PodArray<CMenuRender3DModelMgr::SModelData>& modelData);

	// Updates to models
	void UpdateModel(TAddedModelIndex modelIndex, tukk pFilename, tukk pMaterialOverrideFilename,bool bUpdateModelPos=false);
	void UpdateAnim(TAddedModelIndex modelIndex, tukk pAnimName, float speedMultiplier = 1.0f, i32 layer = 0);

	bool IsModelStreamed(TAddedModelIndex modelIndex) const;

	// Specific model functions
	bool GetModelIndexFromName(tukk pName,TAddedModelIndex& outModelIndex) const;
	TAddedModelIndex AddModel(const SModelParams& modelParams);
	void AttachChildToParent(TAddedModelIndex childIndex, TAddedModelIndex parentIndex, tukk pAttachPointName);
	void SetSilhouette(TAddedModelIndex modelIndex, const Vec3& silhouetteColor);
	void SetAlpha(TAddedModelIndex modelIndex,float alpha);
	void UseSecondOffset(TAddedModelIndex modelIndex, bool enabled);
	void SetDebugScale(float debugscale) {m_fDebugScale = debugscale;}

private:

	enum ERenderSingleEntityFlags
	{
		eRSE_IsCharacter		= (1<<0),
		eRSE_Attached				=	(1<<1),
		eRSE_StreamedIn			= (1<<2),	
		eRSE_HasModel				= (1<<3),
		eRSE_Invalid			  = (1<<4)
	};

	struct SRenderSingleEntityData
	{
		SRenderSingleEntityData()
		{
			pos=ZERO;
			posOffset=ZERO;
			posSecondOffset=ZERO;
			rot=ZERO;
			secondRot=ZERO;
			continuousRot=ZERO;
			rotationOverTime=ZERO;
			silhouetteColor=ZERO;
			memset(screenRect,0,sizeof(screenRect));
			userRotScale=0.0f;
			scale=0.0f;
			secondScale=0.0f;
			alpha=0.0f;
			maxAlpha=1.0f;
			entityId=0;
			memset(firstPrecacheRoundIds, 0, sizeof(firstPrecacheRoundIds));
			topParentIndex=kAddedModelIndex_Invalid;
			parentIndex=kAddedModelIndex_Invalid;
			flags=0;
			groupId=0;
			bUseSecondOffset = false;
		}

		DrxFixedStringT<128>	animName;
		DrxFixedStringT<64>		attachmentLocation;
		DrxFixedStringT<64>		name;
		Vec3									pos;
		Vec3									posOffset;
		Vec3									posSecondOffset;
		Vec3									silhouetteColor;
		Ang3									rot;
		Ang3									secondRot;
		Ang3									continuousRot;
		Ang3									rotationOverTime;
		float									screenRect[4];
		float									userRotScale;
		float									scale;
		float									secondScale;
		float									alpha;
		float									maxAlpha;
		EntityId							entityId;
		i32										firstPrecacheRoundIds[MAX_STREAM_PREDICTION_ZONES];
		TAddedModelIndex			topParentIndex;
		TAddedModelIndex			parentIndex;
		u8									flags;
		u8									groupId;
		bool									bUseSecondOffset;
	};

	// CMenuRender3DModelMgr
	~CMenuRender3DModelMgr();
	void UpdateLightSettings(float frameTime);
	void UpdateLight(i32 lightIndex,float frameTime);
	void AttachToCharacter(TAddedModelIndex childIndex, TAddedModelIndex parentIndex, tukk pAttachPointName);
	void AttachToStaticObject(TAddedModelIndex childIndex, TAddedModelIndex parentIndex, tukk pMountPointName);
	void UpdateEntities();
	void UpdateStreaming(bool bUpdateModelPos=true);
	void SetVisibilityOfAllEntities(bool bVisible);
	bool HasAttachmentStreamedIn(SRenderSingleEntityData& rd, IAttachment* pAttachment);
	bool HasCharacterStreamedIn(SRenderSingleEntityData& rd, ICharacterInstance* pCharacter);
	bool HasStatObjStreamedIn(SRenderSingleEntityData& rd, IStatObj* pStatObj);
	bool HasEntityStreamedIn(SRenderSingleEntityData& rd, IEntity* pEntity);
	void PreCacheMaterial(SRenderSingleEntityData& rd, IEntity* pEntity,bool bIsCharacter);
	void CalcWorldPosFromScreenRect(SRenderSingleEntityData& renderEntityData);
	bool IsCharacterFile(tukk pFilename);
	void SetCharacterFlags(IEntity* pEntity);
	void ApplyMaterialOverride(IEntity* pEntity,tukk pOverrideMaterialFilename);
	void SetAsPost3dRenderObject(IEntity* pEntity,u8 groupId,float* pScreenRect);
	void FreeUnusedGroup(u8 unusedGroupId);
	void HideModel(bool bHide,i32 nEntityDataIdx,bool bUpdateModelPos=true);
	void UpdateContinuousRot(float frameTime);
	void ForceAnimationUpdate(IEntity* pEntity);
	void GetPostRenderCamera(CCamera& postRenderCamera);
	void SwapIfMinBiggerThanMax(float& minValue,float& maxValue);
	void ReleaseLights();

#if DEBUG_MENU_RENDER_3D_MODEL_MGR
	void DebugDraw();
#endif

	// IGameFrameworkListener
	void OnPostUpdate(float frameTime);
	void OnLoadGame(ILoadGame* pLoadGame) {}
	void OnSaveGame(ISaveGame* pSaveGame) {}
	void OnLevelEnd(tukk nextLevel) {}
	void OnActionEvent(const SActionEvent& event) {}
	void OnPreRender();

	// IInputEventListener
	i32 GetPriority() const { return 2; } // Need priority to be ahead of FlashFrontEnd.h which uses 1

	static CMenuRender3DModelMgr* s_pInstance;
	static CMenuRender3DModelMgr* s_pFirstNonFreedModelMgr;
	static u8									s_instanceCount;

#if DEBUG_MENU_RENDER_3D_MODEL_MGR
	static float									s_menuLightColorScale;
#endif

	DrxFixedArray<SRenderSingleEntityData,kAddedModelIndex_MaxEntities>	m_renderEntityData;
	SSceneSettings					m_sceneSettings;

	CMenuRender3DModelMgr*	m_pNextNonFreedModelMgr;
	CMenuRender3DModelMgr*	m_pPrevNonFreedModelMgr;

	u8										m_groupCount;
	u8										m_framesUntilDestroy;
	bool										m_bReleaseMe;
	bool										m_bUserRotation;
	float										m_fDebugScale;

};//------------------------------------------------------------------------------------------------

#endif