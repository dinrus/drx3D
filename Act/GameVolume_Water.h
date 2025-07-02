// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _GAME_VOLUME_WATER_H
#define _GAME_VOLUME_WATER_H

#pragma once

class CGameVolume_Water : public CGameObjectExtensionHelper<CGameVolume_Water, IGameObjectExtension>
{
	struct WaterProperties
	{
		WaterProperties(IEntity* pEntity)
		{
			DRX_ASSERT(pEntity != NULL);

			memset(this, 0, sizeof(WaterProperties));

			SmartScriptTable properties;
			IScriptTable* pScriptTable = pEntity->GetScriptTable();

			if ((pScriptTable != NULL) && pScriptTable->GetValue("Properties", properties))
			{
				properties->GetValue("StreamSpeed", streamSpeed);
				properties->GetValue("FogDensity", fogDensity);
				properties->GetValue("UScale", uScale);
				properties->GetValue("VScale", vScale);
				properties->GetValue("Depth", depth);
				properties->GetValue("ViewDistanceRatio", viewDistanceRatio);
				properties->GetValue("MinSpec", minSpec);
				properties->GetValue("MaterialLayerMask", materialLayerMask);
				properties->GetValue("FogColorMultiplier", fogColorMultiplier);
				properties->GetValue("color_FogColor", fogColor);
				properties->GetValue("bFogColorAffectedBySun", fogColorAffectedBySun);
				properties->GetValue("FogShadowing", fogShadowing);
				properties->GetValue("bCapFogAtVolumeDepth", capFogAtVolumeDepth);
				properties->GetValue("bAwakeAreaWhenMoving", awakeAreaWhenMoving);
				properties->GetValue("bIsRiver", isRiver);
				properties->GetValue("bCaustics", caustics);
				properties->GetValue("CausticIntensity", causticIntensity);
				properties->GetValue("CausticTiling", causticTiling);
				properties->GetValue("CausticHeight", causticHeight);
			}
		}

		float streamSpeed;
		float fogDensity;
		float uScale;
		float vScale;
		float depth;
		i32   viewDistanceRatio;
		i32   minSpec;
		i32   materialLayerMask;
		float fogColorMultiplier;
		Vec3  fogColor;
		bool  fogColorAffectedBySun;
		float fogShadowing;
		bool  capFogAtVolumeDepth;
		bool  awakeAreaWhenMoving;
		bool  isRiver;
		bool  caustics;
		float causticIntensity;
		float causticTiling;
		float causticHeight;
	};

public:
	CGameVolume_Water();
	virtual ~CGameVolume_Water();

	// IGameObjectExtension
	virtual bool                 Init(IGameObject* pGameObject);
	virtual void                 InitClient(i32 channelId)                                                       {};
	virtual void                 PostInit(IGameObject* pGameObject);
	virtual void                 PostInitClient(i32 channelId)                                                   {};
	virtual bool                 ReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& params);
	virtual void                 PostReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& params) {}
	virtual void                 Release();
	virtual void                 FullSerialize(TSerialize ser)                                                 {};
	virtual bool                 NetSerialize(TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags) { return false; };
	virtual void                 PostSerialize()                                                               {};
	virtual void                 SerializeSpawnInfo(TSerialize ser)                                            {}
	virtual ISerializableInfoPtr GetSpawnInfo()                                                                { return 0; }
	virtual void                 Update(SEntityUpdateContext& ctx, i32 slot);
	virtual void                 HandleEvent(const SGameObjectEvent& gameObjectEvent);
	virtual void                 ProcessEvent(const SEntityEvent&);
	virtual uint64               GetEventMask() const final;;
	virtual void                 SetChannelId(u16 id)     {};
	virtual void                 PostUpdate(float frameTime) { DRX_ASSERT(false); }
	virtual void                 PostRemoteSpawn()           {};
	virtual void                 GetMemoryUsage(IDrxSizer* pSizer) const;
	// ~IGameObjectExtension

private:

	struct SWaterSegment
	{
		SWaterSegment() : m_pWaterRenderNode(NULL), m_pWaterArea(NULL), m_physicsLocalAreaCenter(ZERO) {}
		IWaterVolumeRenderNode* m_pWaterRenderNode;
		IPhysicalEntity*        m_pWaterArea;
		Vec3                    m_physicsLocalAreaCenter;
	};

	typedef std::vector<SWaterSegment> WaterSegments;

	void CreateWaterRenderNode(IWaterVolumeRenderNode*& pWaterRenderNode);
	void SetupVolume();
	void SetupVolumeSegment(const WaterProperties& waterProperties, u32k segmentIndex, const Vec3* pVertices, u32k vertexCount);

	void CreatePhysicsArea(u32k segmentIndex, const Matrix34& baseMatrix, const Vec3* pVertices, u32 vertexCount, const bool isRiver, const float streamSpeed);
	void DestroyPhysicsAreas();

	void AwakeAreaIfRequired(bool forceAwake);
	void UpdateRenderNode(IWaterVolumeRenderNode* pWaterRenderNode, const Matrix34& newLocation);

	void FillOutRiverSegment(u32k segmentIndex, const Vec3* pVertices, u32k vertexCount, Vec3* pVerticesOut);

	void DebugDrawVolume();

	WaterSegments m_segments;

	Matrix34      m_baseMatrix;
	Matrix34      m_initialMatrix;
	Vec3          m_lastAwakeCheckPosition;
	float         m_volumeDepth;
	float         m_streamSpeed;
	bool          m_awakeAreaWhenMoving;
	bool          m_isRiver;
};

#endif
