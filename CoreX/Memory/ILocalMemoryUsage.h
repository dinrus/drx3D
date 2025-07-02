// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ILocalMemoryUsage.h
//  Version:     v1.00
//  Created:     Istvan Kiss (Spidy) [11/5/2008]
//  Описание:    Интерфейс для LocalMemoryUsage
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef _ILOCALMEMORYUSAGE_H_
#define _ILOCALMEMORYUSAGE_H_

class CCamera;
struct IRenderer;

struct ILocalMemoryUsage
{
	virtual ~ILocalMemoryUsage(){}
	virtual void OnRender(IRenderer* pRenderer, const CCamera* camera) = 0;
	virtual void OnUpdate() = 0;
	virtual void DeleteGlobalData() = 0;
};

#endif
