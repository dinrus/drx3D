// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

class EDITOR_COMMON_API CCommandModuleDescription
{
public:
	CCommandModuleDescription(tukk name, tukk uiName, tukk description)
		: m_name(name)
		, m_uiName(uiName)
		, m_desription(description)
	{}

	CCommandModuleDescription(tukk name)
		: m_name(name)
		, m_uiName(FormatCommandForUI(name))
		, m_desription("")
	{}

	virtual ~CCommandModuleDescription() {}

	virtual string FormatCommandForUI(tukk command) const
	{
		string forUi(command);

		i32k size = forUi.size();

		if (size > 0)
		{
			forUi.replace(0, 1, 1, toupper(forUi[0]));
		}

		for (i32 i = 0; i < size; ++i)
		{
			if (forUi[i] == '_')
			{
				forUi.replace(i, 1, 1, ' ');
				if (i < size - 1)
					forUi.replace(i + 1, 1, 1, toupper(forUi[i + 1]));
			}
			else if (forUi[i] == ' ' && i < size - 1)
			{
				forUi.replace(i + 1, 1, 1, toupper(forUi[i + 1]));
			}
			else if (forUi[i] == '&')
			{
				forUi.erase(i, 1);
			}
		}

		return forUi;
	}

	const string& GetName() const        { return m_name; }
	const string& GetUiName() const      { return m_uiName; }
	const string& GetDescription() const { return m_desription; }

private:
	string m_name;
	string m_uiName;
	string m_desription;
};

