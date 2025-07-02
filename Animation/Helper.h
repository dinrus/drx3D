// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace Helper
{

template<class T>
T* FindFromSorted(T* pValues, u32 count, T& valueToFind)
{
	T* pLast = &pValues[count];
	T* pValue = std::lower_bound(pValues, pLast, valueToFind);
	if (pValue == pLast || *pValue != valueToFind)
		return NULL;

	return pValue;
}

template<class T>
T* FindFromSorted(std::vector<T>& values, const T& valueToFind)
{
	return FindFromSorted(&values[0], values.size(), valueToFind);
}

} //endns Helper
