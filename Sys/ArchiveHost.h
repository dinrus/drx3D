// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

//#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/CoreX/Serialization/IArchiveHost.h>

namespace Serialization
{

IArchiveHost* CreateArchiveHost();
void RegisterArchiveHostCVars();
}
