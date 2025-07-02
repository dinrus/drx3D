// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Objects/BaseObject.h"
#include "EntityScript.h"

#include <DrxMovie/IMovieSystem.h>
#include "EntityPrototype.h"
#include "Gizmos/Gizmo.h"
#include <drx3D/CoreX/Containers/DrxListenerSet.h>

class CHyperFlowGraph;
class CEntityObject;
class CPopupMenuItem;
class IOpticsElementBase;
struct SPyWrappedProperty;

#include <drx3D/CoreX/Serialization/Serializer.h>
#include <drx3D/CoreX/Serialization/STL.h>

/*!
 *	CEntityEventTarget is an Entity event target and type.
 */
struct CEntityEventTarget
{
	CBaseObject*       target; //! Target object.
	string            event;
	string            sourceEvent;
};

//////////////////////////////////////////////////////////////////////////
// Named link from entity to entity.
//////////////////////////////////////////////////////////////////////////
struct CEntityLink
{
	DrxGUID             targetId; // Target entity id.
	string              name;     // Name of the link.

	void Serialize(Serialization::IArchive& ar);
	CEntityObject* GetTarget() const;
};

inline const EntityGUID& ToEntityGuid(DrxGUID guid)
{
	return guid;
}

class IEntityObjectListener
{
public:
	virtual void OnNameChanged(tukk pName) = 0;
	virtual void OnSelectionChanged(const bool bSelected) = 0;
	virtual void OnDone() = 0;
};

struct IPickEntitesOwner
{
	virtual void         AddEntity(CBaseObject* const pBaseObject) = 0;
	virtual CBaseObject* GetEntity(size_t const index) = 0;
	virtual size_t       GetEntityCount() const = 0;
	virtual void         RemoveEntity(size_t const index) = 0;
};

/*!
 *	CEntity is an static object on terrain.
 *
 */
class SANDBOX_API CEntityObject : public CBaseObject
{
public:
	DECLARE_DYNCREATE(CEntityObject)

	//////////////////////////////////////////////////////////////////////////
	// Overrides from CBaseObject.
	//////////////////////////////////////////////////////////////////////////
	//! Return type name of Entity.
	string GetTypeDescription() const { return GetEntityClass(); }

	//////////////////////////////////////////////////////////////////////////
	bool              IsSameClass(CBaseObject* obj);

	virtual bool      Init(CBaseObject* prev, const string& file);
	virtual void      InitVariables();
	virtual void      Done();
	virtual bool      CreateGameObject();
	virtual void      SetScriptName(const string& file, CBaseObject* pPrev);

	bool              GetEntityPropertyBool(tukk name) const;
	i32               GetEntityPropertyInteger(tukk name) const;
	float             GetEntityPropertyFloat(tukk name) const;
	string           GetEntityPropertyString(tukk name) const;
	void              SetEntityPropertyBool(tukk name, bool value);
	void              SetEntityPropertyInteger(tukk name, i32 value);
	void              SetEntityPropertyFloat(tukk name, float value);
	void              SetEntityPropertyString(tukk name, const string& value);

	virtual void      Display(CObjectRenderHelper& objRenderHelper);

	virtual void      GetDisplayBoundBox(AABB& box);

	virtual float     GetCreationOffsetFromTerrain() const override;

	virtual void      OnContextMenu(CPopupMenuItem* menu);

	virtual void      SetFrozen(bool bFrozen);
	void              SetName(const string& name);
	void              SetSelected(bool bSelect);

	virtual bool      IsScalable() const override;
	virtual bool      IsRotatable() const override;

	virtual IStatObj* GetIStatObj();

	// Override IsScalable value from script file.
	bool         GetForceScale()                 { return m_bForceScale; }
	void         SetForceScale(bool bForceScale) { m_bForceScale = bForceScale; }

	virtual void GetLocalBounds(AABB& box);

	virtual void SetModified(bool boModifiedTransformOnly, bool bNotifyObjectManager);

