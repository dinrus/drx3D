// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace Drx
{
	namespace Memory
	{
		//! Получить смещение переменной-члена
		template<typename TYPE, typename MEMBER_TYPE> inline ptrdiff_t GetMemberOffset(MEMBER_TYPE TYPE::* pMember)
		{
			return reinterpret_cast<u8*>(&(reinterpret_cast<TYPE*>(1)->*pMember)) - reinterpret_cast<u8*>(1);
		}

		// Получить смещение базовой структуры / класса
		template<typename TYPE, typename BASE_TYPE> inline ptrdiff_t GetBaseOffset()
		{
			return reinterpret_cast<u8*>(static_cast<BASE_TYPE*>(reinterpret_cast<TYPE*>(1))) - reinterpret_cast<u8*>(1);
		}
	}
}