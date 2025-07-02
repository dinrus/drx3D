// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "BoostPythonMacros.h"

class CEditorPythonManager : public IPythonManager
{
public:
	CEditorPythonManager() {}
	~CEditorPythonManager() { Deinit(); }

	void                              Init();
	void                              Deinit();

	virtual void                      RegisterPythonCommand(const SPythonCommand& command) override;
	virtual void                      RegisterPythonModule(const SPythonModule& module) override;
	virtual void                      Execute(tukk command) override;
	virtual float                     GetAsFloat(tukk variable) override;
	const std::vector<SPythonModule>& GetPythonModules() const { return m_pythonModules; }

private:

	std::vector<SPythonModule> m_pythonModules;
};

