// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"

#include "CharacterPhysics.h"
#include "Serialization.h"

struct ICharacterInstance;

namespace CharacterTool
{

void SPhysicsComponent::Serialize(Serialization::IArchive& ar)
{
	ar(enabled, "enabled", "^");
	ar(name, "name", "^<");
}

void SCharacterPhysicsContent::Serialize(Serialization::IArchive& ar)
{
	ar(m_components, "components", "Components");
}

void SCharacterPhysicsContent::ApplyToCharacter(ICharacterInstance* instance)
{

}

}

