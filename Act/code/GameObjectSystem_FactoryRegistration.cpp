// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/GameObjectSystem.h>

#include <drx3D/Act/WorldQuery.h>
#include <drx3D/Act/Interactor.h>
#include <drx3D/Act/GameVolume_Water.h>
#include <drx3D/Act/MannequinObject.h>
#include <drx3D/Act/EntityContainerObject.h>

#include <drx3D/CoreX/Game/IGameVolumes.h>
#include <drx3D/CoreX/Game/IGameFramework.h>

// entityClassString can be different from extensionClassName to allow mapping entity classes to differently
// named C++ Classes
#define REGISTER_GAME_OBJECT_EXTENSION(framework, entityClassString, extensionClassName, script)  \
  {                                                                                               \
    IEntityClassRegistry::SEntityClassDesc clsDesc;                                               \
    clsDesc.sName = entityClassString;                                                            \
    clsDesc.sScriptFile = script;                                                                 \
    struct C ## extensionClassName ## Creator : public IGameObjectExtensionCreatorBase            \
    {                                                                                             \
      IGameObjectExtension* Create(IEntity *pEntity)                                            \
      {                                                                                           \
        return pEntity->GetOrCreateComponentClass<C ## extensionClassName>();                          \
      }                                                                                           \
      void GetGameObjectExtensionRMIData(uk * ppRMI, size_t * nCount)                           \
      {                                                                                           \
        C ## extensionClassName::GetGameObjectExtensionRMIData(ppRMI, nCount);                    \
      }                                                                                           \
    };                                                                                            \
    static C ## extensionClassName ## Creator _creator;                                           \
    framework->GetIGameObjectSystem()->RegisterExtension(entityClassString, &_creator, &clsDesc); \
  }                                                                                               \

#define HIDE_FROM_EDITOR(className)                                                           \
  { IEntityClass* pItemClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(className); \
    pItemClass->SetFlags(pItemClass->GetFlags() | ECLF_INVISIBLE); }

#define REGISTER_EDITOR_VOLUME_CLASS(frameWork, className)                                         \
  {                                                                                                \
    IGameVolumes* pGameVolumes = frameWork->GetIGameVolumesUpr();                              \
    IGameVolumesEdit* pGameVolumesEdit = pGameVolumes ? pGameVolumes->GetEditorInterface() : NULL; \
    if (pGameVolumesEdit != NULL)                                                                  \
    {                                                                                              \
      pGameVolumesEdit->RegisterEntityClass(className);                                            \
    }                                                                                              \
  }

void CGameObjectSystem::RegisterFactories(IGameFramework* pFrameWork)
{
	LOADING_TIME_PROFILE_SECTION;
	REGISTER_FACTORY(pFrameWork, "WorldQuery", CWorldQuery, false);
	REGISTER_FACTORY(pFrameWork, "Interactor", CInteractor, false);

	CDrxFile file;
	if (file.Open("Scripts/Entities/Environment/WaterVolume.lua", "r"))
	{
		file.Close();
		REGISTER_GAME_OBJECT_EXTENSION(pFrameWork, "WaterVolume", GameVolume_Water, "Scripts/Entities/Environment/WaterVolume.lua");

		HIDE_FROM_EDITOR("WaterVolume");
		REGISTER_EDITOR_VOLUME_CLASS(pFrameWork, "WaterVolume");
	}

	IEntityClassRegistry::SEntityClassDesc clsDesc;
	clsDesc.sName = "MannequinEntity";

	clsDesc.editorClassInfo.sCategory = "Animation";
	clsDesc.editorClassInfo.sIcon = "User.bmp";
	clsDesc.editorClassInfo.bIconOnTop = true;

	// If we load a legacy project we still want to expose the legacy entity.
	if(gEnv->pGameFramework->GetIGame() == nullptr)
		clsDesc.flags |= ECLF_INVISIBLE;

	struct CObjectCreator
	{
		static IEntityComponent* Create(IEntity* pEntity, SEntitySpawnParams& params, uk pUserData)
		{
			return pEntity->GetOrCreateComponentClass<CMannequinObject>();
		}
	};
	clsDesc.pUserProxyCreateFunc = &CObjectCreator::Create;
	gEnv->pEntitySystem->GetClassRegistry()->RegisterStdClass(clsDesc);

	// Special case since entity container is Hunt specific, skip entity registration if no script is available
	if (file.Open("Scripts/Entities/Containers/EntityContainerObject.lua", "r"))
	{
		file.Close();
		REGISTER_GAME_OBJECT_EXTENSION(pFrameWork, "EntityContainerObject", EntityContainerObject, "Scripts/Entities/Containers/EntityContainerObject.lua");
	}
}
