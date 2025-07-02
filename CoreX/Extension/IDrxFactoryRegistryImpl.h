// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   IDrxFactoryRegistryImpl.h
//  Version:     v1.00
//  Created:     02/25/2009 by CarstenW
//  Описание: Part of drx3D's extension framework.
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#ifndef _IDRXFACTORYREGISTRYIMPL_H_
#define _IDRXFACTORYREGISTRYIMPL_H_

#pragma once

#include <drx3D/CoreX/Extension/IDrxFactoryRegistry.h>

struct SRegFactoryNode;

struct IDrxFactoryRegistryCallback
{
	virtual void OnNotifyFactoryRegistered(IDrxFactory* pFactory) = 0;
	virtual void OnNotifyFactoryUnregistered(IDrxFactory* pFactory) = 0;

protected:
	virtual ~IDrxFactoryRegistryCallback() {}
};

struct IDrxFactoryRegistryImpl : public IDrxFactoryRegistry
{
	virtual IDrxFactory* GetFactory(const DrxClassID& cid) const = 0;
	virtual void         IterateFactories(const DrxInterfaceID& iid, IDrxFactory** pFactories, size_t& numFactories) const = 0;

	virtual void         RegisterCallback(IDrxFactoryRegistryCallback* pCallback) = 0;
	virtual void         UnregisterCallback(IDrxFactoryRegistryCallback* pCallback) = 0;

	virtual void         RegisterFactories(const SRegFactoryNode* pFactories) = 0;
	virtual void         UnregisterFactories(const SRegFactoryNode* pFactories) = 0;

	virtual void         UnregisterFactory(IDrxFactory* const pFactory) = 0;

protected:
	//! Prevent explicit destruction from client side (delete, shared_ptr, etc).
	virtual ~IDrxFactoryRegistryImpl() {}
};

#endif // #ifndef _IDRXFACTORYREGISTRYIMPL_H_