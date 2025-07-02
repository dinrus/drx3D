// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$

-------------------------------------------------------------------------
История:
- 3:4:2005   15:11 : Created by Márcio Martins

*************************************************************************/
#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/ItemSharedParams.h>
#include <drx3D/Game/IItem.h>
#include <drx3D/Game/ItemDefinitions.h>
#include <drx3D/Game/ItemResourceCache.h>
#include <drx3D/Game/Utility/DesignerWarning.h>
#include <drx3D/Game/GameCVars.h>
#include <drx3D/Game/WeaponSharedParams.h>
#include <drx3D/Game/GameXmlParamReader.h>
#include <drx3D/Game/ItemParamsRegistrationOperators.h>
#include <drx3D/Act/IDrxMannequin.h>

#undef ReadOptionalParams
#define ReadOptionalParams(paramString, param) {									\
	CGameXmlParamReader optionalReader(rootNode);									\
	XmlNodeRef optionalParamsNode= optionalReader.FindFilteredChild(paramString);	\
	if(optionalParamsNode) {														\
		if(!p##param) { p##param = new S##param; }									\
		Read##param(optionalParamsNode);											\
	} else { SAFE_DELETE(p##param); } }										

IMPLEMENT_OPERATORS_WITH_ARRAYS(AIMLOOK_PARAMS_MEMBERS, AIMLOOK_ARRAY_MEMBERS, SAimLookParameters)

AUTOENUM_BUILDNAMEARRAY(WeaponAimAnim::s_weaponAimAnimNames, WeaponAimAnimList);
AUTOENUM_BUILDNAMEARRAY(WeaponAnimLayerID::s_weaponAnimLayerIDNames, WeaponAnimLayerIDList);

static SLaserParams MakeDefaultLaserParameters()
{
	SLaserParams laserGuiderParams;
	laserGuiderParams.laser_dot[0] = "Crysis2_weapon_attachments.lasersight.aimdot";
	laserGuiderParams.laser_dot[1] = "Crysis2_weapon_attachments.lasersight.aimdot";
	laserGuiderParams.laser_geometry_tp = "objects/weapons/attachments/laser_beam/laser_beam.cgf";
	laserGuiderParams.laser_range[0] = 30.0f;
	laserGuiderParams.laser_range[1] = 30.0f;
	laserGuiderParams.laser_thickness[0] = 1.f;
	laserGuiderParams.laser_thickness[1] = 1.f;
	laserGuiderParams.show_dot = false;
	return laserGuiderParams;
}

static const SLaserParams s_defaultLaserParameters = MakeDefaultLaserParameters();

SAimLookParameters::SAimLookParameters()
{
	Reset();
}

void SAimLookParameters::Reset()
{
	easeFactorInc = 5.25f;
	easeFactorDec = 10.0f;
	strafeScopeFactor = 0.0f;
	rotateScopeFactor = 5.0f;
	velocityInterpolationMultiplier = 1.2f;
	velocityLowPassFilter = 6.0f;
	accelerationSmoothing = 0.8f;
	accelerationFrontAugmentation = 3.0f;
	verticalVelocityScale = -0.3f;
	sprintCameraAnimation = 0;
	
	for (i32 i=0; i<WeaponAimAnim::Total; i++)
	{
		blendFactors[i] = 1.0f;
	}
}

void SAimLookParameters::Read(const XmlNodeRef& paramsNode)
{
	CGameXmlParamReader reader(paramsNode);

	reader.ReadParamValue<float>("easeFactorInc", easeFactorInc);
	reader.ReadParamValue<float>("easeFactorDec", easeFactorDec);
	reader.ReadParamValue<float>("strafeScopeFactor", strafeScopeFactor);
	reader.ReadParamValue<float>("rotateScopeFactor", rotateScopeFactor);
	reader.ReadParamValue<float>("velocityInterpolationMultiplier", velocityInterpolationMultiplier);
	reader.ReadParamValue<float>("velocityLowPassFilter", velocityLowPassFilter);
	reader.ReadParamValue<float>("accelerationSmoothing", accelerationSmoothing);
	reader.ReadParamValue<float>("accelerationFrontAugmentation", accelerationFrontAugmentation);
	reader.ReadParamValue<float>("verticalVelocityScale", verticalVelocityScale);
	reader.ReadParamValue<bool>("sprintCameraAnimation", sprintCameraAnimation);

	for (i32 i = 0; i < WeaponAimAnim::Total; i++)
	{
		reader.ReadParamValue<float>(WeaponAimAnim::s_weaponAimAnimNames[i], blendFactors[i]);
	}
}



CItemSharedParams::CItemSharedParams() 
: pLaserParams(NULL)
,	pFlashLightParams(NULL)
, pMountParams(NULL)
{

}

CItemSharedParams::~CItemSharedParams()
{
	SAFE_DELETE(pMountParams);
	SAFE_DELETE(pLaserParams);
	SAFE_DELETE(pFlashLightParams);
};

void CItemSharedParams::GetMemoryUsage(IDrxSizer *pSizer) const
{		
	pSizer->AddObject(this, sizeof(*this));
	pSizer->AddObject(params);	
	pSizer->AddContainer(helpers);	
	pSizer->AddContainer(geometry);	
	pSizer->AddContainer(damageLevels);	
	pSizer->AddContainer(accessoryparams);
	pSizer->AddObject(pMountParams);
	pSizer->AddObject(pLaserParams);
	pSizer->AddContainer(animationPrecache);

	//pSizer->AddContainer(m_cachedAnimationResourceNames);
}

//------------------------------------------------------------------------
bool CItemSharedParams::ReadItemParams(const XmlNodeRef& rootNode)
{
	if (rootNode == NULL)
		return false;

	CGameXmlParamReader reader(rootNode);

	XmlNodeRef paramsNode = reader.FindFilteredChild("params");
	if(paramsNode)		
	{
		ReadParams(paramsNode);
	}

	XmlNodeRef geometryNode	= reader.FindFilteredChild("geometry");
	if(geometryNode)
	{
		ReadGeometry(geometryNode);
	}

	XmlNodeRef accessoriesNode = reader.FindFilteredChild("accessories");
	if(accessoriesNode)
	{
		ReadAccessories(accessoriesNode);
	}

	XmlNodeRef damagelevelsNode = reader.FindFilteredChild("damagelevels");
	if(damagelevelsNode)	
	{
		ReadDamageLevels(damagelevelsNode);
	}

	XmlNodeRef accessoryAmmoNode = reader.FindFilteredChild("accessoryAmmo");
	if(accessoryAmmoNode)
	{
		ReadAccessoryAmmo(accessoryAmmoNode);
	}
	
	ReadOptionalParams("laser", LaserParams);
	ReadOptionalParams("flashlight", FlashLightParams);
			
	return true;
}

//------------------------------------------------------------------------
void CItemSharedParams::ReadOverrideItemParams(const XmlNodeRef& overrideParamsNode)
{
	if (overrideParamsNode == NULL)
		return;

	CGameXmlParamReader reader(overrideParamsNode);

	XmlNodeRef geometryNode	= reader.FindFilteredChild("geometry");
	if(geometryNode)
	{
		ReadGeometry(geometryNode);
	}
}

//------------------------------------------------------------------------			
bool CItemSharedParams::ReadParams(const XmlNodeRef& paramsNode)
{
	params.Reset();

	CGameXmlParamReader reader(paramsNode);

	reader.ReadParamValue<bool>("selectable", params.selectable);
	reader.ReadParamValue<bool>("droppable", params.droppable);
	reader.ReadParamValue<bool>("pickable", params.pickable);
	reader.ReadParamValue<bool>("mountable", params.mountable);
	reader.ReadParamValue<bool>("usable", params.usable);
	reader.ReadParamValue<bool>("giveable", params.giveable);
	reader.ReadParamValue<bool>("unique", params.unique);
	reader.ReadParamValue<bool>("usable_under_water", params.usable_under_water);
	reader.ReadParamValue<bool>("can_overcharge", params.can_overcharge);
	reader.ReadParamValue<float>("mass", params.mass);
	reader.ReadParamValue<float>("drop_impulse", params.drop_impulse);

	params.tag = reader.ReadParamValue("tag", params.tag.c_str());
	params.itemClass = reader.ReadParamValue("itemClass", params.itemClass.c_str());

	params.adbFile = reader.ReadParamValue("adb", params.adbFile.c_str());
	params.soundAdbFile = reader.ReadParamValue("soundAdb", params.soundAdbFile.c_str());

	params.actionControllerFile = reader.ReadParamValue("actionController", params.actionControllerFile.c_str());

	PrefixPathIfFilename("Animations/Mannequin/ADB/", params.adbFile);
	PrefixPathIfFilename("Animations/Mannequin/ADB/", params.soundAdbFile);
	PrefixPathIfFilename("Animations/Mannequin/ADB/", params.actionControllerFile);

	reader.ReadParamValue<float>("select_override", params.select_override);
	
	params.attachment[IItem::eIH_Right] = reader.ReadParamValue("attachment_right", params.attachment[IItem::eIH_Right].c_str());
	params.attachment[IItem::eIH_Left] = reader.ReadParamValue("attachment_left", params.attachment[IItem::eIH_Left].c_str());
	params.aiAttachment[IItem::eIH_Right] = reader.ReadParamValue("ai_attachment_right", params.aiAttachment[IItem::eIH_Right].c_str());
	params.aiAttachment[IItem::eIH_Left] = reader.ReadParamValue("ai_attachment_left", params.aiAttachment[IItem::eIH_Left].c_str());

	reader.ReadParamValue<bool>("auto_droppable", params.auto_droppable);
	reader.ReadParamValue<bool>("auto_pickable", params.auto_pickable);
	reader.ReadParamValue<bool>("has_first_select", params.has_first_select);
	reader.ReadParamValue<bool>("attach_to_back", params.attach_to_back);
	reader.ReadParamValue<i32>("scopeAttachment", params.scopeAttachment);
	reader.ReadParamValue<bool>("attachment_gives_ammo", params.attachment_gives_ammo);
	reader.ReadParamValue<bool>("can_ledge_grab", params.can_ledge_grab);
	reader.ReadParamValue<bool>("can_rip_off", params.can_rip_off);
	reader.ReadParamValue<bool>("check_clip_size_after_drop", params.check_clip_size_after_drop);
	reader.ReadParamValue<bool>("check_bonus_ammo_after_drop", params.check_bonus_ammo_after_drop);
	reader.ReadParamValue("remove_on_drop", params.remove_on_drop);

	Vec3 rot_offset(0.0f, 0.0f, 0.0f);
	reader.ReadParamValue<Vec3>("fp_rot_offset", rot_offset);
	params.fp_rot_offset.SetRotationXYZ(Ang3(DEG2RAD(rot_offset.x), DEG2RAD(rot_offset.y), DEG2RAD(rot_offset.z)));
	reader.ReadParamValue<Vec3>("fp_offset", params.fp_offset);

	params.display_name = reader.ReadParamValue("display_name");
	params.bone_attachment_01 = reader.ReadParamValue("bone_attachment_01");
	params.bone_attachment_02 = reader.ReadParamValue("bone_attachment_02");

	reader.ReadParamValue<bool>("fast_select", params.fast_select);
	reader.ReadParamValue<bool>("select_delayed_grab_3P", params.select_delayed_grab_3P);

	reader.ReadParamValue<float>("sprintToFireDelay", params.sprintToFireDelay);
	reader.ReadParamValue<float>("sprintToZoomDelay", params.sprintToZoomDelay);
	reader.ReadParamValue<float>("sprintToMeleeDelay", params.sprintToMeleeDelay);
	reader.ReadParamValue<float>("autoReloadDelay", params.autoReloadDelay);
	reader.ReadParamValue<float>("runToSprintBlendTime", params.runToSprintBlendTime);
	reader.ReadParamValue<float>("sprintToRunBlendTime", params.sprintToRunBlendTime);
	reader.ReadParamValue<float>("selectTimeMultiplier", params.selectTimeMultiplier);
	reader.ReadParamValue<float>("zoomTimeMultiplier", params.zoomTimeMultiplier);

	params.weaponStats.ReadStats(paramsNode);

	XmlNodeRef mountedParamsNode = reader.FindFilteredChild("mount");
	if (mountedParamsNode)
	{
		if(!pMountParams)
		{
			pMountParams = new SMountParams;
		}
		
		pMountParams->Reset();

		CGameXmlParamReader mountedParamsReader(mountedParamsNode);

		pMountParams->pivot = mountedParamsReader.ReadParamValue("pivot");
		mountedParamsReader.ReadParamValue<float>("body_distance", pMountParams->body_distance);
		pMountParams->left_hand_helper = mountedParamsReader.ReadParamValue("left_hand_helper");
		pMountParams->right_hand_helper = mountedParamsReader.ReadParamValue("right_hand_helper");
		mountedParamsReader.ReadParamValue<float>("ground_distance", pMountParams->ground_distance);
		mountedParamsReader.ReadParamValue<Vec3>("fpBody_offset", pMountParams->fpBody_offset);
		mountedParamsReader.ReadParamValue<Vec3>("fpBody_offset_ironsight", pMountParams->fpBody_offset_ironsight);
		pMountParams->rotate_sound_fp = mountedParamsReader.ReadParamValue("rotate_sound_fp");
		pMountParams->rotate_sound_tp = mountedParamsReader.ReadParamValue("rotate_sound_tp");
	}
	else 
	{
		SAFE_DELETE(pMountParams);
	}

	XmlNodeRef aimAnimsNode = reader.FindFilteredChild("aimAnims");
	if (aimAnimsNode)
	{
		CGameXmlParamReader aimAnimsReader(aimAnimsNode);
		
		params.hasAimAnims = true;
		aimAnimsReader.ReadParamValue<float>("ironsightAimAnimFactor", params.ironsightAimAnimFactor);
		
		params.aimAnims.Read( aimAnimsNode );
	}

	animationPrecache.resize(0);

	XmlNodeRef animPreCacheNode = reader.FindFilteredChild("animPrecache");
	if (animPreCacheNode)
	{
		animationGroup = animPreCacheNode->getAttr("name");

		CGameXmlParamReader animPreCacheReader(animPreCacheNode);

		u32k numPrecaches = animPreCacheReader.GetUnfilteredChildCount();
		animationPrecache.reserve(numPrecaches);
		for (u32 i = 0; i < numPrecaches; i++)
		{
			XmlNodeRef dbaNode = animPreCacheReader.GetFilteredChildAt(i);
			if (dbaNode)
			{
				SAnimationPreCache entry;
				entry.DBAfile = dbaNode->getAttr("DBAfile");
				dbaNode->getAttr("thirdPerson", entry.thirdPerson);

				animationPrecache.push_back(entry);
			}
		}
	}

	XmlNodeRef mountedAnimsNode = reader.FindFilteredChild("mountedAimAnims");
	if (mountedAnimsNode)
	{
		params.mountedAimAnims.Read( mountedAnimsNode );
		tukk tpToken[MountedTPAimAnim::Total] =
		{
			"TP_Up", "TP_Down"
		};

		CGameXmlParamReader mountedAnimsReader( mountedAnimsNode );
		for (i32 i=0; i < MountedTPAimAnim::Total; i++)
		{
			params.mountedTPAimAnims[i] = mountedAnimsReader.ReadParamValue(tpToken[i]);
		}
	}

	XmlNodeRef aimLookNode = reader.FindFilteredChild("aimLookParams");
	if (aimLookNode)
	{
		params.aimLookParams.Read(aimLookNode);
	}

	XmlNodeRef crosshairTextureNode = reader.FindFilteredChild("CrosshairTexture");
	if (crosshairTextureNode)
	{
		params.crosshairTexture = crosshairTextureNode->getAttr("name");
	}
	
	return true;
}

//------------------------------------------------------------------------
void SAimAnimsBlock::Reset()
{
	for (i32 i = 0; i < WeaponAimAnim::Total; i++)
	{
		anim[i].clear();
	}
}

//------------------------------------------------------------------------
void SAimAnimsBlock::Read( const XmlNodeRef& paramsNode )
{
	CGameXmlParamReader reader( paramsNode );

	for (i32 i = 0; i < WeaponAimAnim::Total; i++)
	{
		anim[i] = reader.ReadParamValue(WeaponAimAnim::s_weaponAimAnimNames[i]);
	}
}


//------------------------------------------------------------------------
i32 GetItemCategoryType( tukk category )
{
	i32 itemCategoryType = eICT_None;

	const DrxFixedStringT<32> categoryStr(category);
	if (categoryStr.find("primary") != -1)
		itemCategoryType = eICT_Primary;

	if (categoryStr.find("secondary") != -1)
		itemCategoryType |= eICT_Secondary;

	if (categoryStr.find("heavy") != -1)
		itemCategoryType |= eICT_Heavy;

	if (categoryStr.find("grenade") != -1)
		itemCategoryType |= eICT_Grenade;

	if (categoryStr.find("medium") != -1)
		itemCategoryType |= eICT_Medium;

	if (categoryStr.find("explosive") != -1)
		itemCategoryType |= eICT_Explosive;

	if (categoryStr.find("special") != -1)
		itemCategoryType |= eICT_Special;

	return itemCategoryType;
}

//------------------------------------------------------------------------
i32 GetItemCategoryTypeByClass( IEntityClass* pClass )
{
	IItemSystem* pItemSystem = g_pGame->GetIGameFramework()->GetIItemSystem();
	IF_UNLIKELY(!pItemSystem)
	{
		return eICT_None;
	}

	IF_UNLIKELY(!pClass)
	{
		return eICT_None;
	}

	tukk cat = pItemSystem->GetItemCategory(pClass->GetName());
	return GetItemCategoryType(cat);
}

//------------------------------------------------------------------------
bool CItemSharedParams::ReadGeometry(const XmlNodeRef& paramsNode)
{
	FUNCTION_PROFILER(GetISystem(), PROFILE_GAME);

	// read the helpers
	helpers.resize(0);
	geometry.resize(0);

	CGameXmlParamReader reader(paramsNode);

	XmlNodeRef attachmentsNode = reader.FindFilteredChild("boneattachments");
	if (attachmentsNode)
	{
		CGameXmlParamReader attachmentsReader(attachmentsNode);
		i32k attachmentCount = attachmentsReader.GetUnfilteredChildCount();
		helpers.reserve(attachmentCount);
		
		SAttachmentHelper helper;
		for (i32 i = 0; i < attachmentCount; i++)
		{
			XmlNodeRef childNode = attachmentsReader.GetFilteredChildAt(i);
			if (childNode)
			{
				tukk slot = childNode->getAttr("target");
				tukk name = childNode->getAttr("name");
				tukk bone = childNode->getAttr("bone");

				i32 islot = TargetToSlot(slot);
				if (islot == eIGS_Last)
				{
					GameWarning("Invalid attachment helper target for item! Skipping...");
					continue;
				}

				if (!name || !bone)
				{
					GameWarning("Invalid attachment helper specification for item! Skipping...");
					continue;
				}

				helper.name = name;
				helper.bone = bone;
				helper.slot = islot;

				helpers.push_back(helper);
			}
		}
	}

	i32k geometryCount = reader.GetUnfilteredChildCount();
	geometry.reserve(geometryCount);

	for (i32 i = 0; i < geometryCount; i++)
	{
		XmlNodeRef childNode = reader.GetFilteredChildAt(i);
		if (childNode)
		{
			i32 islot = TargetToSlot(childNode->getTag());

			if (islot != eIGS_Last)
			{
				SGeometryDef geometryDef;

				geometryDef.slot = islot;
				childNode->getAttr("position", geometryDef.pos);
				childNode->getAttr("angles", geometryDef.angles);
				childNode->getAttr("scale", geometryDef.scale);
				childNode->getAttr("useParentMaterial", geometryDef.useParentMaterial);

				tukk hand = childNode->getAttr("hand");
				geometryDef.modelPath = childNode->getAttr("name");
				geometryDef.material = childNode->getAttr("material");

				i32 useStreaming = 1;
				childNode->getAttr("useStreaming", useStreaming);
				geometryDef.useCgfStreaming = (useStreaming != 0);

				geometryDef.angles = DEG2RAD(geometryDef.angles);

				geometry.push_back(geometryDef);
			}
		}

	}

	return true;
}

//------------------------------------------------------------------------
const SGeometryDef* CItemSharedParams::GetGeometryForSlot(eGeometrySlot geomSlot) const
{
	i32 numGeometry = geometry.size();

	for(i32 i = 0; i < numGeometry; i++)
	{
		if(geometry[i].slot == geomSlot)
		{
			return &geometry[i];
		}
	}

	return NULL;
}

//------------------------------------------------------------------------
void CItemSharedParams::LoadGeometryForItem(CItem* pItem, eGeometrySlot skipSlot) const
{
	i32 geomSlots = geometry.size();

	for(i32 i = 0; i < geomSlots; i++)
	{
		const SGeometryDef& geomDef = geometry[i];

		assert(geomDef.slot < eIGS_Last);

		if(geomDef.slot != skipSlot)
		{
			pItem->SetGeometry(geomDef.slot, geomDef.modelPath, geomDef.material, geomDef.useParentMaterial, geomDef.pos, geomDef.angles, geomDef.scale, false);
		}
	}
}

//------------------------------------------------------------------------
bool CItemSharedParams::ReadDamageLevels(const XmlNodeRef& paramsNode)
{
	damageLevels.resize(0);

	CGameXmlParamReader reader(paramsNode);

	i32k damageLevelsCount = reader.GetUnfilteredChildCount();
	damageLevels.reserve(damageLevelsCount);

	for (i32 i = 0; i < damageLevelsCount; i++)
	{
		XmlNodeRef levelParamsNode = reader.GetFilteredChildAt(i);
		if (levelParamsNode)
		{
			SDamageLevel level;

			levelParamsNode->getAttr("min_health", level.min_health);
			levelParamsNode->getAttr("max_health", level.max_health);
			levelParamsNode->getAttr("scale", level.scale);

			tukk helper = levelParamsNode->getAttr("helper");
			tukk effect = levelParamsNode->getAttr("effect");

			if (effect)
				level.effect = effect;
			if (helper)
				level.helper = helper;

			damageLevels.push_back(level);
		}
	}

	return true;
}

//------------------------------------------------------------------------
bool CItemSharedParams::ReadAccessories(const XmlNodeRef& paramsNode)
{
	accessoryparams.clear();
	initialSetup.clear();
	defaultAccessories.clear();

	CGameXmlParamReader reader(paramsNode);

	i32k accessoryCount = reader.GetUnfilteredChildCount();
	accessoryparams.reserve(accessoryCount);

	for (i32 i = 0; i < accessoryCount; i++)
	{
		XmlNodeRef childNode = reader.GetFilteredChildAt(i);
		if (childNode == NULL)
			continue;

		tukk childName = childNode->getTag();

		if (!stricmp(childName, "accessory"))
		{
			SAccessoryParams params;

			if (!ReadAccessoryParams(childNode, &params))
				continue;

			accessoryparams.push_back(params);
		}
		else if (!stricmp(childName, "defaultAccessoires"))
		{
			CGameXmlParamReader defaultAccessoriesReader(childNode);
			i32k numOfAttachments = defaultAccessoriesReader.GetUnfilteredChildCount();
			for (i32 k = 0; k < numOfAttachments; k++)
			{
				XmlNodeRef accessoryNode = defaultAccessoriesReader.GetFilteredChildAt(k);
				if (accessoryNode)
				{
					if (!stricmp(accessoryNode->getTag(), "accessory"))
					{
						tukk name = accessoryNode->getAttr("name");
						if (!name || !name[0])
						{
							GameWarning("Missing accessory name for default accessory in item! Skipping...");
							continue;
						}
						IEntityClass* pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(name);
						if (pClass)
							defaultAccessories.push_back(pClass);
						else
							GameWarning("Accessory class '%s' not found", name);
					}
					else
						GameWarning("Unknown param '%s' in initial setup for item! Skipping...", accessoryNode->getTag());
				}
			}
		}
		else if (!stricmp(childName, "initialsetup"))
		{
			CGameXmlParamReader initialSetupReader(childNode);

			i32k numOfAttachments = initialSetupReader.GetUnfilteredChildCount();
			i32 insertedAttachments = 0;
			i32k maxNumberOfAttachments = initialSetup.max_size();
			bool tooManyAttachments = false;

			for (i32 k = 0; k < numOfAttachments; k++)
			{
				XmlNodeRef accessoryNode = initialSetupReader.GetFilteredChildAt(k);
				if(accessoryNode == NULL)
					continue;

				if (insertedAttachments >= maxNumberOfAttachments)
				{
					tooManyAttachments = true;
					continue;
				}

				if (!stricmp(accessoryNode->getTag(), "accessory"))
				{
					tukk name = accessoryNode->getAttr("name");
					if (!name || !name[0])
					{
						GameWarning("Missing accessory name for initial setup in item! Skipping...");
						continue;
					}

					IEntityClass* pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(name);
					if (pClass)
					{
						initialSetup.push_back(pClass);
						insertedAttachments++;
					}
					else
					{
						GameWarning("Accessory class '%s' not found", name);
						continue;
					}
				}
				else
				{
					GameWarning("Unknown param '%s' in initial setup for item! Skipping...", accessoryNode->getTag());
					continue;
				}
			}

			if (tooManyAttachments)
			{
				DRX_ASSERT_MESSAGE(!tooManyAttachments, "This weapon has too many initial attachments");
				GameWarning("Too many initial attachments defined for this weapon, maximum allowed is (%d)", initialSetup.max_size());
			}
		}
	}

	return true;
}

//------------------------------------------------------------------------
bool CItemSharedParams::ReadAccessoryParams(const XmlNodeRef& paramsNode, SAccessoryParams* params)
{
	tukk name = paramsNode->getAttr("name");
	if (!name || !name[0])
	{
		GameWarning("Missing accessory name for item! Skipping...");
		return false;
	}

	params->pAccessoryClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(name);

	if(!params->pAccessoryClass)
	{
		GameWarning("'%s' is not a valid accessory type", name);
		return false;
	}
	
	CGameXmlParamReader reader(paramsNode);

	XmlNodeRef showNode = reader.FindFilteredChild("show");
	XmlNodeRef attachNode = reader.FindFilteredChild("attach");
	XmlNodeRef detachNode = reader.FindFilteredChild("detach");
	XmlNodeRef statsNode = reader.FindFilteredChild("stats_changes");

	if ((attachNode == NULL) || (detachNode == NULL))
	{
		GameWarning("Missing attach/detach details for accessory '%s' in item! Skipping...", name);
		return false;
	}

	params->attach_helper = attachNode->getAttr("helper");
	params->select = attachNode->getAttr("select");
	params->select_empty = attachNode->getAttr("select_empty");

	if (showNode)
		params->show_helper = showNode->getAttr("helper");

	if(statsNode)
	{
		params->weaponStats.ReadStatsByAttribute(statsNode);
	}

	params->attachToOwner = false;
	attachNode->getAttr("attachToOwner", params->attachToOwner);

	string firemodes = paramsNode->getAttr("firemodes");
	i32 curPos = 0;
	string curToken, nextToken;
	nextToken = firemodes.Tokenize(",", curPos);
	while (!nextToken.empty())
	{
		curToken = nextToken;
		curToken.Trim();
#ifdef ITEM_USE_SHAREDSTRING
		params->firemodes.push_back(curToken.c_str());
#else
		params->firemodes.push_back(curToken);
#endif
		nextToken = firemodes.Tokenize(",", curPos);
	}

	firemodes = paramsNode->getAttr("disableFiremodes");
	curPos = 0;
	nextToken = firemodes.Tokenize(",", curPos);
	while (!nextToken.empty())
	{
		curToken = nextToken;
		curToken.Trim();
#ifdef ITEM_USE_SHAREDSTRING
		params->disableFiremodes.push_back(curToken.c_str());
#else
		params->disableFiremodes.push_back(curToken);
#endif
		nextToken = firemodes.Tokenize(",", curPos);
	}

	params->switchToFireMode = paramsNode->getAttr("switchToFireMode");
	params->secondaryFireMode = paramsNode->getAttr("secondaryFireMode");
	params->zoommode = paramsNode->getAttr("zoommode");
	params->zoommodeSecondary = paramsNode->getAttr("zoommodeSecondary");
	params->category = paramsNode->getAttr("category");

	i32 exclusive = 0;
	i32 selectable = 1; //Selectable by default
	i32 defaultAccessory = 0;
	i32 enableBaseModifier = 0;
	i32 alsoAttachDefault = 0;
	i32 extendsMagazine = 0;
	i32 client_only = 1;
	i32 beginsDetached = 0; 
	params->reloadSpeedMultiplier = 1.f;

	paramsNode->getAttr("exclusive", exclusive);
	paramsNode->getAttr("selectable", selectable);
	paramsNode->getAttr("default", defaultAccessory);
	paramsNode->getAttr("enableBaseModifier", enableBaseModifier);
	paramsNode->getAttr("alsoAttachDefault", alsoAttachDefault);
	paramsNode->getAttr("extendsMagazine", extendsMagazine);
	paramsNode->getAttr("client_only", client_only);
	paramsNode->getAttr("beginsDetached", beginsDetached);

	paramsNode->getAttr("reloadSpeedMultiplier", params->reloadSpeedMultiplier);

	params->exclusive = (exclusive != 0);
	params->selectable = (selectable != 0);
	params->defaultAccessory = (defaultAccessory != 0);
	params->enableBaseModifier = (enableBaseModifier != 0);
	params->alsoAttachDefault = (alsoAttachDefault != 0);
	params->extendsMagazine = (extendsMagazine != 0);
	params->client_only = (client_only != 0);
	params->beginsDetached = (beginsDetached != 0); 

	//Temporary stored here for parsing, deleted when done
	params->tempAccessoryParams = reader.FindFilteredChild("params");

	return true;
}

//-----------------------------------------------------------------------
bool CItemSharedParams::ReadAccessoryAmmo(const XmlNodeRef& paramsNode)
{
	bonusAccessoryAmmo.clear();

	CGameXmlParamReader reader(paramsNode);

	i32k childCount = reader.GetUnfilteredChildCount();
	for (i32 i = 0; i < childCount; i++)
	{
		XmlNodeRef ammoNode = reader.GetFilteredChildAt(i);
		if (ammoNode == NULL)
			continue;

		if (!strcmpi(ammoNode->getTag(), "ammo"))
		{
			tukk name = ammoNode->getAttr("name");
			IEntityClass* pClass = name ? gEnv->pEntitySystem->GetClassRegistry()->FindClass(name) : NULL;

			if (pClass == NULL)
			{
				GameWarning("Invalid class name '%s' found while reading '%s' node", name, ammoNode->getTag());
			}
			else
			{
				i32 amount = 0;

				ammoNode->getAttr("amount", amount);

				if (amount)
				{
					bonusAccessoryAmmo[pClass]=amount;
				}

				i32 capacity = 0;

				ammoNode->getAttr("capacity", capacity);

				if (amount)
				{
					accessoryAmmoCapacity[pClass]=capacity;
				}
			}
		}
	}

	return true;
}

//------------------------------------------------------
bool CItemSharedParams::ReadLaserParams(const XmlNodeRef& paramsNode)
{ 
	CGameXmlParamReader reader(paramsNode);

	const XmlNodeRef& fpNode = reader.FindFilteredChild("firstperson");
	const XmlNodeRef& tpNode = reader.FindFilteredChild("thirdperson");

	if (fpNode)
	{
		pLaserParams->laser_dot[0] = fpNode->getAttr("laser_dot");
		fpNode->getAttr("laser_range", pLaserParams->laser_range[0]);
		fpNode->getAttr("laser_thickness", pLaserParams->laser_thickness[0]);
	}

	if (tpNode)
	{
		pLaserParams->laser_geometry_tp = tpNode->getAttr("laser_geometry_tp");
		pLaserParams->laser_dot[1] = tpNode->getAttr("laser_dot");
		tpNode->getAttr("laser_range", pLaserParams->laser_range[1]);
		tpNode->getAttr("laser_thickness", pLaserParams->laser_thickness[1]);
	}
	i32 showDot = 1;
	paramsNode->getAttr("show_dot", showDot);
	pLaserParams->show_dot = showDot != 0;

	return true;
}

//------------------------------------------------------
bool CItemSharedParams::ReadFlashLightParams(const XmlNodeRef& paramsNode)
{
	CGameXmlParamReader reader(paramsNode);

	pFlashLightParams->lightCookie = reader.ReadParamValue("lightCookie");
	reader.ReadParamValue("color", pFlashLightParams->color);
	reader.ReadParamValue("diffuseMult", pFlashLightParams->diffuseMult);
	reader.ReadParamValue("specularMult", pFlashLightParams->specularMult);
	reader.ReadParamValue("HDRDynamic", pFlashLightParams->HDRDynamic);
	reader.ReadParamValue("distance", pFlashLightParams->distance);
	reader.ReadParamValue("fov", pFlashLightParams->fov);
	reader.ReadParamValue("style", pFlashLightParams->style);
	reader.ReadParamValue("animSpeed", pFlashLightParams->animSpeed);
	pFlashLightParams->color /= 255.0f;

	reader.ReadParamValue("fogVolumeColor", pFlashLightParams->fogVolumeColor);
	reader.ReadParamValue("fogVolumeRadius", pFlashLightParams->fogVolumeRadius);
	reader.ReadParamValue("fogVolumeSize", pFlashLightParams->fogVolumeSize);
	reader.ReadParamValue("fogVolumeDensity", pFlashLightParams->fogVolumeDensity);
	pFlashLightParams->fogVolumeColor /= 255.0f;

	return true;
}

//------------------------------------------------------------------------
i32 CItemSharedParams::TargetToSlot(tukk slot)
{
	i32 islot = eIGS_Last;
	if (slot)
	{
		if (!stricmp(slot, "firstperson"))
			islot = eIGS_FirstPerson;
		else if (!stricmp(slot, "thirdperson"))
			islot = eIGS_ThirdPerson;
		else if (!stricmp(slot, "owner"))
			islot = eIGS_Owner;
		else if (!stricmp(slot, "aux0"))
			islot = eIGS_Aux0;
		else if (!stricmp(slot, "ownerGraph"))
			islot = eIGS_OwnerAnimGraph;
		else if (!stricmp(slot, "ownerGraphLoop"))
			islot = eIGS_OwnerAnimGraphLooped;
		else if (!stricmp(slot, "destroyed"))
			islot = eIGS_Destroyed;
		else if (!stricmp(slot, "thirdpersonAux"))
			islot = eIGS_ThirdPersonAux;
		else if (!stricmp(slot, "aux1"))
			islot = eIGS_Aux1;
	}

	return islot;
}

//------------------------------------------------------------------------
void CItemSharedParams::CacheResourcesForLevelStartMP(CItemResourceCache& itemResourceCache, const IEntityClass* pItemClass)
{
	i32k allowPreloadDBA = 1;

	assert(gEnv);
	PREFAST_ASSUME(gEnv);

	//--- Currently only called via the Loadout, going forward we should add filters & logic here for single player
	for (u32 i=0; i<animationPrecache.size(); i++)
	{
		if (allowPreloadDBA)
		{
			DrxLog("Preload DBA %s", animationPrecache[i].DBAfile.c_str());
			gEnv->pCharacterUpr->DBA_LockStatus(animationPrecache[i].DBAfile.c_str(), 1, ICharacterUpr::eStreamingDBAPriority_Normal);

			//Update loading screen and important tick functions
			SYNCHRONOUS_LOADING_TICK();
		}
	}

	CacheResources( itemResourceCache, pItemClass );
}

//------------------------------------------------------------------------
void CItemSharedParams::CacheResources( CItemResourceCache& itemResourceCache, const IEntityClass* pItemClass )
{
	//Cache values for class only once
	if (itemResourceCache.AreClassResourcesCached(pItemClass))
		return;

	if (g_pGameCVars->designer_warning_level_resources)
	{
		if (g_pGame->IsLevelLoaded())
		{
			DrxFixedStringT<256> warningMessage;
			warningMessage.Format("Loading resources for item '%s' at run-time. Did you forget to include player loadout info inside level xml?", pItemClass->GetName());
			DesignerWarning(!g_pGame->IsLevelLoaded(), warningMessage.c_str());
		}
	}

	CItemGeometryCache& geomCache = itemResourceCache.GetItemGeometryCache();
	CItemParticleEffectCache& particleCache = itemResourceCache.GetParticleEffectCache();
	CItemMaterialAndTextureCache& materialCache = itemResourceCache.GetMaterialsAndTextureCache();

	TGeometryDefVector::const_iterator geometryEndCit = geometry.end();
	for (TGeometryDefVector::const_iterator geometryCit = geometry.begin(); geometryCit != geometryEndCit; ++geometryCit)
	{
		geomCache.CacheGeometry(geometryCit->modelPath.c_str(), geometryCit->useCgfStreaming);
		if (!geometryCit->material.empty())
			materialCache.CacheMaterial(geometryCit->material.c_str());
	}

	if (!params.crosshairTexture.empty())
	{
		materialCache.CacheTexture(params.crosshairTexture.c_str(), true);
	}

	if (pLaserParams)
	{
		pLaserParams->CacheResources(geomCache);
	}

	if (pFlashLightParams)
	{
		pFlashLightParams->CacheResources(materialCache);
	}

	if (ItemClassUsesDefaultLaser(pItemClass))
	{
		GetDefaultLaserParameters().CacheResources(geomCache);
	}

	// Cache weapon resources too
	CGameSharedParametersStorage* pGameParamsStorage = g_pGame->GetGameSharedParametersStorage();
	if(pGameParamsStorage)
	{
		CWeaponSharedParams* weaponSharedParams = pGameParamsStorage->GetWeaponSharedParameters(pItemClass->GetName(), false);
		if(weaponSharedParams)
		{
			weaponSharedParams->CacheResources();
		}
	}

	//Cache resources for initial setup from xml
	for(TInitialSetup::iterator accessoryIt = initialSetup.begin(); accessoryIt != initialSetup.end(); ++accessoryIt)
	{
		const IEntityClass* pAccessoryClass = *accessoryIt;
		DRX_ASSERT(pAccessoryClass);

		CItemSharedParams* pAccessoryParams = g_pGame->GetGameSharedParametersStorage()->GetItemSharedParameters(pAccessoryClass->GetName(), false);
		if (pAccessoryParams)
		{
			pAccessoryParams->CacheResources(itemResourceCache, pAccessoryClass);
		}
	}


	IAnimationDatabaseUpr& rDatabaseUpr = g_pGame->GetIGameFramework()->GetMannequinInterface().GetAnimationDatabaseUpr();
	if(!params.adbFile.empty())
	{
		rDatabaseUpr.Load(params.adbFile.c_str());
	}

	if(!params.soundAdbFile.empty())
	{
		rDatabaseUpr.Load(params.soundAdbFile.c_str());
	}

	if(!params.actionControllerFile.empty())
	{
		rDatabaseUpr.LoadControllerDef(params.actionControllerFile.c_str());
	}
	
	itemResourceCache.CachedResourcesForClassDone(pItemClass);
}

void CItemSharedParams::ReleaseLevelResources()
{
}

const SLaserParams& CItemSharedParams::GetDefaultLaserParameters()
{
	return s_defaultLaserParameters;
}

bool CItemSharedParams::ItemClassUsesDefaultLaser( const IEntityClass* pItemClass ) const
{
	DRX_ASSERT(pItemClass);

	tukk itemClassName = pItemClass->GetName();

	return ((strcmp(itemClassName, "JAW") == 0) || (strcmp(itemClassName, "swarmer") == 0));
}

void CItemSharedParams::PrefixPathIfFilename( tukk pPath, ItemString& filename )
{
	if(!filename.empty())
	{
		string fullFilePath(pPath);
		fullFilePath.append(filename.c_str());
		filename = fullFilePath;
	}
}

void SLaserParams::CacheResources(CItemGeometryCache& geometryCache) const
{
	geometryCache.CacheGeometry(laser_geometry_tp.c_str(), false);
	gEnv->pParticleUpr->FindEffect(laser_dot[0].c_str());
	gEnv->pParticleUpr->FindEffect(laser_dot[1].c_str());
}




SFlashLightParams::SFlashLightParams()
	:	color(ZERO)
	,	diffuseMult(0.0f)
	,	specularMult(0.0f)
	,	HDRDynamic(0.0f)
	,	distance(0.0f)
	,	fov(0.0f)
	,	style(0)
	,	animSpeed(1.0f)
	,	fogVolumeColor(ZERO)
	,	fogVolumeRadius(0.0f)
	,	fogVolumeSize(0.0f)
	,	fogVolumeDensity(0.0f)
{
}


void SFlashLightParams::CacheResources(CItemMaterialAndTextureCache& textureCache) const
{
	textureCache.CacheTexture(lightCookie.c_str());
}
