// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __VehicleFXPanel_h__
#define __VehicleFXPanel_h__
#pragma once

#include "PropertyCtrlExt.h"
#include "VehicleDialogComponent.h"

class CVehicleEditorDialog;

/*!
 * CVehicleFXPanel
 */
class CVehicleFXPanel : public CWnd, public IVehicleDialogComponent
{
	DECLARE_DYNAMIC(CVehicleFXPanel)
public:
	CVehicleFXPanel(CVehicleEditorDialog* pDialog);
	virtual ~CVehicleFXPanel();

	void UpdateVehiclePrototype(CVehiclePrototype* pProt);
	void OnPaneClose() {}

	void OnFXTypeChange(IVariable* pVar);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg i32  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, i32 cx, i32 cy);

	void         ExpandProps(CPropertyItem* pItem, bool expand = true);
	void         AddCategory(IVariable* pVar);

	CVehiclePrototype* m_pVehicle;

	// pointer to main dialog
	CVehicleEditorDialog* m_pDialog;

	// controls
	CPropertyCtrlExt         m_propsCtrl;

	IVariable::OnSetCallback m_onSetCallback;
};

#endif // __VehicleFXPanel_h__

