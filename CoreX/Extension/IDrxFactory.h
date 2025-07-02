// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "DrxTypeID.h"

struct IDrxUnknown;
struct SRegFactoryNode;

typedef std::shared_ptr<IDrxUnknown> IDrxUnknownPtr;
typedef std::shared_ptr<const IDrxUnknown> IDrxUnknownConstPtr;

struct IDrxFactory
{
	virtual tukk            GetName() const = 0;
	virtual const DrxClassID&      GetClassID() const = 0;
	virtual bool                   ClassSupports(const DrxInterfaceID& iid) const = 0;
	virtual void                   ClassSupports(const DrxInterfaceID*& pIIDs, size_t& numIIDs) const = 0;
	virtual IDrxUnknownPtr         CreateClassInstance() const = 0;

protected:
	//! Prevent explicit destruction from client side (delete, shared_ptr, etc).
	virtual ~IDrxFactory() = default;
};
