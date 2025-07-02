// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "AIWave.h"

#include "AI/AIManager.h"

REGISTER_CLASS_DESC(CAIWaveObjectClassDesc);

IMPLEMENT_DYNCREATE(CAIWaveObject, CEntityObject)

//////////////////////////////////////////////////////////////////////////
CAIWaveObject::CAIWaveObject()
{
	m_entityClass = "AIWave";
	UseMaterialLayersMask(false);
}

void CAIWaveObject::SetName(const string& newName)
{
	string oldName = GetName();

	__super::SetName(newName);

	GetIEditorImpl()->GetAI()->GetAISystem()->Reset(IAISystem::RESET_INTERNAL);

	GetObjectManager()->FindAndRenameProperty2("aiwave_Wave", oldName, newName);
}

