// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/VehicleSystem.h>

#include <drx3D/Act/Vehicle.h>

#include <drx3D/Act/VehicleDamageBehaviorAISignal.h>
#include <drx3D/Act/VehicleDamageBehaviorDestroy.h>
#include <drx3D/Act/VehicleDamageBehaviorDetachPart.h>
#include <drx3D/Act/VehicleDamageBehaviorEffect.h>
#include <drx3D/Act/VehicleDamageBehaviorGroup.h>
#include <drx3D/Act/VehicleDamageBehaviorHitPassenger.h>
#include <drx3D/Act/VehicleDamageBehaviorImpulse.h>
#include <drx3D/Act/VehicleDamageBehaviorMovementNotification.h>
#include <drx3D/Act/VehicleDamageBehaviorSink.h>
#include <drx3D/Act/VehicleDamageBehaviorSpawnDebris.h>
#include <drx3D/Act/VehicleDamageBehaviorIndicator.h>
#include <drx3D/Act/VehicleDamageBehaviorDisableSeatAction.h>

#include <drx3D/Act/VehiclePartBase.h>
#include <drx3D/Act/VehiclePartAnimated.h>
#include <drx3D/Act/VehiclePartAnimatedJoint.h>
#include <drx3D/Act/VehiclePartLight.h>
#include <drx3D/Act/VehiclePartPulsingLight.h>
#include <drx3D/Act/VehiclePartMassBox.h>
#include <drx3D/Act/VehiclePartStatic.h>
#include <drx3D/Act/VehiclePartSubPart.h>
#include <drx3D/Act/VehiclePartSubPartWheel.h>
#include <drx3D/Act/VehiclePartSuspensionPart.h>
#include <drx3D/Act/VehiclePartTread.h>
#include <drx3D/Act/VehiclePartAttachment.h>
#include <drx3D/Act/VehiclePartEntity.h>
#include <drx3D/Act/VehiclePartEntityDelayedDetach.h>
#include <drx3D/Act/VehiclePartParticleEffect.h>
#include <drx3D/Act/VehiclePartAnimatedChar.h>
#include <drx3D/Act/VehiclePartWaterRipplesGenerator.h>
#include <drx3D/Act/VehiclePartDetachedEntity.h>

#include <drx3D/Act/VehicleSeatSerializer.h>

#include <drx3D/Act/VehicleSeatActionLights.h>
#include <drx3D/Act/VehicleSeatActionMovement.h>
#include <drx3D/Act/VehicleSeatActionPassengerIK.h>
#include <drx3D/Act/VehicleSeatActionRotateTurret.h>
#include <drx3D/Act/VehicleSeatActionSteeringWheel.h>
#include <drx3D/Act/VehicleSeatActionWeapons.h>
#include <drx3D/Act/VehicleSeatActionWeaponsBone.h>
#include <drx3D/Act/VehicleSeatActionAnimation.h>
#include <drx3D/Act/VehicleSeatActionPassiveAnimation.h>
#include <drx3D/Act/VehicleSeatActionOrientatePartToView.h>
#include <drx3D/Act/VehicleSeatActionOrientateBoneToView.h>
#include <drx3D/Act/VehicleSeatActionRotateBone.h>
#include <drx3D/Act/VehicleSeatActionShakeParts.h>

#include <drx3D/Act/VehicleUsableActionEnter.h>
#include <drx3D/Act/VehicleUsableActionFlip.h>

#include <drx3D/Act/VehicleViewActionThirdPerson.h>
#include <drx3D/Act/VehicleViewSteer.h>
#include <drx3D/Act/VehicleViewFirstPerson.h>
#include <drx3D/Act/VehicleViewThirdPerson.h>

#include <drx3D/Act/DinrusAction.h>

