// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/ThreadConfigUpr.h>
#include <drx3D/Sys/IConsole.h>
#include <drx3D/Sys/System.h>
#include <drx3D/CoreX/String/StringUtils.h>
#include <drx3D/CoreX/DrxCustomTypes.h>

namespace
{
tukk sCurThreadConfigFilename = "";
u32k sPlausibleStackSizeLimitKB = (1024 * 100); // 100mb
}

//////////////////////////////////////////////////////////////////////////
CThreadConfigUpr::CThreadConfigUpr()
{
	m_defaultConfig.szThreadName = "DrxThread_Unnamed";
	m_defaultConfig.stackSizeBytes = 0;
	m_defaultConfig.affinityFlag = -1;
	m_defaultConfig.priority = THREAD_PRIORITY_NORMAL;
	m_defaultConfig.bDisablePriorityBoost = false;
	m_defaultConfig.paramActivityFlag = (SThreadConfig::TThreadParamFlag)~0;
}

//////////////////////////////////////////////////////////////////////////
const SThreadConfig* CThreadConfigUpr::GetThreadConfig(tukk szThreadName, ...)
{
	va_list args;
	va_start(args, szThreadName);

	// Format thread name
	char strThreadName[THREAD_NAME_LENGTH_MAX];
	if (!drx_vsprintf(strThreadName, szThreadName, args))
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadInfo>: ThreadName \"%s\" has been truncated to \"%s\". Max characters allowed: %i. ", szThreadName, strThreadName, (i32)sizeof(strThreadName) - 1);
	}

	// Get Thread Config
	const SThreadConfig* retThreasdConfig = GetThreadConfigImpl(strThreadName);

	va_end(args);
	return retThreasdConfig;
}

//////////////////////////////////////////////////////////////////////////
const SThreadConfig* CThreadConfigUpr::GetThreadConfigImpl(tukk szThreadName)
{
	// Get thread config for platform
	ThreadConfigMapConstIter threatRet = m_threadConfig.find(DrxFixedStringT<THREAD_NAME_LENGTH_MAX>(szThreadName));
	if (threatRet == m_threadConfig.end())
	{
		// Search in wildcard setups
		ThreadConfigMapConstIter wildCardIter = m_wildcardThreadConfig.begin();
		ThreadConfigMapConstIter wildCardIterEnd = m_wildcardThreadConfig.end();
		for (; wildCardIter != wildCardIterEnd; ++wildCardIter)
		{
			if (DrxStringUtils::MatchWildcard(szThreadName, wildCardIter->second.szThreadName))
			{
				// Store new thread config
				SThreadConfig threadConfig = wildCardIter->second;
				std::pair<ThreadConfigMapIter, bool> res;
				res = m_threadConfig.insert(ThreadConfigMapPair(DrxFixedStringT<THREAD_NAME_LENGTH_MAX>(szThreadName), threadConfig));

				// Store name (ref to key)
				SThreadConfig& rMapThreadConfig = res.first->second;
				rMapThreadConfig.szThreadName = res.first->first.c_str();

				// Return new thread config
				return &res.first->second;
			}
		}

		// Failure case, no match found
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadInfo>: Unable to find config for thread:%s", szThreadName);
		return &m_defaultConfig;
	}

	// Return thread config
	return &threatRet->second;
}

//////////////////////////////////////////////////////////////////////////
const SThreadConfig* CThreadConfigUpr::GetDefaultThreadConfig() const
{
	return &m_defaultConfig;
}

//////////////////////////////////////////////////////////////////////////
bool CThreadConfigUpr::LoadConfig(tukk pcPath)
{
	// Adjust filename for OnDisk or in .pak file loading
	char szFullPathBuf[IDrxPak::g_nMaxPath];
	gEnv->pDrxPak->AdjustFileName(pcPath, szFullPathBuf, 0);

	// Open file
	XmlNodeRef xmlRoot = GetISystem()->LoadXmlFromFile(szFullPathBuf);
	if (!xmlRoot)
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadConfigInfo>: File \"%s\" not found!", pcPath);
		return false;
	}

	// Load config for active platform
	sCurThreadConfigFilename = pcPath;
	tukk strPlatformId = IdentifyPlatform();
	DrxFixedStringT<32> tmpPlatformStr;
	bool retValueCommon = false;
	bool retValueCPU = false;

	// Try load common platform settings
	tmpPlatformStr.Format("%s_Common", strPlatformId);
	retValueCommon = LoadPlatformConfig(xmlRoot, tmpPlatformStr.c_str());
	if (retValueCommon)
	{
		DrxLogAlways("<ThreadConfigInfo>: Thread profile loaded: \"%s\" (%s)  ", tmpPlatformStr.c_str(), pcPath);
	}

