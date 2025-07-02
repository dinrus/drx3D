// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>
#include <drx3D/Schema2/ScriptUserDocumentation.h>

namespace sxema2
{
	//////////////////////////////////////////////////////////////////////////
	void SScriptUserDocumentation::SetCurrentUserAsAuthor()
	{
		author = gEnv->pSystem->GetUserName();
	}

	//////////////////////////////////////////////////////////////////////////
	void SScriptUserDocumentation::Serialize(Serialization::IArchive& archive)
	{
		archive(author, "author", "Author(s)");
		archive(description, "description", "Description");
	}
}
