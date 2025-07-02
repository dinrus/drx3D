// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   FlashUI.h
//  Version:     v1.00
//  Created:     10/9/2010 by Paul Reindell.
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __FlashUI_H__
#define __FlashUI_H__

#include <drx3D/Sys/IFlashUI.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>
#include <drx3D/Input/IHardwareMouse.h>
#include <drx3D/Input/IInput.h>
#include <drx3D/CoreX/Game/IGameFramework.h>
#include <drx3D/Act/ILevelSystem.h>
#include <drx3D/Act/FlashUIEventSystem.h>

#if !defined (_RELEASE) || defined(RELEASE_LOGGING)
	#define UIACTION_LOGGING
#endif

#if defined (UIACTION_LOGGING)
	#define UIACTION_LOG(...)     { if (CFlashUI::CV_gfx_uiaction_log) CFlashUI::LogUIAction(IFlashUI::eLEL_Log, __VA_ARGS__); }
	#define UIACTION_WARNING(...) { CFlashUI::LogUIAction(IFlashUI::eLEL_Warning, __VA_ARGS__); }
	#define UIACTION_ERROR(...)   { CFlashUI::LogUIAction(IFlashUI::eLEL_Error, __VA_ARGS__); }
#else
	#define UIACTION_LOG     (void)
	#define UIACTION_WARNING (void)
	#define UIACTION_ERROR   (void)
#endif

class CFlashUiFlowNodeFactory;
struct CUIActionUpr;
class CFlashUIActionEvents;

