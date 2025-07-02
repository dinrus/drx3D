// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/IScriptFile.h>

namespace sxema2
{
	struct SScriptUserDocumentation
	{
		void SetCurrentUserAsAuthor();
		void Serialize(Serialization::IArchive& archive);

		string	author;
		string	description;
	};
}
