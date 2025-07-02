// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#if !DRX_PLATFORM
	#error DrxPlatformDefines.h is not included.
#endif

#if DRX_PLATFORM_WINDOWS && defined(DRX_ENABLE_RC_HELPER)

	#include "SettingsUprHelpers.h"

//! Manages storage and loading of all information for tools and DRXENGINE, by either registry or an INI file.
//! Information can be read and set by key-to-value functions.
//! Specific information can be set by a dialog application called by this class.
//! If the engine root path is not found, a fall-back dialog is opened.
class CEngineSettingsUpr
{
	friend class CResourceCompilerHelper;

public:
	//! Prepares CEngineSettingsUpr to get requested information either from registry or an INI file,
	//! if existent as a file with name an directory equal to the module, or from registry.
	CEngineSettingsUpr(const wchar_t* moduleName = NULL, const wchar_t* iniFileName = NULL);

	void RestoreDefaults();

	//! Stores/loads user specific information for modules to/from registry or INI file.
	bool GetModuleSpecificStringEntryUtf16(tukk key, SettingsUprHelpers::CWCharBuffer wbuffer);
	bool GetModuleSpecificStringEntryUtf8(tukk key, SettingsUprHelpers::CCharBuffer buffer);
	bool GetModuleSpecificIntEntry(tukk key, i32& value);
	bool GetModuleSpecificBoolEntry(tukk key, bool& value);

	bool SetModuleSpecificStringEntryUtf16(tukk key, const wchar_t* str);
	bool SetModuleSpecificStringEntryUtf8(tukk key, tukk str);
	bool SetModuleSpecificIntEntry(tukk key, i32k& value);
	bool SetModuleSpecificBoolEntry(tukk key, const bool& value);

	bool GetValueByRef(tukk key, SettingsUprHelpers::CWCharBuffer wbuffer) const;
	bool GetValueByRef(tukk key, bool& value) const;
	bool GetValueByRef(tukk key, i32& value) const;

	void SetKey(tukk key, const wchar_t* value);
	void SetKey(tukk key, bool value);
	void SetKey(tukk key, i32 value);

	bool StoreData(bool bForceFileOutput);
	void CallSettingsDialog(uk hParent);
	void CallRootPathDialog(uk hParent);

	void SetRootPath(const wchar_t* szRootPath);

	//! \return Path determined either by registry or by INI file.
	void GetRootPathUtf16(SettingsUprHelpers::CWCharBuffer wbuffer);
	void GetRootPathAscii(SettingsUprHelpers::CCharBuffer buffer);

	bool GetInstalledBuildRootPathUtf16(i32k index, SettingsUprHelpers::CWCharBuffer name, SettingsUprHelpers::CWCharBuffer path);

	void SetParentDialog(u64 window);

	long HandleProc(uk pWnd, long uMsg, long wParam, long lParam);

private:
	bool HasKey(tukk key);

	void LoadEngineSettingsFromRegistry();
	bool StoreEngineSettingsToRegistry();
	bool StoreLicenseSettingsToRegistry();

	//! Parses a file and stores all flags in a private key-value-map.
	bool LoadValuesFromConfigFile(const wchar_t* szFileName);

	bool SetRegValue(uk key, tukk valueName, const wchar_t* value);
	bool SetRegValue(uk key, tukk valueName, bool value);
	bool SetRegValue(uk key, tukk valueName, i32 value);
	bool GetRegValue(uk key, tukk valueName, SettingsUprHelpers::CWCharBuffer wbuffer);
	bool GetRegValue(uk key, tukk valueName, bool& value);
	bool GetRegValue(uk key, tukk valueName, i32& value);

private:
	SettingsUprHelpers::CFixedString<wchar_t, 256> m_sModuleName;         //!< Name to store key-value pairs of modules in (registry) or to identify INI file.
	SettingsUprHelpers::CFixedString<wchar_t, 256> m_sModuleFileName;     //!< Used in case of data being loaded from INI file.
	bool m_bGetDataFromRegistry;
	SettingsUprHelpers::CKeyValueArray<30>         m_keyValueArray;

	uk         m_hBtnBrowse;
	u64 m_hWndParent;
};

#endif
