// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once
// Means to serialize and edit DrxExtension pointers.

#include <drx3D/CoreX/Extension/IDrxFactory.h>

namespace Serialization
{

	//! Allows us to have std::shared_ptr<TPointer> but serialize it by
	//! interface-casting to TSerializable, i.e. implementing Serialization through
	//! separate interface.
	template<class TPointer, class TSerializable = TPointer>
	struct DrxExtensionPointer
	{
		std::shared_ptr<TPointer>& ptr;

		DrxExtensionPointer(std::shared_ptr<TPointer>& ptr) : ptr(ptr) {}
		void Serialize(Serialization::IArchive& ar);
	};

	//! This function treats T as a type derived from DrxUnknown type.
	template<class TPointer, class TSerializable = TPointer>
	bool Serialize(Serialization::IArchive& ar, Serialization::DrxExtensionPointer<TPointer, TSerializable>& ptr, tukk name, tukk label);

}

#include "DrxExtensionImpl.h"
//! \endcond