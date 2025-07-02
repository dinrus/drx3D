// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

struct EDITOR_COMMON_API ViewportInteraction
{
	enum Key
	{
		eKey_Forward,
		eKey_Backward,
		eKey_Left,
		eKey_Right
	};

	static bool CheckPolledKey(Key key);
};

