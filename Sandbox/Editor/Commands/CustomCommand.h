// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

class ICommandManager;

class CCustomCommand : public CCommand0
{
public:
	CCustomCommand(const string& name, const string& command);

	void Register();
	void Unregister();

	// Only want to be able to set name on custom commands
	void           SetName(tukk name);
	void           SetCommandString(tukk commandStr);

	virtual bool   IsCustomCommand() const override  { return true; }
	virtual string GetCommandString() const override { return m_command; }

private:

	string m_command;
};

