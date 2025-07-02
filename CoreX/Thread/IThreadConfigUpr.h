// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

struct SThreadConfig
{
	enum eThreadParamFlag
	{
		eThreadParamFlag_ThreadName    = BIT(0),
		eThreadParamFlag_StackSize     = BIT(1),
		eThreadParamFlag_Affinity      = BIT(2),
		eThreadParamFlag_Priority      = BIT(3),
		eThreadParamFlag_PriorityBoost = BIT(4),
	};

	typedef u32 TThreadParamFlag;

	tukk      szThreadName;
	u32           stackSizeBytes;
	u32           affinityFlag;
	i32            priority;
	bool             bDisablePriorityBoost;

	TThreadParamFlag paramActivityFlag;
};

class IThreadConfigUpr
{
public:
	virtual ~IThreadConfigUpr()
	{
	}

	//! Called once during System startup.
	//! Loads the thread configuration for the executing platform from file.
	virtual bool LoadConfig(tukk pcPath) = 0;

	//! Returns true if a config has been loaded.
	virtual bool ConfigLoaded() const = 0;

	//! Gets the thread configuration for the specified thread on the active platform.
	//! If no matching config is found a default configuration is returned (which does not have the same name as the search string).
	virtual const SThreadConfig* GetThreadConfig(tukk sThreadName, ...) = 0;
	virtual const SThreadConfig* GetDefaultThreadConfig() const = 0;

	//! Dump a detailed description of the thread startup configurations for this platform to the log file.
	virtual void DumpThreadConfigurationsToLog() = 0;
};
