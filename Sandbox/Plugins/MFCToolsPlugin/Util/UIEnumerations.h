// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef UiEnumerations_h__
#define UiEnumerations_h__

#pragma once

class CUIEnumerations
{
public:
	// For XML standard values.
	typedef std::vector<string>        TDValues;
	typedef std::map<string, TDValues> TDValuesContainer;
protected:
private:

public:
	static CUIEnumerations& GetUIEnumerationsInstance();

	TDValuesContainer&      GetStandardNameContainer();
protected:
private:
};

#endif // UiEnumerations_h__

