// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __OFFMESH_NAVIGATION_MANAGER_H__
#define __OFFMESH_NAVIGATION_MANAGER_H__

#pragma once

#include <drx3D/AI/INavigationSystem.h>
#include <drx3D/AI/IOffMeshNavigationUpr.h>
#include <drx3D/AI/OffGridLinks.h>

class CSmartObject;
class CSmartObjectClass;

//////////////////////////////////////////////////////////////////////////
/// OffMesh Navigation Upr
/// Keeps track of off-mesh navigation objects associated to each navigation mesh
/// and all the smart-objects which can generate off-mesh connections
///
/// Interfaces also with the Navigation System, to update the right objects
/// when meshes are created, updated or deleted
//////////////////////////////////////////////////////////////////////////
class OffMeshNavigationUpr : public IOffMeshNavigationUpr
{
private:
	struct SLinkInfo
	{
		SLinkInfo() {}
		SLinkInfo(NavigationMeshID _meshId, MNM::TriangleID _startTriangleID, MNM::TriangleID _endTriangleID, MNM::OffMeshLinkPtr _offMeshLink)
			: meshID(_meshId)
			, startTriangleID(_startTriangleID)
			, endTriangleID(_endTriangleID)
			, offMeshLink(_offMeshLink)
		{}

		bool operator==(const NavigationMeshID _meshID) const
		{
			return (meshID == _meshID);
		}

		NavigationMeshID    meshID;
		MNM::TriangleID     startTriangleID;
		MNM::TriangleID     endTriangleID;
		MNM::OffMeshLinkPtr offMeshLink;
	};

	struct SObjectMeshInfo
	{
		SObjectMeshInfo(NavigationMeshID _meshId)
			: meshID(_meshId)
		{

		}

		bool operator==(const NavigationMeshID _meshID)
		{
			return (meshID == _meshID);
		}

		typedef std::vector<MNM::TriangleID> TTriangleList;
		TTriangleList    triangleList;
		NavigationMeshID meshID;
	};
	typedef std::vector<SObjectMeshInfo> TObjectInfo;

public:
#if DEBUG_MNM_ENABLED
	struct ProfileMemoryStats
	{
		ProfileMemoryStats()
			: linkInfoSize(0)
			, totalSize(0)
		{
		}

		size_t linkInfoSize;      // SLinkInfo
		size_t totalSize;         // currently redundant, but good for easier extension in the future
	};
#endif

public:
	OffMeshNavigationUpr(i32k offMeshMapSize);

	const MNM::OffMeshNavigation& GetOffMeshNavigationForMesh(const NavigationMeshID& meshID) const;

	//Note: Try to kill this one, outside should never manipulate this
	// Needs to make a lot of code in SmartObjects const correct first...
	MNM::OffMeshNavigation& GetOffMeshNavigationForMesh(const NavigationMeshID& meshID);

	void                    RegisterSmartObject(CSmartObject* pSmartObject, CSmartObjectClass* pSmartObjectClass);
	void                    UnregisterSmartObjectForAllClasses(CSmartObject* pSmartObject);

	void                    RefreshConnections(const NavigationMeshID meshID, const MNM::TileID tileID);
	void                    Clear();
	void                    Enable();

	void                    OnNavigationMeshCreated(const NavigationMeshID& meshID);
	void                    OnNavigationMeshDestroyed(const NavigationMeshID& meshID);
	void                    OnNavigationLoadedComplete();

	bool                    IsObjectLinkedWithNavigationMesh(const EntityId objectId) const;

	// IOffMeshNavigationUpr
	virtual void                    QueueCustomLinkAddition(const MNM::LinkAdditionRequest& request) override;
	virtual void                    QueueCustomLinkRemoval(const MNM::LinkRemovalRequest& request) override;
	virtual MNM::OffMeshLink*       GetOffMeshLink(const MNM::OffMeshLinkID& linkID) override;
	virtual const MNM::OffMeshLink* GetOffMeshLink(const MNM::OffMeshLinkID& linkID) const override;
	virtual void                    RegisterListener(IOffMeshNavigationListener* pListener, tukk listenerName) override;
	virtual void                    UnregisterListener(IOffMeshNavigationListener* pListener) override;
	virtual void                    RemoveAllQueuedAdditionRequestForEntity(const EntityId requestOwner) override;
	// ~IOffMeshNavigationUpr

