// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _IGAME_VOLUMES_H_
#define _IGAME_VOLUMES_H_

struct IGameVolumesEdit;

struct IGameVolumes
{
	struct VolumeInfo
	{
		VolumeInfo()
			: pVertices(NULL)
			, verticesCount(0)
			, volumeHeight(0.0f)
			, closed(false)
		{
		}

		const Vec3* pVertices;
		u32      verticesCount;
		f32         volumeHeight;
		bool        closed;
	};

	// <interfuscator:shuffle>
	virtual ~IGameVolumes() {};

	virtual IGameVolumesEdit* GetEditorInterface() = 0;

	virtual bool              GetVolumeInfoForEntity(EntityId entityId, VolumeInfo* pOutInfo) const = 0;
	virtual void              Load(tukk fileName) = 0;
	virtual void              Reset() = 0;
	// </interfuscator:shuffle>
};

struct IGameVolumesEdit
{
	// <interfuscator:shuffle>
	virtual ~IGameVolumesEdit() {};

	virtual void        SetVolume(EntityId entityId, const IGameVolumes::VolumeInfo& volumeInfo) = 0;
	virtual void        DestroyVolume(EntityId entityId) = 0;

	virtual void        RegisterEntityClass(tukk className) = 0;
	virtual size_t      GetVolumeClassesCount() const = 0;
	virtual tukk GetVolumeClass(size_t index) const = 0;

	virtual void        Export(tukk fileName) const = 0;
	// </interfuscator:shuffle>
};

#endif