class CFlashUI
	: public IFlashUI
	  , public IHardwareMouseEventListener
	  , public IInputEventListener
	  , public IGameFrameworkListener
	  , public ILevelSystemListener
	  , public ISystemEventListener
	  , public ILoadtimeCallback
{
	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(IFlashUI)
	DRXINTERFACE_END()

	DRXGENERATE_SINGLETONCLASS_GUID(CFlashUI, "FlashUI", "35ae7f0f-bb13-437b-9c5f-fcd2568616a5"_drx_guid)

	CFlashUI();
	virtual ~CFlashUI() {}

public:
	// IFlashUI
	virtual void                      Init() override;
	virtual bool                      PostInit() override;
	virtual void                      Update(float fDeltatime) override;
	virtual void                      Reload() override;
	virtual void                      ClearUIActions() override { ClearActions(); }
	virtual void                      Shutdown() override;

	virtual bool                      LoadElementsFromFile(tukk sFileName) override;
	virtual bool                      LoadActionFromFile(tukk sFileName, IUIAction::EUIActionType type) override;

	virtual IUIElement*               GetUIElement(tukk sName) const override { return const_cast<IUIElement*>(m_elements(sName)); }
	virtual IUIElement*               GetUIElement(i32 index) const override         { return index < m_elements.size() ? const_cast<IUIElement*>(m_elements[index]) : NULL; }
	virtual i32                       GetUIElementCount() const override             { return m_elements.size(); }

	virtual                        IUIElement*  GetUIElementByInstanceStr(tukk sUIInstanceStr) const override;
	virtual std::pair<IUIElement*, IUIElement*> GetUIElementsByInstanceStr(tukk sUIInstanceStr) const override;
	virtual std::pair<string, i32>              GetUIIdentifiersByInstanceStr(tukk sUIInstanceStr) const override;

	virtual IUIAction*                GetUIAction(tukk sName) const override { return const_cast<IUIAction*>(m_actions(sName)); }
	virtual IUIAction*                GetUIAction(i32 index) const override         { return index < m_actions.size() ? const_cast<IUIAction*>(m_actions[index]) : NULL; }
	virtual i32                       GetUIActionCount() const override             { return m_actions.size(); }

	virtual IUIActionUpr*         GetUIActionUpr() const override;
	virtual void                      UpdateFG() override;
	virtual void                      EnableEventStack(bool bEnable) override;
	virtual void                      RegisterModule(IUIModule* pModule, tukk name) override;
	virtual void                      UnregisterModule(IUIModule* pModule) override;

	virtual void                      SetHudElementsVisible(bool bVisible) override;

	virtual IUIEventSystem*           CreateEventSystem(tukk sName, IUIEventSystem::EEventSystemType eType) override;
	virtual IUIEventSystem*           GetEventSystem(tukk name, IUIEventSystem::EEventSystemType eType) override;
	virtual IUIEventSystemIteratorPtr CreateEventSystemIterator(IUIEventSystem::EEventSystemType eType) override;

	virtual void                      DispatchControllerEvent(IUIElement::EControllerInputEvent event, IUIElement::EControllerInputState state, float value) override;
	virtual void                      SendFlashMouseEvent(SFlashCursorEvent::ECursorState evt, i32 iX, i32 iY, i32 iButton = 0, float wheel = 0.f, bool bFromController = false) override;
	virtual bool                      DisplayVirtualKeyboard(u32 flags, tukk title, tukk initialInput, i32 maxInputLength, IVirtualKeyboardEvents* pInCallback) override;
	virtual bool                      IsVirtualKeyboardRunning() override;
	virtual bool                      CancelVirtualKeyboard() override;

	virtual void                      GetScreenSize(i32& width, i32& height) override;
	virtual void                      SetEditorScreenSizeCallback(TEditorScreenSizeCallback& cb) override;
	virtual void                      RemoveEditorScreenSizeCallback() override;

	virtual void                      SetEditorUILogEventCallback(TEditorUILogEventCallback& cb) override;
	virtual void                      RemoveEditorUILogEventCallback() override;

	virtual void                      SetEditorPlatformCallback(TEditorPlatformCallback& cb) override;
	virtual void                      RemoveEditorPlatformCallback() override;

	virtual bool                      UseSharedRT(tukk instanceStr, bool defVal) const override;

	virtual void                      CheckPreloadedTexture(ITexture* pTexture) const override;

	virtual void                      GetMemoryStatistics(IDrxSizer* s) const override;

#if !defined(_LIB)
	virtual SUIItemLookupSet_Impl<SUIParameterDesc>* CreateLookupParameter() override { return new SUIItemLookupSet_Impl<SUIParameterDesc>(); };
	virtual SUIItemLookupSet_Impl<SUIMovieClipDesc>* CreateLookupMovieClip() override { return new SUIItemLookupSet_Impl<SUIMovieClipDesc>(); };
	virtual SUIItemLookupSet_Impl<SUIEventDesc>*     CreateLookupEvent() override { return new SUIItemLookupSet_Impl<SUIEventDesc>(); };
#endif
	// ~IFlashUI

	// IHardwareMouseEventListener
	void OnHardwareMouseEvent(i32 iX, i32 iY, EHARDWAREMOUSEEVENT eHardwareMouseEvent, i32 wheelDelta) override;
	// ~IHardwareMouseEventListener

	// IInputEventListener
	virtual bool OnInputEvent(const SInputEvent& event) override;
	virtual bool OnInputEventUI(const SUnicodeEvent& event) override;
	// ~IInputEventListener

	// ISystemEventListener
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override;
	// ~ISystemEventListener

	// IGameFrameworkListener
	virtual void OnPostUpdate(float fDeltaTime) override    {}
	virtual void OnSaveGame(ISaveGame* pSaveGame) override  {}
	virtual void OnLoadGame(ILoadGame* pLoadGame) override  {}
	virtual void OnLevelEnd(tukk nextLevel) override {}
	virtual void OnActionEvent(const SActionEvent& event) override;
	// ~IGameFrameworkListener

	// ILevelSystemListener
	virtual void OnLevelNotFound(tukk levelName) override;
	virtual void OnLoadingStart(ILevelInfo* pLevel) override              {}
	virtual void OnLoadingLevelEntitiesStart(ILevelInfo* pLevel) override {}
	virtual void OnLoadingComplete(ILevelInfo* pLevel) override           {}
	virtual void OnLoadingError(ILevelInfo* pLevel, tukk error) override;
	virtual void OnLoadingProgress(ILevelInfo* pLevel, i32 progressAmount) override;
	virtual void OnUnloadComplete(ILevelInfo* pLevel) override {}
	// ~ILevelSystemListener

	// ILoadtimeCallback
	virtual void LoadtimeUpdate(float fDeltaTime) override;
	virtual void LoadtimeRender() override;
	// ~ILoadtimeCallback

	// logging
	static void LogUIAction(ELogEventLevel level, tukk format, ...) PRINTF_PARAMS(2, 3);

	// cvars
	DeclareStaticConstIntCVar(CV_gfx_draw, 1);
	DeclareStaticConstIntCVar(CV_gfx_debugdraw, 0);
	DeclareStaticConstIntCVar(CV_gfx_uiaction_log, 0);
	DeclareStaticConstIntCVar(CV_gfx_uiaction_enable, 1);
	DeclareStaticConstIntCVar(CV_gfx_loadtimethread, 1);
	DeclareStaticConstIntCVar(CV_gfx_reloadonlanguagechange, 1);
	DeclareStaticConstIntCVar(CV_gfx_uievents_editorenabled, 1);
	DeclareStaticConstIntCVar(CV_gfx_ampserver, 0);
	static i32    CV_gfx_enabled;
	static float  CV_gfx_inputevents_triggerstart;
	static float  CV_gfx_inputevents_triggerrepeat;
	static ICVar* CV_gfx_uiaction_log_filter;
	static ICVar* CV_gfx_uiaction_folder;

	static void ReloadAllElements(IConsoleCmdArgs* /* pArgs */);

	void        InvalidateSortedElements();

	bool        IsLoadtimeThread() const { return m_bLoadtimeThread; };

	EPlatformUI GetCurrentPlatform();

private:
	CFlashUI(const CFlashUI&) : m_modules(8) {}

	// cppcheck-suppress operatorEqVarError
	void operator=(const CFlashUI&) {}

	void RegisterListeners();
	void UnregisterListeners();

	void ReloadAll();

	void LoadElements();
	void ClearElements();

	void LoadActions();
	void ClearActions();

	void ResetActions();
	void ReloadScripts();

	void CreateNodes();
	void ClearNodes();

	void LoadFromFile(tukk sFolderName, tukk pSearch, bool (CFlashUI::* fhFileLoader)(tukk));
	bool LoadFGActionFromFile(tukk sFileName);
	bool LoadLuaActionFromFile(tukk sFileName);

	void PreloadTextures(tukk pLevelName = NULL);
	void PreloadTexturesFromNode(const XmlNodeRef& node);
	bool PreloadTexture(tukk pFileName);
	void ReleasePreloadedTextures(bool bReleaseTextures = true);

	typedef std::multimap<i32, IUIElement*> TSortedElementList;
	inline const TSortedElementList& GetSortedElements();
	inline void                      UpdateSortedElements();

	void                             CreateMouseClick(IUIElement::EControllerInputState state);

	void                             TriggerEvent(const SInputEvent& event);

	SFlashKeyEvent                   MapToFlashKeyEvent(const SInputEvent& inputEvent);

	TUIEventSystemMap*               GetEventSystemMap(IUIEventSystem::EEventSystemType eType);

	void                             StartRenderThread();
	void                             StopRenderThread();

	inline void                      CheckLanguageChanged();
	inline void                      CheckResolutionChange();

	void                             ReloadAllBootStrapper();
	void                             ResetDirtyFlags();

	static bool                      CheckFilter(const string& str);

private:
	CFlashUIActionEvents* m_pFlashUIActionEvents;

	TUIEventSystemMap     m_eventSystemsUiToSys;
	TUIEventSystemMap     m_eventSystemsSysToUi;

	TUIElementsLookup     m_elements;
	TUIActionsLookup      m_actions;

	TSortedElementList    m_sortedElements;
	bool                  m_bSortedElementsInvalidated;

	bool                  m_bLoadtimeThread;
	typedef std::vector<std::shared_ptr<IFlashPlayer>> TPlayerList;
	TPlayerList           m_loadtimePlayerList;

	std::vector<CFlashUiFlowNodeFactory*> m_UINodes;

	typedef std::map<ITexture*, string> TTextureMap;
	TTextureMap       m_preloadedTextures;

	CUIActionUpr* m_pUIActionUpr;

	typedef CListenerSet<IUIModule*> TUIModules;
	TUIModules                m_modules;

	i32                       m_iWidth;
	i32                       m_iHeight;
	TEditorScreenSizeCallback m_ScreenSizeCB;
	TEditorUILogEventCallback m_LogCallback;
	TEditorPlatformCallback   m_plattformCallback;

	enum ESystemState
	{
		eSS_NoLevel,
		eSS_Loading,
		eSS_LoadingDone,
		eSS_GameStarted,
		eSS_Unloading,
	};

	ESystemState m_systemState;
	float        m_fLastAdvance;
	float        m_lastTimeTriggered;
	bool         m_bHudVisible;
};

#endif // #ifndef __FlashUI_H__
