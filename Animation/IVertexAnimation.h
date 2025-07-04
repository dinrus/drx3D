// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//
////////////////////////////////////////////////////////////////////////////

#ifndef IVertexAnimation_h
#define IVertexAnimation_h

struct IVertexFrames
{
	virtual ~IVertexFrames() {}

	virtual uint        GetCount() const = 0;

	virtual tukk GetNameByIndex(const uint index) const = 0;
	virtual uint        GetIndexByName(tukk name) const = 0;
};

struct IVertexAnimation
{
	virtual ~IVertexAnimation() {}

	virtual void OverrideFrameWeightByName(const ISkin* pISkin, tukk name, float weight) = 0;
	virtual bool OverridenFrameWeights() = 0;
	virtual void OverrideFrameWeights(bool val) = 0;
	virtual void ClearAllFramesWeight() = 0;
};

#endif // IVertexAnimation_h
