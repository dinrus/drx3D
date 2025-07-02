// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/FundamentalTypes.h>
#include <drx3D/Schema/TypeDesc.h>
#include <drx3D/Schema/Any.h>
#include <drx3D/Schema/HybridArray.h>
#include <drx3D/Schema/UniqueId.h>

namespace sxema
{

class CRuntimeParamMap
{
public:

	struct SInput
	{
		SInput(const CUniqueId& _id, const CAnyConstPtr& _pValue);

		CUniqueId    id;
		CAnyConstPtr pValue;
	};

	struct SOutput
	{
		SOutput(const CUniqueId& _id, const CAnyPtr& _pValue);

		CUniqueId id;
		CAnyPtr   pValue;
	};

public:

	CRuntimeParamMap();
	CRuntimeParamMap(const CRuntimeParamMap& rhs);
	CRuntimeParamMap(const SInPlaceStorageParams& inputStorage, const SInPlaceStorageParams& outputStorage);
	CRuntimeParamMap(const SInPlaceStorageParams& inputStorage, const SInPlaceStorageParams& outputStorage, const CRuntimeParamMap& rhs);

	void                             ReserveInputs(u32 capacity);
	bool                             BindInput(const CUniqueId& id, const CAnyConstPtr& pValue);
	CAnyConstPtr                     GetInput(const CUniqueId& id) const;
	bool                             GetInput(const CUniqueId& id, const CAnyRef& value) const;
	template <typename VISITOR> void VisitInputs(VISITOR visitor) const;

	void                             ReserveOutputs(u32 capacity);
	bool                             BindOutput(const CUniqueId& id, const CAnyPtr& pValue);
	bool                             SetOutput(const CUniqueId& id, const CAnyConstRef& value) const;
	CAnyPtr                          GetOutput(const CUniqueId& id);
	CAnyConstPtr                     GetOutput(const CUniqueId& id) const;
	bool                             GetOutput(const CUniqueId& id, const CAnyRef& value) const;
	template <typename VISITOR> void VisitOutputs(VISITOR visitor);
	template <typename VISITOR> void VisitOutputs(VISITOR visitor) const;

	CRuntimeParamMap& operator=(const CRuntimeParamMap& rhs);

private:

	CHybridArray<SInput>  m_inputs;
	CHybridArray<SOutput> m_outputs;
};

template<u32 INPUT_CAPACITY, u32 OUTPUT_CAPACITY> class CInPlaceRuntimeParamMap : public CRuntimeParamMap
{
public:

	inline CInPlaceRuntimeParamMap()
		: CRuntimeParamMap(SInPlaceStorageParams(INPUT_CAPACITY, m_inputStorage), SInPlaceStorageParams(OUTPUT_CAPACITY, m_outputStorage))
	{}

	inline CInPlaceRuntimeParamMap(const CRuntimeParamMap& rhs)
		: CRuntimeParamMap(SInPlaceStorageParams(INPUT_CAPACITY, m_inputStorage), SInPlaceStorageParams(OUTPUT_CAPACITY, m_outputStorage), rhs)
	{}

	inline CInPlaceRuntimeParamMap(const CInPlaceRuntimeParamMap& rhs)
		: CRuntimeParamMap(SInPlaceStorageParams(INPUT_CAPACITY, m_inputStorage), SInPlaceStorageParams(OUTPUT_CAPACITY, m_outputStorage), rhs)
	{}

	template <u32 RHS_INPUT_CAPACITY, u32 RHS_OUTPUT_CAPACITY> inline CInPlaceRuntimeParamMap(const CInPlaceRuntimeParamMap<RHS_INPUT_CAPACITY, RHS_OUTPUT_CAPACITY>& rhs)
		: CRuntimeParamMap(SInPlaceStorageParams(INPUT_CAPACITY, m_inputStorage), SInPlaceStorageParams(OUTPUT_CAPACITY, m_outputStorage), rhs)
	{}

private:

	u8 m_inputStorage[INPUT_CAPACITY];
	u8 m_outputStorage[OUTPUT_CAPACITY];
};

typedef CRuntimeParamMap HeapRuntimeParamMap;
#if SXEMA_HASH_UNIQUE_IDS
typedef CInPlaceRuntimeParamMap<128, 128> StackRuntimeParamMap;
#else
typedef CInPlaceRuntimeParamMap<256, 256> StackRuntimeParamMap;
#endif

namespace RuntimeParamMap
{

bool FromInputClass(const CClassDesc& typeDesc, CRuntimeParamMap& params, ukk pInput);
template<typename TYPE> bool FromInputClass(CRuntimeParamMap& params, const TYPE& input);

bool ToInputClass(const CClassDesc& typeDesc, uk pOutput, const CRuntimeParamMap& params);
template<typename TYPE> inline bool ToInputClass(TYPE& output, const CRuntimeParamMap& params);

} // RuntimeParamMap
} // sxema

#include <drx3D/Schema/RuntimeParamMap.inl>
