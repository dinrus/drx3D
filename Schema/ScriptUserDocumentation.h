// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>

namespace sxema
{
struct SScriptUserDocumentation
{
	void SetCurrentUserAsAuthor();
	void Serialize(Serialization::IArchive& archive);

	string author;
	string description;
};
}