#if defined(DRX_PLATFORM_DESKTOP)
	// Handle PC specifically as we do not know the core setup of the executing machine.
	// Try and find the next power of 2 core setup. Otherwise fallback to a lower power of 2 core setup spec

	// Try and load next pow of 2 setup for active pc core configuration
	u32k numCPUs = ((CSystem*)GetISystem())->GetCPUFeatures()->GetLogicalCPUCount();
	u32 i = numCPUs;
	for (; i > 0; --i)
	{
		tmpPlatformStr.Format("%s_%i", strPlatformId, i);
		retValueCPU = LoadPlatformConfig(xmlRoot, tmpPlatformStr.c_str());
		if (retValueCPU)
		{
			break;
		}
	}

	if (retValueCPU && i != numCPUs)
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadConfigInfo>: (%s: %u core) Unable to find platform config \"%s\". Next valid config found was %s_%u.",
		           strPlatformId, numCPUs, tmpPlatformStr.c_str(), strPlatformId, i);
	}

#else
	tmpPlatformStr.Format("%s", strPlatformId);
	retValueCPU = LoadPlatformConfig(xmlRoot, strPlatformId);
#endif

	// Print out info
	if (retValueCPU)
	{
		DrxLogAlways("<ThreadConfigInfo>: Thread profile loaded: \"%s\" (%s)  ", tmpPlatformStr.c_str(), pcPath);
	}
	
	if (!retValueCommon && !retValueCPU)
	{
		// Could not find any matching platform
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadConfigInfo>: Active platform identifier string \"%s\" not found in config \"%s\".", strPlatformId, sCurThreadConfigFilename);
	}

	sCurThreadConfigFilename = "";
	return (retValueCommon || retValueCPU);
}

//////////////////////////////////////////////////////////////////////////
bool CThreadConfigUpr::ConfigLoaded() const
{
	return !m_threadConfig.empty();
}

