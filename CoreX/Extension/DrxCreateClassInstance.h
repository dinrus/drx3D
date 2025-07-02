// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   DrxCreateClassInstance.h
//  Version:     v1.00
//  Created:     02/25/2009 by CarstenW
//  Описание: Part of DinrusX's extension framework.
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#ifndef _DRXCREATECLASSINSTANCE_H_
#define _DRXCREATECLASSINSTANCE_H_

#pragma once

#include "IDrxUnknown.h"
#include "IDrxFactory.h"
#include "IDrxFactoryRegistry.h"
#include <drx3D/Sys/ISystem.h> // <> required for Interfuscator

template<class T>
bool DrxCreateClassInstance(const DrxClassID& cid, std::shared_ptr<T>& p)
{
	p = std::shared_ptr<T>();
	IDrxFactoryRegistry* pFactoryReg = gEnv->pSystem->GetDrxFactoryRegistry();
	if (pFactoryReg)
	{
		IDrxFactory* pFactory = pFactoryReg->GetFactory(cid);
		if (pFactory && pFactory->ClassSupports(drxiidof<T>()))
		{
			IDrxUnknownPtr pUnk = pFactory->CreateClassInstance();
			std::shared_ptr<T> pT = drxinterface_cast<T>(pUnk);
			if (pT)
				p = pT;
		}
	}
	return p.get() != NULL;
}

template<class T>
bool DrxCreateClassInstanceForInterface(const DrxInterfaceID& iid, std::shared_ptr<T>& p)
{
	p = std::shared_ptr<T>();
	IDrxFactoryRegistry* pFactoryReg = gEnv->pSystem->GetDrxFactoryRegistry();
	if (pFactoryReg)
	{
		size_t numFactories = 1;
		IDrxFactory* pFactory = 0;
		pFactoryReg->IterateFactories(iid, &pFactory, numFactories);
		if (numFactories == 1 && pFactory)
		{
			IDrxUnknownPtr pUnk = pFactory->CreateClassInstance();
			std::shared_ptr<T> pT = drxinterface_cast<T>(pUnk);
			if (pT)
				p = pT;
		}
	}
	return p.get() != NULL;
}

#endif // #ifndef _DRXCREATECLASSINSTANCE_H_
