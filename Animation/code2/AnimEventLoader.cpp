// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/AnimEventLoader.h>

#include <drx3D/Animation/Model.h>
#include <drx3D/Animation/CharacterUpr.h>

namespace AnimEventLoader
{
void DoAdditionalAnimEventInitialization(tukk animationFilePath, CAnimEventData& animEvent);
}

static bool s_bPreLoadParticleEffects = true;

void AnimEventLoader::SetPreLoadParticleEffects(bool bPreLoadParticleEffects)
{
	s_bPreLoadParticleEffects = bPreLoadParticleEffects;
}

bool AnimEventLoader::LoadAnimationEventDatabase(CDefaultSkeleton* pDefaultSkeleton, tukk pFileName)
{
	if (pFileName == 0 || pFileName[0] == 0)
		return false;

	DRX_DEFINE_ASSET_SCOPE("AnimEvents", pFileName);

	// Parse the xml.
	XmlNodeRef root = g_pISystem->LoadXmlFromFile(pFileName);
	if (!root)
	{
		//AnimWarning("Animation Event Database (.animevent) file \"%s\" could not be read (associated with model \"%s\")", pFileName, pDefaultSkeleton->GetFile() );
		return false;
	}

	// Load the events from the xml.
	u32 numAnimations = root->getChildCount();
	for (u32 nAnimationNode = 0; nAnimationNode < numAnimations; ++nAnimationNode)
	{
		XmlNodeRef animationRoot = root->getChild(nAnimationNode);

		// Check whether this is an animation.
		if (stack_string("animation") != animationRoot->getTag())
			continue;

		u32k numEvents = animationRoot->getChildCount();
		if (numEvents == 0)
			continue;

		XmlString animationFilePath = animationRoot->getAttr("name");

		IAnimEventList* pAnimEventList = g_AnimationUpr.GetAnimEventList(animationFilePath.c_str());
		if (!pAnimEventList)
			continue;

		u32k currentAnimationEventCount = pAnimEventList->GetCount();
		if (currentAnimationEventCount != 0)
			continue;

		for (u32 nEventNode = 0; nEventNode < numEvents; ++nEventNode)
		{
			XmlNodeRef eventNode = animationRoot->getChild(nEventNode);

			CAnimEventData animEvent;
			const bool couldLoadXmlData = g_AnimationUpr.LoadAnimEventFromXml(eventNode, animEvent);
			if (!couldLoadXmlData)
				continue;

			pAnimEventList->Append(animEvent);

			DoAdditionalAnimEventInitialization(animationFilePath, animEvent);
		}

		g_AnimationUpr.InitializeSegmentationDataFromAnimEvents(animationFilePath.c_str());
	}

	return true;
}

void AnimEventLoader::DoAdditionalAnimEventInitialization(tukk animationFilePath, CAnimEventData& animEvent)
{
	// NB. Currently, this is only executed when the events are created from an xml file, so for example the editor will
	// not be executing logic contained here while editing animation events.
	assert(animationFilePath);

	static u32k s_crc32_effect = CCrc32::ComputeLowercase("effect");
	u32k eventNameCRC32 = animEvent.GetNameLowercaseCRC32();

	if (s_bPreLoadParticleEffects)
	{
		if (eventNameCRC32 == s_crc32_effect)
		{
			tukk effectName = animEvent.GetCustomParameter();
			gEnv->pParticleUpr->FindEffect(effectName);
		}
	}
}
