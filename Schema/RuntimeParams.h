// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/FundamentalTypes.h>
#include <drx3D/Schema/RuntimeParamMap.h>
#include <drx3D/Schema/Any.h>
#include <drx3D/Schema/Scratchpad.h>
#include <drx3D/Schema/UniqueId.h>

namespace sxema
{

class CRuntimeParams : public CRuntimeParamMap
{
public:

	CRuntimeParams();
	CRuntimeParams(const CRuntimeParams& rhs);
	explicit CRuntimeParams(const CRuntimeParamMap& rhs);
	CRuntimeParams(const SInPlaceStorageParams& inputStorage, const SInPlaceStorageParams& outputStorage, const SInPlaceStorageParams& scratchpadStorage);
	CRuntimeParams(const SInPlaceStorageParams& inputStorage, const SInPlaceStorageParams& outputStorage, const SInPlaceStorageParams& scratchpadStorage, const CRuntimeParams& rhs);
	explicit CRuntimeParams(const SInPlaceStorageParams& inputStorage, const SInPlaceStorageParams& outputStorage, const SInPlaceStorageParams& scratchpadStorage, const CRuntimeParamMap& rhs);

	void ReserveScratchpad(u32 capacity);

	bool AddInput(const CUniqueId& id, const CAnyConstRef& value);
	bool AddOutput(const CUniqueId& id, const CAnyConstRef& value);

	CRuntimeParams& operator=(const CRuntimeParams& rhs);
	CRuntimeParams& operator=(const CRuntimeParamMap& rhs);

private:

	CScratchpad m_scratchpad;
};

template<u32 INPUT_CAPACITY, u32 OUTPUT_CAPACITY, u32 SCRATCHPAD_CAPACITY> class CInPlaceRuntimeParams : public CRuntimeParams
{
public:

	inline CInPlaceRuntimeParams()
		: CRuntimeParams(SInPlaceStorageParams(INPUT_CAPACITY, m_inputStorage), SInPlaceStorageParams(OUTPUT_CAPACITY, m_outputStorage), SInPlaceStorageParams(SCRATCHPAD_CAPACITY, m_scratchpadStorage))
	{}

	inline CInPlaceRuntimeParams(const CRuntimeParams& rhs)
		: CRuntimeParams(SInPlaceStorageParams(INPUT_CAPACITY, m_inputStorage), SInPlaceStorageParams(OUTPUT_CAPACITY, m_outputStorage), SInPlaceStorageParams(SCRATCHPAD_CAPACITY, m_scratchpadStorage), rhs)
	{}

	explicit inline CInPlaceRuntimeParams(const CRuntimeParamMap& rhs)
		: CRuntimeParams(SInPlaceStorageParams(INPUT_CAPACITY, m_inputStorage), SInPlaceStorageParams(OUTPUT_CAPACITY, m_outputStorage), SInPlaceStorageParams(SCRATCHPAD_CAPACITY, m_scratchpadStorage), rhs)
	{}

	inline CInPlaceRuntimeParams(const CInPlaceRuntimeParams& rhs)
		: CRuntimeParams(SInPlaceStorageParams(INPUT_CAPACITY, m_inputStorage), SInPlaceStorageParams(OUTPUT_CAPACITY, m_outputStorage), SInPlaceStorageParams(SCRATCHPAD_CAPACITY, m_scratchpadStorage), rhs)
	{}

	template <u32 RHS_INPUT_CAPACITY, u32 RHS_OUTPUT_CAPACITY, u32 RHS_SCRATCHPAD_CAPACITY> inline CInPlaceRuntimeParams(const CInPlaceRuntimeParams<RHS_INPUT_CAPACITY, RHS_OUTPUT_CAPACITY, RHS_SCRATCHPAD_CAPACITY>& rhs)
		: CRuntimeParams(SInPlaceStorageParams(INPUT_CAPACITY, m_inputStorage), SInPlaceStorageParams(OUTPUT_CAPACITY, m_outputStorage), SInPlaceStorageParams(SCRATCHPAD_CAPACITY, m_scratchpadStorage), rhs)
	{}

private:

	u8 m_inputStorage[INPUT_CAPACITY];
	u8 m_outputStorage[OUTPUT_CAPACITY];
	u8 m_scratchpadStorage[SCRATCHPAD_CAPACITY];
};

typedef CRuntimeParams HeapRuntimeParams;
#if SXEMA_HASH_UNIQUE_IDS
typedef CInPlaceRuntimeParams<64, 64, 256> StackRuntimeParams;
#else
typedef CInPlaceRuntimeParams<256, 256, 256> StackRuntimeParams;
#endif

namespace RuntimeParams
{

bool FromInputClass(const CClassDesc& typeDesc, CRuntimeParams& params, ukk pInput);
template<typename TYPE> inline bool FromInputClass(CRuntimeParams& params, const TYPE& input);

bool ToInputClass(const CClassDesc& typeDesc, uk pOutput, const CRuntimeParams& params);
template<typename TYPE> inline bool ToInputClass(TYPE& output, const CRuntimeParams& params);

} // RuntimeParams
} // sxema

#include <drx3D/Schema/RuntimeParams.inl>
