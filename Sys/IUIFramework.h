// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <drx3D/Sys/IFlashPlayer.h>

struct IUILayout;
struct SUILayoutEvent;
struct IUILayoutListener;
struct IUIObject;
struct IInworldUI;
struct IGameFramework;

#define DEFAULT_LAYOUT_ID 0
#define INVALID_LAYOUT_ID 0xFFFFFFFF

typedef DynArray<string>         UINameDynArray;
typedef DynArray<SUILayoutEvent> UIEventDynArray;

enum EUINavigate
{
	eUINavigate_None = 0,
	eUINavigate_Up,
	eUINavigate_Down,
	eUINavigate_Left,
	eUINavigate_Right,
	eUINavigate_Confirm,
	eUINavigate_Back
};

struct IUILayoutBase
{
	virtual ~IUILayoutBase(){}
	virtual void          Unload() = 0;

	virtual std::shared_ptr<IFlashPlayer> GetPlayer() = 0;
};

namespace UIFramework
{
struct IUIFramework : public IDrxUnknown
{
	DRXINTERFACE_DECLARE_GUID(IUIFramework, "89f04b15-741a-40de-94ad-79a8ac3b7419"_drx_guid)

	virtual IUILayout*     GetLayout(tukk layoutName, u32k layoutId = DEFAULT_LAYOUT_ID) = 0;
	virtual IUILayoutBase* GetLayoutBase(tukk layoutName, u32k layoutId = DEFAULT_LAYOUT_ID) = 0;
	virtual void           GetAllLayoutNames(UINameDynArray& layoutNames) const = 0;
	virtual u32         LoadLayout(tukk layoutName) = 0;
	virtual void           UnloadLayout(tukk layoutName, u32k layoutId = DEFAULT_LAYOUT_ID) = 0;
	virtual void           SetLoadingThread(const bool bLoadTime) = 0;
	virtual IInworldUI*    GetInworldUI() = 0;
	virtual void           Init() = 0;
	virtual void           Clear() = 0;
	virtual void           ScheduleReload() = 0;

	virtual bool           IsEditing() = 0;

	virtual void           Navigate(const EUINavigate navigate) = 0;

	virtual void           RegisterAutoLayoutListener(IUILayoutListener* pListener) = 0;
	virtual void           UnregisterAutoLayoutListener(IUILayoutListener* pListener) = 0;
};

IUIFramework* CreateFramework(IGameFramework* pGameFramework);
}