	virtual bool HitTest(HitContext& hc);
	virtual bool HitHelperTest(HitContext& hc);
	virtual bool HitTestRect(HitContext& hc);
	void         UpdateVisibility(bool bVisible);
	bool         ConvertFromObject(CBaseObject* object);

	virtual void Serialize(CObjectArchive& ar);
	virtual void PostLoad(CObjectArchive& ar);

	XmlNodeRef   Export(const string& levelPath, XmlNodeRef& xmlNode);

	void         SetLookAt(CBaseObject* target);

	//////////////////////////////////////////////////////////////////////////
	void                     OnEvent(ObjectEvent event);

	virtual void             SetTransformDelegate(ITransformDelegate* pTransformDelegate, bool bInvalidateTM) override;

	virtual IPhysicalEntity* GetCollisionEntity() const;

	virtual void             SetMaterial(IEditorMaterial* mtl);
	virtual IEditorMaterial*       GetRenderMaterial() const;
	

	virtual void InvalidateGeometryFile(const string& gamePath) override;

	//////////////////////////////////////////////////////////////////////////
	virtual void SetMinSpec(u32 nSpec, bool bSetChildren = true);
	virtual void SetMaterialLayersMask(u32 nLayersMask);

	// Gets matrix of parent attachment point
	virtual Matrix34 GetLinkAttachPointWorldTM() const override;

	// Checks if the attachment point is valid
	virtual bool IsParentAttachmentValid() const override;

	// Set attach flags and target
	enum EAttachmentType
	{
		eAT_Pivot,
		eAT_GeomCacheNode,
		eAT_CharacterBone,
	};

	void            SetAttachType(const EAttachmentType attachmentType) { m_attachmentType = attachmentType; }
	void            SetAttachTarget(tukk target)                 { m_attachmentTarget = target; }
	EAttachmentType GetAttachType() const                               { return m_attachmentType; }
	string         GetAttachTarget() const                             { return m_attachmentTarget; }

	// Updates transform if attached
	void                 UpdateTransform();

	virtual void         SetHelperScale(float scale);
	virtual float        GetHelperScale();

	virtual void         GatherUsedResources(CUsedResources& resources);
	virtual bool         IsSimilarObject(CBaseObject* pObject);

	virtual IRenderNode* GetEngineNode() const;
	virtual bool         HasMeasurementAxis() const { return false; }

	//////////////////////////////////////////////////////////////////////////
	// END CBaseObject
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// CEntity interface.
	//////////////////////////////////////////////////////////////////////////
	virtual bool SetClass(const string& entityClass, bool bForceReload = false, XmlNodeRef xmlEntityNode = XmlNodeRef());
	virtual void SpawnEntity();

	void         CopyPropertiesToScriptTables();

	virtual void DeleteEntity();
	virtual void UnloadScript();

	string      GetEntityClass() const { return m_entityClass; }
	EntityId    GetEntityId() const    { return m_entityId; }

	//! Return entity prototype class if present.
	virtual CEntityPrototype* GetPrototype() const { return m_prototype; }

	//! Get EntityScript object associated with this entity.
	CEntityScript* GetScript() const { return m_pEntityScript.get(); }

	//! Reload entity script.
	virtual void Reload(bool bReloadScript = false);

