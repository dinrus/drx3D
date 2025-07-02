// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------

- 16:09:2008   Benito G.R.

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/GameParameters.h>
#include <drx3D/Game/ItemSharedParams.h>
#include <drx3D/Game/WeaponSharedParams.h>
#include <drx3D/Game/ItemResourceCache.h>
#include <drx3D/Game/FireModeParams.h>

CGameSharedParametersStorage::CGameSharedParametersStorage()
: m_pItemResourceCache(NULL)
{
	m_pItemResourceCache = new CItemResourceCache();
}

CGameSharedParametersStorage::~CGameSharedParametersStorage()
{
	SAFE_DELETE(m_pItemResourceCache);

	m_itemParametersMap.clear();
	m_weaponParametersMap.clear();
}

//================================================================
void CGameSharedParametersStorage::GetMemoryStatistics(IDrxSizer *s)
{	
	s->AddObject(m_itemParametersMap);
	s->AddObject(m_weaponParametersMap);
	m_pItemResourceCache->GetMemoryStatistics(s);
}

//========================================================
template <typename TSharedParams>
TSharedParams* GetSharedParameters(CGameSharedParametersStorage::TSharedParamsMap &paramsMap, tukk className, bool create)
{
	CGameSharedParametersStorage::TSharedParamsMap::iterator it=paramsMap.find(CONST_TEMP_STRING(className));
	if (it!=paramsMap.end())
		return static_cast<TSharedParams*>(it->second.get());

	if (create)
	{
		TSharedParams *params = new TSharedParams();
		paramsMap.insert(CGameSharedParametersStorage::TSharedParamsMap::value_type(className, params));

		return params;
	}

	return 0;
}

//========================================================
CItemSharedParams *CGameSharedParametersStorage::GetItemSharedParameters(tukk className, bool create)
{
	return GetSharedParameters<CItemSharedParams>(m_itemParametersMap, className, create);
}

//========================================================
CWeaponSharedParams *CGameSharedParametersStorage::GetWeaponSharedParameters(tukk className, bool create)
{
	return GetSharedParameters<CWeaponSharedParams>(m_weaponParametersMap, className, create);
}


//========================================================
i32 CGameSharedParametersStorage::GetWeaponSharedParametersCount() const
{
	return m_weaponParametersMap.size();
}

//========================================================
tukk CGameSharedParametersStorage::GetWeaponSharedParametersName(i32 index) const
{
	DRX_ASSERT(index >= 0 && index < (i32)m_weaponParametersMap.size());
	TSharedParamsMap::const_iterator iter = m_weaponParametersMap.begin();
	std::advance(iter, index);
	return iter->first.c_str();
}

//========================================================
void CGameSharedParametersStorage::ReleaseLevelResources()
{
	ReleaseLevelResources(m_itemParametersMap);
	ReleaseLevelResources(m_weaponParametersMap);
}

//========================================================
void CGameSharedParametersStorage::ReleaseLevelResources( TSharedParamsMap& paramsMap )
{
	TSharedParamsMap::iterator paramsMapEnd = paramsMap.end();
	for (TSharedParamsMap::iterator paramsIt = paramsMap.begin(); paramsIt != paramsMapEnd; ++paramsIt)
	{
		paramsIt->second->ReleaseLevelResources();
	}
}

//========================================================
void CGameSharedParametersStorage::GetDetailedMemoryStatics(TSharedParamsMap* paramsMap, tukk typeName)
{
	const float kbInvert = 1.0f/1024.0f;

	i32 totalSize = 0;

	float white[4] = {1.f, 1.f, 1.f, 1.f};
	float purple[4] = {1.f, 0.f, 1.f, 1.f};

	float posY = 50.f;
	float posX = 50.f;

	i32 num = 0;
	
	for (TSharedParamsMap::iterator iter = paramsMap->begin(); iter != paramsMap->end(); ++iter)
	{
		IDrxSizer *pSizer = gEnv->pSystem->CreateSizer();

		iter->second->GetMemoryUsage(pSizer);

		i32 paramSize = pSizer->GetTotalSize();
		totalSize += paramSize;

		gEnv->pRenderer->Draw2dLabel(posX, posY, 1.5f, purple, false, "%s Params: %s. Mem: %.3f Kb", typeName, iter->first.c_str(), paramSize*kbInvert);
		posY += 15.f;

		if(posY > 600.f)
		{
			posX += 500.f;
			posY = 50.f;
		}

		num++;

		pSizer->Release();
	}

	gEnv->pRenderer->Draw2dLabel(50.0f, 30.0f, 1.5f, white, false, "%s Params Total: %d. Mem: %.2f Kb.", typeName, num, totalSize*kbInvert);
}

void CGameSharedParametersStorage::ClearItemParamSets()
{
	ITEM_PARAM_STRUCT_TYPES(CLEAR_SETS)
}

ITEM_PARAM_STRUCT_TYPES(IMPLEMENT_SET_FUNCS)
