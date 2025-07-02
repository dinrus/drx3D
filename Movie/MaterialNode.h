// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __MaterialNode_h__
#define __MaterialNode_h__
#pragma once

#include <drx3D/Movie/AnimNode.h>

class CAnimMaterialNode : public CAnimNode
{
public:
	CAnimMaterialNode(i32k id);
	static void            Initialize();

	virtual void           SetName(tukk name) override;

	virtual EAnimNodeType  GetType() const override { return eAnimNodeType_Material; }

	virtual void           Animate(SAnimContext& animContext) override;
	virtual u32   GetParamCount() const override;
	virtual CAnimParamType GetParamType(u32 nIndex) const override;
	virtual tukk    GetParamName(const CAnimParamType& paramType) const override;

	virtual void           InitializeTrackDefaultValue(IAnimTrack* pTrack, const CAnimParamType& paramType) override;

	virtual void           UpdateDynamicParams() override;

protected:
	virtual bool GetParamInfoFromType(const CAnimParamType& paramId, SParamInfo& info) const override;

private:
	void       SetScriptValue();
	void       AnimateNamedParameter(SAnimContext& animContext, IRenderShaderResources* pShaderResources, tukk name, IAnimTrack* pTrack);
	IMaterial* GetMaterialByName(tukk pName);

	std::vector<CAnimNode::SParamInfo> m_dynamicShaderParamInfos;
	typedef std::unordered_map<string, size_t, stl::hash_stricmp<string>, stl::hash_stricmp<string>> TDynamicShaderParamsMap;
	TDynamicShaderParamsMap            m_nameToDynamicShaderParam;
};

#endif // __MaterialNode_h__
