// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Containers/VectorMap.h>

#include <drx3D/Schema/FundamentalTypes.h>
#include <drx3D/Schema/IObject.h>

namespace sxema
{
// Forward declare classes.
class CObject;
// Forward declare shared pointers.
DECLARE_SHARED_POINTERS(CObject)

class CObjectPool
{
private:

	struct SSlot
	{
		ObjectId   objectId;
		CObjectPtr pObject;
	};

	typedef std::vector<SSlot>  Slots;
	typedef std::vector<u32> FreeSlots;

public:

	bool     CreateObject(const sxema::SObjectParams& params, IObject*& pObjectOut);
	IObject* GetObject(ObjectId objectId);
	void     DestroyObject(ObjectId objectId);

	void     SendSignal(ObjectId objectId, const SObjectSignal& signal);
	void     BroadcastSignal(const SObjectSignal& signal);

private:

	bool     AllocateSlots(u32 slotCount);
	ObjectId CreateObjectId(u32 slotIdx, u32 salt) const;
	bool     ExpandObjectId(ObjectId objectId, u32& slotIdx, u32& salt) const;
	u32   IncrementSalt(u32 salt) const;

private:

	Slots     m_slots;
	FreeSlots m_freeSlots;
};
} // sxema