	//////////////////////////////////////////////////////////////////////////
	//! Return number of event targets of Script.
	i32                 GetEventTargetCount() const { return m_eventTargets.size(); }
	CEntityEventTarget& GetEventTarget(i32 index)   { return m_eventTargets[index]; }
	//! Add new event target, returns index of created event target.
	//! Event targets are Always entities.
	i32  AddEventTarget(CBaseObject* target, const string& event, const string& sourceEvent, bool bUpdateScript = true);
	//! Remove existing event target by index.
	void RemoveEventTarget(i32 index, bool bUpdateScript = true);
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Entity Links.
	//////////////////////////////////////////////////////////////////////////
	//! Return number of event targets of Script.
	i32          GetEntityLinkCount() const { return m_links.size(); }
	CEntityLink& GetEntityLink(i32 index)   { return m_links[index]; }
	virtual i32  AddEntityLink(const string& name, DrxGUID targetEntityId);
	virtual bool EntityLinkExists(const string& name, DrxGUID targetEntityId);
	void         RenameEntityLink(i32 index, const string& newName);
	void         RemoveEntityLink(i32 index);
	void         RemoveAllEntityLinks();
	virtual void EntityLinked(const string& name, DrxGUID targetEntityId)   {}
	virtual void EntityUnlinked(const string& name, DrxGUID targetEntityId) {}
	void         LoadLink(XmlNodeRef xmlNode, CObjectArchive* pArchive = NULL);
	void         SaveLink(XmlNodeRef xmlNode);
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	//! Get Game entity interface.
	virtual IEntity* GetIEntity() override        { return m_pEntity; }
	const IEntity*   GetIEntity() const           { return m_pEntity; }

	i32              GetCastShadowMinSpec() const { return mv_castShadowMinSpec; }
	i32              GetGIMode() const { return mv_giMode; }

	float            GetRatioLod() const          { return (float)mv_ratioLOD; }
	float            GetRatioViewDist() const     { return (float)mv_ratioViewDist; }

	CVarBlock*       GetProperties() const        { return m_pLuaProperties; };
	CVarBlock*       GetProperties2() const       { return m_pLuaProperties2; };

	bool             IsLight()  const             { return m_bLight;    }

	void             Validate();

	virtual string GetAssetPath() const override;
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	//! Takes position/orientation from the game entity and apply on editor entity.
	void             AcceptPhysicsState();
	void             ResetPhysicsState();

	bool             CreateFlowGraphWithGroupDialog();
	void             SetFlowGraph(CHyperFlowGraph* pGraph);
	// Open Flow Graph associated with this entity in the view window.
	void             OpenFlowGraph(const string& groupName);
	void             RemoveFlowGraph();
	CHyperFlowGraph* GetFlowGraph() const { return m_pFlowGraph; }

	// Find CEntity from game entityId.
	static CEntityObject* FindFromEntityId(EntityId id);

	// Retrieve smart object class for this entity if exist.
	string               GetSmartObjectClass() const;

	// Get the name of the light animation node assigned to this, if any.
	string               GetLightAnimation() const;

	IVariable*            GetLightVariable(tukk name) const;
	IOpticsElementBasePtr GetOpticsElement();
	void                  SetOpticsElement(IOpticsElementBase* pOptics);
	void                  ApplyOptics(const string& opticsFullName, IOpticsElementBasePtr pOptics);
	void                  SetOpticsName(const string& opticsFullName);
	bool                  GetValidFlareName(string& outFlareName) const;

	void                  PreInitLightProperty();
	void                  UpdateLightProperty();
	void                  OnMenuReloadScripts();

	IStatObj* GetVisualObject()
	{
		return m_visualObject;
	}

	static void StoreUndoEntityLink(const std::vector<CBaseObject*>& objects);

	static tukk s_LensFlarePropertyName;
	static tukk s_LensFlareMaterialName;

	// Overridden from CBaseObject.
	void InvalidateTM(i32 nWhyFlags) override;

	void RegisterListener(IEntityObjectListener* pListener);
	void UnregisterListener(IEntityObjectListener* pListener);

protected:
	template<typename T> void SetEntityProperty(tukk name, T value);
	template<typename T> T    GetEntityProperty(tukk name, T defaultvalue) const;

	void                      PySetEntityProperty(tukk name, const SPyWrappedProperty& value);
	SPyWrappedProperty        PyGetEntityProperty(tukk name) const;

	virtual bool              SetEntityScript(std::shared_ptr<CEntityScript> pEntityScript, bool bForceReload = false, XmlNodeRef xmlEntityNode = XmlNodeRef());
	virtual void              GetScriptProperties(XmlNodeRef xmlEntityNode);

	SRenderLight*                  GetLightProperty() const;

