// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _AISIGNAL_H_
#define _AISIGNAL_H_

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/AI/IAISystem.h>
#include <drx3D/Network/ISerialize.h>
#include <drx3D/CoreX/Memory/PoolAllocator.h>

struct AISignalExtraData : public IAISignalExtraData
{
public:
	static void CleanupPool();

public:
	AISignalExtraData();
	AISignalExtraData(const AISignalExtraData& other) : IAISignalExtraData(other), sObjectName() { SetObjectName(other.sObjectName); }
	virtual ~AISignalExtraData();

	AISignalExtraData& operator=(const AISignalExtraData& other);
	virtual void       Serialize(TSerialize ser);

	inline uk       operator new(size_t size)
	{
		return m_signalExtraDataAlloc.Allocate();
	}

	inline void operator delete(uk p)
	{
		return m_signalExtraDataAlloc.Deallocate(p);
	}

	virtual tukk GetObjectName() const { return sObjectName ? sObjectName : ""; }
	virtual void        SetObjectName(tukk objectName);

	// To/from script table
	virtual void ToScriptTable(SmartScriptTable& table) const;
	virtual void FromScriptTable(const SmartScriptTable& table);

private:
	tuk sObjectName;

	typedef stl::PoolAllocator<sizeof(IAISignalExtraData) + sizeof(uk ),
	                           stl::PoolAllocatorSynchronizationSinglethreaded> SignalExtraDataAlloc;
	static SignalExtraDataAlloc m_signalExtraDataAlloc;
};

#endif
