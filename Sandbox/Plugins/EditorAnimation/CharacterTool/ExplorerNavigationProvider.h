// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace Serialization {
struct INavigationProvider;
}

namespace CharacterTool
{
struct System;
Serialization::INavigationProvider* CreateExplorerNavigationProvider(System* system);
}