//////////////////////////////////////////////////////////////////////////
bool CThreadConfigUpr::LoadPlatformConfig(const XmlNodeRef& rXmlRoot, tukk sPlatformId)
{

	// Validate node
	if (!rXmlRoot->isTag("ThreadConfig"))
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadConfigInfo>: Unable to find root xml node \"ThreadConfig\"");
		return false;
	}

	// Find active platform
	u32k numPlatforms = rXmlRoot->getChildCount();
	for (u32 i = 0; i < numPlatforms; ++i)
	{
		const XmlNodeRef xmlPlatformNode = rXmlRoot->getChild(i);

		// Is platform node
		if (!xmlPlatformNode->isTag("Platform"))
		{
			continue;
		}

		// Is has Name attribute
		if (!xmlPlatformNode->haveAttr("Name"))
		{
			continue;
		}

		// Is platform of interest
		tukk platformName = xmlPlatformNode->getAttr("Name");
		if (_stricmp(sPlatformId, platformName) == 0)
		{
			// Load platform
			LoadThreadDefaultConfig(xmlPlatformNode);
			LoadPlatformThreadConfigs(xmlPlatformNode);
			return true;
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
void CThreadConfigUpr::LoadPlatformThreadConfigs(const XmlNodeRef& rXmlPlatformRef)
{
	// Get thread configurations for active platform
	u32k numThreads = rXmlPlatformRef->getChildCount();
	for (u32 j = 0; j < numThreads; ++j)
	{
		const XmlNodeRef xmlThreadNode = rXmlPlatformRef->getChild(j);

		if (!xmlThreadNode->isTag("Thread"))
		{
			continue;
		}

		// Ensure thread config has name
		if (!xmlThreadNode->haveAttr("Name"))
		{
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadConfigInfo>: [XML Parsing] Thread node without \"name\" attribute encountered.");
			continue;
		}

		// Load thread config
		SThreadConfig loadedThreadConfig = SThreadConfig(m_defaultConfig);
		LoadThreadConfig(xmlThreadNode, loadedThreadConfig);

		// Get thread name and check if it contains wildcard characters
		tukk szThreadName = xmlThreadNode->getAttr("Name");
		bool bWildCard = strchr(szThreadName, '*') ? true : false;
		ThreadConfigMap& threadConfig = bWildCard ? m_wildcardThreadConfig : m_threadConfig;

		// Check for duplicate and override it with new config if found
		if (threadConfig.find(szThreadName) != threadConfig.end())
		{
			DrxLogAlways("<ThreadConfigInfo>: [XML Parsing] Thread with name \"%s\" already loaded. Overriding with new configuration", szThreadName);
			threadConfig[szThreadName] = loadedThreadConfig;
			continue;
		}

		// Store new thread config
		std::pair<ThreadConfigMapIter, bool> res;
		res = threadConfig.insert(ThreadConfigMapPair(DrxFixedStringT<THREAD_NAME_LENGTH_MAX>(szThreadName), loadedThreadConfig));

		// Store name (ref to key)
		SThreadConfig& rMapThreadConfig = res.first->second;
		rMapThreadConfig.szThreadName = res.first->first.c_str();
	}
}

//////////////////////////////////////////////////////////////////////////
bool CThreadConfigUpr::LoadThreadDefaultConfig(const XmlNodeRef& rXmlPlatformRef)
{
	// Find default thread config node
	u32k numNodes = rXmlPlatformRef->getChildCount();
	for (u32 j = 0; j < numNodes; ++j)
	{
		const XmlNodeRef xmlNode = rXmlPlatformRef->getChild(j);

		// Load default config
		if (xmlNode->isTag("ThreadDefault"))
		{
			LoadThreadConfig(xmlNode, m_defaultConfig);
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CThreadConfigUpr::LoadAffinity(const XmlNodeRef& rXmlThreadRef, u32& rAffinity, SThreadConfig::TThreadParamFlag& rParamActivityFlag)
{
	tukk szValidCharacters = "-,0123456789";
	u32 affinity = 0;

	// Validate node
	if (!rXmlThreadRef->haveAttr("Affinity"))
		return;

	// Validate token
	DrxFixedStringT<32> affinityRawStr(rXmlThreadRef->getAttr("Affinity"));
	if (affinityRawStr.empty())
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadConfigInfo>: [XML Parsing] Empty attribute \"Affinity\" encountered");
		return;
	}

	if (affinityRawStr.compareNoCase("ignore") == 0)
	{
		// Param is inactive, clear bit
		rParamActivityFlag &= ~SThreadConfig::eThreadParamFlag_Affinity;
		return;
	}

	DrxFixedStringT<32>::size_type nPos = affinityRawStr.find_first_not_of(" -,0123456789");
	if (nPos != DrxFixedStringT<32>::npos)
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING,
		           "<ThreadConfigInfo>: [XML Parsing] Invalid character \"%c\" encountered in \"Affinity\" attribute. Valid characters:\"%s\" Offending token:\"%s\"", affinityRawStr.at(nPos),
		           szValidCharacters, affinityRawStr.c_str());
		return;
	}

	// Tokenize comma separated string
	i32 pos = 0;
	DrxFixedStringT<32> affnityTokStr = affinityRawStr.Tokenize(",", pos);
	while (!affnityTokStr.empty())
	{
		affnityTokStr.Trim();

		long affinityId = strtol(affnityTokStr.c_str(), NULL, 10);
		if (affinityId == LONG_MAX || affinityId == LONG_MIN)
		{
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadConfigInfo>: [XML Parsing] Unknown value \"%s\" encountered for attribute \"Affinity\"", affnityTokStr.c_str());
			return;
		}

		// Allow scheduler to pick thread
		if (affinityId == -1)
		{
			affinity = ~0;
			break;
		}

		// Set affinity bit
		affinity |= BIT(affinityId);

		// Move to next token
		affnityTokStr = affinityRawStr.Tokenize(",", pos);
	}

	// Set affinity reference
	rAffinity = affinity;
}

