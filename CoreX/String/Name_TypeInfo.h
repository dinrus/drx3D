// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "DrxName.h"

// CDrxName TypeInfo

TYPE_INFO_BASIC(CDrxName)

string ToString(CDrxName const& val)
{
	return string(val.c_str());
}
bool FromString(CDrxName& val, tukk s)
{
	val = s;
	return true;
}
