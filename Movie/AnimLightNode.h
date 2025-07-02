// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Movie/EntityNode.h>

// Node for light animation set sequences
// Normal lights are handled by CEntityNode
class CAnimLightNode : public CAnimEntityNode
{
public:
	CAnimLightNode(i32k id);
	~CAnimLightNode();

	static void            Initialize();
	virtual EAnimNodeType  GetType() const override { return eAnimNodeType_Light; }
	virtual void           Animate(SAnimContext& animContext) override;
	virtual void           CreateDefaultTracks() override;
	virtual void           OnReset() override;
	virtual void           Activate(bool bActivate) override;

	virtual void           Serialize(XmlNodeRef& xmlNode, bool bLoading, bool bLoadEmptyTracks) override;

	virtual u32   GetParamCount() const override;
	virtual CAnimParamType GetParamType(u32 nIndex) const override;
	virtual void            InitializeTrackDefaultValue(IAnimTrack* pTrack, const CAnimParamType& paramType) override;

private:
	virtual bool            GetParamInfoFromType(const CAnimParamType& paramId, SParamInfo& info) const override;

private:
	bool GetValueFromTrack(CAnimParamType type, float time, float& value) const;
	bool GetValueFromTrack(CAnimParamType type, float time, Vec3& value) const;

private:
	float m_fRadius;
	float m_fDiffuseMultiplier;
	float m_fHDRDynamic;
	float m_fSpecularMultiplier;
	float m_fSpecularPercentage;
	Vec3 m_clrDiffuseColor;
	bool m_bJustActivated;
};
