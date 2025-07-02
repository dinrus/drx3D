// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

/*
   This class is needed to keep track of the navigation shapes created in Sandbox and their
   respective IDs. It's necessary to keep consistency with the exported Navigation data
   since the AISystem has no knowledge of the Editor's shape
 */

class CVolumesUpr : public ISystemEventListener
{
public:
	CVolumesUpr();
	~CVolumesUpr();

	// ISystemEventListener
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override;
	// ~ISystemEventListener

	bool               RegisterArea(tukk volumeName, NavigationVolumeID& outVolumeId);
	void               UnRegisterArea(tukk volumeName);

	void               Clear();

	void               RegisterAreaFromLoadedData(tukk szVolumeName, NavigationVolumeID id);
	void               ClearLoadedAreas();
	void               ValidateAndSanitizeLoadedAreas(INavigationSystem& navigationSystem);

	bool               SetAreaID(tukk volumeName, NavigationVolumeID id);
	void               InvalidateID(NavigationVolumeID id);
	void               UpdateNameForAreaID(const NavigationVolumeID id, tukk newName);

	bool               IsAreaPresent(tukk volumeName) const;
	NavigationVolumeID GetAreaID(tukk volumeName) const;
	bool               GetAreaName(NavigationVolumeID id, string& name) const;

	void               GetVolumesNames(std::vector<string>& names) const;

	bool               IsLoadedAreaPresent(tukk volumeName) const;
	NavigationVolumeID GetLoadedAreaID(tukk volumeName) const;
	void               GetLoadedUnregisteredVolumes(std::vector<NavigationVolumeID>& volumes) const;

	bool               RegisterEntityMarkups(INavigationSystem& navigationSystem, const IEntity& entity, tukk* pShapeNamesArray, const size_t count, NavigationVolumeID* pOutIds);
	bool               UnregisterEntityMarkups(INavigationSystem& navigationSystem, const IEntity& entity);

	void               SaveData(CDrxFile& file, u16k version) const;
	void               LoadData(CDrxFile& file, u16k version);

private:
	typedef std::map<string, NavigationVolumeID> VolumesMap;
	typedef std::unordered_map<EntityGUID, VolumesMap> EntitiesWithMarkupsMap;
	VolumesMap m_volumeAreas;
	VolumesMap m_loadedVolumeAreas;

	EntitiesWithMarkupsMap m_registeredEntityMarkupsMap;
};

