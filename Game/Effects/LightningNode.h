// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _LIGHTINING_NODE_
#define _LIGHTINING_NODE_



#include <drx3D/Game/IGameRenderNode.h>



struct SLightningParams;
struct SLightningStats;



class CLightningRenderNode : public IRenderNode
{
private:
	typedef SVF_P3F_C4B_T2F SLightningVertex;

	class CTriStrip
	{
	private:
		typedef std::vector<SLightningVertex> TVertexArray;
		typedef std::vector<u16> TIndexArray;

	public:
		void Reset();
		void PushVertex(SLightningVertex v);
		void Branch();
		void Draw(const SRendParams& renderParams, const SRenderingPassInfo& passInfo, IRenderer* pRenderer, CRenderObject* pRenderObject, IMaterial* pMaterial, float distanceToCamera);
		void Clear();

		void AddStats(SLightningStats* pStats) const;

	private:
		TVertexArray m_vertices;
		TIndexArray m_indices;
		i32 m_firstVertex;
	};

	struct SPointData
	{
		std::vector<Vec3> m_points;
		std::vector<Vec3> m_velocity;
		std::vector<Vec3> m_fuzzyPoints;
	};

	class CSegment
	{
	public:
		void Create(const SLightningParams& desc, SPointData* m_pointData, i32 _parentSegmentIdx, i32 _parentPointIdx, Vec3 _origin, Vec3 _destany, float _duration, float _intensity);
		void Update(const SLightningParams& desc);
		void Draw(const SLightningParams& desc, const SPointData& pointData, CTriStrip* strip, Vec3 cameraPosition, float deviationMult);
		bool IsDone(const SLightningParams& desc);

		i32 GetNumPoints() const {return m_numFuzzyPoints;}
		Vec3 GetPoint(const SLightningParams& desc, const SPointData& pointData, i32 idx, float deviationMult) const;

		void SetOrigin(Vec3 _origin);
		void SetDestany(Vec3 _destany);

		i32 GetParentSegmentIdx() const {return m_parentSegmentIdx;}
		i32 GetParentPointIdx() const {return m_parentPointIdx;}
		void DecrementParentIdx() {--m_parentSegmentIdx;}

	private:
		i32 m_firstPoint;
		i32 m_numPoints;
		i32 m_firstFuzzyPoint;
		i32 m_numFuzzyPoints;

		Vec3 m_origin;
		Vec3 m_destany;

		float m_duration;
		float m_time;
		float m_intensity;

		i32 m_parentSegmentIdx;
		i32 m_parentPointIdx;
	};

	typedef std::vector<CSegment> TSegments;

public:
	CLightningRenderNode();
	virtual ~CLightningRenderNode();

	// IRenderNode
	virtual EERType GetRenderNodeType();
	virtual tukk GetEntityClassName() const;
	virtual tukk GetName() const;
	virtual Vec3 GetPos(bool bWorldOnly = true) const;
	virtual void Render(const struct SRendParams& rParam, const SRenderingPassInfo& passInfo);
	virtual IPhysicalEntity* GetPhysics() const;
	virtual void SetPhysics(IPhysicalEntity*);
	virtual void SetMaterial(IMaterial* pMat);
	virtual IMaterial* GetMaterial(Vec3* pHitPos = 0) const;
	virtual IMaterial* GetMaterialOverride();
	virtual float GetMaxViewDist();
	virtual void Precache();
	virtual void GetMemoryUsage(IDrxSizer* pSizer) const;
	virtual const AABB GetBBox() const;
	virtual void FillBBox(AABB &aabb);
	virtual void SetBBox(const AABB& WSBBox);
	virtual void OffsetPosition(const Vec3& delta);
	virtual bool IsAllocatedOutsideOf3DEngineDLL();

	void Reset();
	float TriggerSpark();
	void SetLightningParams(const SLightningParams* pDescriptor);
	void SetEmiterPosition(Vec3 emiterPosition);
	void SetReceiverPosition(Vec3 receiverPosition);
	void SetSparkDeviationMult(float deviationMult);

	void AddStats(SLightningStats* pStats) const;

private:
	void Update();
	void Draw(CTriStrip* strip, Vec3 cameraPosition);
	void CreateSegment(Vec3 originPosition, i32 parentIdx, i32 parentPointIdx, float duration, i32 level);
	void PopSegment();

	CTriStrip m_triStrip;
	TSegments m_segments;
	Vec3 m_emmitterPosition;
	Vec3 m_receiverPosition;
	SPointData m_pointData;
	float m_deviationMult;

	const SLightningParams* m_pLightningDesc;
	IMaterial* m_pMaterial;

	mutable AABB m_aabb;
	mutable bool m_dirtyBBox;
};


#endif