//////////////////////////////////////////////////////////////////////////
void CThreadConfigUpr::LoadPriority(const XmlNodeRef& rXmlThreadRef, i32& rPriority, SThreadConfig::TThreadParamFlag& rParamActivityFlag)
{
	tukk szValidCharacters = "-,0123456789";

	// Validate node
	if (!rXmlThreadRef->haveAttr("Priority"))
		return;

	// Validate token
	DrxFixedStringT<32> threadPrioStr(rXmlThreadRef->getAttr("Priority"));
	threadPrioStr.Trim();
	if (threadPrioStr.empty())
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadConfigInfo>: [XML Parsing] Empty attribute \"Priority\" encountered");
		return;
	}

	if (threadPrioStr.compareNoCase("ignore") == 0)
	{
		// Param is inactive, clear bit
		rParamActivityFlag &= ~SThreadConfig::eThreadParamFlag_Priority;
		return;
	}

	// Test for character string (no numbers allowed)
	if (threadPrioStr.find_first_of(szValidCharacters) == DrxFixedStringT<32>::npos)
	{
		threadPrioStr.MakeLower();

		// Set priority
		if (threadPrioStr.compare("below_normal") == 0)
		{
			rPriority = THREAD_PRIORITY_BELOW_NORMAL;
		}
		else if (threadPrioStr.compare("normal") == 0)
		{
			rPriority = THREAD_PRIORITY_NORMAL;
		}
		else if (threadPrioStr.compare("above_normal") == 0)
		{
			rPriority = THREAD_PRIORITY_ABOVE_NORMAL;
		}
		else if (threadPrioStr.compare("idle") == 0)
		{
			rPriority = THREAD_PRIORITY_IDLE;
		}
		else if (threadPrioStr.compare("lowest") == 0)
		{
			rPriority = THREAD_PRIORITY_LOWEST;
		}
		else if (threadPrioStr.compare("highest") == 0)
		{
			rPriority = THREAD_PRIORITY_HIGHEST;
		}
		else if (threadPrioStr.compare("time_critical") == 0)
		{
			rPriority = THREAD_PRIORITY_TIME_CRITICAL;
		}
		else
		{
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadConfigInfo>: [XML Parsing] Platform unsupported value \"%s\" encountered for attribute \"Priority\"", threadPrioStr.c_str());
			return;
		}
	}
	// Test for number string (no alphabetical characters allowed)
	else if (threadPrioStr.find_first_not_of(szValidCharacters) == DrxFixedStringT<32>::npos)
	{
		long numValue = strtol(threadPrioStr.c_str(), NULL, 10);
		if (numValue == LONG_MAX || numValue == LONG_MIN)
		{
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadConfigInfo>: [XML Parsing] Unsupported number type \"%s\" for for attribute \"Priority\"", threadPrioStr.c_str());
			return;
		}

		// Set priority
		rPriority = numValue;
	}
	else
	{
		// String contains characters and numbers
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadConfigInfo>: [XML Parsing] Unsupported type \"%s\" encountered for attribute \"Priority\". Token containers numbers and characters", threadPrioStr.c_str());
		return;
	}
}

//////////////////////////////////////////////////////////////////////////
void CThreadConfigUpr::LoadDisablePriorityBoost(const XmlNodeRef& rXmlThreadRef, bool& rPriorityBoost, SThreadConfig::TThreadParamFlag& rParamActivityFlag)
{
	tukk sValidCharacters = "-,0123456789";

	// Validate node
	if (!rXmlThreadRef->haveAttr("DisablePriorityBoost"))
	{
		return;
	}

	// Extract bool info
	DrxFixedStringT<16> sAttribToken(rXmlThreadRef->getAttr("DisablePriorityBoost"));
	sAttribToken.Trim();
	sAttribToken.MakeLower();

	if (sAttribToken.compare("ignore") == 0)
	{
		// Param is inactive, clear bit
		rParamActivityFlag &= ~SThreadConfig::eThreadParamFlag_PriorityBoost;
		return;
	}
	else if (sAttribToken.compare("true") == 0 || sAttribToken.compare("1") == 0)
	{
		rPriorityBoost = true;
	}
	else if (sAttribToken.compare("false") == 0 || sAttribToken.compare("0") == 0)
	{
		rPriorityBoost = false;
	}
	else
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadConfigInfo>: [XML Parsing] Unsupported bool type \"%s\" encountered for attribute \"DisablePriorityBoost\"",
		           rXmlThreadRef->getAttr("DisablePriorityBoost"));
		return;
	}
}

