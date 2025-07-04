// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "../stdafx.h"
#include "../UIEnumsDatabase.h"

//////////////////////////////////////////////////////////////////////////
const string& CUIEnumsDatabase_SEnum::NameToValue(const string& name)
{
	i32 n = (i32)strings.size();
	for (i32 i = 0; i < n; i++)
	{
		if (name == strings[i])
			return values[i];
	}
	return name;
}

//////////////////////////////////////////////////////////////////////////
const string& CUIEnumsDatabase_SEnum::ValueToName(const string& value)
{
	i32 n = (i32)strings.size();
	for (i32 i = 0; i < n; i++)
	{
		if (value.CompareNoCase(values[i]) == 0)
			return strings[i];
	}
	return value;
}

//////////////////////////////////////////////////////////////////////////
CUIEnumsDatabase::CUIEnumsDatabase()
{
}

//////////////////////////////////////////////////////////////////////////
CUIEnumsDatabase::~CUIEnumsDatabase()
{
	// Free enums.
	for (Enums::iterator it = m_enums.begin(); it != m_enums.end(); ++it)
	{
		delete it->second;
	}
}

//////////////////////////////////////////////////////////////////////////
void CUIEnumsDatabase::SetEnumStrings(const string& enumName, const std::vector<string>& sStringsArray)
{
	i32 nStringCount = sStringsArray.size();

	CUIEnumsDatabase_SEnum* pEnum = stl::find_in_map(m_enums, enumName, 0);
	if (!pEnum)
	{
		pEnum = new CUIEnumsDatabase_SEnum;
		pEnum->name = enumName;
		m_enums[enumName] = pEnum;
	}
	pEnum->strings.resize(nStringCount);
	pEnum->values.resize(nStringCount);
	for (i32 i = 0; i < nStringCount; i++)
	{
		string str = sStringsArray[i];
		string value = str;
		i32 pos = str.Find('=');
		if (pos >= 0)
		{
			value = str.Mid(pos + 1);
			str = str.Mid(0, pos);
		}
		pEnum->strings[i] = str;
		pEnum->values[i] = value;
	}
}

//////////////////////////////////////////////////////////////////////////
CUIEnumsDatabase_SEnum* CUIEnumsDatabase::FindEnum(const string& enumName) const
{
	CUIEnumsDatabase_SEnum* pEnum = stl::find_in_map(m_enums, enumName, 0);
	if (!pEnum)
	{
		string temp = enumName;
		temp.replace(" ", ""); //remove whitespaces (these were added by QtUtil::CamelCaseToUIString)
		pEnum = stl::find_in_map(m_enums, temp, 0);
	}
	return pEnum;
}

