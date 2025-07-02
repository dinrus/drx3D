// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
// Sandbox plugin wrapper.
#include "StdAfx.h"
#include "SandboxPlugin.h"

#include <drx3D/CoreX/Platform/platform_impl.inl>

#include <drx3D/CoreX/Platform/DrxLibrary.h>
#include <drx3D/Sandbox/Editor/Plugin/QtViewPane.h>
#include "IEditor.h"
#include <DrxSystem/ISystem.h>
#include <EditorFramework/PersonalizationManager.h>
#include "SubstanceCommon.h"
#include "EditorSubstanceManager.h"
#include "FilePathUtil.h"

#include <drx3D/CoreX/ToolsHelpers/ResourceCompilerHelper.h>
#include <drx3D/CoreX/ToolsHelpers/ResourceCompilerHelper.inl>
#include <drx3D/CoreX/ToolsHelpers/SettingsManagerHelpers.inl>
#include <drx3D/CoreX/ToolsHelpers/EngineSettingsManager.inl>

// Plugin versioning
static tukk g_szPluginName = "EditorSubstance";
static tukk g_szPluginDesc = "Support for Alegorythmic Substance";
static DWORD g_pluginVersion = 1;

// Plugin instance
static CSubstancePlugin* g_pInstance = nullptr;

// Global lock instance
DrxCriticalSection Detail::g_lock;

REGISTER_PLUGIN(CSubstancePlugin);


class CFileHandler : public IFileManipulator
{
	virtual bool ReadFile(const string& filePath, std::vector<char>& buffer, size_t& readSize, const string& mode) override
	{
		IDrxPak* const pPak = GetISystem()->GetIPak();
		FILE* pFile = pPak->FOpen(filePath, mode);
		if (!pFile)
		{
			return false;
		}
		buffer.resize(pPak->FGetSize(pFile));
		const uint64 timestamp = pPak->GetModificationTime(pFile);
		readSize = pPak->FReadRawAll(buffer.data(), buffer.size(), pFile);
		pPak->FClose(pFile);
		return true;		
	}

	virtual string GetAbsolutePath(const string& filename) const override
	{
		//char buffer[IDrxPak::g_nMaxPath];
		//tukk absfilename = gEnv->pDrxPak->AdjustFileName(filename, buffer, IDrxPak::FLAGS_FOR_WRITING);
		return PathUtil::Make(PathUtil::GetGameProjectAssetsPath(), filename);
	}

};

CSubstancePlugin::CSubstancePlugin()
{
	CScopedGlobalLock lock;

	assert(g_pInstance == nullptr);
	g_pInstance = this;
	CFileHandler* fileHandler = new CFileHandler();

	string iniPath;
	{
		std::vector<char> buffer;
		buffer.resize(MAX_PATH);
		auto getPath = [](std::vector<char>& buffer)
		{
			return CResourceCompilerHelper::GetResourceCompilerConfigPath(&buffer[0], buffer.size(), CResourceCompilerHelper::eRcExePath_editor);
		};
		i32k len = getPath(buffer);
		if (len >= buffer.size())
		{
			buffer.resize(len + 1);
			getPath(buffer);
		}
		iniPath.assign(&buffer[0]);
	}

	InitDrxSubstanceLib(fileHandler);
	EditorSubstance::CManager::Instance()->Init();
}

CSubstancePlugin::~CSubstancePlugin()
{
	CScopedGlobalLock lock;
	assert(g_pInstance == this);
	g_pInstance = nullptr;
}


CSubstancePlugin* CSubstancePlugin::GetInstance()
{
	return g_pInstance;
}

tukk CSubstancePlugin::GetPluginName()
{
	return g_szPluginName;
}


i32 CSubstancePlugin::GetPluginVersion()
{
	return g_pluginVersion;
}

tukk CSubstancePlugin::GetPluginDescription()
{
	return g_szPluginDesc;
}

static void LogVPrintf(tukk szFormat, va_list args)
{
	string format;
	format.Format("[%s] %s", g_szPluginName, szFormat);
	gEnv->pLog->LogV(IMiniLog::eMessage, format.c_str(), args);
}

void LogPrintf(tukk szFormat, ...)
{
	va_list args;
	va_start(args, szFormat);
	LogVPrintf(szFormat, args);
	va_end(args);
}

namespace EditorSubstance
{
namespace Detail
{

void Log(tukk szFormat, ...)
{
	va_list args;
	va_start(args, szFormat);
	LogVPrintf(szFormat, args);
	va_end(args);
}

} //endns Detail
} //endns EditorSubstance



