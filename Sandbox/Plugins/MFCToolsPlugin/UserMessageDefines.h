// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef USERMESSAGEDEFINES_H
#define USERMESSAGEDEFINES_H

enum ESandboxUserMessages
{
	// InPlaceComboBox
	WM_USER_ON_SELECTION_CANCEL = WM_USER + 1,
	WM_USER_ON_SELECTION_OK,
	WM_USER_ON_NEW_SELECTION,
	WM_USER_ON_EDITCHANGE,
	WM_USER_ON_OPENDROPDOWN,
	WM_USER_ON_EDITKEYDOWN,
	WM_USER_ON_EDITCLICK,
	// ACListWnd
	ENAC_UPDATE,
	// EditWithButton
	WM_USER_EDITWITHBUTTON_CLICKED,
	// FillSliderCtrl
	WMU_FS_CHANGED,
	WMU_FS_LBUTTONDOWN,
	WMU_FS_LBUTTONUP,
	FLM_EDITTEXTCHANGED,
	FLM_FILTERTEXTCHANGED,
	// NumberCtrlEdit
	WMU_LBUTTONDOWN,
	WMU_LBUTTONUP,
	// Mannequin/CharacterEditor
	WM_ONWINDOWFOCUSCHANGES,
	// SelectObjectDialog
	IDT_TIMER_0,
	IDT_TIMER_1,
	// LensFlareEditor
	WM_FLAREEDITOR_UPDATETREECONTROL,
	// EquipPackDialog
	UM_EQUIPLIST_CHECKSTATECHANGE,
	// Hypergraph
	WM_HYPERGRAPHVIEW_SELECTION_CHANGE,
	// MaterialSender/MatEditMainDlg
	WM_MATEDITPICK,
	// GridMapWindow
	WM_USER_ON_DBL_CLICK,
	// LMCompDialog
	WM_UPDATE_LIGHTMAP_GENERATION_PROGRESS,
	WM_UPDATE_LIGHTMAP_GENERATION_MEMUSAGE,
	WM_UPDATE_LIGHTMAP_GENERATION_MEMUSAGE_STATIC,
	WM_UPDATE_GLM_NAME_EDIT,
	// Viewport
	WM_VIEWPORT_ON_TITLE_CHANGE,
};
#endif

