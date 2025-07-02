// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _GAME_VOLUMES_MANAGER_H_
#define _GAME_VOLUMES_MANAGER_H_

#pragma once

#include <drx3D/CoreX/Game/IGameVolumes.h>

class CGameVolumesUpr : public IGameVolumes, IGameVolumesEdit
{

private:

	typedef std::vector<Vec3> Vertices;
	struct EntityVolume
	{
		EntityVolume()
			: entityId(0)
			, height(0.0f)
			, closed(false)
		{
		}

		bool operator==(const EntityId& id) const
		{
			return entityId == id;
		}

		EntityId entityId;
		f32      height;
		bool     closed;
		Vertices vertices;
	};

	typedef std::vector<EntityVolume>   TEntityVolumes;
	typedef std::vector<IEntityClass*>  TVolumeClasses;
	typedef VectorMap<EntityId, u32> TEntityToIndexMap;

public:
	CGameVolumesUpr();
	virtual ~CGameVolumesUpr();

	// IGameVolumes
	virtual IGameVolumesEdit* GetEditorInterface();
	virtual bool              GetVolumeInfoForEntity(EntityId entityId, VolumeInfo* pOutInfo) const;
	virtual void              Load(tukk fileName);
	virtual void              Reset();
	// ~IGameVolumes

	// IGameVolumesEdit
	virtual void        SetVolume(EntityId entityId, const IGameVolumes::VolumeInfo& volumeInfo);
	virtual void        DestroyVolume(EntityId entityId);

	virtual void        RegisterEntityClass(tukk className);
	virtual size_t      GetVolumeClassesCount() const;
	virtual tukk GetVolumeClass(size_t index) const;

	virtual void        Export(tukk fileName) const;
	// ~IGameVolumesEdit

private:
	void RebuildIndex();

private:
	TEntityToIndexMap m_entityToIndexMap; // Level memory
	TEntityVolumes    m_volumesData; // Level memory
	TVolumeClasses    m_classes;     // Global memory, initialized at start-up

	const static u32 GAME_VOLUMES_FILE_VERSION = 2;
};

#endif
