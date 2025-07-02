// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "VehiclePaintsPanel.h"

#include "VehicleData.h"
#include "FilePathUtil.h"

#include <drx3D/CoreX/Renderer/IRenderer.h>

#include "Material/MaterialManager.h"

IMPLEMENT_DYNAMIC(CVehiclePaintsPanel, CXTResizeDialog)

BEGIN_MESSAGE_MAP(CVehiclePaintsPanel, CXTResizeDialog)
ON_BN_CLICKED(IDC_VEHICLE_PAINTS_NEW, OnAddNewPaint)
ON_BN_CLICKED(IDC_VEHICLE_PAINTS_REMOVE, OnRemoveSelectedPaint)
ON_BN_CLICKED(IDC_VEHICLE_PAINTS_ASSIGN_MATERIAL, OnAssignMaterialToSelectedPaint)
ON_BN_CLICKED(IDC_VEHICLE_PAINTS_APPLY, OnApplyPaintToVehicle)
ON_LBN_SELCHANGE(IDC_VEHICLE_PAINTS_NAMES, OnPaintNameSelectionChanged)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
CVehiclePaintsPanel::CVehiclePaintsPanel()
	: m_pPaints(NULL)
	, m_pSelectedPaint(NULL)
{
	GetIEditor()->GetMaterialManager()->AddListener(this);
}

//////////////////////////////////////////////////////////////////////////
CVehiclePaintsPanel::~CVehiclePaintsPanel()
{
	GetIEditor()->GetMaterialManager()->RemoveListener(this);
}

//////////////////////////////////////////////////////////////////////////
BOOL CVehiclePaintsPanel::OnInitDialog()
{
	__super::OnInitDialog();

	SetResize(IDC_VEHICLE_PAINTS_NAMES, SZ_TOP_LEFT, SZ_BOTTOM_RIGHT);
	SetResize(IDC_VEHICLE_PAINTS_NEW, SZ_TOP_LEFT, SZ_TOP_LEFT);
	SetResize(IDC_VEHICLE_PAINTS_REMOVE, SZ_TOP_LEFT, SZ_TOP_LEFT);
	SetResize(IDC_VEHICLE_PAINTS_ASSIGN_MATERIAL, SZ_TOP_LEFT, SZ_TOP_LEFT);
	SetResize(IDC_VEHICLE_PAINTS_APPLY, SZ_TOP_RIGHT, SZ_TOP_RIGHT);
	SetResize(IDC_VEHICLE_PAINTS_PROPERTIES, SZ_BOTTOM_LEFT, SZ_BOTTOM_RIGHT);

	UpdateAssignMaterialButtonState();

	// Hiding the apply to vehicle button as this functionality is not yet implemented.
	m_applyToVehicleButton.ShowWindow(SW_HIDE);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
void CVehiclePaintsPanel::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_VEHICLE_PAINTS_NAMES, m_paintNames);
	DDX_Control(pDX, IDC_VEHICLE_PAINTS_PROPERTIES, m_paintProperties);
	DDX_Control(pDX, IDC_VEHICLE_PAINTS_ASSIGN_MATERIAL, m_applyMaterialButton);
	DDX_Control(pDX, IDC_VEHICLE_PAINTS_APPLY, m_applyToVehicleButton);
}

//////////////////////////////////////////////////////////////////////////
void CVehiclePaintsPanel::InitPaints(IVariable* pPaints)
{
	if (pPaints)
	{
		m_pPaints = pPaints;
		RebuildPaintNames();
	}
	else
	{
		DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_WARNING, "Could not initialize paints for vehicle");
	}
}

//////////////////////////////////////////////////////////////////////////
void CVehiclePaintsPanel::Clear()
{
	ClearPaintNames();
	m_pPaints = NULL;
}

