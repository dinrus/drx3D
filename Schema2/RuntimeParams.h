// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Make sure we can allocate on stack! Can we use CScratchPad to do this?
// #SchematycTODO : Do we need to be able to pass IAny data?
// #SchematycTODO : If we're going to be using this in other dlls we need to replace std::vector!

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/Any.h>
#include <drx3D/Schema2/BasicTypes.h>

namespace sxema2
{
	class CRuntimeParams
	{
	private:

		struct SParamHeader
		{
			inline SParamHeader(u32 _id = s_invalidId, u32 _pos = s_invalidIdx)
				: id(_id)
				, pos(_pos)
			{}

			u32 id;
			u32 pos;
		};

		typedef std::vector<SParamHeader> ParamHeaders;
		typedef std::vector<IAnyPtr>      Data;

	public:

		inline CRuntimeParams() {}

		template <typename TYPE> inline bool Set(u32 paramId, const TYPE& value)
		{
			u32k paramIdx = ParamIdToIdx(paramId);
			if(ParamIdToIdx(paramId) != s_invalidIdx)
			{
				const IAnyPtr& pValue = m_data[m_paramHeaders[paramIdx].pos];
				if(pValue->GetTypeInfo() != GetTypeInfo<TYPE>())
				{
					// Error : Type mismatch!!!
					return false;
				}
				*static_cast<TYPE*>(pValue->ToVoidPtr()) = value;
				return true;
			}
			else if(paramId != s_invalidId)
			{
				u32k pos = m_data.size();
				m_paramHeaders.push_back(SParamHeader(paramId, pos));
				m_data.push_back(MakeAnyShared(value));
				return true;
			}
			return false;
		}

		template <typename TYPE> inline bool Get(u32 paramId, TYPE& value) const
		{
			u32k paramIdx = ParamIdToIdx(paramId);
			if(ParamIdToIdx(paramId) != s_invalidIdx)
			{
				const IAnyPtr& pValue = m_data[m_paramHeaders[paramIdx].pos];
				if(pValue->GetTypeInfo() != GetTypeInfo<TYPE>())
				{
					// Error : Type mismatch!!!
					return false;
				}
				value = *static_cast<const TYPE*>(pValue->ToVoidPtr());
				return true;
			}
			return false;
		}

	private:

		inline u32 ParamIdToIdx(u32 paramId) const
		{
			for(u32 paramIdx = 0, paramCount = m_paramHeaders.size(); paramIdx < paramCount; ++ paramIdx)
			{
				if(m_paramHeaders[paramIdx].id == paramId)
				{
					return paramIdx;
				}
			}
			return s_invalidIdx;
		}

	private:

		ParamHeaders m_paramHeaders;
		Data         m_data;
	};
}
