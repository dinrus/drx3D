// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   IDrxFactoryRegistry.h
//  Version:     v1.00
//  Created:     02/25/2009 by CarstenW
//  Описание: Part of drx3D's extension framework.
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#ifndef _IDRXFACTORYREGISTRY_H_
#define _IDRXFACTORYREGISTRY_H_

#pragma once

#include "DrxTypeID.h"

struct IDrxFactory;

struct IDrxFactoryRegistry
{
	virtual IDrxFactory* GetFactory(const DrxClassID& cid) const = 0;
	virtual void         IterateFactories(const DrxInterfaceID& iid, IDrxFactory** pFactories, size_t& numFactories) const = 0;

protected:
	//! Prevent explicit destruction from client side (delete, shared_ptr, etc).
	virtual ~IDrxFactoryRegistry() {}
};

#endif // #ifndef _IDRXFACTORYREGISTRY_H_