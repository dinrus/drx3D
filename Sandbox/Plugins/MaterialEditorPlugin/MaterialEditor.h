// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <AssetSystem/AssetEditor.h>

#include <memory>

class QLineEdit;
class CMaterial;
class CMaterialSerializer;

//! Material editor integrated with the asset system
class CMaterialEditor : public CAssetEditor, public IAutoEditorNotifyListener, public IDataBaseManagerListener
{
public:
	CMaterialEditor();
	~CMaterialEditor();

	virtual tukk GetEditorName() const override { return "Material Editor"; }

	virtual bool OnOpenAsset(CAsset* pAsset) override;
	virtual bool OnSaveAsset(CEditableAsset& editAsset) override;
	virtual void OnCloseAsset() override;
	virtual void OnDiscardAssetChanges() override;

	void SetMaterial(CMaterial* pMaterial);
	void SelectMaterialForEdit(CMaterial* pMaterial);

	//! Returns main material that is loaded
	CMaterial* GetLoadedMaterial() { return m_pMaterial; }

	//! Returns material for editing, can be a sub material of the main material
	CMaterial* GetMaterialSelectedForEdit() { return m_pEditedMaterial; }

	void FillMaterialMenu(CAbstractMenu* menu);

	//! The main material that is loaded, may be composed of submaterials
	CDrxSignal<void(CMaterial*)> signalMaterialLoaded;

	//! Emitted when properties of the loaded material or ANY submaterial changes
	CDrxSignal<void(CMaterial*)> signalMaterialPropertiesChanged;

	//! The material or submaterial that is currently edited
	CDrxSignal<void(CMaterial*)> signalMaterialForEditChanged;

	//Actions that can be called from components of the material editor

	void OnResetSubMaterial(i32 slot);
	void OnRemoveSubMaterial(i32 slot);

private:

	void InitMenuBar();
	virtual void CreateDefaultLayout(CDockableContainer* sender) override;
	virtual void OnLayoutChange(const QVariantMap& state) override;
	void BroadcastPopulateInspector();

	void OnConvertToMultiMaterial();
	void OnConvertToSingleMaterial();
	void OnAddSubMaterial();
	void OnSetSubMaterialSlotCount();
	void OnPickMaterial();

	virtual void OnEditorNotifyEvent(EEditorNotifyEvent event) override;
	virtual void OnDataBaseItemEvent(IDataBaseItem* pItem, EDataBaseItemEvent event) override;
	void OnSubMaterialsChanged(CMaterial::SubMaterialChange change);
	void OnReadOnlyChanged() override;

	_smart_ptr<CMaterial> m_pMaterial;
	_smart_ptr<CMaterial> m_pEditedMaterial;
	_smart_ptr<CMaterialSerializer> m_pMaterialSerializer;
};
