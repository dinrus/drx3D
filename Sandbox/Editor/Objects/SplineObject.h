// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

const float kSplinePointSelectionRadius = 0.8f;

// Spline Point
struct CSplinePoint
{
	Vec3  pos;
	Vec3  back;
	Vec3  forw;
	float angle;
	float width;
	bool  isDefaultWidth;
	CSplinePoint()
	{
		angle = 0;
		width = 0;
		isDefaultWidth = true;
	}
};

//////////////////////////////////////////////////////////////////////////
class CSplineObject : public CBaseObject
{
protected:
	CSplineObject();

public:
	virtual void  SetPoint(i32 index, const Vec3& pos);
	i32           InsertPoint(i32 index, const Vec3& point);
	void          RemovePoint(i32 index);
	i32           GetPointCount() const     { return m_points.size(); }
	const Vec3&   GetPoint(i32 index) const { return m_points[index].pos; }
	Vec3          GetBezierPos(i32 index, float t) const;
	float         GetBezierSegmentLength(i32 index, float t = 1.0f) const;
	Vec3          GetBezierNormal(i32 index, float t) const;
	Vec3          GetLocalBezierNormal(i32 index, float t) const;
	Vec3          GetBezierTangent(i32 index, float t) const;
	i32           GetNearestPoint(const Vec3& raySrc, const Vec3& rayDir, float& distance);
	float         GetSplineLength() const;
	float         GetPosByDistance(float distance, i32& outIndex) const;

	float         GetPointAngle() const;
	void          SetPointAngle(float angle);
	float         GetPointWidth() const;
	void          SetPointWidth(float width);
	bool          IsPointDefaultWidth() const;
	void          PointDafaultWidthIs(bool isDefault);

	void          SelectPoint(i32 index);
	i32           GetSelectedPoint() const     { return m_selectedPoint; }

	void          SetEditMode(bool isEditMode) { m_isEditMode = isEditMode; }

	void          SetMergeIndex(i32 index)     { m_mergeIndex = index; }
	void          ReverseShape();
	void          Split(i32 index, const Vec3& point);
	void          Merge(CSplineObject* pSpline);

	void          CalcBBox();
	virtual float CreationZOffset() const { return 0.1f; }

	void          GetNearestEdge(const Vec3& raySrc, const Vec3& rayDir, i32& p1, i32& p2, float& distance, Vec3& intersectPoint);

	virtual void  OnUpdate()                  {}
	virtual void  SetLayerId(u16 nLayerId) {}
	virtual void  SetPhysics(bool isPhysics)  {}
	virtual float GetAngleRange() const       { return 180.0f; }

	virtual void  OnContextMenu(CPopupMenuItem* menu);

protected:
	DECLARE_DYNAMIC(CSplineObject);
	void          OnUpdateUI();

	void          DrawJoints(DisplayContext& dc);
	bool          RayToLineDistance(const Vec3& rayLineP1, const Vec3& rayLineP2, const Vec3& pi, const Vec3& pj, float& distance, Vec3& intPnt);

	virtual i32   GetMaxPoints() const { return 1000; }
	virtual i32   GetMinPoints() const { return 2; }
	virtual float GetWidth() const     { return 0.0f; }
	virtual float GetStepSize() const  { return 1.0f; }

	void          BezierCorrection(i32 index);
	void          BezierAnglesCorrection(i32 index);

	// from CBaseObject
	bool         Init(CBaseObject* prev, const string& file) override;
	void         Done() override;

	void CreateInspectorWidgets(CInspectorWidgetCreator& creator) override;

	void         GetBoundBox(AABB& box) override;
	void         GetLocalBounds(AABB& box) override;
	void         Display(CObjectRenderHelper& objRenderHelper) override;
	bool         HitTest(HitContext& hc) override;
	bool         HitTestRect(HitContext& hc) override;
	void         Serialize(CObjectArchive& ar) override;
	i32          MouseCreateCallback(IDisplayViewport* pView, EMouseEvent event, CPoint& point, i32 flags) override;
	virtual bool IsScalable() const override  { return !m_isEditMode; }
	virtual bool IsRotatable() const override { return !m_isEditMode; }

	void         EditSpline();

protected:
	void         SerializeProperties(Serialization::IArchive& ar, bool bMultiEdit);

protected:
	std::vector<CSplinePoint>  m_points;
	AABB                       m_bbox;

	i32                        m_selectedPoint;
	i32                        m_mergeIndex;

	bool                       m_isEditMode;

	static class CSplinePanel* m_pSplinePanel;
	static i32                 m_splineRollupID;
};

