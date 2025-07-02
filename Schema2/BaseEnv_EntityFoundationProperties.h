// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/BaseEnv_Prerequisites.h>

namespace SchematycBaseEnv
{
	struct SEntityFoundationProperties
	{
		SEntityFoundationProperties();

		void Serialize(Serialization::IArchive& archive);

		string icon;
		bool   bHideInEditor;
		bool   bTriggerAreas;
	};
}