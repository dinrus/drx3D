// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>

template<class T> class _smart_ptr;

template<class T>
bool Serialize(Serialization::IArchive& ar, _smart_ptr<T>& ptr, tukk name, tukk label);

#include "SmartPtrImpl.h"