	bool                      HitTestCharacter(ICharacterInstance* pCharacter, HitContext& hc, SRayHitInfo& hitInfo, bool& bHavePhysics);
	virtual bool              HitTestEntity(HitContext& hc, bool& bHavePhysics);

	virtual void              UpdateHighlightPassState(bool bSelected, bool bHighlighted);

	//////////////////////////////////////////////////////////////////////////
	//! Must be called after cloning the object on clone of object.
	//! This will make sure object references are cloned correctly.
	virtual void PostClone(CBaseObject* pFromObject, CObjectCloneContext& ctx);

	//! Draw default object items.
	virtual void DrawDefault(DisplayContext& dc, COLORREF labelColor = RGB(255, 255, 255)) override;
	virtual void DrawTextureIcon(DisplayContext& dc, const Vec3& pos, float alpha, bool bDisplaySelectionHelper, float distanceSquared = 0) override;
	void         DrawProjectorPyramid(DisplayContext& dc, float dist);
	void         DrawProjectorFrustum(DisplayContext& dc, Vec2 size, float dist);
	void         DrawEntityLinks(DisplayContext& dc);

	// !Recreates the icons buffer
	void RenewTextureIconsBuffer();

	// Saves the previous dirty flag of the entity
	i32 m_entityDirtyFlag;

	//! Returns if object is in the camera view.
	virtual bool  IsInCameraView(const CCamera& camera);
	//! Returns vis ratio of object in camera
	virtual float GetCameraVisRatio(const CCamera& camera);

	// Do basic intersection tests
	virtual bool IntersectRectBounds(const AABB& bbox);
	virtual bool IntersectRayBounds(const Ray& ray);

	CVarBlock*   CloneProperties(CVarBlock* srcProperties);

	//////////////////////////////////////////////////////////////////////////
	//! Callback called when one of entity properties have been modified.
	void         OnPropertyChange(IVariable* var);
	virtual void OnPropertyChangeDone(IVariable* var)
	{
		if (var)
		{
			UpdateGroup();
			UpdatePrefab();
		}
	}
	
	//////////////////////////////////////////////////////////////////////////
	virtual void OnEventTargetEvent(CBaseObject* target, i32 event);
	void         ResolveEventTarget(CBaseObject* object, u32 index);
	void         ReleaseEventTargets();
	void         UpdateMaterialInfo();

	// Called when link info changes.
	virtual void UpdateIEntityLinks(bool bCallOnPropertyChange = true);

	//! Dtor must be protected.
	CEntityObject();
	void DeleteThis() { delete this; }

	//! Recalculate bounding box of entity.
	void CalcBBox();

	//! Force IEntity to the local position/angles/scale.
	void XFormGameEntity();

	//! Sets correct binding for IEntity.
	virtual void BindToParent();
	virtual void BindIEntityChilds();
	virtual void UnbindIEntity();

	void         DrawAIInfo(DisplayContext& dc, struct IAIObject* aiObj);

	//////////////////////////////////////////////////////////////////////////
	// Callbacks.
	//////////////////////////////////////////////////////////////////////////
	void OnRenderFlagsChange(IVariable* var);
	void OnEntityFlagsChange(IVariable* var);

	//////////////////////////////////////////////////////////////////////////
	// Radius callbacks.
	//////////////////////////////////////////////////////////////////////////
	void OnRadiusChange(IVariable* var);
	void OnInnerRadiusChange(IVariable* var);
	void OnOuterRadiusChange(IVariable* var);
	void OnBoxSizeXChange(IVariable* var);
	void OnBoxSizeYChange(IVariable* var);
	void OnBoxSizeZChange(IVariable* var);
	void OnProjectorFOVChange(IVariable* var);
	void OnProjectorTextureChange(IVariable* var);
	void OnProjectInAllDirsChange(IVariable* var);
	//////////////////////////////////////////////////////////////////////////
	// Area light callbacks.
	//////////////////////////////////////////////////////////////////////////
	void OnAreaLightChange(IVariable* var);
	void OnAreaWidthChange(IVariable* var);
	void OnAreaHeightChange(IVariable* var);
	void OnAreaFOVChange(IVariable* var);
	void OnAreaLightSizeChange(IVariable* var);
	void OnColorChange(IVariable* var);
	//////////////////////////////////////////////////////////////////////////
	// Box projection callbacks.
	//////////////////////////////////////////////////////////////////////////
	void OnBoxProjectionChange(IVariable* var);
	void OnBoxWidthChange(IVariable* var);
	void OnBoxHeightChange(IVariable* var);
	void OnBoxLengthChange(IVariable* var);
	//////////////////////////////////////////////////////////////////////////