	void ProcessQueuedRequests();

#if DEBUG_MNM_ENABLED
	void               UpdateEditorDebugHelpers();
	ProfileMemoryStats GetMemoryStats(IDrxSizer* pSizer, const NavigationMeshID meshID) const;
#else
	void               UpdateEditorDebugHelpers() {};
#endif

private:
	bool       AddCustomLink(const NavigationMeshID& meshID, MNM::OffMeshLinkPtr& pLinkData, MNM::OffMeshLinkID& linkID, MNM::TriangleID* pOutStartTriangleID, MNM::TriangleID* pOutEndTriangleID, const bool bCloneLinkData);
	void       RemoveCustomLink(const MNM::OffMeshLinkID& linkID);

	bool       IsLinkRemovalRequested(const MNM::OffMeshLinkID& linkID) const;

	void       UnregisterSmartObject(CSmartObject* pSmartObject, const string& smartObjectClassName);

	bool       ObjectRegistered(const EntityId objectId, const string& smartObjectClassName) const;
	ILINE bool CanRegisterObject() const { return m_objectRegistrationEnabled; };

	void       NotifyAllListenerAboutLinkAddition(const MNM::OffMeshLinkID& linkID);
	void       NotifyAllListenerAboutLinkDeletion(const MNM::OffMeshLinkID& linkID);

	// For every navigation mesh, there will be always an off-mesh navigation object
	typedef id_map<u32, MNM::OffMeshNavigation> TOffMeshMap;
	TOffMeshMap            m_offMeshMap;

	MNM::OffMeshNavigation m_emptyOffMeshNavigation;

	// Tracking of objects registered
	// All registered objects are stored here, and some additional data
	// like to witch mesh they belong (only one), or bound triangles/tiles
	struct OffMeshLinkIDList
	{
		typedef std::vector<MNM::OffMeshLinkID> TLinkIDList;

		TLinkIDList&       GetLinkIDList()       { return offMeshLinkIDList; }
		const TLinkIDList& GetLinkIDList() const { return offMeshLinkIDList; }

		void               OnLinkAdditionRequestForSmartObjectServiced(const MNM::SOffMeshOperationCallbackData& callbackData)
		{
			if (callbackData.operationSucceeded)
			{
				assert(callbackData.linkID != MNM::Constants::eOffMeshLinks_InvalidOffMeshLinkID);
				offMeshLinkIDList.push_back(callbackData.linkID);
			}
		}
		void               OnLinkRepairRequestForSmartObjectServiced(const MNM::SOffMeshOperationCallbackData& callbackData)
		{
			if (!callbackData.operationSucceeded)
			{
				auto it = std::find(offMeshLinkIDList.begin(), offMeshLinkIDList.end(), callbackData.linkID);
				assert(it != offMeshLinkIDList.end());
				if (it != offMeshLinkIDList.end())
				{
					offMeshLinkIDList.erase(it);
				}
			}
		}

	private:
		TLinkIDList offMeshLinkIDList;
	};

	typedef stl::STLPoolAllocator<std::pair<u32k, OffMeshLinkIDList>, stl::PoolAllocatorSynchronizationSinglethreaded> TSOClassInfoAllocator;
	typedef std::map<u32, OffMeshLinkIDList, std::less<u32>, TSOClassInfoAllocator>             TSOClassInfo;
	typedef std::map<EntityId, TSOClassInfo>                                                          TRegisteredObjects;

	OffMeshLinkIDList* GetClassInfoFromLinkInfo(const SLinkInfo& linkInfo);

	TRegisteredObjects m_registeredObjects;

	// List of registered links
	typedef std::map<MNM::OffMeshLinkID, SLinkInfo> TLinkInfoMap;
	TLinkInfoMap m_links;

	bool         m_objectRegistrationEnabled;

	typedef std::list<MNM::OffMeshOperationRequestBase> Operations; // it's a list to safely allow queuing further operations from their callbacks while the queue is being processed
	Operations m_operations;

	typedef CListenerSet<IOffMeshNavigationListener*> Listeners;
	Listeners m_listeners;
};

#endif  //__OFFMESH_NAVIGATION_MANAGER_H__
