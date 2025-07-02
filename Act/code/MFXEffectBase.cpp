// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/MFXEffectBase.h>

#include <drx3D/Act/IMaterialEffects.h>

CMFXEffectBase::CMFXEffectBase(u16k _typeFilterFlag)
{
	m_runtimeExecutionFilter.AddFlags(_typeFilterFlag);
}

void CMFXEffectBase::SetCustomParameter(tukk customParameter, const SMFXCustomParamValue& customParameterValue)
{

}

void CMFXEffectBase::PreLoadAssets()
{

}

void CMFXEffectBase::ReleasePreLoadAssets()
{

}

bool CMFXEffectBase::CanExecute(const SMFXRunTimeEffectParams& params) const
{
	return ((params.playflags & m_runtimeExecutionFilter.GetRawFlags()) != 0);
}
