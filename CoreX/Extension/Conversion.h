// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   Conversion.h
//  Version:     v1.00
//  Created:     07/26/2011 by CarstenW
//  Описание: Part of DinrusX's extension framework.
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#ifndef _CONVERSION_H_
#define _CONVERSION_H_

#pragma once

namespace TC
{

//template <class T, class U>
//struct Conversion
//{
//private:
//	typedef char y[1];
//	typedef char n[2];
//	static y& Test(U);
//	static n& Test(...);
//	static T MakeT();

//public:
//	enum
//	{
//		exists = sizeof(Test(MakeT())) == sizeof(y),
//		sameType = false
//	};
//};

//template <class T>
//struct Conversion<T, T>
//{
//public:
//	enum
//	{
//		exists = true,
//		sameType = true
//	};
//};

//template<typename Base, typename Derived>
//struct CheckInheritance
//{
//	enum
//	{
//		exists = Conversion<const Derived*, const Base*>::exists && !Conversion<const Base*, ukk>::sameType
//	};
//};

//template<typename Base, typename Derived>
//struct CheckStrictInheritance
//{
//	enum
//	{
//		exists = CheckInheritance<Base, Derived>::exists && !Conversion<const Base*, const Derived*>::sameType
//	};
//};

template<typename Base, typename Derived>
struct SuperSubClass
{
private:
	typedef char y[1];
	typedef char n[2];

	template<typename T>
	static y& check(const  Derived &, T);
	static n& check(const  Base&, i32);

	struct C
	{
		operator const  Base&() const;
		operator const  Derived&();
	};

	static C getC();

public:
	enum
	{
		exists   = sizeof(check(getC(), 0)) == sizeof(y),
		sameType = false
	};
};

template<typename T>
struct SuperSubClass<T, T>
{
	enum
	{
		exists = true
	};
};

}

#endif // #ifndef _CONVERSION_H_
