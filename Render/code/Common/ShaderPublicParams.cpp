// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/ShaderPublicParams.h>

#include <drx3D/Render/DriverD3D.h>

SShaderParam& CShaderPublicParams::GetParam(i32 nIndex)
{
	assert(nIndex >= 0 && nIndex < (i32)m_shaderParams.size());
	return m_shaderParams[nIndex];
}

const SShaderParam& CShaderPublicParams::GetParam(i32 nIndex) const
{
	assert(nIndex >= 0 && nIndex < (i32)m_shaderParams.size());
	return m_shaderParams[nIndex];
}

SShaderParam* CShaderPublicParams::GetParamByName(tukk pszName)
{
	for (i32 i = 0; i < m_shaderParams.size(); i++)
	{
		if (!stricmp(pszName, m_shaderParams[i].m_Name))
		{
			return &m_shaderParams[i];
		}
	}

	return 0;
}

const SShaderParam* CShaderPublicParams::GetParamByName(tukk pszName) const
{
	for (i32 i = 0; i < m_shaderParams.size(); i++)
	{
		if (!stricmp(pszName, m_shaderParams[i].m_Name))
		{
			return &m_shaderParams[i];
		}
	}

	return 0;
}

SShaderParam* CShaderPublicParams::GetParamBySemantic(u8 eParamSemantic)
{
	for (i32 i = 0; i < m_shaderParams.size(); ++i)
	{
		if (m_shaderParams[i].m_eSemantic == eParamSemantic)
		{
			return &m_shaderParams[i];
		}
	}

	return NULL;
}

const SShaderParam* CShaderPublicParams::GetParamBySemantic(u8 eParamSemantic) const
{
	for (i32 i = 0; i < m_shaderParams.size(); ++i)
	{
		if (m_shaderParams[i].m_eSemantic == eParamSemantic)
		{
			return &m_shaderParams[i];
		}
	}

	return NULL;
}

void CShaderPublicParams::SetParam(tukk pszName, UParamVal& pParam, EParamType nType /*= eType_FLOAT*/, u8 eSemantic /*= 0*/)
{
	i32 i;

	for (i = 0; i < m_shaderParams.size(); i++)
	{
		if (!stricmp(pszName, m_shaderParams[i].m_Name))
		{
			break;
		}
	}

	if (i == m_shaderParams.size())
	{
		SShaderParam pr;
		drx_strcpy(pr.m_Name, pszName);
		pr.m_Type = nType;
		pr.m_eSemantic = eSemantic;
		m_shaderParams.push_back(pr);
	}

	SShaderParam::SetParam(pszName, &m_shaderParams, pParam);
}

void CShaderPublicParams::SetParam(i32 nIndex, const SShaderParam& param)
{
	assert(nIndex >= 0 && nIndex < (i32)m_shaderParams.size());

	m_shaderParams[nIndex] = param;
}

void CShaderPublicParams::SetShaderParams(const DynArray<SShaderParam>& pParams)
{
	m_shaderParams = pParams;
}

void CShaderPublicParams::AssignToRenderParams(struct SRendParams& rParams)
{
	if (!m_shaderParams.empty())
		rParams.pShaderParams = &m_shaderParams;
}

void CShaderPublicParams::AddParam(const SShaderParam& param)
{
	// shouldn't add existing parameter ?
	m_shaderParams.push_back(param);
}

void CShaderPublicParams::RemoveParamByName(tukk pszName)
{
	for (i32 i = 0; i < m_shaderParams.size(); i++)
	{
		if (!stricmp(pszName, m_shaderParams[i].m_Name))
		{
			m_shaderParams.erase(i);
		}
	}
}

void CShaderPublicParams::RemoveParamBySemantic(u8 eParamSemantic)
{
	for (i32 i = 0; i < m_shaderParams.size(); ++i)
	{
		if (eParamSemantic == m_shaderParams[i].m_eSemantic)
		{
			m_shaderParams.erase(i);
		}
	}
}

DynArray<SShaderParam>* CShaderPublicParams::GetShaderParams()
{
	if (m_shaderParams.empty())
	{
		return 0;
	}

	return &m_shaderParams;
}

const DynArray<SShaderParam>* CShaderPublicParams::GetShaderParams() const
{
	if (m_shaderParams.empty())
	{
		return 0;
	}

	return &m_shaderParams;
}

u8 CShaderPublicParams::GetSemanticByName(tukk szName)
{
	static_assert(ECGP_COUNT <= 0xff, "8 bits are not enough to store all ECGParam values");

	if (strcmp(szName, "WrinkleMask0") == 0)
	{
		return ECGP_PI_WrinklesMask0;
	}
	if (strcmp(szName, "WrinkleMask1") == 0)
	{
		return ECGP_PI_WrinklesMask1;
	}
	if (strcmp(szName, "WrinkleMask2") == 0)
	{
		return ECGP_PI_WrinklesMask2;
	}

	return ECGP_Unknown;
}
