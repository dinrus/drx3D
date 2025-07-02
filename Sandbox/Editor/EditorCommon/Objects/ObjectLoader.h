// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/ToolsHelpers/GuidUtil.h>
#include <drx3D/CoreX/Extension/DrxGUID.h>

class CPakFile;
class IGuidProvider;

typedef std::map<DrxGUID, DrxGUID> TGUIDRemap;

/** CObjectLoader used to load BaseObject and resolve ObjectId references while loading.
 */
class EDITOR_COMMON_API CObjectArchive
{
public:
	XmlNodeRef node; //!< Current archive node.
	bool       bLoading;
	bool       bUndo;

	CObjectArchive(IObjectManager* objMan, XmlNodeRef xmlRoot, bool loading);
	~CObjectArchive();

	//! Resolve callback with only one parameter of CBaseObject.
	typedef Functor1<CBaseObject*>               ResolveObjRefFunctor1;
	//! Resolve callback with two parameters one is pointer to CBaseObject and second use data integer.
	typedef Functor2<CBaseObject*, u32> ResolveObjRefFunctor2;

	/** Register Object id.
	   @param objectId Original object id from the file.
	   @param realObjectId Changed object id.
	 */
	//void RegisterObjectId( i32 objectId,i32 realObjectId );

	// Return object ID remapped after loading.
	DrxGUID ResolveID(const DrxGUID& id);

	//! Set object resolve callback, it will be called once object with specified Id is loaded.
	void SetResolveCallback(CBaseObject* fromObject, const DrxGUID& objectId, ResolveObjRefFunctor1& func);
	//! Set object resolve callback, it will be called once object with specified Id is loaded.
	void SetResolveCallback(CBaseObject* fromObject, const DrxGUID& objectId, ResolveObjRefFunctor2& func, u32 userData);
	//! Resolve all object ids and call callbacks on resolved objects.
	void ResolveObjects(bool bProcessEvents = false);

	// Save object to archive.
	void SaveObject(CBaseObject* pObject, bool bSaveInGroupObjects = false, bool bSaveInPrefabObjects = false);

	//! Load multiple objects from archive.
	void LoadObjects(XmlNodeRef& rootObjectsNode);

	//! Load one object from archive.
	CBaseObject* LoadObject(XmlNodeRef& objNode, CBaseObject* pPrevObject = NULL);

	//////////////////////////////////////////////////////////////////////////
	i32          GetLoadedObjectsCount()           { return m_loadedObjects.size(); }
	CBaseObject* GetLoadedObject(i32 nIndex) const { return m_loadedObjects[nIndex].pObject; }

	//! Set a guid provider that will be used to generate guids for loaded objects. Should not be set if guid's need to be loaded
	void SetGuidProvider(IGuidProvider* pGuidProvider) { m_pGuidProvider = pGuidProvider; }
	IGuidProvider* GetGuidProvider() const { return m_pGuidProvider; }

	//! If true new loaded objects will be added to current instead of saved layer.
	void LoadInCurrentLayer(bool bEnable);

	void EnableReconstructPrefabObject(bool bEnable);

	//! Remap object ids.
	void RemapID(DrxGUID oldId, DrxGUID newId);

	void         EnableProgressBar(bool bEnable) { m_bProgressBarEnabled = bEnable; };

	CBaseObject* GetCurrentObject();

	void         AddSequenceIdMapping(u32 oldId, u32 newId);
	u32       RemapSequenceId(u32 id) const;
	bool         IsAmongPendingIds(u32 id) const;

	bool         IsReconstructingPrefab() const     { return (m_nFlags & eObjectLoader_ReconstructPrefabs) != 0; }
	bool         IsSavingInPrefab() const           { return (m_nFlags & eObjectLoader_SavingInPrefabLib) != 0; }
	void         SetShouldResetInternalMembers(bool reset);
	bool         ShouldResetInternalMembers() const { return (m_nFlags & eObjectLoader_ResetInternalMembers) != 0; }
	bool         IsLoadingToCurrentLayer() const    { return (m_nFlags & eObjectLoader_LoadToCurrentLayer) != 0; }

private:
	void SerializeObjects(bool processEvents);
	void SortObjects();
	void ResolveObjectsGuids();
	void CreateObjects(bool processEvents);
	void CallPostLoadOnObjects();

	struct SLoadedObjectInfo
	{
		i32                     nSortOrder;
		_smart_ptr<CBaseObject> pObject;
		XmlNodeRef              xmlNode;
		DrxGUID                 newGuid;
		bool operator<(const SLoadedObjectInfo& oi) const { return nSortOrder < oi.nSortOrder; }
	};

	IObjectManager* m_objectManager;
	struct Callback
	{
		ResolveObjRefFunctor1   func1;
		ResolveObjRefFunctor2   func2;
		u32                  userData;
		_smart_ptr<CBaseObject> fromObject;
		Callback() { func1 = 0; func2 = 0; userData = 0; };
	};
	typedef std::multimap<DrxGUID, Callback> Callbacks;
	Callbacks m_resolveCallbacks;

	// Set of all saved objects to this archive.
	typedef std::set<_smart_ptr<CBaseObject>> ObjectsSet;
	ObjectsSet m_savedObjects;

	//typedef std::multimap<i32,_smart_ptr<CBaseObject> > OrderedObjects;
	//OrderedObjects m_orderedObjects;
	std::vector<SLoadedObjectInfo> m_loadedObjects;

	// Loaded objects IDs, used for remapping of GUIDs.
	TGUIDRemap m_IdRemap;

	enum EObjectLoaderFlags
	{
		eObjectLoader_ReconstructPrefabs   = 0x0001, // If the loading is for reconstruction from prefab.
		eObjectLoader_ResetInternalMembers = 0x0002, // In case we are deserializing and we would like to wipe all previous state
		eObjectLoader_SavingInPrefabLib    = 0x0004, // We are serializing in the prefab library (used to omit some not needed attributes of objects)
		eObjectLoader_LoadToCurrentLayer   = 0x0008  // If true new loaded objects will be added to current instead of saved layer.
	};
	i32             m_nFlags;
	CPakFile*       m_pGeometryPak;
	CBaseObject*    m_pCurrentObject;
	IGuidProvider*  m_pGuidProvider{ nullptr };

	bool m_bNeedResolveObjects;
	bool m_bProgressBarEnabled;

	// This table is used when there is any collision of ids while importing TrackView sequences.
	std::map<u32, u32> m_sequenceIdRemap;
	std::vector<u32>      m_pendingIds;
};