	void            FreeGameData();

	void            CheckSpecConfig();

	IVariable*      FindVariableInSubBlock(CVarBlockPtr& properties, IVariable* pSubBlockVar, tukk pVarName);

	void            OnMenuCreateFlowGraph();
	void            OnMenuScriptEvent(i32 eventIndex);
	void            OnMenuReloadAllScripts();
	void            OnMenuConvertToPrefab();

	virtual string GetMouseOverStatisticsText() const override;

	void            SetFlareName(const string& name)
	{
		SetEntityPropertyString(s_LensFlarePropertyName, name);
	}

	//////////////////////////////////////////////////////////////////////////
	// UI Part
	//////////////////////////////////////////////////////////////////////////
	void CreateInspectorWidgets(CInspectorWidgetCreator& creator) override;
	void CreateComponentWidgets(CInspectorWidgetCreator& creator);

	void SerializeLuaInstanceProperties(Serialization::IArchive& ar, bool bMultiEdit);
	void SerializeLuaProperties(Serialization::IArchive& ar, bool bMultiEdit);
	void SerializeFlowgraph(Serialization::IArchive& ar, bool bMultiEdit);
	void SerializeLinks(Serialization::IArchive& ar, bool bMultiEdit);
	void SerializeArchetype(Serialization::IArchive& ar, bool bMultiEdit);

	void OnEditScript();
	void OnReloadScript();
	void OnOpenArchetype();
	void OnBnClickedOpenFlowGraph();
	void OnBnClickedRemoveFlowGraph();
	void OnBnClickedListFlowGraphs();
	//////////////////////////////////////////////////////////////////////////

	// Reset information obtained from editor class info
	void ResetEditorClassInfo();

	bool IsLegacyObject() const;

protected:

	u32 m_bLoadFailed            : 1;
	u32 m_bEntityXfromValid      : 1;
	u32 m_bCalcPhysics           : 1;
	u32 m_bDisplayBBox           : 1;
	u32 m_bDisplaySolidBBox      : 1;
	u32 m_bDisplayAbsoluteRadius : 1;
	u32 m_bDisplayArrow          : 1;
	u32 m_bVisible               : 1;
	u32 m_bLight                 : 1;
	u32 m_bAreaLight             : 1;
	u32 m_bProjectorHasTexture   : 1;
	u32 m_bProjectInAllDirs      : 1;
	u32 m_bBoxProjectedCM        : 1;
	u32 m_bBBoxSelection         : 1;
	u32 m_bInitVariablesCalled   : 1;

	Vec3         m_lightColor;

	//! Entity class.
	string    m_entityClass;
	DrxGUID   m_entityClassGUID;

	//! Id of spawned entity.
	EntityId  m_entityId;
	IEntity*  m_pEntity;
	IEntityClass* m_pEntityClass;

	IStatObj* m_visualObject;

	// Entity class description.
	std::shared_ptr<CEntityScript> m_pEntityScript;

	std::vector<i32> m_componentIconTextureIds;

	//////////////////////////////////////////////////////////////////////////
	// Main entity parameters.
	//////////////////////////////////////////////////////////////////////////
	CVariable<bool>         mv_outdoor;
	CVariable<bool>         mv_castShadow; // Legacy, required for backwards compatibility
	CSmartVariableEnum<i32> mv_castShadowMinSpec;
	CVariable<bool>         mv_DynamicDistanceShadows;
	CSmartVariableEnum<i32> mv_giMode;

