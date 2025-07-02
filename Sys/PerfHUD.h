// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   PerfHUD.h
//  Created:     26/08/2009 by Timur.
//  Описание: Button implementation in the MiniGUI
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __PerfHUD_h__
#define __PerfHUD_h__

#include <drx3D/Sys/IPerfHud.h>

#ifdef USE_PERFHUD

	#include <drx3D/Input/IInput.h>
	#include <drx3D/Sys/MiniInfoBox.h>
	#include <drx3D/Sys/MiniTable.h>

//Macros for console based widget control

	#define SET_WIDGET_DECL(WIDGET_NAME) \
	  static i32 s_cvar_ ## WIDGET_NAME; \
	  static void Set_ ## WIDGET_NAME ## _Widget(ICVar * pCvar);

	#define SET_WIDGET_DEF(WIDGET_NAME, WIDGET_ID)                     \
	  i32 CPerfHUD::s_cvar_ ## WIDGET_NAME = 0;                        \
	  void CPerfHUD::Set_ ## WIDGET_NAME ## _Widget(ICVar * pCvar)     \
	  {                                                                \
	    IDrxPerfHUD* pPerfHud = gEnv->pSystem->GetPerfHUD();           \
	    if (pPerfHud)                                                  \
	    {                                                              \
	      i32 val = pCvar->GetIVal();                                  \
	      if (val)                                                     \
	      {                                                            \
	        pPerfHud->SetState(eHudOutOfFocus);                        \
	        pPerfHud->EnableWidget(IDrxPerfHUDWidget::WIDGET_ID, val); \
	      }                                                            \
	      else                                                         \
	      {                                                            \
	        pPerfHud->DisableWidget(IDrxPerfHUDWidget::WIDGET_ID);     \
	      }                                                            \
	    }                                                              \
	  }

	#define SET_WIDGET_COMMAND(COMMAND_NAME, WIDGET_NAME) \
	  REGISTER_CVAR2_CB(COMMAND_NAME, &s_cvar_ ## WIDGET_NAME, 0, VF_ALWAYSONCHANGE, "", Set_ ## WIDGET_NAME ## _Widget);

//////////////////////////////////////////////////////////////////////////
// Root window all other controls derive from
class CPerfHUD : public IDrxPerfHUD, public minigui::IMiniGUIEventListener, public IInputEventListener
{
public:
	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(IDrxPerfHUD)
	DRXINTERFACE_END()
	DRXGENERATE_SINGLETONCLASS_GUID(CPerfHUD, "PerfHUD", "006945f9-985e-4ce2-8721-20bfdec09ca5"_drx_guid)

	CPerfHUD();
	virtual ~CPerfHUD() {}

public:
	//////////////////////////////////////////////////////////////////////////
	// IDrxPerfHUD implementation
	//////////////////////////////////////////////////////////////////////////
	virtual void Init() override;
	virtual void Done() override;
	virtual void Draw() override;
	virtual void LoadBudgets() override;
	virtual void SaveStats(tukk filename) override;
	virtual void ResetWidgets() override;
	virtual void Reset() override;
	virtual void Destroy() override;

	//Set state through code (rather than using joypad input)
	virtual void SetState(EHudState state) override;

	virtual void Show(bool bRestoreState) override;

	virtual void AddWidget(IDrxPerfHUDWidget* pWidget) override;
	virtual void RemoveWidget(IDrxPerfHUDWidget* pWidget) override;

	//////////////////////////////////////////////////////////////////////////
	// Gui Creation helper funcs
	//////////////////////////////////////////////////////////////////////////
	virtual minigui::IMiniCtrl*    CreateMenu(tukk name, minigui::IMiniCtrl* pParent = NULL) override;
	virtual bool                   CreateCVarMenuItem(minigui::IMiniCtrl* pMenu, tukk name, tukk controlVar, float controlVarOn, float controlVarOff) override;
	virtual bool                   CreateCallbackMenuItem(minigui::IMiniCtrl* pMenu, tukk name, minigui::ClickCallback callback, uk pCallbackData) override;
	virtual minigui::IMiniInfoBox* CreateInfoMenuItem(minigui::IMiniCtrl* pMenu, tukk name, minigui::RenderCallback renderCallback, const minigui::Rect& rect, bool onAtStart = false) override;
	virtual minigui::IMiniTable*   CreateTableMenuItem(minigui::IMiniCtrl* pMenu, tukk name) override;

	virtual minigui::IMiniCtrl*    GetMenu(tukk name) override;

	virtual void                   EnableWidget(IDrxPerfHUDWidget::EWidgetID id, i32 mode) override;
	virtual void                   DisableWidget(IDrxPerfHUDWidget::EWidgetID id) override;

	//////////////////////////////////////////////////////////////////////////
	// WARNINGS - Widget Specific Interface
	//////////////////////////////////////////////////////////////////////////
	virtual void AddWarning(float duration, tukk fmt, va_list argList) override;
	virtual bool WarningsWindowEnabled() const override;

	//////////////////////////////////////////////////////////////////////////
	// FPS - Widget Specific Interface
	//////////////////////////////////////////////////////////////////////////
	virtual const std::vector<PerfBucket>* GetFpsBuckets(float& totalTime) const override;

	//////////////////////////////////////////////////////////////////////////
	// IMiniGUIEventListener implementation
	//////////////////////////////////////////////////////////////////////////
	virtual void OnCommand(minigui::SCommand& cmd) override;
	//////////////////////////////////////////////////////////////////////////

	// IInputEventListener
	virtual bool OnInputEvent(const SInputEvent& rInputEvent) override;

	//////////////////////////////////////////////////////////////////////////
	// CLICK CALLBACKS
	//////////////////////////////////////////////////////////////////////////

	static void ResetCallback(uk data, bool status);
	static void ReloadBudgetsCallback(uk data, bool status);
	static void SaveStatsCallback(uk data, bool status);

	//////////////////////////////////////////////////////////////////////////
	// RENDER CALLBACKS
	//////////////////////////////////////////////////////////////////////////

	static void DisplayRenderInfoCallback(const minigui::Rect& rect);

	//////////////////////////////////////////////////////////////////////////
	// CVAR CALLBACK
	//////////////////////////////////////////////////////////////////////////

	static void CVarChangeCallback(ICVar* pCvar);

	SET_WIDGET_DECL(Warnings);
	SET_WIDGET_DECL(RenderSummary);
	SET_WIDGET_DECL(RenderBatchStats);
	SET_WIDGET_DECL(FpsBuckets);
	SET_WIDGET_DECL(Particles);
	SET_WIDGET_DECL(PakFile);

	//////////////////////////////////////////////////////////////////////////
	// Static Data
	//////////////////////////////////////////////////////////////////////////

	static const float  OVERSCAN_X;
	static const float  OVERSCAN_Y;

	static const ColorB COL_NORM;
	static const ColorB COL_WARN;
	static const ColorB COL_ERROR;

	static const float  TEXT_SIZE_NORM;
	static const float  TEXT_SIZE_WARN;
	static const float  TEXT_SIZE_ERROR;

	static const float  ACTIVATE_TIME_FROM_GAME;
	static const float  ACTIVATE_TIME_FROM_HUD;

protected:
	void InitUI(minigui::IMiniGUI* pGUI);
	void SetNextState();

protected:

	static i32 m_sys_perfhud;
	static i32 m_sys_perfhud_pause;

	i32        m_sys_perfhud_prev;

	//record last menu position
	float     m_menuStartX;
	float     m_menuStartY;

	bool      m_hudCreated;

	bool      m_L1Pressed;
	bool      m_L2Pressed;
	bool      m_R1Pressed;
	bool      m_R2Pressed;
	bool      m_changingState;
	bool      m_hwMouseEnabled;

	float     m_triggersDownStartTime;

	EHudState m_hudState;
	EHudState m_hudLastState;

	typedef std::vector<_smart_ptr<IDrxPerfHUDWidget>, stl::STLGlobalAllocator<_smart_ptr<IDrxPerfHUDWidget>>>::iterator TWidgetIterator;

	std::vector<_smart_ptr<IDrxPerfHUDWidget>, stl::STLGlobalAllocator<_smart_ptr<IDrxPerfHUDWidget>>> m_widgets;
	std::vector<minigui::IMiniCtrl*, stl::STLGlobalAllocator<minigui::IMiniCtrl*>>                     m_rootMenus;
};

class CFpsWidget : public IDrxPerfHUDWidget
{
public:

	CFpsWidget(minigui::IMiniCtrl* pParentMenu, IDrxPerfHUD* pPerfHud);

	virtual void                                Reset();
	virtual void                                Update();
	virtual bool                                ShouldUpdate();
	virtual void                                LoadBudgets(XmlNodeRef perfXML);
	virtual void                                SaveStats(XmlNodeRef statsXML);
	virtual void                                Enable(i32 mode) { m_pInfoBox->Hide(false); }
	virtual void                                Disable()        { m_pInfoBox->Hide(true); }

	void                                        Init();

	const std::vector<IDrxPerfHUD::PerfBucket>* GetFpsBuckets(float& totalTime) const;

	static void                                 ResetCallback(uk data, bool status);

protected:

	static u32k NUM_FPS_BUCKETS_DEFAULT = 6;

	struct PerfBucketsStat
	{
		std::vector<IDrxPerfHUD::PerfBucket> buckets;
		float                                totalTime;
	};

	template<bool LESS_THAN>
	void UpdateBuckets(PerfBucketsStat& buckets, float frameTime, tukk name, float stat);

	// Data
	static i32 m_cvarPerfHudFpsExclusive;

	enum EPerfBucketType
	{
		BUCKET_FPS = 0,
		BUCKET_GPU,
		BUCKET_DP,
		BUCKET_TYPE_NUM
	};

	PerfBucketsStat        m_perfBuckets[BUCKET_TYPE_NUM];

	float                  m_fpsBucketSize;
	float                  m_fpsBudget;
	float                  m_dpBudget;
	float                  m_dpBucketSize;

	minigui::IMiniInfoBox* m_pInfoBox;
};

class CRenderStatsWidget : public IDrxPerfHUDWidget
{
public:

	CRenderStatsWidget(minigui::IMiniCtrl* pParentMenu, IDrxPerfHUD* pPerfHud);

	virtual void Reset() {}
	virtual void Update();
	virtual bool ShouldUpdate();
	virtual void LoadBudgets(XmlNodeRef perfXML);
	virtual void SaveStats(XmlNodeRef statsXML);
	virtual void Enable(i32 mode) { m_pInfoBox->Hide(false); }
	virtual void Disable()        { m_pInfoBox->Hide(true); }

protected:

	//budgets
	float  m_fpsBudget;
	u32 m_dpBudget;
	u32 m_polyBudget;
	u32 m_postEffectBudget;
	u32 m_shadowCastBudget;
	u32 m_particlesBudget;

	//runtime data
	struct SRuntimeData
	{
		Vec3   cameraPos;
		Ang3   cameraRot;
		float  fps;
		u32 nDrawPrims;
		u32 nPolys;
		u32 nPostEffects;
		u32 nFwdLights;
		u32 nFwdShadowLights;
		u32 nDefLights;
		u32 nDefShadowLights;
		u32 nDefCubeMaps;
		i32    nParticles;
		bool   hdrEnabled;
		bool   renderThreadEnabled;
	};

	SRuntimeData           m_runtimeData;
	minigui::IMiniInfoBox* m_pInfoBox;
	IDrxPerfHUD*           m_pPerfHUD;
	u32                 m_buildNum;
};

class CStreamingStatsWidget : public IDrxPerfHUDWidget
{
public:

	CStreamingStatsWidget(minigui::IMiniCtrl* pParentMenu, IDrxPerfHUD* pPerfHud);

	virtual void Reset() {}
	virtual void Update();
	virtual bool ShouldUpdate();
	virtual void LoadBudgets(XmlNodeRef perfXML);
	virtual void SaveStats(XmlNodeRef statsXML) {};
	virtual void Enable(i32 mode)               { m_pInfoBox->Hide(false); }
	virtual void Disable()                      { m_pInfoBox->Hide(true); }

protected:
	//float m_maxMeshSizeArroundMB;
	//float m_maxTextureSizeArroundMB;
	minigui::IMiniInfoBox* m_pInfoBox;
	IDrxPerfHUD*           m_pPerfHUD;
};

class CWarningsWidget : public IDrxPerfHUDWidget
{
public:

	CWarningsWidget(minigui::IMiniCtrl* pParentMenu, IDrxPerfHUD* pPerfHud);

	virtual void Reset();
	virtual void Update();
	virtual bool ShouldUpdate();
	virtual void LoadBudgets(XmlNodeRef perfXML) {};
	virtual void SaveStats(XmlNodeRef statsXML);
	virtual void Enable(i32 mode)                { m_pInfoBox->Hide(false); }
	virtual void Disable()                       { m_pInfoBox->Hide(true); }

	void         AddWarningV(float duration, tukk fmt, va_list argList);
	void         AddWarning(float duration, tukk warning);

protected:

	static u32k WARNING_LENGTH = 64;

	struct SWarning
	{
		char  text[WARNING_LENGTH];
		float remainingDuration;
	};

	minigui::IMiniInfoBox* m_pInfoBox;

	typedef std::vector<SWarning> TSWarnings;

	TSWarnings             m_warnings;

	DrxMT::queue<SWarning> m_threadWarnings;

	threadID               m_nMainThreadId;
};

class CRenderBatchWidget : public IDrxPerfHUDWidget
{
public:

	CRenderBatchWidget(minigui::IMiniCtrl* pParentMenu, IDrxPerfHUD* pPerfHud);

	virtual void Reset();
	virtual void Update();
	virtual bool ShouldUpdate();
	virtual void LoadBudgets(XmlNodeRef perfXML) {};
	virtual void SaveStats(XmlNodeRef statsXML);
	virtual void Enable(i32 mode);
	virtual void Disable();

	void         Update_ModeGpuTimes();
	void         Update_ModeBatchStats();

protected:

	struct BatchInfoGpuTimes
	{
		tukk name;
		u32      nBatches;
		float       gpuTime;
		i32         numVerts;
		i32         numIndices;
	};

	struct BatchInfoSortGpuTimes
	{
		inline bool operator()(const BatchInfoGpuTimes& lhs, const BatchInfoGpuTimes& rhs) const
		{
			return lhs.gpuTime > rhs.gpuTime;
		}
	};

	struct BatchInfoPerPass
	{
		tukk name;
		u16      nBatches;
		u16      nInstances;
		u16      nZpass;
		u16      nShadows;
		u16      nGeneral;
		u16      nTransparent;
		u16      nMisc;
		ColorB      col;

		BatchInfoPerPass()
		{
			Reset();
		}

		void Reset()
		{
			name = NULL;
			nBatches = 0;
			nInstances = 0;
			nZpass = 0;
			nShadows = 0;
			nGeneral = 0;
			nTransparent = 0;
			nMisc = 0;
			col.set(255, 255, 255, 255);
		}

		void operator+=(const BatchInfoPerPass& rhs)
		{
			nBatches += rhs.nBatches;
			nInstances += rhs.nInstances;
			nZpass += rhs.nZpass;
			nShadows += rhs.nShadows;
			nGeneral += rhs.nGeneral;
			nTransparent += rhs.nTransparent;
			nMisc += rhs.nMisc;
		}
	};

	struct BatchInfoSortPerPass
	{
		inline bool operator()(const BatchInfoPerPass* lhs, const BatchInfoPerPass* rhs) const
		{
			return lhs->nBatches > rhs->nBatches;
		}
	};

	enum EDisplayMode
	{
		DISPLAY_MODE_NONE = 0,
		DISPLAY_MODE_BATCH_STATS,
		DISPLAY_MODE_GPU_TIMES,
		DISPLAY_MODE_NUM,
	};

	minigui::IMiniTable* m_pTable;
	ICVar*               m_pRStatsCVar;
	EDisplayMode         m_displayMode;
};

#endif //USE_PERFHUD

#endif // __PerfHUD_h__
