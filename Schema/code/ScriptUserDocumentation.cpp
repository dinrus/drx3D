// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptUserDocumentation.h>

#include <drx3D/CoreX/Serialization/STL.h>

namespace sxema
{
void SScriptUserDocumentation::SetCurrentUserAsAuthor()
{
	author = gEnv->pSystem->GetUserName();
}

void SScriptUserDocumentation::Serialize(Serialization::IArchive& archive)
{
	archive(author, "author", "Author(s)");
	archive(description, "description", "Description");
}
}