//------------------------------------------------------------------------
void CVehicleSystem::RegisterVehicles(IGameFramework* gameFramework)
{
	LOADING_TIME_PROFILE_SECTION;
	IEntityClassRegistry::SEntityClassDesc serializerClass;
	serializerClass.sName = "VehicleSeatSerializer";
	serializerClass.sScriptFile = "";
	serializerClass.flags = ECLF_INVISIBLE;

	static IGameFramework::CGameObjectExtensionCreator<CVehicleSeatSerializer> createVehicleSeatSerializer;
	CDrxAction::GetDrxAction()->GetIGameObjectSystem()->RegisterExtension(serializerClass.sName, &createVehicleSeatSerializer, &serializerClass);

	// register the detached part entity

	IEntityClassRegistry::SEntityClassDesc detachedPartClass;
	detachedPartClass.sName = "VehiclePartDetached";
	detachedPartClass.sScriptFile = "Scripts/Entities/Vehicles/VehiclePartDetached.lua";
	detachedPartClass.flags = ECLF_INVISIBLE;

	static IGameFramework::CGameObjectExtensionCreator<CVehiclePartDetachedEntity> createVehicleDetachedPartEntity;
	CDrxAction::GetDrxAction()->GetIGameObjectSystem()->RegisterExtension(detachedPartClass.sName, &createVehicleDetachedPartEntity, &detachedPartClass);

	// register all the vehicles

	IDrxPak* pack = gEnv->pDrxPak;
	_finddata_t fd;
	i32 res;
	intptr_t handle;
	std::set<string> setVehicles;

	string sExt(".xml");
	string sPath("Scripts/Entities/Vehicles/Implementations/Xml/");

	if ((handle = pack->FindFirst(sPath + string("*") + sExt, &fd)) != -1)
	{
		do
		{
			if (XmlNodeRef root = GetISystem()->LoadXmlFromFile(sPath + string(fd.name)))
			{
				tukk name = root->getAttr("name");
				if (name[0])
				{
					// Allow the name to contain relative path, but use only the name part as class name.
					string className(PathUtil::GetFile(name));

					// register only once
					std::pair<std::set<string>::iterator, bool> result = setVehicles.insert(className);
					if (result.second)
					{
						IEntityClassRegistry::SEntityClassDesc vehicleClass;
						vehicleClass.sName = className.c_str();

						char scriptName[1024];

						tukk isOld = root->getAttr("isOld");
						if (!strcmp("1", isOld))
							drx_sprintf(scriptName, "Scripts/Entities/Vehicles/Old/VehiclePool.lua");
						else
							drx_sprintf(scriptName, "Scripts/Entities/Vehicles/VehiclePool.lua");

						bool show = true;
						if (root->getAttr("show", show))
						{
							if (!show && VehicleCVars().v_show_all == 0)
								vehicleClass.flags |= ECLF_INVISIBLE;
						}

						vehicleClass.sScriptFile = scriptName;

						static IGameFramework::CGameObjectExtensionCreator<CVehicle> vehicleCreator;
						CDrxAction::GetDrxAction()->GetIGameObjectSystem()->RegisterExtension(name, &vehicleCreator, &vehicleClass);
						m_classes.insert(TVehicleClassMap::value_type(name, &vehicleCreator));
					}
					else
						DrxLog("Vehicle <%s> already registered", name);
				}
				else
				{
					DrxLog("VehicleSystem: %s is missing 'name' attribute, skipping", fd.name);
				}
			}
			res = pack->FindNext(handle, &fd);
		}
		while (res >= 0);

		pack->FindClose(handle);
	}

#define REGISTER_VEHICLEOBJECT(name, obj)                    \
  REGISTER_FACTORY((IVehicleSystem*)this, name, obj, false); \
  obj::m_objectId = this->AssignVehicleObjectId(name);

	// register other factories

	// vehicle views
	REGISTER_VEHICLEOBJECT("ActionThirdPerson", CVehicleViewActionThirdPerson);
	REGISTER_VEHICLEOBJECT("SteerThirdPerson", CVehicleViewSteer);
	REGISTER_VEHICLEOBJECT("FirstPerson", CVehicleViewFirstPerson);
	REGISTER_VEHICLEOBJECT("ThirdPerson", CVehicleViewThirdPerson);

	// vehicle parts
	REGISTER_VEHICLEOBJECT("Base", CVehiclePartBase);
	REGISTER_VEHICLEOBJECT("Animated", CVehiclePartAnimated);
	REGISTER_VEHICLEOBJECT("AnimatedJoint", CVehiclePartAnimatedJoint);
	REGISTER_VEHICLEOBJECT("SuspensionPart", CVehiclePartSuspensionPart);
	REGISTER_VEHICLEOBJECT("Light", CVehiclePartLight);
	REGISTER_VEHICLEOBJECT("PulsingLight", CVehiclePartPulsingLight);
	REGISTER_VEHICLEOBJECT("MassBox", CVehiclePartMassBox);
	REGISTER_VEHICLEOBJECT("Static", CVehiclePartStatic);
	REGISTER_VEHICLEOBJECT("SubPart", CVehiclePartSubPart);
	REGISTER_VEHICLEOBJECT("SubPartWheel", CVehiclePartSubPartWheel);
	REGISTER_VEHICLEOBJECT("Tread", CVehiclePartTread);
	REGISTER_VEHICLEOBJECT("EntityAttachment", CVehiclePartEntityAttachment);
	REGISTER_VEHICLEOBJECT("Entity", CVehiclePartEntity);
	REGISTER_VEHICLEOBJECT("EntityDelayedDetach", CVehiclePartEntityDelayedDetach);
	REGISTER_VEHICLEOBJECT("ParticleEffect", CVehiclePartParticleEffect);
	REGISTER_VEHICLEOBJECT("AnimatedChar", CVehiclePartAnimatedChar);
	REGISTER_VEHICLEOBJECT("WaterRipplesGenerator", CVehiclePartWaterRipplesGenerator);

	// vehicle damage behaviors
	REGISTER_VEHICLEOBJECT("AISignal", CVehicleDamageBehaviorAISignal);
	REGISTER_VEHICLEOBJECT("Destroy", CVehicleDamageBehaviorDestroy);
	REGISTER_VEHICLEOBJECT("DetachPart", CVehicleDamageBehaviorDetachPart);
	REGISTER_VEHICLEOBJECT("Effect", CVehicleDamageBehaviorEffect);
	REGISTER_VEHICLEOBJECT("Group", CVehicleDamageBehaviorGroup);
	REGISTER_VEHICLEOBJECT("HitPassenger", CVehicleDamageBehaviorHitPassenger);
	REGISTER_VEHICLEOBJECT("Impulse", CVehicleDamageBehaviorImpulse);
	REGISTER_VEHICLEOBJECT("Indicator", CVehicleDamageBehaviorIndicator);
	REGISTER_VEHICLEOBJECT("MovementNotification", CVehicleDamageBehaviorMovementNotification);
	REGISTER_VEHICLEOBJECT("Sink", CVehicleDamageBehaviorSink);
	REGISTER_VEHICLEOBJECT("SpawnDebris", CVehicleDamageBehaviorSpawnDebris);
	REGISTER_VEHICLEOBJECT("DisableSeatAction", CVehicleDamageBehaviorDisableSeatAction);

	// seat actions
	REGISTER_VEHICLEOBJECT("Lights", CVehicleSeatActionLights);
	REGISTER_VEHICLEOBJECT("Movement", CVehicleSeatActionMovement);
	REGISTER_VEHICLEOBJECT("PassengerIK", CVehicleSeatActionPassengerIK);
	REGISTER_VEHICLEOBJECT("RotateTurret", CVehicleSeatActionRotateTurret);
	REGISTER_VEHICLEOBJECT("SteeringWheel", CVehicleSeatActionSteeringWheel);
	REGISTER_VEHICLEOBJECT("Weapons", CVehicleSeatActionWeapons);
	REGISTER_VEHICLEOBJECT("WeaponsBone", CVehicleSeatActionWeaponsBone);
	REGISTER_VEHICLEOBJECT("Animation", CVehicleSeatActionAnimation);
	REGISTER_VEHICLEOBJECT("PassiveAnimation", CVehicleSeatActionPassiveAnimation);
	REGISTER_VEHICLEOBJECT("OrientatePartToView", CVehicleSeatActionOrientatePartToView);
	REGISTER_VEHICLEOBJECT("OrientateBoneToView", CVehicleSeatActionOrientateBoneToView);
	REGISTER_VEHICLEOBJECT("RotateBone", CVehicleSeatActionRotateBone);
	REGISTER_VEHICLEOBJECT("ShakeParts", CVehicleSeatActionShakeParts);

	// actions
	REGISTER_FACTORY((IVehicleSystem*)this, "Enter", CVehicleUsableActionEnter, false);
	REGISTER_FACTORY((IVehicleSystem*)this, "Flip", CVehicleUsableActionFlip, false);
}
