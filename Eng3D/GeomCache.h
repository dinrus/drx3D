// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// ------------------------------------------------------------------------
//  Имя файла:   GeomCache.h
//  Created:     19/7/2012 by Axel Gneiting
//  Описание: Manages geometry cache data
// -------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////

#ifndef _GEOMCACHE_
#define _GEOMCACHE_

#pragma once

#if defined(USE_GEOM_CACHES)

	#include <drx3D/Eng3D/IGeomCache.h>
	#include <drx3D/Eng3D/GeomCacheFileFormat.h>

struct SGeomCacheStaticMeshData
{
	bool                                  m_bUsePredictor;
	u8                                 m_positionPrecision[3];
	u32                                m_numVertices;
	GeomCacheFile::EStreams               m_constantStreams;
	GeomCacheFile::EStreams               m_animatedStreams;
	uint64                                m_hash;
	AABB                                  m_aabb;
	string                                m_name;

	std::vector<vtx_idx>                  m_indices;
	std::vector<u32>                   m_numIndices;
	stl::aligned_vector<Vec3, 16>         m_positions;
	stl::aligned_vector<UCol, 16>         m_colors;
	stl::aligned_vector<Vec2, 16>         m_texcoords;
	stl::aligned_vector<SPipTangents, 16> m_tangents;
	std::vector<u16>                   m_materialIds;
	std::vector<u16>                   m_predictorData;
};

struct SGeomCacheStaticNodeData
{
	u32                        m_meshOrGeometryIndex;
	u32                        m_numChildren;
	GeomCacheFile::ENodeType      m_type;
	GeomCacheFile::ETransformType m_transformType;
	QuatTNS                       m_localTransform;
	u32                        m_nameHash;
	string                        m_name;
};

class CGeomCacheStreamReader
{
public:
	CGeomCacheStreamReader(tukk pData, const size_t length)
		: m_pData(pData), m_length(length), m_position(0) {}

	template<class T>
	bool Read(T* pDest, size_t numElements)
	{
		const size_t numBytes = sizeof(T) * numElements;

		if (m_position + numBytes > m_length)
		{
			return false;
		}

		memcpy(pDest, &m_pData[m_position], numBytes);
		m_position += numBytes;

		return true;
	}

	template<class T>
	bool Read(T* pDest)
	{
		const size_t numBytes = sizeof(T);

		if (m_position + numBytes > m_length)
		{
			return false;
		}

		memcpy(pDest, &m_pData[m_position], numBytes);
		m_position += numBytes;

		return true;
	}

private:
	tukk  m_pData;
	const size_t m_length;
	size_t       m_position;
};

struct IGeomCacheListener
{
public:
	virtual ~IGeomCacheListener() {}

	virtual void OnGeomCacheStaticDataLoaded() = 0;
	virtual void OnGeomCacheStaticDataUnloaded() = 0;
};

class CGeomCache : public IGeomCache, public IStreamCallback, public DinrusX3dEngBase
{
	friend class CGeomCacheUpr;

public:
	CGeomCache(tukk pFileName);
	~CGeomCache();

	// Gets number of frames
	uint GetNumFrames() const;

	// Returns true if cache plays back from memory
	bool        PlaybackFromMemory() const;

	tukk GetFrameData(const uint frameIndex) const;
	uint64      GetCompressedAnimationDataSize() const;

	// Gets the max extend of the geom cache through the entire animation
	AABB GetAABB() const;

	// Returns frame for specific time. Rounds to ceil or floor
	uint GetFloorFrameIndex(const float time) const;
	uint GetCeilFrameIndex(const float time) const;

	// Frame infos
	GeomCacheFile::EFrameType GetFrameType(const uint frameIndex) const;
	uint64                    GetFrameOffset(const uint frameIndex) const;
	u32                    GetFrameSize(const uint frameIndex) const;
	float                     GetFrameTime(const uint frameIndex) const;
	uint                      GetPrevIFrame(const uint frameIndex) const;
	uint                      GetNextIFrame(const uint frameIndex) const;

	// Returns true if this frame uses motion prediction and needs the last two frames
	bool NeedsPrevFrames(const uint frameIndex) const;

	// Validates a frame range for reading from disk
	void ValidateReadRange(const uint start, uint& end) const;

	// Get block compression format
	GeomCacheFile::EBlockCompressionFormat GetBlockCompressionFormat() const;

	// Access to the mesh and node lists
	const std::vector<SGeomCacheStaticMeshData>& GetStaticMeshData() const    { return m_staticMeshData; }
	const std::vector<SGeomCacheStaticNodeData>& GetStaticNodeData() const    { return m_staticNodeData; }
	const std::vector<phys_geometry*>&           GetPhysicsGeometries() const { return m_physicsGeometries; }

	// Listener interface for async loading
	void AddListener(IGeomCacheListener* pListener);
	void RemoveListener(IGeomCacheListener* pListener);

