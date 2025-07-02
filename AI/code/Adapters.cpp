// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   Adapters.cpp
//  Created:     02/02/2009 by Matthew
//  Описание: Реализует адаптеры для объектов AI с внешних интерфейсов на внутренние.
//               Это число слой трансляции, без конкретных экземпляров.
//               Они могут не иметь состояния и должны оставаться абстрактными.
// --------------------------------------------------------------------------------------
//  История:
//
/////////////////////////////////////////////////////////////////////////////////////////

#include <drx3D/AI/StdAfx.h>

#include <drx3D/AI/Adapters.h>

CWeakRef<CAIObject> GetWeakRefSafe(IAIObject* pObj) { return (pObj ? GetWeakRef((CAIObject*)pObj) : NILREF); }

IAIObject*          CAIGroupAdapter::GetAttentionTarget(bool bHostileOnly, bool bLiveOnly) const
{
	CWeakRef<CAIObject> refTarget = GetAttentionTarget(bHostileOnly, bLiveOnly, NILREF);
	return refTarget.GetIAIObject();
}
