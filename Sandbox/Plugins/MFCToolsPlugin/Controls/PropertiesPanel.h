// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#if !defined(AFX_PROPERTIESPANEL_H__AD3E2ECE_EFEB_4A4C_81A7_216B2BC11BC5__INCLUDED_)
#define AFX_PROPERTIESPANEL_H__AD3E2ECE_EFEB_4A4C_81A7_216B2BC11BC5__INCLUDED_

#if _MSC_VER > 1000
	#pragma once
#endif // _MSC_VER > 1000
// PropertiesPanel.h : header file
//

#include "Controls\PropertyCtrl.h"
#include "Util\Variable.h"

/////////////////////////////////////////////////////////////////////////////
// CPropertiesPanel dialog

class PLUGIN_API CPropertiesPanel : public CDialog
{
public:
	CPropertiesPanel(CWnd* pParent = NULL);   // standard constructor

	typedef Functor1<IVariable*> UpdateCallback;

	void ReloadValues();

	void DeleteVars();
	void AddVars(class CVarBlock* vb, const UpdateCallback& func = NULL);

	void SetVarBlock(class CVarBlock* vb, const UpdateCallback& func = NULL, const bool resizeToFit = true);

	void         SetMultiSelect(bool bEnable);
	bool         IsMultiSelect() const { return m_multiSelect; };

	void         SetTitle(tukk title);
	virtual void ResizeToFitProperties();

	//////////////////////////////////////////////////////////////////////////
	// Helper functions.
	//////////////////////////////////////////////////////////////////////////
	template<class T>
	void SyncValue(CSmartVariable<T>& var, T& value, bool bCopyToUI, IVariable* pSrcVar = NULL)
	{
		if (bCopyToUI)
			var = value;
		else
		{
			if (!pSrcVar || pSrcVar == var.GetVar())
				value = var;
		}
	}
	void SyncValueFlag(CSmartVariable<bool>& var, i32& nFlags, i32 flag, bool bCopyToUI, IVariable* pVar = NULL)
	{
		if (bCopyToUI)
		{
			var = (nFlags & flag) != 0;
		}
		else
		{
			if (!pVar || var.GetVar() == pVar)
				nFlags = (var) ? (nFlags | flag) : (nFlags & (~flag));
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void AddVariable(CVariableBase& varArray, CVariableBase& var, tukk varName, u8 dataType = IVariable::DT_SIMPLE)
	{
		if (varName)
			var.SetName(varName);
		var.SetDataType(dataType);
		varArray.AddVariable(&var);
	}
	//////////////////////////////////////////////////////////////////////////
	void AddVariable(CVarBlock* vars, CVariableBase& var, tukk varName, u8 dataType = IVariable::DT_SIMPLE)
	{
		if (varName)
			var.SetName(varName);
		var.SetDataType(dataType);
		vars->AddVariable(&var);
	}
	void UpdateVarBlock(CVarBlock* vars);

	//////////////////////////////////////////////////////////////////////////
	CPropertyCtrl* GetPropertyCtrl() { return m_pWndProps.get(); }

protected:
	void         OnPropertyChanged(IVariable* pVar);

	virtual void OnOK()     {};
	virtual void OnCancel() {};

	//{{AFX_VIRTUAL(CPropertiesPanel)
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CPropertiesPanel)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSize(UINT nType, i32 cx, i32 cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	std::unique_ptr<CPropertyCtrl> m_pWndProps;
	XmlNodeRef                   m_template;
	bool                         m_multiSelect;
	_smart_ptr<CVarBlock>        m_varBlock;

	std::list<UpdateCallback>    m_updateCallbacks;
	i32                          m_titleAdjust;
};

#endif // !defined(AFX_PROPERTIESPANEL_H__AD3E2ECE_EFEB_4A4C_81A7_216B2BC11BC5__INCLUDED_)

