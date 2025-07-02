// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include <vector>

struct ICharacterInstance;
#include <drx3D/CoreX/Serialization/Forward.h>

namespace CharacterTool
{

using std::vector;
struct SCharacterRigDummyComponent
{
	string name;

	void   Serialize(Serialization::IArchive& ar);
};

struct SCharacterRigContent
{
	vector<SCharacterRigDummyComponent> m_components;

	void Serialize(Serialization::IArchive& ar);
	void ApplyToCharacter(ICharacterInstance* instance);

	void Reset() { *this = SCharacterRigContent(); }
};

}

