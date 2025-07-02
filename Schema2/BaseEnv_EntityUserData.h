// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/BaseEnv_Prerequisites.h>

namespace SchematycBaseEnv
{
	struct SEntityUserData
	{
		inline SEntityUserData(bool _bIsPreview, sxema2::EObjectFlags _objectFlags)
			: bIsPreview(_bIsPreview)
			, objectFlags(_objectFlags)
		{}

		bool                    bIsPreview;
		sxema2::EObjectFlags objectFlags;
	};
}