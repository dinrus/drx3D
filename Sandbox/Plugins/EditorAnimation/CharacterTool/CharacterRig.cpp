// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"

#include "CharacterRig.h"
#include "Serialization.h"

struct ICharacterInstance;

namespace CharacterTool
{

void SCharacterRigDummyComponent::Serialize(Serialization::IArchive& ar)
{
	ar(name, "name", "^<");
}

void SCharacterRigContent::Serialize(Serialization::IArchive& ar)
{
	ar(m_components, "components", "Components");
}

void SCharacterRigContent::ApplyToCharacter(ICharacterInstance* instance)
{

}

}

