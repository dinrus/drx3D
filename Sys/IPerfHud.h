// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
 
//! \cond INTERNAL

#pragma once

#include <drx3D/CoreX/Extension/IDrxUnknown.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>
#include <drx3D/Sys/IDrxMiniGUI.h>
#include <drx3D/Sys/IXml.h>

struct IDrxPerfHUDWidget : public _reference_target_t
{
	enum EWidgetID
	{
		eWidget_Warnings = 0,
		eWidget_RenderStats,
		eWidget_StreamingStats,
		eWidget_RenderBatchStats,
		eWidget_FpsBuckets,
		eWidget_Particles,
		eWidget_PakFile,
		eWidget_Num,    //!< Number of widgets.
	};

	IDrxPerfHUDWidget(i32 id = -1) :
		m_id(id)
	{}

	// <interfuscator:shuffle>
	virtual ~IDrxPerfHUDWidget() {}

	virtual void Reset() = 0;
	virtual void Update() = 0;
	virtual bool ShouldUpdate() = 0;
	virtual void LoadBudgets(XmlNodeRef perfXML) = 0;
	virtual void SaveStats(XmlNodeRef statsXML) = 0;
	virtual void Enable(i32 mode) = 0;
	virtual void Disable() = 0;
	// </interfuscator:shuffle>

	i32 m_id;
};

//! Base Interface for all engine module extensions.
struct IDrxPerfHUD : public IDrxUnknown
{
	DRXINTERFACE_DECLARE_GUID(IDrxPerfHUD, "268d142e-043d-464c-a077-6580f81b988a"_drx_guid);

	struct PerfBucket
	{
		ILINE PerfBucket(float _target)
		{
			target = _target;
			timeAtTarget = 0.f;
		}

		float target;
		float timeAtTarget;
	};

	enum EHudState
	{
		eHudOff = 0,
		eHudInFocus,
		eHudOutOfFocus,
		eHudNumStates,
	};

	// <interfuscator:shuffle>
	//! Called once to initialize HUD.
	virtual void Init() = 0;
	virtual void Done() = 0;
	virtual void Draw() = 0;
	virtual void LoadBudgets() = 0;
	virtual void SaveStats(tukk filename = NULL) = 0;
	virtual void ResetWidgets() = 0;
	virtual void SetState(EHudState state) = 0;
	virtual void Reset() = 0;
	virtual void Destroy() = 0;

	//! Retrieve name of the extension module.
	virtual void                   Show(bool bRestoreState) = 0;

	virtual void                   AddWidget(IDrxPerfHUDWidget* pWidget) = 0;
	virtual void                   RemoveWidget(IDrxPerfHUDWidget* pWidget) = 0;

	virtual minigui::IMiniCtrl*    CreateMenu(tukk name, minigui::IMiniCtrl* pParent = NULL) = 0;
	virtual bool                   CreateCVarMenuItem(minigui::IMiniCtrl* pMenu, tukk name, tukk controlVar, float controlVarOn, float controlVarOff) = 0;
	virtual bool                   CreateCallbackMenuItem(minigui::IMiniCtrl* pMenu, tukk name, minigui::ClickCallback clickCallback, uk pCallbackData) = 0;
	virtual minigui::IMiniInfoBox* CreateInfoMenuItem(minigui::IMiniCtrl* pMenu, tukk name, minigui::RenderCallback renderCallback, const minigui::Rect& rect, bool onAtStart = false) = 0;
	virtual minigui::IMiniTable*   CreateTableMenuItem(minigui::IMiniCtrl* pMenu, tukk name) = 0;

	virtual minigui::IMiniCtrl*    GetMenu(tukk name) = 0;

	virtual void                   EnableWidget(IDrxPerfHUDWidget::EWidgetID id, i32 mode) = 0;
	virtual void                   DisableWidget(IDrxPerfHUDWidget::EWidgetID id) = 0;

	//! Warnings-widget Specific interface.
	virtual void AddWarning(float duration, tukk fmt, va_list argList) = 0;
	virtual bool WarningsWindowEnabled() const = 0;

	//! FPS-widget Specific interface.
	virtual const std::vector<PerfBucket>* GetFpsBuckets(float& totalTime) const = 0;
	// </interfuscator:shuffle>
};

DECLARE_SHARED_POINTERS(IDrxPerfHUD);

void        DrxPerfHUDWarning(float duration, tukk , ...) PRINTF_PARAMS(2, 3);
inline void DrxPerfHUDWarning(float duration, tukk format, ...)
{
	if (gEnv && gEnv->pSystem)
	{
		IDrxPerfHUD* pPerfHud = gEnv->pSystem->GetPerfHUD();

		if (pPerfHud)
		{
			va_list args;
			va_start(args, format);
			pPerfHud->AddWarning(duration, format, args);
			va_end(args);
		}
	}
}

//! \endcond