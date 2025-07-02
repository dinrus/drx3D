// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <limits>

#include <drx3D/CoreX/Containers/DrxArray.h>

class CChunkData
{
public:

	tukk GetData() const
	{
		return m_data.data();
	}

	i32 GetSize() const
	{
		return m_data.size();
	}

	template<class T>
	void Add(const T& object)
	{
		AddData(&object, sizeof(object));
	}

	void AddData(ukk pSrcData, i32 srcDataSize)
	{
		if (srcDataSize > 0)
		{
			const auto p = static_cast<tukk >(pSrcData);
			m_data.insert(m_data.end(), p, p + srcDataSize);
			assert(m_data.size() > 0 && m_data.size() <= (std::numeric_limits<i32>::max)());
		}
	}

private:

	DynArray<char, i32> m_data;
};
