// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/MannequinUtils.h>

#include <drx3D/Act/IDrxMannequin.h>

tukk mannequin::FindProcClipTypeName(const IProceduralClipFactory::THash& typeNameHash)
{
	if (gEnv && gEnv->pGameFramework)
	{
		const IProceduralClipFactory& proceduralClipFactory = gEnv->pGameFramework->GetMannequinInterface().GetProceduralClipFactory();
		tukk const typeName = proceduralClipFactory.FindTypeName(typeNameHash);
		return typeName;
	}
	return NULL;
}