	CVariable<i32>          mv_ratioLOD;
	CVariable<i32>          mv_ratioViewDist;
	CVariable<bool>         mv_hiddenInGame; // Entity is hidden in game (on start).
	CVariable<bool>         mv_recvWind;
	CVariable<bool>         mv_renderNearest;
	CVariable<bool>         mv_noDecals;

	//////////////////////////////////////////////////////////////////////////
	// Temp variables (Not serializable) just to display radii from properties.
	//////////////////////////////////////////////////////////////////////////
	// Used for proximity entities.
	float m_proximityRadius;
	float m_innerRadius;
	float m_outerRadius;
	// Used for probes
	float m_boxSizeX;
	float m_boxSizeY;
	float m_boxSizeZ;
	// Used for light entities
	float m_projectorFOV;
	// Used for area lights
	float m_fAreaWidth;
	float m_fAreaHeight;
	float m_fAreaLightSize;
	// Used for box projected cubemaps
	float m_fBoxWidth;
	float m_fBoxHeight;
	float m_fBoxLength;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Event Targets.
	//////////////////////////////////////////////////////////////////////////
	//! Array of event targets of this Entity.
	typedef std::vector<CEntityEventTarget> EventTargets;
	EventTargets m_eventTargets;

	//////////////////////////////////////////////////////////////////////////
	// Links
	typedef std::vector<CEntityLink> Links;
	Links m_links;

	//! Entity prototype. only used by EntityPrototypeObject.
	TSmartPtr<CEntityPrototype> m_prototype;

	//! Entity properties variables.
	CVarBlockPtr          m_pLuaProperties;
	//! Per instance entity properties variables
	CVarBlockPtr          m_pLuaProperties2;

	// Physics state, as a string.
	XmlNodeRef m_physicsState;

	// Loaded data for the entity;
	// This xml data is passed to the entity for loading data from it.
	XmlNodeRef m_loadedXmlNodeData;
	bool       m_bCloned = false;

	//////////////////////////////////////////////////////////////////////////
	// Associated FlowGraph.
	CHyperFlowGraph* m_pFlowGraph;

	static i32   m_rollupId;
	static float m_helperScale;

	// Override IsScalable value from script file.
	bool              m_bForceScale;

	EAttachmentType   m_attachmentType;
	string           m_attachmentTarget;

private:
	void         ResetCallbacks();
	void         SetVariableCallback(IVariable* pVar, IVariable::OnSetCallback func);
	void         ClearCallbacks();

	virtual void OnAttachChild(CBaseObject* pChild) override;
	virtual void OnDetachThis() override;

	virtual void OnLink(CBaseObject* pParent) override;
	virtual void OnUnLink() override;

	CListenerSet<IEntityObjectListener*>                         m_listeners;
	std::vector<std::pair<IVariable*, IVariable::OnSetCallback>> m_callbacks;

	friend SPyWrappedProperty PyGetEntityProperty(tukk entityName, tukk propName);
	friend void               PySetEntityProperty(tukk entityName, tukk propName, SPyWrappedProperty value);
	friend SPyWrappedProperty PyGetEntityParam(tukk pObjectName, tukk pVarPath);
	friend void               PySetEntityParam(tukk pObjectName, tukk pVarPath, SPyWrappedProperty value);
};

/*!
 * Class Description of Entity
 */
class CEntityClassDesc : public CObjectClassDesc
{
public:
	ObjectType          GetObjectType()                     { return OBJTYPE_ENTITY; }
	tukk         ClassName()                         { return "StdEntity"; }
	tukk         Category()                          { return ""; }
	CRuntimeClass*      GetRuntimeClass()                   { return RUNTIME_CLASS(CEntityObject); }
	tukk         GetFileSpec()                       { return "*EntityClass"; }
	virtual tukk GetDataFilesFilterString() override { return not_implemented; }
};

