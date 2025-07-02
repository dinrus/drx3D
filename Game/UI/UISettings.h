// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   UISettings.h
//  Version:     v1.00
//  Created:     10/8/2011 by Paul Reindell.
//  Описание: 
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __UISettings_H__
#define __UISettings_H__

#include <drx3D/Game/IUIGameEventSystem.h>
#include <drx3D/Sys/Scaleform/IFlashUI.h>
#include <drx3D/CoreX/Game/IGameFramework.h>
#include <DrxAudio/IAudioSystem.h>
#include <drx3D/CoreX/SFunctor.h>

struct SNullCVar : public ICVar
{
	virtual ~SNullCVar() {}
	virtual void Release() { return; }
	virtual i32 GetIVal() const { return 0; }
	virtual int64 GetI64Val() const { return 0; }
	virtual float GetFVal() const { return 0; }
	virtual tukk GetString() const { return "NULL"; }
#if defined( DEDICATED_SERVER )
	virtual void SetDataProbeString( tukk pDataProbeString )  { return; }
#endif
	virtual tukk GetDataProbeString() const { return "NULL"; }
	virtual void Set(tukk s) { return; }
	virtual void ForceSet(tukk s) { return; }
	virtual void Set(const float f) { return; }
	virtual void Set(i32k i) { return; }
	virtual void ClearFlags (i32 flags) { return; }
	virtual i32 GetFlags() const { return 0; }
	virtual i32 SetFlags( i32 flags ) { return 0; }
	virtual i32 GetType() { return 0; }
	virtual tukk GetName() const { return "NULL"; }
	virtual tukk GetHelp() { return "NULL"; }
	virtual bool IsConstCVar() const { return 0; }

	virtual void SetOnChangeCallback( ConsoleVarFunc pChangeFunc ){return;}
	virtual void AddOnChangeFunctor( const SFunctor& pChangeFunctor) {return;}
	virtual bool RemoveOnChangeFunctor(const uint64 nElement) {return true;}
	virtual uint64 GetNumberOfOnChangeFunctors()const {return 0;}
	virtual const SFunctor& GetOnChangeFunctor(uint64 nFunctorIndex)const {static SFunctor oDummy;return oDummy;};
	virtual ConsoleVarFunc GetOnChangeCallback() const {return NULL;}

	virtual void GetMemoryUsage( class IDrxSizer* pSizer ) const { return; }
	virtual i32 GetRealIVal() const { return 0; }
	virtual void DebugLog( i32k iExpectedValue, const EConsoleLogMode mode ) const {}

	static SNullCVar* Get()
	{
		static SNullCVar inst;
		return &inst;
	}

private:
	SNullCVar() {}
};

class CUISettings
	: public IUIGameEventSystem
	, public IUIModule
{
public:
	CUISettings();

	// IUIGameEventSystem
	UIEVENTSYSTEM( "UISettings" );
	virtual void InitEventSystem() override;
	virtual void UnloadEventSystem() override;

	//IUIModule
	virtual void Init() override;
	virtual void Update(float fDelta);

private:
	// ui functions
	void SendResolutions();
	void SendGraphicSettingsChange();
	void SendSoundSettingsChange();
	void SendGameSettingsChange();

	// ui events
	void OnSetGraphicSettings( i32 resIndex, i32 graphicsQuality, bool fullscreen );
	void OnSetResolution( i32 resX, i32 resY, bool fullscreen );
	void OnSetSoundSettings( float music, float sfx, float video );
	void OnSetGameSettings( float sensitivity, bool invertMouse, bool invertController );

	void OnGetResolutions();
	void OnGetCurrGraphicsSettings();
	void OnGetCurrSoundSettings();
	void OnGetCurrGameSettings();

	void OnGetLevels( string levelPathFilter );

	void OnLogoutUser();

private:
	enum EUIEvent
	{
		eUIE_GraphicSettingsChanged,
		eUIE_SoundSettingsChanged,
		eUIE_GameSettingsChanged,

		eUIE_OnGetResolutions,
		eUIE_OnGetResolutionItems,
		eUIE_OnGetLevelItems,
	};

	SUIEventReceiverDispatcher<CUISettings> m_eventDispatcher;
	SUIEventSenderDispatcher<EUIEvent> m_eventSender;
	IUIEventSystem* m_pUIEvents;
	IUIEventSystem* m_pUIFunctions;

	ICVar* m_pRXVar;
	ICVar* m_pRYVar;
 	ICVar* m_pFSVar;
	ICVar* m_pGQVar;


	AudioControlId m_musicVolumeId;
	AudioControlId m_sfxVolumeId;

	ICVar* m_pVideoVar;

	ICVar* m_pMouseSensitivity;
	ICVar* m_pInvertMouse;
	ICVar* m_pInvertController;

	i32 m_currResId;

	typedef std::vector< std::pair<i32,i32> > TResolutions;
	TResolutions m_Resolutions;
};


#endif // __UISettings_H__
