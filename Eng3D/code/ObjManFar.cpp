// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   statobjmanfar.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:    Отрисовка дальних объектов как спрайтов.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Eng3D/StdAfx.h>

#include <drx3D/Eng3D/StatObj.h>
#include <drx3D/Eng3D/ObjMan.h>
#include <drx3D/Eng3D/3dEngine.h>

static PodArray<SVegetationSpriteInfo> arrVegetationSprites[RT_COMMAND_BUF_COUNT][MAX_RECURSION_LEVELS];

void CObjUpr::UnloadFarObjects()
{
	for (i32 i = 0; i < RT_COMMAND_BUF_COUNT; ++i)
		for (i32 j = 0; j < MAX_RECURSION_LEVELS; ++j)
			stl::free_container(arrVegetationSprites[i][j]);
}

void CObjUpr::RenderFarObjects(const SRenderingPassInfo& passInfo)
{
}
/*
   static inline i32 Compare(CVegetation *& p1, CVegetation *& p2)
   {
   if(p1->m_fDistance > p2->m_fDistance)
    return 1;
   else
   if(p1->m_fDistance < p2->m_fDistance)
    return -1;

   return 0;
   } */

void CObjUpr::DrawFarObjects(float fMaxViewDist, const SRenderingPassInfo& passInfo)
{
	if (!GetCVars()->e_VegetationSprites)
		return;

	FUNCTION_PROFILER_3DENGINE;

	if (passInfo.GetRecursiveLevel() >= MAX_RECURSION_LEVELS)
	{
		assert(!"Recursion depther than MAX_RECURSION_LEVELS is not supported");
		return;
	}

	//////////////////////////////////////////////////////////////////////////////////////
	//  Draw all far
	//////////////////////////////////////////////////////////////////////////////////////

	//PodArray<SVegetationSpriteInfo> * pList = &arrVegetationSprites[m_nRenderStackLevel];
	//if (pList->Count())
	//  GetRenderer()->DrawObjSprites(pList);
}

void CObjUpr::GenerateFarObjects(float fMaxViewDist, const SRenderingPassInfo& passInfo)
{
	if (!GetCVars()->e_VegetationSprites)
		return;

	FUNCTION_PROFILER_3DENGINE;

	if (passInfo.GetRecursiveLevel() >= MAX_RECURSION_LEVELS)
	{
		assert(!"Recursion depther than MAX_RECURSION_LEVELS is not supported");
		return;
	}

	//////////////////////////////////////////////////////////////////////////////////////
	//  Draw all far
	//////////////////////////////////////////////////////////////////////////////////////

	/*arrVegetationSprites[m_nRenderStackLevel].Clear();

	   for(i32 t=0; t<nThreadsNum; t++)
	   {
	   PodArray<SVegetationSpriteInfo> * pList = &m_arrVegetationSprites[m_nRenderStackLevel][t];
	   if (pList->Count())
	    arrVegetationSprites[m_nRenderStackLevel].AddList(*pList);
	   }*/

	//PodArray<SVegetationSpriteInfo> * pList = &arrVegetationSprites[m_nRenderStackLevel];
	//if (pList->Count())
	//  GetRenderer()->GenerateObjSprites(pList);
}