//////////////////////////////////////////////////////////////////////////
void CVehiclePaintsPanel::RebuildPaintNames()
{
	assert(m_pPaints != NULL);
	if (m_pPaints == NULL)
	{
		return;
	}

	ClearPaintNames();

	for (i32 i = 0; i < m_pPaints->GetNumVariables(); i++)
	{
		IVariable* pPaint = m_pPaints->GetVariable(i);
		IVariable* pPaintName = GetChildVar(pPaint, "name");
		if (pPaintName != NULL)
		{
			string paintName;
			pPaintName->Get(paintName);

			AddPaintName(paintName, i);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CVehiclePaintsPanel::CreateNewPaint(const string& paintName)
{
	if (m_pPaints == NULL)
	{
		return;
	}

	IVariablePtr pNewPaint(new CVariableArray());
	pNewPaint->SetName("Paint");

	IVariablePtr pPaintName(new CVariable<string>());
	pPaintName->SetName("name");
	pPaintName->Set(paintName);
	pNewPaint->AddVariable(pPaintName);

	IVariablePtr pPaintMaterial(new CVariable<string>());
	pPaintMaterial->SetName("material");
	pNewPaint->AddVariable(pPaintMaterial);

	m_pPaints->AddVariable(pNewPaint);
	i32 paintVarIndex = m_pPaints->GetNumVariables() - 1;

	AddPaintName(paintName, paintVarIndex);
}

//////////////////////////////////////////////////////////////////////////
void CVehiclePaintsPanel::OnAddNewPaint()
{
	CreateNewPaint("NewPaint");
}

//////////////////////////////////////////////////////////////////////////
void CVehiclePaintsPanel::OnRemoveSelectedPaint()
{
	if (m_pPaints == NULL)
	{
		return;
	}
	i32 selectedPaintVarIndex = GetSelectedPaintNameVarIndex();

	IVariable* pPaintToRemove = GetPaintVarByIndex(selectedPaintVarIndex);
	if (pPaintToRemove == NULL)
	{
		return;
	}

	m_pPaints->DeleteVariable(pPaintToRemove);

	// Removing a paint invalidates the indices stored in the user data of the paint names control
	// The easiest way to restore it is to just rebuild the items stored in there.
	RebuildPaintNames();
}

//////////////////////////////////////////////////////////////////////////
void CVehiclePaintsPanel::OnDataBaseItemEvent(IDataBaseItem* pItem, EDataBaseItemEvent event)
{
	// Listening to material browser item selection changes.
	if (event == EDB_ITEM_EVENT_SELECTED)
	{
		UpdateAssignMaterialButtonState();
	}
}

//////////////////////////////////////////////////////////////////////////
void CVehiclePaintsPanel::OnAssignMaterialToSelectedPaint()
{
	if (m_pSelectedPaint == NULL)
	{
		return;
	}

	CMaterial* pMaterial = GetIEditor()->GetMaterialManager()->GetCurrentMaterial();
	assert(pMaterial != NULL);
	if (pMaterial == NULL)
	{
		return;
	}

	IVariable* pMaterialVar = GetChildVar(m_pSelectedPaint, "material");
	if (pMaterialVar == NULL)
	{
		return;
	}

	string materialName = PathUtil::AbsolutePathToGamePath(pMaterial->GetFilename().GetString());
	pMaterialVar->Set(materialName);
}

//////////////////////////////////////////////////////////////////////////
void CVehiclePaintsPanel::OnApplyPaintToVehicle()
{

}

//////////////////////////////////////////////////////////////////////////
bool CVehiclePaintsPanel::IsPaintNameSelected() const
{
	i32 selectedItemIndex = m_paintNames.GetCurSel();
	bool validItemIndex = (0 <= selectedItemIndex && selectedItemIndex < m_paintNames.GetCount());
	return validItemIndex;
}

//////////////////////////////////////////////////////////////////////////
string CVehiclePaintsPanel::GetSelectedPaintName() const
{
	assert(IsPaintNameSelected());

	CString selectedPaintName;

	i32 selectedItemIndex = m_paintNames.GetCurSel();
	m_paintNames.GetText(selectedItemIndex, selectedPaintName);

	return selectedPaintName.GetString();
}

//////////////////////////////////////////////////////////////////////////
i32 CVehiclePaintsPanel::GetSelectedPaintNameVarIndex() const
{
	assert(IsPaintNameSelected());

	i32 selectedItemIndex = m_paintNames.GetCurSel();
	i32 paintVarIndex = static_cast<i32>(m_paintNames.GetItemData(selectedItemIndex));

	return paintVarIndex;
}

//////////////////////////////////////////////////////////////////////////
void CVehiclePaintsPanel::RenameSelectedPaintName(const string& name)
{
	if (!IsPaintNameSelected())
	{
		return;
	}

	i32 selectedItemIndex = m_paintNames.GetCurSel();

	DWORD_PTR itemData = m_paintNames.GetItemData(selectedItemIndex);

	m_paintNames.DeleteString(selectedItemIndex);
	i32 newSelectedItemIndex = m_paintNames.AddString(name);

	m_paintNames.SetItemData(newSelectedItemIndex, itemData);
	m_paintNames.SetCurSel(newSelectedItemIndex);
}

//////////////////////////////////////////////////////////////////////////
void CVehiclePaintsPanel::AddPaintName(const string& paintName, i32 paintVarIndex)
{
	i32 itemPosition = m_paintNames.AddString(paintName);
	m_paintNames.SetItemData(itemPosition, static_cast<DWORD_PTR>(paintVarIndex));

	m_paintNames.SetCurSel(itemPosition);
	OnPaintNameSelectionChanged(); // Called manually as SetCurSel doesn't trigger ON_LBN_SELCHANGE
}

//////////////////////////////////////////////////////////////////////////
void CVehiclePaintsPanel::ClearPaintNames()
{
	HidePaintProperties();
	m_paintNames.ResetContent();
}

//////////////////////////////////////////////////////////////////////////
void CVehiclePaintsPanel::OnPaintNameSelectionChanged()
{
	if (!IsPaintNameSelected())
	{
		HidePaintProperties();
		return;
	}

	i32 selectedPaintVarIndex = GetSelectedPaintNameVarIndex();
	ShowPaintPropertiesByVarIndex(selectedPaintVarIndex);
}

//////////////////////////////////////////////////////////////////////////
void CVehiclePaintsPanel::HidePaintProperties()
{
	if (m_pSelectedPaint != NULL)
	{
		IVariable* pSelectedPaintName = GetChildVar(m_pSelectedPaint, "name");
		if (pSelectedPaintName != NULL)
		{
			pSelectedPaintName->RemoveOnSetCallback(functor(*this, &CVehiclePaintsPanel::OnSelectedPaintNameChanged));
		}

		IVariable* pSeletedPaintMaterial = GetChildVar(m_pSelectedPaint, "material");
		if (pSeletedPaintMaterial != NULL)
		{
			pSeletedPaintMaterial->RemoveOnSetCallback(functor(*this, &CVehiclePaintsPanel::OnSelectedPaintMaterialChanged));
		}
	}

	m_paintProperties.RemoveAllItems();
	m_pSelectedPaint = NULL;
}

//////////////////////////////////////////////////////////////////////////
void CVehiclePaintsPanel::ShowPaintPropertiesByName(const string& paintName)
{
	IVariable* pPaint = GetPaintVarByName(paintName);
	ShowPaintProperties(pPaint);
}

//////////////////////////////////////////////////////////////////////////
void CVehiclePaintsPanel::ShowPaintPropertiesByVarIndex(i32 paintVarIndex)
{
	IVariable* pPaint = GetPaintVarByIndex(paintVarIndex);
	ShowPaintProperties(pPaint);
}

//////////////////////////////////////////////////////////////////////////
void CVehiclePaintsPanel::ShowPaintProperties(IVariable* pPaint)
{
	assert(pPaint);
	if (pPaint == NULL)
	{
		return;
	}

	HidePaintProperties();
	CVarBlockPtr pVarBlock(new CVarBlock());
	pVarBlock->AddVariable(pPaint);
	m_paintProperties.AddVarBlock(pVarBlock);
	m_paintProperties.EnableUpdateCallback(true);

	m_pSelectedPaint = pPaint;

	if (m_pSelectedPaint != NULL)
	{
		IVariable* pSelectedPaintName = GetChildVar(m_pSelectedPaint, "name");
		if (pSelectedPaintName != NULL)
		{
			pSelectedPaintName->AddOnSetCallback(functor(*this, &CVehiclePaintsPanel::OnSelectedPaintNameChanged));
		}

		IVariable* pSeletedPaintMaterial = GetChildVar(m_pSelectedPaint, "material");
		if (pSeletedPaintMaterial != NULL)
		{
			pSeletedPaintMaterial->AddOnSetCallback(functor(*this, &CVehiclePaintsPanel::OnSelectedPaintMaterialChanged));
		}
	}
}

//////////////////////////////////////////////////////////////////////////
IVariable* CVehiclePaintsPanel::GetPaintVarByName(const string& paintName)
{
	if (m_pPaints == NULL)
	{
		return NULL;
	}

	for (i32 i = 0; i < m_pPaints->GetNumVariables(); i++)
	{
		IVariable* pPaint = m_pPaints->GetVariable(i);
		IVariable* pPaintName = GetChildVar(pPaint, "name");
		if (pPaintName != NULL)
		{
			string currentPaintName;
			pPaintName->Get(currentPaintName);

			if (paintName.CompareNoCase(currentPaintName) == 0)
			{
				return pPaint;
			}
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
IVariable* CVehiclePaintsPanel::GetPaintVarByIndex(i32 index)
{
	if (m_pPaints == NULL)
	{
		return NULL;
	}

	if (index < 0 || m_pPaints->GetNumVariables() <= index)
	{
		return NULL;
	}

	return m_pPaints->GetVariable(index);
}

//////////////////////////////////////////////////////////////////////////
void CVehiclePaintsPanel::OnSelectedPaintNameChanged(IVariable* pVar)
{
	assert(pVar != NULL);

	string name;
	pVar->Get(name);

	RenameSelectedPaintName(name);
}

//////////////////////////////////////////////////////////////////////////
void CVehiclePaintsPanel::OnSelectedPaintMaterialChanged(IVariable* pVar)
{

}

//////////////////////////////////////////////////////////////////////////
void CVehiclePaintsPanel::UpdateAssignMaterialButtonState()
{
	CMaterial* pMaterial = GetIEditor()->GetMaterialManager()->GetCurrentMaterial();
	bool materialSelected = (pMaterial != NULL);

	m_applyMaterialButton.EnableWindow(materialSelected);
}

