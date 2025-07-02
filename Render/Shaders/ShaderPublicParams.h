// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

//////////////////////////////////////////////////////////////////////////
// IShaderPublicParams implementation class.
//////////////////////////////////////////////////////////////////////////
class CShaderPublicParams : public IShaderPublicParams
{
public:
	CShaderPublicParams() {}

	virtual void          SetParamCount(i32 nParam) { m_shaderParams.resize(nParam); }
	virtual i32           GetParamCount() const { return m_shaderParams.size(); };

	virtual SShaderParam& GetParam(i32 nIndex);

	virtual const SShaderParam& GetParam(i32 nIndex) const;

	virtual SShaderParam* GetParamByName(tukk pszName);

	virtual const SShaderParam* GetParamByName(tukk pszName) const;

	virtual SShaderParam* GetParamBySemantic(u8 eParamSemantic);

	virtual const SShaderParam* GetParamBySemantic(u8 eParamSemantic) const;

	virtual void SetParam(i32 nIndex, const SShaderParam& param);

	virtual void AddParam(const SShaderParam& param);

	virtual void RemoveParamByName(tukk pszName);

	virtual void RemoveParamBySemantic(u8 eParamSemantic);

	virtual void SetParam(tukk pszName, UParamVal& pParam, EParamType nType = eType_FLOAT, u8 eSemantic = 0);

	virtual void SetShaderParams(const DynArray<SShaderParam>& pParams);

	virtual void AssignToRenderParams(struct SRendParams& rParams);

	virtual DynArray<SShaderParam>* GetShaderParams();

	virtual const DynArray<SShaderParam>* GetShaderParams() const;

	virtual u8 GetSemanticByName(tukk szName);

private:
	DynArray<SShaderParam> m_shaderParams;
};
