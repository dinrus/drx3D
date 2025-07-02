// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Group.h"
#include "Prefabs/PrefabItem.h"

class CPopupMenuItem;

#define PREFAB_OBJECT_CLASS_NAME "Prefab"
#define CATEGORY_PREFABS         "Prefab"

//////////////////////////////////////////////////////////////////////////
//! Undo object for change pivot point of a prefab
class CUndoChangePivot : public IUndoObject
{
public:
	CUndoChangePivot(CBaseObject* obj, tukk undoDescription);

protected:
	virtual tukk GetDescription() override { return m_undoDescription; };
	virtual tukk GetObjectName() override;

	virtual void        Undo(bool bUndo) override;
	virtual void        Redo() override;

private:

	DrxGUID m_guid;
	string  m_undoDescription;
	Vec3    m_undoPivotPos;
	Vec3    m_redoPivotPos;
};

/*!
 *		CPrefabObject is prefabricated object which can contain multiple other objects, in a group like manner,
    but internal objects can not be modified, they are only created from PrefabItem.
 */
class SANDBOX_API CPrefabObject : public CGroup
{
public:
	DECLARE_DYNCREATE(CPrefabObject)

	//////////////////////////////////////////////////////////////////////////
	// CGroup overrides.
	//////////////////////////////////////////////////////////////////////////
	virtual bool Init(CBaseObject* prev, const string& file) override;
	virtual void PostInit(const string& file) override;
	virtual void Done() override;

	virtual void Display(CObjectRenderHelper& objRenderHelper) override;
	virtual bool HitTest(HitContext& hc) override;
	
	virtual void CreateInspectorWidgets(CInspectorWidgetCreator& creator) override;

	virtual void OnEvent(ObjectEvent event) override;

	virtual void Serialize(CObjectArchive& ar) override;
	virtual void PostLoad(CObjectArchive& ar) override;

	virtual void SetMaterialLayersMask(u32 nLayersMask) override;

	virtual void SetName(const string& name) override;

	virtual void AddMember(CBaseObject* pMember, bool bKeepPos = true) override;
	virtual void RemoveMembers(std::vector<CBaseObject*>& members, bool shouldKeepPos = true, bool shouldPlaceOnRoot = false) override;
	virtual void DeleteAllMembers() override;

	virtual void SetMaterial(IEditorMaterial* pMaterial) override;
	virtual void SetWorldTM(const Matrix34& tm, i32 flags = 0) override;
	virtual void SetWorldPos(const Vec3& pos, i32 flags = 0) override;
	virtual void UpdatePivot(const Vec3& newWorldPivotPos) override;

	DrxGUID GetPrefabGuid() const { return m_prefabGUID; }

	void SetPivot(const Vec3& newWorldPivotPos);
	void SetAutoUpdatePrefab(bool autoUpdate);
	bool GetAutoUpdatePrefab() const { return m_autoUpdatePrefabs; }

	virtual string GetTypeDescription() const override { return m_pPrefabItem ? m_pPrefabItem->GetName() : ""; }

	virtual XmlNodeRef Export(const string& levelPath, XmlNodeRef& xmlNode);

	//////////////////////////////////////////////////////////////////////////
	// CPrefabObject.
	//////////////////////////////////////////////////////////////////////////
	virtual void SetPrefab(DrxGUID guid, bool bForceReload);
	virtual void SetPrefab(CPrefabItem* pPrefab, bool bForceReload);

	CPrefabItem* GetPrefabItem() const { return m_pPrefabItem; }

	// Extract all objects inside.
	void         CloneAll(std::vector<CBaseObject*>& extractedObjects);
	void         SyncPrefab(const SObjectChangedContext& context);
	bool         SuspendUpdate(bool bForceSuspend = true);
	void         ResumeUpdate();
	bool         CanObjectBeAddedAsMember(CBaseObject* pObject);
	void         InitObjectPrefabId(CBaseObject* object);
	bool         IsModifyInProgress() const { return m_bModifyInProgress; }
	void         SetModifyInProgress(const bool inProgress) { m_bModifyInProgress = inProgress; }
	void         CloneSelected(CSelectionGroup* pSelectedGroup, std::vector<CBaseObject*>& clonedObjects);
	void         SetObjectPrefabFlagAndLayer(CBaseObject* object);
	void         SetChangePivotMode(bool changePivotMode) { m_bChangePivotPoint = changePivotMode; }
	virtual void OnContextMenu(CPopupMenuItem* menu);

protected:
	void         SerializeMembers(Serialization::IArchive& ar);
	virtual void RemoveChild(CBaseObject* child);

	CPrefabObject();

	void         SetPrefabGuid(DrxGUID guid) { m_prefabGUID = guid; }
	bool         HitTestMembers(HitContext& hcOrg);
	virtual void PostClone(CBaseObject* pFromObject, CObjectCloneContext& ctx);
	virtual void CalcBoundBox();
	void         DeleteThis() { delete this; };

	void         RecursivelyDisplayObject(CBaseObject* object, CObjectRenderHelper& objRenderHelper);
	void         DeleteAllPrefabObjects();

	void         SyncParentObject();

	void         OnShowInFG();
	void         ConvertToProceduralObject();

private:
	void DeleteChildrenWithoutUpdating();
	
	void AttachLoadedChildrenToPrefab(CObjectArchive &ar, IObjectLayer* pThisLayer);

protected:
	_smart_ptr<CPrefabItem>           m_pPrefabItem;
	std::deque<SObjectChangedContext> m_pendingChanges;

	string                            m_prefabName;
	DrxGUID                           m_prefabGUID;
	bool                              m_autoUpdatePrefabs;
	bool                              m_bModifyInProgress;
	bool                              m_bChangePivotPoint;
	bool                              m_bSettingPrefabObj;
};

class CPrefabChildGuidProvider : public IGuidProvider
{
public:
	static bool IsValidChildGUid(const DrxGUID& guid, CPrefabObject* pPrefabObject);

	CPrefabChildGuidProvider(CPrefabObject* pPrefabObject)
		: m_pPrefabObject(pPrefabObject)
	{}

	virtual DrxGUID GetFrom(const DrxGUID& loadedGuid) const override;

private:
	_smart_ptr<CPrefabObject> m_pPrefabObject{ nullptr };
public:
};

/*!
 * Class Description of Prefab.
 */
class CPrefabObjectClassDesc : public CObjectClassDesc
{
public:
	ObjectType     GetObjectType()   { return OBJTYPE_PREFAB; };
	tukk    ClassName()       { return PREFAB_OBJECT_CLASS_NAME; };
	tukk    Category()        { return CATEGORY_PREFABS; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(CPrefabObject); };

	//! Select all prefabs.
	//! ObjectTreeBrowser object can recognize this hard-coded name.
	tukk         GetFileSpec()                       { return "*Prefabs"; };
	virtual tukk GetDataFilesFilterString() override { return ""; }
	virtual void        EnumerateObjects(IObjectEnumerator* pEnumerator) override;
	virtual tukk GetTextureIcon()                    { return "%EDITOR%/ObjectIcons/prefab.bmp"; };
	virtual bool        IsCreatable() const override;
};

