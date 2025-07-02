// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/Delegate.h>

namespace sxema
{
enum class EValidatorMessageType
{
	Warning = 0,
	Error
};

typedef std::function<void(EValidatorMessageType, tukk )> Validator;
} // sxema
