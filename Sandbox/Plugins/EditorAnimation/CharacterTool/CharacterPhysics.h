// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include <vector>

struct ICharacterInstance;
#include <drx3D/CoreX/Serialization/Forward.h>

namespace CharacterTool
{
using std::vector;

struct SPhysicsComponent
{
	bool   enabled;
	string name;

	void   Serialize(Serialization::IArchive& ar);
};

struct SCharacterPhysicsContent
{
	vector<SPhysicsComponent> m_components;

	void Serialize(Serialization::IArchive& ar);

	void ApplyToCharacter(ICharacterInstance* instance);

	void Reset() { *this = SCharacterPhysicsContent(); }
};

}

