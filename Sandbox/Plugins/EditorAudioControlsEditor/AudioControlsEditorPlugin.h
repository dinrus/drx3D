// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <IPlugin.h>
#include <IEditor.h>

#include "AssetsManager.h"

namespace DrxAudio
{
struct IObject;
} //endns DrxAudio

namespace ACE
{
class CImplementationManager;
extern CAssetsManager g_assetsManager;
extern CImplementationManager g_implementationManager;
extern Platforms g_platforms;

enum class EReloadFlags
{
	None                 = 0,
	ReloadSystemControls = BIT(0),
	ReloadImplData       = BIT(1),
	ReloadScopes         = BIT(2),
	SendSignals          = BIT(3),
	SetPlatforms         = BIT(4),
	BackupConnections    = BIT(5),
};
DRX_CREATE_ENUM_FLAG_OPERATORS(EReloadFlags);

class CAudioControlsEditorPlugin final : public IPlugin, public ISystemEventListener
{
public:

	explicit CAudioControlsEditorPlugin();
	virtual ~CAudioControlsEditorPlugin() override;

	// IPlugin
	virtual i32       GetPluginVersion() override     { return 1; }
	virtual char const* GetPluginName() override        { return "Audio Controls Editor"; }
	virtual char const* GetPluginDescription() override { return "The Audio Controls Editor enables browsing and configuring audio events exposed from the audio middleware"; }
	// ~IPlugin

	static void       SaveData();
	static void       ReloadData(EReloadFlags const flags);
	static void       ExecuteTrigger(string const& sTriggerName);
	static void       StopTriggerExecution();
	static EErrorCode GetLoadingErrorMask() { return s_loadingErrorMask; }

	static CDrxSignal<void()> SignalAboutToLoad;
	static CDrxSignal<void()> SignalLoaded;
	static CDrxSignal<void()> SignalAboutToSave;
	static CDrxSignal<void()> SignalSaved;

private:

	// ISystemEventListener
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override;
	// ~ISystemEventListener

	void        InitPlatforms();
	static void ReloadImplData(EReloadFlags const flags);

	static FileNames           s_currentFilenames;
	static DrxAudio::IObject*  s_pIAudioObject;
	static DrxAudio::ControlId s_audioTriggerId;

	static EErrorCode          s_loadingErrorMask;
};
} //endns ACE

