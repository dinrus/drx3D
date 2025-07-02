// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>
#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Serialization/STL.h>

template<class T, class I, class STORE>
struct DynArray;

template<class T, class I, class S>
bool Serialize(Serialization::IArchive& ar, DynArray<T, I, S>& container, tukk name, tukk label)
{
	Serialization::ContainerSTL<DynArray<T, I, S>, T> ser(&container);
	return ar(static_cast<Serialization::IContainer&>(ser), name, label);
}