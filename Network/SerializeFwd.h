// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//
//	File: SerializeFwd.h
//  Описание: forward declaration of TSerialize
//
//	История:
//	-12/05/2005: Kirill Bulatsev, Created
//
//////////////////////////////////////////////////////////////////////

template<class T> class CSerializeWrapper;
struct ISerialize;
typedef CSerializeWrapper<ISerialize> TSerialize;
