// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <map>
#include <drx3D/CoreX/Thread/IThreadConfigUpr.h>

/*
   ThreadConfigUpr:
   Loads a thread configuration from an xml file and stores them.

   == XML File Layout and Rules: ===

   = Platform names =
   (case insensitive)
   "ANDROID"
   "DURANGO"
   "ORBIS"
   "PC"
   "MAC"

   = Basic Layout =
   <ThreadConfig>
   <Platform name="XXX">
   <ThreadDefault Affinity="XX" Priority="XX" StackSizeKB="XX">
   <Thread name ="A" Affinity="XX" Priority="XX" StackSizeKB="XX">
   <Thread name ="B" Affinity="XX" >
   ...
   </Platform>

   <Platform name="YYY">
   ...
   </Platform>
   </ThreadConfig>

   = Parser Order for Platform =
   1. PlatformName_Common (valid for all potential platform configurations. Can be overridden by concert platform configuration)
   2. PlatformName or PlatformName_X (for platforms with unknown CPU count where X is the number of potential cores. The equal or next lower matching configuration for the identified core count at runtime will be taken)

   Note: Overriding of thread configuration by later parsed configuration allowed.

   = <ThreadDefault> and <Thread> XML attributes =

   !!!
   Note: Use "ignore" as value if you do not want the thread system to set the value specifically!
      If a value is not defines the <ThreadDefault> value of the parameter will be used.
      This is useful when dealing with 3rdParty threads where you are not in control of the parameter setup.
   !!!

   Name:
   "x" (string)		: Name of thread
   "x*y" (string)	: Name of thread with wildcard character

   Affinity:
   "-1"           : Put SW thread affinity in the hands of the scheduler - (default) -
   "x"          : Run thread on specified core
   "x, y, ..."  : Run thread on specified cores

   Priority:
   "idle"             : Hint to DinrusX to run thread with pre-set priority
   "below_normal"		: Hint to DinrusX to run thread with pre-set priority
   "normal"           : Hint to DinrusX to run thread with pre-set priority - (default) -
   "above_normal"		: Hint to DinrusX to run thread with pre-set priority
   "highest"					: Hint to DinrusX to run thread with pre-set priority
   "time_critical"		: Hint to DinrusX to run thread with pre-set priority
   "x" (number)			: User defined thread priority number

   StackSizeKB:
   "0"  : Let platform decide on the stack size - (default) -
   "x"  : Create thread with "x" KB of stack size

   DisablePriorityBoost:
   "true"   : Disable priority boosting - (default) -
   "false"	: Enable priority boosting
 */

class CThreadConfigUpr : public IThreadConfigUpr
{
public:
	typedef std::map<DrxFixedStringT<THREAD_NAME_LENGTH_MAX>, SThreadConfig>                 ThreadConfigMap;
	typedef std::pair<DrxFixedStringT<THREAD_NAME_LENGTH_MAX>, SThreadConfig>                ThreadConfigMapPair;
	typedef std::map<DrxFixedStringT<THREAD_NAME_LENGTH_MAX>, SThreadConfig>::iterator       ThreadConfigMapIter;
	typedef std::map<DrxFixedStringT<THREAD_NAME_LENGTH_MAX>, SThreadConfig>::const_iterator ThreadConfigMapConstIter;

public:
	CThreadConfigUpr();
	~CThreadConfigUpr()
	{
	}

	// Called once during System startup.
	// Loads the thread configuration for the executing platform from file.
	virtual bool LoadConfig(tukk pcPath) override;

	// Returns true if a config has been loaded
	virtual bool ConfigLoaded() const override;

	// Gets the thread configuration for the specified thread on the active platform.
	// If no matching config is found a default configuration is returned
	// (which does not have the same name as the search string).
	virtual const SThreadConfig* GetThreadConfig(tukk sThreadName, ...) override;
	virtual const SThreadConfig* GetDefaultThreadConfig() const override;

	virtual void                 DumpThreadConfigurationsToLog() override;

private:
	tukk          IdentifyPlatform();

	const SThreadConfig* GetThreadConfigImpl(tukk cThreadName);

	bool                 LoadPlatformConfig(const XmlNodeRef& rXmlRoot, tukk sPlatformId);

	void                 LoadPlatformThreadConfigs(const XmlNodeRef& rXmlPlatformRef);
	bool                 LoadThreadDefaultConfig(const XmlNodeRef& rXmlPlatformRef);
	void                 LoadThreadConfig(const XmlNodeRef& rXmlThreadRef, SThreadConfig& rThreadConfig);

	void                 LoadAffinity(const XmlNodeRef& rXmlThreadRef, u32& rAffinity, SThreadConfig::TThreadParamFlag& rParamActivityFlag);
	void                 LoadPriority(const XmlNodeRef& rXmlThreadRef, i32& rPriority, SThreadConfig::TThreadParamFlag& rParamActivityFlag);
	void                 LoadDisablePriorityBoost(const XmlNodeRef& rXmlThreadRef, bool& rPriorityBoost, SThreadConfig::TThreadParamFlag& rParamActivityFlag);
	void                 LoadStackSize(const XmlNodeRef& rXmlThreadRef, u32& rStackSize, SThreadConfig::TThreadParamFlag& rParamActivityFlag);

private:
	ThreadConfigMap m_threadConfig; // Note: The map key is referenced by as tukk by the value's storage class. Other containers may not support this behaviour as they will re-allocate memory as they grow/shrink.
	ThreadConfigMap m_wildcardThreadConfig;
	SThreadConfig   m_defaultConfig;
};
