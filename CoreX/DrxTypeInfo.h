// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   TypeInfo.h
//  Version:     v1.00
//  Created:     03/05/2005 by Scott.
//  Описание: Declaration of CTypeInfo and related types.
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRX_TYPEINFO_H
#define __DRX_TYPEINFO_H
#pragma once

#include <drx3D/CoreX/Platform/platform.h>
#include <drx3D/CoreX/Containers/DrxArray.h>
#include <drx3D/CoreX/ParticleSys/Options.h>
#include "TypeInfo_decl.h"

class IDrxSizer;

class CDrxName;
string ToString(CDrxName const& val);
bool   FromString(CDrxName& val, tukk s);

//! Specify options for converting data to/from strings.
struct FToString
{
	OPT_STRUCT(FToString);
	OPT_VAR(bool, SkipDefault);   //!< Omit default values on writing.
	OPT_VAR(bool, NamedFields);   //!< Add Name= text to sub-values.
	OPT_VAR(bool, Sub);           //!< Write sub-structures (internal usage).
};

struct FFromString
{
	OPT_STRUCT(FFromString);
	OPT_VAR(bool, SkipEmpty);     //!< Do not set values from empty strings (otherwise, set to zero).
};

//! Specify which limits a variable has.
enum ENumericLimit
{
	eLimit_Min,
	eLimit_Max,
	eLimit_SoftMin,
	eLimit_SoftMax,
	eLimit_MinIsInfinite,
	eLimit_Step,
};

//! Type info base class, and default implementation.
struct CTypeInfo
{
	cstr   Name;
	size_t Size;
	size_t Alignment;

	CTypeInfo(cstr name, size_t size, size_t align)
		: Name(name), Size(size), Alignment(align) {}

	virtual ~CTypeInfo()
	{}

	//
	// Inheritance.
	//
	virtual bool IsType(CTypeInfo const& Info) const
	{ return this == &Info; }

	template<class T> bool IsType() const
	{ return IsType(TypeInfo((T*)0)); }

	//
	// Data access interface.
	//

	//! Convert value to string.
	virtual string ToString(ukk data, FToString flags = {}, ukk def_data = 0) const
	{ return ""; }

	//! Write value from string, return success.
	virtual bool FromString(uk data, cstr str, FFromString flags = {}) const
	{ return false; }

	//! Write values of a specified type.
	virtual bool ToValue(ukk data, uk value, const CTypeInfo& typeVal) const
	{ return false; }

	//! Read values of a specified type.
	virtual bool FromValue(uk data, ukk value, const CTypeInfo& typeVal) const
	{ return false; }

	// Templated interface to above functions.
	template<class T> bool ToValue(ukk data, T& value) const
	{ return ToValue(data, &value, TypeInfo(&value)); }
	template<class T> bool FromValue(uk data, const T& value) const
	{ return FromValue(data, &value, TypeInfo(&value)); }

	virtual bool ValueEqual(ukk data, ukk def_data = 0) const
	{ return ToString(data, FToString().SkipDefault(1), def_data).empty(); }

	virtual bool GetLimit(ENumericLimit eLimit, float& fVal) const
	{ return false; }

	//! Convert numeric formats from big-to-little endian or vice versa.
	//! Swaps bitfield order as well (which may be separate from integer bit order).
	virtual void SwapEndian(uk pData, size_t nCount, bool bWriting) const;

	//! Track memory used by any internal structures (not counting object size itself).
	//! Add to DrxSizer as needed, return remaining mem count.
	virtual void GetMemoryUsage(IDrxSizer* pSizer, ukk data) const
	{}

	//
	// Structure interface.
	//
	struct CVarInfo
	{
		const CTypeInfo& Type;                  //!< Info for type of variable.
		cstr             Name;                  //!< Display name of variable.
		cstr             Attrs;                 //!< Var-specific attribute string, of form:
		                                        //!< "<name=value>" for each attr, concatenated.
		                                        //!< Remaining text considered as comment.
		u32 Offset;                          //!< Offset in bytes from struct start.
		u32 ArrayDim     : 22,               //!< Number of array elements, or bits if bitfield.
		       bBaseClass   : 1,                //!< Sub-var is actually a base class.
		       bBitfield    : 1,                //!< Var is a bitfield, ArrayDim is number of bits.
		       BitOffset    : 6,                //!< Additional offset in bits for bitfields.
		                                        //!< Bit offset is computed in declaration order; on some platforms, it goes high to low.
		       BitWordWidth : 2;                //!< Width of bitfield = 1 byte << BitWordWidth

		// Accessors.
		cstr   GetName() const     { return Name; }
		size_t GetDim() const      { return bBitfield ? 1 : ArrayDim; }
		size_t GetSize() const     { return bBitfield ? (size_t)1 << BitWordWidth : Type.Size* ArrayDim; }
		size_t GetElemSize() const { return bBitfield ? (size_t)1 << BitWordWidth : Type.Size; }
		size_t GetBits() const     { return bBitfield ? ArrayDim : ArrayDim* Type.Size* 8; }
		bool   IsBaseClass() const { return bBaseClass; }
		bool   IsInline() const
		{
			const CVarInfo* pFirst;
			return bBaseClass && Offset == 0 && (pFirst = Type.NextSubVar(0)) && pFirst->IsBaseClass();
		}

		bool GetLimit(ENumericLimit eLimit, float& fVal) const
		{
			return Type.GetLimit(eLimit, fVal);
		}

		// Useful functions.
		uk GetAddress(uk base) const
		{
			return (tuk)base + Offset;
		}
		ukk GetAddress(ukk base) const
		{
			return (tukk)base + Offset;
		}
		bool FromString(uk base, cstr str, FFromString flags = {}) const
		{
			assert(!bBitfield);
			return Type.FromString((tuk)base + Offset, str, flags);
		}
		string ToString(ukk base, FToString flags = {}, ukk def_base = 0) const
		{
			assert(!bBitfield);
			return Type.ToString((tukk)base + Offset, flags, def_base ? (tukk)def_base + Offset : 0);
		}

		// Attribute access. Not fast.
		bool GetAttr(cstr name) const;
		bool GetAttr(cstr name, float& val) const;
		bool GetAttr(cstr name, string& val) const;

		//! Comment, excluding attributes.
		cstr GetComment() const;
	};

	//! Structure var iteration.
	virtual CVarInfo const* NextSubVar(CVarInfo const* pPrev, bool bRecurseBase = false) const
	{ return 0; }
	inline bool             HasSubVars() const
	{ return NextSubVar(0) != 0; }
#define ForAllSubVars(pVar, Info) \
  for (const CTypeInfo::CVarInfo* pVar = 0; pVar = (Info).NextSubVar(pVar); )

	//! Named var search.
	virtual const CVarInfo* FindSubVar(cstr name) const
	{ return 0; }

	virtual CTypeInfo const* const* NextTemplateType(CTypeInfo const* const* pPrev) const
	{ return 0; }
	inline bool                     IsTemplate() const
	{ return NextTemplateType(0) != 0; }

	//! String enumeration interface.
	//! String/i32 conversion is handled by ToString/FromString.
	//! \return sequential strings in enumeration, then 0 when out of range.
	virtual cstr EnumElem(uint nIndex) const { return 0; }
};

#endif // __DRX_TYPEINFO_H
