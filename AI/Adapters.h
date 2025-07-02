// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// --------------------------------------------------------------------------------------
//  Имя файла:   Adapters.h
//  Created:     02/02/2009 by Matthew
//  Описание: Реализует адаптеры для объектов ИИ из внешних интерфейсов во внутренние.
//               Это чисто "переводной" слой без конкретных экземпляров.
//               У них может отсутствовать состояние и они должны оставаться абстрактными.
//
//               * Помните, что это быстро становится redundant ("вторящим уже выраженное")!
//               В основном теперь уже играет роль оболочки... *
//
// --------------------------------------------------------------------------------------
//  История:
//
/////////////////////////////////////////////////////////////////////////////////////////

#ifndef __AI_ADAPTERS_H_
#define __AI_ADAPTERS_H_

#pragma once

#include <drx3D/AI/IAgent.h>
#include <drx3D/AI/AIObject.h>
#include <drx3D/AI/ObjectContainer.h>

#include <drx3D/AI/IAIGroup.h>

CWeakRef<CAIObject> GetWeakRefSafe(IAIObject* pObj);

class CPipeUserAdapter : public IPipeUser
//Класс АдаптерПользователяПайпа, наследует интерфейс ИПользовательПайпа
{
public:
	virtual ~CPipeUserAdapter() {}

	virtual bool       SelectPipe(i32 id, tukk name, CWeakRef<CAIObject> refArgument, i32 goalPipeId = 0, bool resetAlways = false, const GoalParams* node = 0) = 0;
	virtual IGoalPipe* InsertSubPipe(i32 id, tukk name, CWeakRef<CAIObject> refArgument = NILREF, i32 goalPipeId = 0, const GoalParams* node = 0) = 0;

private:
	bool       SelectPipe(i32 id, tukk name, IAIObject* pArgument = 0, i32 goalPipeId = 0, bool resetAlways = false, const GoalParams* node = 0)
	{ return SelectPipe(id, name, GetWeakRefSafe(pArgument), goalPipeId, resetAlways, node); }
	IGoalPipe* InsertSubPipe(i32 id, tukk name, IAIObject* pArgument = 0, i32 goalPipeId = 0, const GoalParams* node = 0)
	{ return InsertSubPipe(id, name, GetWeakRefSafe(pArgument), goalPipeId, node); }

};

class CAIGroupAdapter
	: public IAIGroup
{
public:
	// cppcheck-suppress passedByValue
	virtual CWeakRef<CAIObject> GetAttentionTarget(bool bHostileOnly = false, bool bLiveOnly = false, const CWeakRef<CAIObject> refSkipTarget = NILREF) const = 0;

private:
	IAIObject* GetAttentionTarget(bool bHostileOnly = false, bool bLiveOnly = false) const;
};

#endif //__AI_ADAPTERS_H_
