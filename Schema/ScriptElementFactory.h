// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptElement.h>

namespace sxema
{
// #SchematycTODO : Resolve disconnect between factory method used to construct elements during serialization and in place methods used when editing?
// #SchematycTODO : Allocate elements in pools?
class CScriptElementFactory
{
public:

	IScriptElementPtr CreateElement(EScriptElementType elementType);
};
} // sxema