//////////////////////////////////////////////////////////////////////////
void CThreadConfigUpr::LoadStackSize(const XmlNodeRef& rXmlThreadRef, u32& rStackSize, SThreadConfig::TThreadParamFlag& rParamActivityFlag)
{
	tukk sValidCharacters = "0123456789";

	if (rXmlThreadRef->haveAttr("StackSizeKB"))
	{
		i32 nPos = 0;

		// Read stack size
		DrxFixedStringT<32> stackSize(rXmlThreadRef->getAttr("StackSizeKB"));

		// Validate stack size
		if (stackSize.empty())
		{
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadConfigInfo>: [XML Parsing] Empty attribute \"StackSize\" encountered");
			return;
		}
		else if (stackSize.compareNoCase("ignore") == 0)
		{
			// Param is inactive, clear bit
			rParamActivityFlag &= ~SThreadConfig::eThreadParamFlag_StackSize;
			return;
		}
		else if (stackSize.find_first_not_of(sValidCharacters) == DrxFixedStringT<32>::npos)
		{
			// Convert string to long
			long stackSizeVal = strtol(stackSize.c_str(), NULL, 10);
			if (stackSizeVal == LONG_MAX || stackSizeVal == LONG_MIN)
			{
				DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadConfigInfo>: [XML Parsing] Invalid number for \"StackSize\" encountered. \"%s\"", stackSize.c_str());
				return;
			}
			else if (stackSizeVal <= 0 || stackSizeVal > sPlausibleStackSizeLimitKB)
			{
				DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadConfigInfo>: [XML Parsing] \"StackSize\" value not plausible \"%" PRId64 "KB\"", (int64)stackSizeVal);
				return;
			}

			// Set stack size
			rStackSize = stackSizeVal * 1024; // Convert to bytes
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CThreadConfigUpr::LoadThreadConfig(const XmlNodeRef& rXmlThreadRef, SThreadConfig& rThreadConfig)
{
	LoadAffinity(rXmlThreadRef, rThreadConfig.affinityFlag, rThreadConfig.paramActivityFlag);
	LoadPriority(rXmlThreadRef, rThreadConfig.priority, rThreadConfig.paramActivityFlag);
	LoadDisablePriorityBoost(rXmlThreadRef, rThreadConfig.bDisablePriorityBoost, rThreadConfig.paramActivityFlag);
	LoadStackSize(rXmlThreadRef, rThreadConfig.stackSizeBytes, rThreadConfig.paramActivityFlag);
}

//////////////////////////////////////////////////////////////////////////
tukk CThreadConfigUpr::IdentifyPlatform()
{
#if defined(DURANGO)
	return "durango";
#elif defined(ORBIS)
	return "orbis";
	// ANDROID **needs** to be tested before LINUX because both are defined for android
#elif defined(ANDROID)
	return "android";
#elif defined(LINUX) || defined(flagLINUX)
	return "linux";
#elif defined(APPLE)
	return "mac";
#elif defined(WIN32) || defined(WIN64)
	return "pc";
#else
	#error "Undefined platform"
#endif
}

//////////////////////////////////////////////////////////////////////////
void CThreadConfigUpr::DumpThreadConfigurationsToLog()
{
#if !defined(RELEASE)

	// Print header
	DrxLogAlways("== Thread Startup Config List (\"%s\") ==", IdentifyPlatform());

	// Print loaded default config
	DrxLogAlways("  (Default) 1. \"%s\" (StackSize:%uKB | Affinity:%u | Priority:%i | PriorityBoost:\"%s\")", m_defaultConfig.szThreadName, m_defaultConfig.stackSizeBytes / 1024,
	             m_defaultConfig.affinityFlag, m_defaultConfig.priority, m_defaultConfig.bDisablePriorityBoost ? "disabled" : "enabled");

	// Print loaded thread configs
	i32 listItemCounter = 1;
	ThreadConfigMapConstIter iter = m_threadConfig.begin();
	ThreadConfigMapConstIter iterEnd = m_threadConfig.end();
	for (; iter != iterEnd; ++iter)
	{
		const SThreadConfig& threadConfig = iter->second;
		DrxLogAlways("%3d.\"%s\" %s (StackSize:%uKB %s | Affinity:%u %s | Priority:%i %s | PriorityBoost:\"%s\" %s)", ++listItemCounter,
		             threadConfig.szThreadName, (threadConfig.paramActivityFlag & SThreadConfig::eThreadParamFlag_ThreadName) ? "" : "(ignored)",
		             threadConfig.stackSizeBytes / 1024u, (threadConfig.paramActivityFlag & SThreadConfig::eThreadParamFlag_StackSize) ? "" : "(ignored)",
		             threadConfig.affinityFlag, (threadConfig.paramActivityFlag & SThreadConfig::eThreadParamFlag_Affinity) ? "" : "(ignored)",
		             threadConfig.priority, (threadConfig.paramActivityFlag & SThreadConfig::eThreadParamFlag_Priority) ? "" : "(ignored)",
		             !threadConfig.bDisablePriorityBoost ? "enabled" : "disabled", (threadConfig.paramActivityFlag & SThreadConfig::eThreadParamFlag_PriorityBoost) ? "" : "(ignored)");
	}
#endif
}
