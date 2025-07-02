// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   Описание:  Remote command system implementation
   -------------------------------------------------------------------------
   История:
   - 10/04/2013   : Tomasz Jonarski, Created

*************************************************************************/
#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Network/IServiceNetwork.h>
#include <drx3D/Sys/RemoteCommand.h>
#include <drx3D/Sys/RemoteCommandHelpers.h>

//-----------------------------------------------------------------------------

// remote system internal logging
#ifdef RELEASE
	#define LOG_VERBOSE(level, txt, ...)
#else
	#define LOG_VERBOSE(level, txt, ...) if (GetUpr()->CheckVerbose(level)) { GetUpr()->Log(txt, __VA_ARGS__); }
#endif

//-----------------------------------------------------------------------------

CRemoteCommandUpr::CRemoteCommandUpr()
{
	// Create the CVAR
	m_pVerboseLevel = REGISTER_INT("rc_debugVerboseLevel", 0, VF_DEV_ONLY, "");
}

CRemoteCommandUpr::~CRemoteCommandUpr()
{
	// Release the CVar
	if (NULL != m_pVerboseLevel)
	{
		m_pVerboseLevel->Release();
		m_pVerboseLevel = NULL;
	}
}

IRemoteCommandServer* CRemoteCommandUpr::CreateServer(u16 localPort)
{
	// Create the listener
	IServiceNetworkListener* listener = gEnv->pServiceNetwork->CreateListener(localPort);
	if (NULL == listener)
	{
		return NULL;
	}

	// Create the wrapper
	return new CRemoteCommandServer(this, listener);
}

IRemoteCommandClient* CRemoteCommandUpr::CreateClient()
{
	// Create the wrapper
	return new CRemoteCommandClient(this);
}

void CRemoteCommandUpr::RegisterCommandClass(IRemoteCommandClass& commandClass)
{
	// Make sure command class is not already registered
	const string& className(commandClass.GetName());
	TClassMap::const_iterator it = m_pClasses.find(className);
	if (it != m_pClasses.end())
	{
		LOG_VERBOSE(1, "Class '%s' is already registered",
		            className.c_str());

		return;
	}

	u32k classID = m_pClassesByID.size();
	m_pClassesByID.push_back(&commandClass);
	m_pClassesMap[className] = classID;
	m_pClasses[className] = &commandClass;

	// Verbose
	LOG_VERBOSE(1, "Registered command class '%s' with id %d",
	            className.c_str(),
	            classID);
}

#ifndef RELEASE
bool CRemoteCommandUpr::CheckVerbose(u32k level) const
{
	i32k verboseLevel = m_pVerboseLevel->GetIVal();
	return (i32)level < verboseLevel;
}

void CRemoteCommandUpr::Log(tukk txt, ...)  const
{
	// format the print buffer
	char buffer[512];
	va_list args;
	va_start(args, txt);
	drx_vsprintf(buffer, txt, args);
	va_end(args);

	// pass to log
	gEnv->pLog->LogAlways("%s", buffer);
}
#endif

void CRemoteCommandUpr::BuildClassMapping(const std::vector<string>& classNames, std::vector<IRemoteCommandClass*>& outClasses)
{
	LOG_VERBOSE(3, "Building class mapping for %d classes",
	            classNames.size());

	// Output list size has the same size as class names array
	u32k numClasses = classNames.size();
	outClasses.resize(numClasses);

	// Match the classes
	for (size_t i = 0; i < numClasses; ++i)
	{
		// Find the matching class
		const string& className = classNames[i];
		TClassMap::const_iterator it = m_pClasses.find(className);
		if (it != m_pClasses.end())
		{
			DRX_ASSERT(className == it->second->GetName());
			DRX_ASSERT(it->second != NULL);
			outClasses[i] = it->second;

			// Report class mapping in heavy verbose mode
			LOG_VERBOSE(3, "Class[%d] = %s",
			            i,
			            className.c_str());
		}
		else
		{
			outClasses[i] = NULL;

			// Class not mapped (this can cause errors)
			LOG_VERBOSE(0, "Remote command class '%s' not found on this machine",
			            className.c_str());
		}
	}
}

void CRemoteCommandUpr::SetVerbosityLevel(u32k level)
{
	// propagate the value to CVar (so it is consistent across the engine)
	if (NULL != m_pVerboseLevel)
	{
		m_pVerboseLevel->Set((i32)level);
	}
}

void CRemoteCommandUpr::GetClassList(std::vector<string>& outClassNames) const
{
	u32k numClasses = m_pClassesByID.size();
	outClassNames.resize(numClasses);
	for (size_t id = 0; id < numClasses; ++id)
	{
		IRemoteCommandClass* theClass = m_pClassesByID[id];
		if (NULL != theClass)
		{
			outClassNames[id] = theClass->GetName();
		}
	}
}

bool CRemoteCommandUpr::FindClassId(IRemoteCommandClass* commandClass, u32& outClassId) const
{
	// Local search (linear, slower)
	TClassIDMap::const_iterator it = m_pClassesMap.find(commandClass->GetName());
	if (it != m_pClassesMap.end())
	{
		outClassId = it->second;
		return true;
	}

	// Not found
	return false;
}

//-----------------------------------------------------------------------------

// Do not remove (can mess up the uber file builds)
#undef LOG_VERBOSE

//-----------------------------------------------------------------------------
