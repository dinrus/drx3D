// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Class that loads a config file with the ability to restore the
						 cvars to the values they were before loading them.

-------------------------------------------------------------------------
История:
- 25:07:2012  : Refactored into a class by Martin Sherburn

*************************************************************************/

#ifndef __REVERTIBLE_CONFIG_LOADER_H__
#define __REVERTIBLE_CONFIG_LOADER_H__

#pragma once

#include <drx3D/Game/Utility/SingleAllocTextBlock.h>

class CRevertibleConfigLoader : public ILoadConfigurationEntrySink
{
public:
	CRevertibleConfigLoader(i32 maxCvars, i32 maxTextBufferSize);
	~CRevertibleConfigLoader() {}

	/* ILoadConfigurationEntrySink */
	virtual void OnLoadConfigurationEntry( tukk szKey, tukk szValue, tukk szGroup );
	/* ILoadConfigurationEntrySink */

	void LoadConfiguration(tukk szConfig);
	void ApplyAndStoreCVar(tukk szKey, tukk szValue);
	void RevertCVarChanges();

	void SetAllowCheatCVars(bool allow) { m_allowCheatCVars = allow; }

protected:
	struct SSavedCVar
	{
		tukk  m_name;
		tukk  m_value;
	};

	std::vector<SSavedCVar> m_savedCVars;
	CSingleAllocTextBlock m_cvarsTextBlock;
	bool m_allowCheatCVars;
};

#endif // ~__REVERTIBLE_CONFIG_LOADER_H__