	bool IsLoaded() const { return m_bLoaded; }
	void UnloadData();

	// Ref count for streams
	uint GetNumStreams() const { return m_numStreams; }
	void IncreaseNumStreams()  { ++m_numStreams; }
	void DecreaseNumStreams()  { --m_numStreams; }

	// IGeomCache
	virtual i32                         AddRef();
	virtual i32                         Release();

	virtual bool                        IsValid() const { return m_bValid; }

	virtual void                        SetMaterial(IMaterial* pMaterial);
	virtual _smart_ptr<IMaterial>       GetMaterial();
	virtual const _smart_ptr<IMaterial> GetMaterial() const;

	virtual tukk                 GetFilePath() const;

	virtual float                       GetDuration() const;

	virtual IGeomCache::SStatistics     GetStatistics() const;

	virtual void                        Reload();

	// Static data streaming
	void UpdateStreamableComponents(float importance, const Matrix34A& objMatrix, IRenderNode* pRenderNode, bool bFullUpdate);
	void SetLastDrawMainFrameId(u32k id) { m_lastDrawMainFrameId = id; }

	// IStreamable
	virtual void   StartStreaming(bool bFinishNow, IReadStream_AutoPtr* ppStream);
	virtual i32    GetStreamableContentMemoryUsage(bool bJustForDebug);
	virtual void   ReleaseStreamableContent();
	virtual void   GetStreamableName(string& sName);
	virtual u32 GetLastDrawMainFrameId();
	virtual bool   IsUnloadable() const;

	// IStreamCallback
	virtual void StreamOnComplete(IReadStream* pStream, unsigned nError);
	virtual void StreamAsyncOnComplete(IReadStream* pStream, unsigned nError);

private:
	struct SFrameInfo
	{
		float  m_frameTime;
		u32 m_frameType;
		u32 m_frameSize;
		u32 m_prevIFrame;
		u32 m_nextIFrame;
		uint64 m_frameOffset;
	};

	void Shutdown();

	bool LoadGeomCache();

	bool ReadFrameInfos(FILE* pFile, u32k numFrames);

	bool ReadStaticBlock(FILE* pFile, GeomCacheFile::EBlockCompressionFormat compressionFormat, std::vector<char>& compressedData);
	bool DecompressStaticBlock(GeomCacheFile::EBlockCompressionFormat compressionFormat, tukk pCompressedData, std::vector<char>& decompressedData);

	bool ReadMeshesStaticData(CGeomCacheStreamReader& reader, tukk pFileName);
	bool ReadMeshStaticData(CGeomCacheStreamReader& reader, const GeomCacheFile::SMeshInfo& meshInfo,
	                        SGeomCacheStaticMeshData& mesh, tukk pFileName);

	bool        LoadAnimatedData(tukk pData, const size_t bufferOffset);

	bool        ReadNodesStaticDataRec(CGeomCacheStreamReader& reader);

	static bool CompareFrameTimes(const SFrameInfo& a, const SFrameInfo& b)
	{
		return a.m_frameTime < b.m_frameTime;
	}

	tuk GetFrameData(const uint frameIndex);

	bool                  m_bValid;
	bool                  m_bLoaded;

	i32                   m_refCount;
	_smart_ptr<IMaterial> m_pMaterial;
	string                m_fileName;
	string                m_lastError;

	// Static data streaming state
	bool           m_bUseStreaming;
	u32         m_lastDrawMainFrameId;
	IReadStreamPtr m_pStaticDataReadStream;

	// Cache block compression format
	GeomCacheFile::EBlockCompressionFormat m_blockCompressionFormat;

	// Playback from memory flag
	bool m_bPlaybackFromMemory;

	// Number of frames
	uint m_numFrames;

	// Number of streams reading from this cache
	uint m_numStreams;

	// Offset of static mesh data
	uint64 m_staticMeshDataOffset;

	// Total size of animated data
	uint64 m_compressedAnimationDataSize;

	// Total size of uncompressed animation data
	uint64 m_totalUncompressedAnimationSize;

	// AABB of entire animation
	AABB m_aabb;

	// Static data size;
	GeomCacheFile::SCompressedBlockHeader m_staticDataHeader;

	// Frame infos
	std::vector<SFrameInfo>               m_frameInfos;

	std::vector<SGeomCacheStaticMeshData> m_staticMeshData;
	std::vector<SGeomCacheStaticNodeData> m_staticNodeData;

	// Physics
	std::vector<phys_geometry*> m_physicsGeometries;

	// Holds references of static render meshes until cache object dies
	std::vector<_smart_ptr<IRenderMesh>> m_staticRenderMeshes;

	// Listeners
	std::vector<IGeomCacheListener*> m_listeners;

	// Animation data (memory playback)
	std::vector<char> m_animationData;
};

#endif
#endif
