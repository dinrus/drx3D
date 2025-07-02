// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/CoreX/Serialization/yasli/decorators/BitFlags.h>
#include <drx3D/CoreX/Serialization/yasli/Archive.h>

namespace yasli {

void BitFlagsWrapper::YASLI_SERIALIZE_METHOD(yasli::Archive& ar)
{
	const yasli::EnumDescription& desc = *description;
	i32 count = desc.count();
	if (ar.isInput()) {
		//*variable = 0;
		i32 previousValue = *variable;
		for (size_t i = 0; i < (size_t)count; ++i) {
			i32 flagValue = desc.valueByIndex((i32)i);
			bool flag = (previousValue & flagValue) == flagValue;
			bool previousFlag = flag;
			ar(flag, desc.nameByIndex(i), desc.labelByIndex(i));
			if (flag != previousFlag)
			{
				if (flag)
					*variable |= flagValue;
				else
					*variable &= ~flagValue;
			}
		}
	}
	else {
		for (size_t i = 0; i < (size_t)count; ++i) {
			i32 flagValue = desc.valueByIndex(i);
			bool flag = (*variable & flagValue) == flagValue;
			ar(flag, desc.nameByIndex(i), desc.labelByIndex(i));
		}
	}
}

}
