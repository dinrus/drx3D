// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _XMLUtils_h__
#define _XMLUtils_h__

#pragma once

namespace XMLUtils
{
enum BoolType
{
	Invalid = -1,
	False   = 0,
	True    = 1,
};

inline BoolType ToBoolType(tukk str)
{
	if (!stricmp(str, "1") || !stricmp(str, "true") || !stricmp(str, "yes"))
		return True;

	if (!stricmp(str, "0") || !stricmp(str, "false") || !stricmp(str, "no"))
		return False;

	return Invalid;
}

inline BoolType GetBoolType(const XmlNodeRef& node, tukk attribute, const BoolType& deflt)
{
	if (node->haveAttr(attribute))
	{
		tukk value;
		node->getAttr(attribute, &value);

		return ToBoolType(value);
	}

	return deflt;
}
}

#endif // XMLUtils
