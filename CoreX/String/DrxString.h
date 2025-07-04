// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#if !defined(NOT_USE_DRX_STRING)

	#include "DrxStringUtils.h" // drx_vsprintf(), DrxStringUtils_Internal::compute_length_formatted_va

	#include <string.h>
	#include <wchar.h>
	#include <limits.h>
	#include <stdarg.h>
	#include <ctype.h>

	#define DRX_STRING

// forward declaration of DrxStackString
template<class T, size_t S> class DrxStackStringT;

class CConstCharWrapper;  //forward declaration for special tukk  without memory allocations

//extern void DrxDebugStr( tukk format,... );
//#define DRX_STRING_DEBUG(s) { if (*s) DrxDebugStr( "[%6d] %s",_usedMemory(0),(s) );}
	#define DRX_STRING_DEBUG(s)

//! Reference-tracked string implementation, historically used to avoid string duplication across memory
//! Enforced for use in the engine codebase as opposed to STxt
//! \see string and wstring
template<class T>
class DrxStringT
{
public:
	//////////////////////////////////////////////////////////////////////////
	// Types compatible with STL string.
	//////////////////////////////////////////////////////////////////////////
	typedef DrxStringT<T>                         _Self;
	typedef size_t                                size_type;
	typedef T                                     value_type;
	typedef const value_type*                     const_str;
	typedef value_type*                           pointer;
	typedef const value_type*                     const_pointer;
	typedef value_type&                           reference;
	typedef const value_type&                     const_reference;
	typedef pointer                               iterator;
	typedef const_pointer                         const_iterator;
	typedef std::reverse_iterator<iterator>       reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	enum _npos_type : size_type
	{
		npos = (size_type) ~0
	};

	//////////////////////////////////////////////////////////////////////////
	// Constructors
	//////////////////////////////////////////////////////////////////////////
	DrxStringT();

protected:
	DrxStringT(const CConstCharWrapper& str); //!< Constructor for strings without memory allocations.
	friend class CConstCharWrapper;
public:

	DrxStringT(const _Self& str);
	DrxStringT(const _Self& str, size_type nOff, size_type nCount);

	// Before September 2012 this constructor looked like this:
	//    explicit DrxStringT( value_type ch, size_type nRepeat = 1 );
	// It was very error-prone, because the matching constructor
	// STxt constructor is different:
	//    STxt( size_type nRepeat, value_type ch );
	//
	// To prevent hard-to-catch bugs, we removed all calls of this
	// constructor in the existing DinrusX code (in September 2012)
	// and started using proper order of parameters (matching std::).
	//
	// To catch calls using the reversed arguments in other projects,
	// we retain the previous function with reversed arguments,
	// and declare it private.
	DrxStringT(size_type nRepeat, value_type ch);
private:
	DrxStringT(value_type ch, size_type nRepeat);

public:

	DrxStringT(const_str str);
	DrxStringT(const_str str, size_type nLength);
	DrxStringT(const_iterator _First, const_iterator _Last);
	~DrxStringT();

	//////////////////////////////////////////////////////////////////////////
	// STL string like interface.
	//////////////////////////////////////////////////////////////////////////
	//! Operators.
	size_type length() const;
	size_type size() const;
	bool      empty() const;
	void      clear(); //!< Frees the data.

	//! Returns the storage currently allocated to hold the string, a value at least as large as length().
	size_type capacity() const;

	//! Sets the capacity of the string to a number at least as great as a specified number.
	//! nCount = 0 is shrinking string to fit number of characters in it (equivalent to shrink_to_fit()).
	void                   reserve(size_type nCount = 0);
	void                   shrink_to_fit();

	_Self&                 append(const value_type* _Ptr);
	_Self&                 append(const value_type* _Ptr, size_type nCount);
	_Self&                 append(const _Self& _Str, size_type nOff, size_type nCount);
	_Self&                 append(const _Self& _Str);
	_Self&                 append(size_type nCount, value_type _Ch);
	_Self&                 append(const_iterator _First, const_iterator _Last);

	_Self&                 assign(const_str _Ptr);
	_Self&                 assign(const_str _Ptr, size_type nCount);
	_Self&                 assign(const _Self& _Str, size_type off, size_type nCount);
	_Self&                 assign(const _Self& _Str);
	_Self&                 assign(size_type nCount, value_type _Ch);
	_Self&                 assign(const_iterator _First, const_iterator _Last);

	value_type             at(size_type index) const;

	const_reference        front() const   { return *begin(); }
	const_reference        back() const    { return *(end() - 1); }

	const_iterator         begin() const   { return m_str; }
	const_iterator         cbegin() const  { return begin(); }
	const_iterator         end() const     { return m_str + length(); }
	const_iterator         cend() const    { return end(); }

	const_reverse_iterator rbegin() const  { return const_reverse_iterator(end()); }
	const_reverse_iterator crbegin() const { return rbegin(); }
	const_reverse_iterator rend() const    { return const_reverse_iterator(begin()); }
	const_reverse_iterator crend() const   { return rend(); }

	// Following functions are commented out because they provide direct write access to
	// the string and such access doesn't work properly with our current reference-count
	// implementation.
	// If you really need write access to your string's elements, please consider
	// using DrxStackStringT<> instead of DrxString<>. Alternatively, you can modify
	// your string by multiple calls of erase() and append().
	// Note: If you need *linear memory read* access to your string's elements, use data()
	// or c_str(). If you need *linear memory write* access to your string's elements,
	// use a non-string class (std::vector<>, DynArray<>, etc.) instead of DrxString<>.
	//value_type& at(size_type index);
	//reference front() { return *begin(); }
	//reference back() { return *(end() - 1); }
	//iterator begin() { return m_str; }
	//iterator end() { return m_str + length(); }
	//reverse_iterator rbegin() { return reverse_iterator(end()); }
	//reverse_iterator rend() { return reverse_iterator(begin()); }

	//! cast to C string operator.
	operator const_str() const { return m_str; }

	//! cast to C string.
	const value_type* c_str() const { return m_str; }
	const value_type* data() const  { return m_str; }

	//////////////////////////////////////////////////////////////////////////
	// string comparison.
	//////////////////////////////////////////////////////////////////////////
	i32 compare(const _Self& _Str) const;
	i32 compare(size_type _Pos1, size_type _Num1, const _Self& _Str) const;
	i32 compare(size_type _Pos1, size_type _Num1, const _Self& _Str, size_type nOff, size_type nCount) const;
	i32 compare(tukk _Ptr) const;
	i32 compare(const wchar_t* _Ptr) const;
	i32 compare(size_type _Pos1, size_type _Num1, const value_type* _Ptr, size_type _Num2 = npos) const;

	// Case insensitive comparison
	i32 compareNoCase(const _Self& _Str) const;
	i32 compareNoCase(size_type _Pos1, size_type _Num1, const _Self& _Str) const;
	i32 compareNoCase(size_type _Pos1, size_type _Num1, const _Self& _Str, size_type nOff, size_type nCount) const;
	i32 compareNoCase(const value_type* _Ptr) const;
	i32 compareNoCase(size_type _Pos1, size_type _Num1, const value_type* _Ptr, size_type _Num2 = npos) const;

	//! Copies at most a specified number of characters from an indexed position in a source string to a target character array.
	size_type copy(value_type* _Ptr, size_type nCount, size_type nOff = 0) const;

	void      push_back(value_type _Ch) { _ConcatenateInPlace(&_Ch, 1); }
	void      pop_back()                { erase(size() - 1, 1); }
	void      resize(size_type nCount, value_type _Ch = ' ');

	//! Simple sub-string extraction.
	_Self substr(size_type pos, size_type count = npos) const;

	// Replace part of string.
	_Self& replace(value_type chOld, value_type chNew);
	_Self& replace(const_str strOld, const_str strNew);
	_Self& replace(size_type pos, size_type count, const_str strNew);
	_Self& replace(size_type pos, size_type count, const_str strNew, size_type count2);
	_Self& replace(size_type pos, size_type count, size_type nNumChars, value_type chNew);

	// Insert new elements to string.
	_Self& insert(size_type nIndex, value_type ch);
	_Self& insert(size_type nIndex, size_type nCount, value_type ch);
	_Self& insert(size_type nIndex, const_str pstr);
	_Self& insert(size_type nIndex, const_str pstr, size_type nCount);

	//! Delete count characters starting at zero-based index.
	_Self& erase(size_type nIndex, size_type count = npos);

	//! Searching (return starting index, or -1 if not found).
	//! Look for a single character match (like "C" strchr).
	size_type find(value_type ch, size_type pos = 0) const;

	// look for a specific sub-string, (like "C" strstr).
	size_type find(const_str subs, size_type pos = 0) const;
	size_type rfind(value_type ch, size_type pos = npos) const;
	size_type rfind(const _Self& subs, size_type pos = 0) const;

	size_type find_first_of(value_type _Ch, size_type nOff = 0) const;
	size_type find_first_of(const_str charSet, size_type nOff = 0) const;
	//size_type find_first_of( const value_type* _Ptr,size_type _Off,size_type _Count ) const;
	size_type find_first_of(const _Self& _Str, size_type _Off = 0) const;

	size_type find_first_not_of(value_type _Ch, size_type _Off = 0) const;
	size_type find_first_not_of(const value_type* _Ptr, size_type _Off = 0) const;
	size_type find_first_not_of(const value_type* _Ptr, size_type _Off, size_type _Count) const;
	size_type find_first_not_of(const _Self& _Str, size_type _Off = 0) const;

	size_type find_last_of(value_type _Ch, size_type _Off = npos) const;
	size_type find_last_of(const value_type* _Ptr, size_type _Off = npos) const;
	size_type find_last_of(const value_type* _Ptr, size_type _Off, size_type _Count) const;
	size_type find_last_of(const _Self& _Str, size_type _Off = npos) const;

	size_type find_last_not_of(value_type _Ch, size_type _Off = npos) const;
	size_type find_last_not_of(const value_type* _Ptr, size_type _Off = npos) const;
	size_type find_last_not_of(const value_type* _Ptr, size_type _Off, size_type _Count) const;
	size_type find_last_not_of(const _Self& _Str, size_type _Off = npos) const;

	void      swap(_Self& _Str);

	//////////////////////////////////////////////////////////////////////////
	// overloaded operators.
	//////////////////////////////////////////////////////////////////////////
	// overloaded indexing.
	//	const_reference operator[](size_type pos) const { DRX_ASSERT(pos < length()); return m_str[pos]; }

	// Following functions are commented out because they provide direct write access to
	// the string and such access doesn't work properly with our current reference-count
	// implementation.
	//reference operator[](size_type pos) { DRX_ASSERT(pos < length()); return m_str[pos]; }

	// overloaded assignment
	_Self& operator=(const _Self& str);
	_Self& operator=(value_type ch);
	_Self& operator=(const_str str);

	template<size_t AnySize> DrxStringT(const DrxStackStringT<T, AnySize>& str);

protected:
	//! We prohibit an implicit conversion from DrxStackString to make user aware of allocation!
	//! -> use string(stackedString) instead.
	template<size_t AnySize> _Self& operator=(const DrxStackStringT<T, AnySize>& str) 
	{
		// we add a compile-time error as the Visual C++ compiler seems to ignore the private statement?
		static_assert(AnySize != AnySize, "Use_Explicit_String_Assignment_When_Assigning_From_StackString");
		// not reached, as above will generate a compile time error
		_Assign(str.c_str(), str.length());
		return *this;
	}

public:
	// string concatenation
	_Self& operator+=(const _Self& str);
	_Self& operator+=(value_type ch);
	_Self& operator+=(const_str str);

	//template <class TT> friend DrxStringT<TT> operator+( const DrxStringT<TT>& str1, const DrxStringT<TT>& str2 );
	//template <class TT> friend DrxStringT<TT> operator+( const DrxStringT<TT>& str, value_type ch );
	//template <class TT> friend DrxStringT<TT> operator+( value_type ch, const DrxStringT<TT>& str );
	//template <class TT> friend DrxStringT<TT> operator+( const DrxStringT<TT>& str1, const_str str2 );
	//template <class TT> friend DrxStringT<TT> operator+( const_str str1, const DrxStringT<TT>& str2 );

	size_t GetAllocatedMemory() const
	{
		StrHeader* header = _header();
		if (header == _emptyHeader())
			return 0;
		return sizeof(StrHeader) + (header->nAllocSize + 1) * sizeof(value_type);
	}

	//////////////////////////////////////////////////////////////////////////
	// Extended functions.
	// These functions are not in the STL string.
	// They have an ATL CString interface.
	//////////////////////////////////////////////////////////////////////////
	//! Format string printf-like.
	//! This function exists in 'char' version of the class only: it's
	//! almost impossible to implement it robustly for wchar_t - different
	//! compilers handle wchar_t formatting differently.
	_Self& Format(const value_type* format, ...);

	//! Format string printf-like. va_list version.
	//! This function exists in 'char' version of the class only: it's
	//! almost impossible to implement it robustly for wchar_t - different
	//! compilers handle wchar_t formatting differently.
	_Self& FormatV(const value_type* format, va_list args);

	//! Converts the string to lower-case.
	//! This function uses the "C" locale for case-conversion (ie, A-Z only).
	_Self& MakeLower();

	//! Converts the string to upper-case.
	//! This function uses the "C" locale for case-conversion (ie, A-Z only).
	_Self& MakeUpper();

	_Self& Trim();
	_Self& Trim(value_type ch);
	_Self& Trim(const_str sCharSet);

	_Self& TrimLeft();
	_Self& TrimLeft(value_type ch);
	_Self& TrimLeft(const_str sCharSet);

	_Self& TrimRight();
	_Self& TrimRight(value_type ch);
	_Self& TrimRight(const_str sCharSet);
	void Truncate(size_type nNewLength) { resize(nNewLength); }

	_Self  SpanIncluding(const_str charSet) const;
	_Self  SpanExcluding(const_str charSet) const;
	_Self  Tokenize(const_str charSet, i32& nStart) const;
	_Self  Mid(size_type nFirst, size_type nCount = npos) const { return substr(nFirst, nCount); }

	const_str GetString() const { return c_str(); }
	const_str GetBuffer() const { return c_str(); }
	i32 GetLength() const { return (i32)length(); }
	bool IsEmpty() const { return empty(); }
	void Empty() { clear(); }

	_Self& Insert(size_type nIndex, value_type ch) { return insert(nIndex, ch); }
	_Self& Insert(size_type nIndex, const_str charSet) { return insert(nIndex, charSet); }
	_Self& Insert(size_type nIndex, const _Self& charSet) { return insert(nIndex, charSet); }

	i32 Find(value_type ch, size_type pos = 0) const { return (i32)find(ch, pos); }
	i32 Find(const_str subs, size_type pos = 0) const { return (i32)find(subs, pos); }
	i32 ReverseFind(value_type ch, size_type pos = npos) const { return (i32)rfind(ch, pos); }
	i32 ReverseFind(const _Self& subs, size_type pos = 0) const { return (i32)rfind(subs, pos); }

	i32 FindOneOf(const_str chars) const
	{
		i32 pos = -1;
		while (*chars)
		{
			pos = Find(*chars);
			if (pos != -1)
				break;

			++chars;
		}
		return pos;
	}

	_Self& Replace(value_type chOld, value_type chNew) { return replace(chOld, chNew); }
	_Self& Replace(const_str strOld, const_str strNew) { return replace(strOld, strNew); }

	i32 CompareNoCase(const _Self& _Str) const { return compareNoCase(_Str); }
	i32 CompareNoCase(const value_type* _Ptr) const { return compareNoCase(_Ptr); }
	i32 Compare(const _Self& _Str) const { return compare(_Str); }
	i32 Compare(tukk _Ptr) const { return compare(_Ptr); }

	_Self& Append(const value_type* _Ptr) { return append(_Ptr); }
	_Self& Append(const _Self& _Str) { return append(_Str); }
	_Self& Append(const value_type* _Ptr, size_type nCount) { return append(_Ptr, nCount); }

	_Self& AppendFormat(const value_type* pszFormat, ...)
	{
		_Self formated = *this;
		va_list argList;
		va_start(argList, pszFormat);
		FormatV(pszFormat, argList);
		va_end(argList);
		formated.append(*this);
		return *this = formated;
	}

	void Preallocate(size_type nLength) { reserve(nLength); }
	_Self Delete(size_type nIndex, size_type nCount = 1) { return erase(nIndex, nCount); }

	value_type GetAt(size_type pos) const { return at(pos); }
	void SetAt(size_type pos, value_type ch) { replace(pos, 1, 1, ch); }

	_Self  Left(size_type count) const;
	_Self  Right(size_type count) const;
	//////////////////////////////////////////////////////////////////////////

	// public utilities.
	static size_type   _strlen(const_str str);
	static size_type   _strnlen(const_str str, size_type maxLen);
	static const_str   _strchr(const_str str, value_type c);
	static const_str   _strrchr(const_str str, value_type c);
	static value_type* _strstr(value_type* str, const_str strSearch);
	static bool        _IsValidString(const_str str);

public:
	//////////////////////////////////////////////////////////////////////////
	//! Only used for debugging statistics.
	//////////////////////////////////////////////////////////////////////////
	static size_t _usedMemory(ptrdiff_t size)
	{
		static size_t s_used_memory = 0;
		s_used_memory += size;
		return s_used_memory;
	}

protected:
	value_type* m_str; //!< Pointer to ref counted string data.

	//! String header. Actual string data starts immediately after this header in memory.
	//! \cond INTERNAL
	struct StrHeader
	{

		i32             nRefCount;
		i32             nLength;
		i32             nAllocSize; //!< Size of memory allocated at the end of this class.

		value_type*     GetChars() { return (value_type*)(this + 1); }
		void            AddRef()   { nRefCount++; /*InterlockedIncrement(&_header()->nRefCount);*/ }
		i32             Release()  { return --nRefCount; }
	};
	//! \endcond

	static StrHeader* _emptyHeader()
	{
		// Define 2 static buffers in a row. The 2nd is a dummy object to hold a single empty char string.
		static StrHeader sEmptyStringBuffer[2] = { { -1, 0, 0 }, { 0, 0, 0 } };
		return &sEmptyStringBuffer[0];
	}

	// implementation helpers
	StrHeader*  _header() const;

	void        _AllocData(size_type nLen);
	static void _FreeData(StrHeader* pData);
	void        _Free();
	void        _Initialize();

	void        _Concatenate(const_str sStr1, size_type nLen1, const_str sStr2, size_type nLen2);
	void        _ConcatenateInPlace(const_str sStr, size_type nLen);
	void        _Assign(const_str sStr, size_type nLen);
	void        _MakeUnique();

	static void _copy(value_type* dest, const value_type* src, size_type count);
	static void _move(value_type* dest, const value_type* src, size_type count);
	static void _set(value_type* dest, value_type ch, size_type count);
};

//! \cond INTERNAL
//! Wrapper class for creation of strings without memory allocation.
//! It creates a string with pointer pointing to tukk location.
//! Destructor sets the string to empty.
//! \note Never copy a string from it, just use it as function parameters instead of tukk itself.
class CConstCharWrapper
{
public:
	// Passing *this is safe since the char pointer is already set and therefore is the this-ptr constructed complete enough.
//#pragma warning (push)
//#pragma warning (disable : 4355) //'this' : used in base member initializer list
	CConstCharWrapper(tukk const cpString) : cpChar(cpString), str(*this){ assert(cpString); }//!< Create stack string.
//#pragma warning (pop)
	~CConstCharWrapper(){ str.m_str = DrxStringT<char>::_emptyHeader()->GetChars(); }  //!< Reset string.
	operator const DrxStringT<char> &() const { return str; }                          //!< Cast operator to const string reference.
private:
	tukk const cpChar;
	DrxStringT<char>  str;

	tuk GetCharPointer() const { return const_cast<tuk>(cpChar); }  //!< Access function for string constructor.

	friend class DrxStringT<char>;  //!< Both are bidirectional friends to avoid any other accesses.
};
//! \endcond

//! Macro needed because compiler somehow cannot find the cast operator when not invoked directly.
	#define CONST_TEMP_STRING(a) ((const string&)CConstCharWrapper(a))

/////////////////////////////////////////////////////////////////////////////
// DrxStringT<T> Implementation
//////////////////////////////////////////////////////////////////////////

template<class T>
inline typename DrxStringT<T>::StrHeader * DrxStringT<T>::_header() const
{
	assert(m_str != NULL);
	return ((StrHeader*)m_str) - 1;
}

template<class T>
inline typename DrxStringT<T>::size_type DrxStringT<T >::_strlen(const_str str)
{
	return (str == NULL) ? 0 : (size_type)::strlen(str);
}

template<>
inline DrxStringT<wchar_t>::size_type DrxStringT<wchar_t >::_strlen(const_str str)
{
	return (str == NULL) ? 0 : (size_type)::wcslen(str);
}

template<class T>
inline typename DrxStringT<T>::size_type DrxStringT<T >::_strnlen(const_str str, size_type maxLen)
{
	size_type len = 0;
	if (str)
	{
		while (len < maxLen && *str != '\0')
		{
			len++;
			str++;
		}
	}
	return len;
}

template<class T>
inline typename DrxStringT<T>::const_str DrxStringT<T >::_strchr(const_str str, value_type c)
{
	return (str == NULL) ? 0 : ::strchr(str, c);
}

template<>
inline DrxStringT<wchar_t>::const_str DrxStringT<wchar_t >::_strchr(const_str str, value_type c)
{
	return (str == NULL) ? 0 : ::wcschr(str, c);
}

template<class T>
inline typename DrxStringT<T>::const_str DrxStringT<T >::_strrchr(const_str str, value_type c)
{
	return (str == NULL) ? 0 : ::strrchr(str, c);
}

template<>
inline DrxStringT<wchar_t>::const_str DrxStringT<wchar_t >::_strrchr(const_str str, value_type c)
{
	return (str == NULL) ? 0 : ::wcsrchr(str, c);
}

template<class T>
inline typename DrxStringT<T>::value_type * DrxStringT<T>::_strstr(value_type * str, const_str strSearch)
{
	return (str == NULL) ? 0 : (value_type*)::strstr(str, strSearch);
}

template<>
inline DrxStringT<wchar_t>::value_type * DrxStringT<wchar_t>::_strstr(value_type * str, const_str strSearch)
{
	return (str == NULL) ? 0 : ::wcsstr(str, strSearch);
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline void DrxStringT<T >::_copy(value_type* dest, const value_type* src, size_type count)
{
	if (dest != src)
		memcpy(dest, src, count * sizeof(value_type));
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline void DrxStringT<T >::_move(value_type* dest, const value_type* src, size_type count)
{
	memmove(dest, src, count * sizeof(value_type));
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline void DrxStringT<T >::_set(value_type* dest, value_type ch, size_type count)
{
	static_assert(sizeof(value_type) == sizeof(T), "Invalid type size!");
	static_assert(sizeof(value_type) == 1, "Invalid type size!");
	memset(dest, ch, count);
}

//////////////////////////////////////////////////////////////////////////
template<>
inline void DrxStringT<wchar_t >::_set(value_type* dest, value_type ch, size_type count)
{
	static_assert(sizeof(value_type) == sizeof(wchar_t), "Invalid type size!");
	wmemset(dest, ch, count);
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline bool DrxStringT<T >::_IsValidString(const_str)
{
	/*
	   if (str == NULL)
	   return false;
	   i32 nLength = _strlen(str);
	   return !::IsBadStringPtrA(str, nLength);
	 */
	return true;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline void DrxStringT<T >::_Assign(const_str sStr, size_type nLen)
{
	// Check if this string is shared (reference count greater then 1) or not enough capacity to store new string.
	// Then allocate new string buffer.
	if (_header()->nRefCount > 1 || nLen > capacity())
	{
		_Free();
		_AllocData(nLen);
	}
	// Copy characters from new string to this buffer.
	_copy(m_str, sStr, nLen);
	// Set new length.
	check_convert(_header()->nLength) = nLen;
	// Make null terminated string.
	m_str[nLen] = 0;
	DRX_STRING_DEBUG(m_str)
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline void DrxStringT<T >::_Concatenate(const_str sStr1, size_type nLen1, const_str sStr2, size_type nLen2)
{
	size_type nLen = nLen1 + nLen2;

	if (nLen1 * 2 > nLen)
		nLen = nLen1 * 2;

	if (nLen != 0)
	{
		if (nLen < 8)
			nLen = 8;

		_AllocData(nLen);
		_copy(m_str, sStr1, nLen1);
		_copy(m_str + nLen1, sStr2, nLen2);
		check_convert(_header()->nLength) = nLen1 + nLen2;
		m_str[nLen1 + nLen2] = 0;
	}
	DRX_STRING_DEBUG(m_str)
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline void DrxStringT<T >::_ConcatenateInPlace(const_str sStr, size_type nLen)
{
	if (nLen != 0)
	{
		// Check if this string is shared (reference count greater then 1) or not enough capacity to store new string.
		// Then allocate new string buffer.
		if (_header()->nRefCount > 1 || length() + nLen > capacity())
		{
			StrHeader* pOldData = _header();
			_Concatenate(m_str, length(), sStr, nLen);
			_FreeData(pOldData);
		}
		else
		{
			_copy(m_str + length(), sStr, nLen);
			check_convert(_header()->nLength) = _header()->nLength + nLen;
			m_str[_header()->nLength] = 0; // Make null terminated string.
		}
	}
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline void DrxStringT<T >::_MakeUnique()
{
	if (_header()->nRefCount > 1)
	{
		// If string is shared, make a copy of string buffer.
		StrHeader* pOldData = _header();
		// This will not free header because reference count is greater then 1.
		_Free();
		// Allocate a new string buffer.
		_AllocData(pOldData->nLength);
		// Full copy of null terminated string.
		_copy(m_str, pOldData->GetChars(), pOldData->nLength + 1);
		DRX_STRING_DEBUG(m_str)
	}
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline void DrxStringT<T >::_Initialize()
{
	m_str = _emptyHeader()->GetChars();
}

//! Always allocate one extra character for '\0' termination.
//! Assumes [optimistically] that data length will equal allocation length.
template<class T>
inline void DrxStringT<T >::_AllocData(size_type nLen)
{
	assert(nLen >= 0);
	assert(nLen <= INT_MAX - 1);    // max size (enough room for 1 extra)

	if (nLen == 0)
		_Initialize();
	else
	{
		size_type allocLen = sizeof(StrHeader) + (nLen + 1) * sizeof(value_type);

		StrHeader* pData = (StrHeader*)DrxModuleMalloc(allocLen);

		_usedMemory(allocLen);   // For statistics.
		pData->nRefCount = 1;
		m_str = pData->GetChars();
		check_convert(pData->nLength) = nLen;
		check_convert(pData->nAllocSize) = nLen;
		m_str[nLen] = 0; // null terminated string.
	}
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline void DrxStringT<T >::_Free()
{
	if (_header()->nRefCount >= 0) // Not empty string.
	{
		_FreeData(_header());
		_Initialize();
	}
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline void DrxStringT<T >::_FreeData(StrHeader* pData)
{
	if (pData->nRefCount >= 0) // Not empty string.
	{
		assert(pData->nRefCount != 0);
		if (pData->Release() <= 0)
		{
			size_t allocLen = sizeof(StrHeader) + (pData->nAllocSize + 1) * sizeof(value_type);
			_usedMemory(-check_cast<i32>(allocLen));   // For statistics.

			DrxModuleFree((uk )pData);
			//i32 allocLen = sizeof(StrHeader) + (pData->nAllocSize+1)*sizeof(value_type);
			//string_alloc::deallocate( (value_type*)pData,allocLen );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>::DrxStringT()
{
	_Initialize();
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>::DrxStringT(const DrxStringT<T>& str)
{
	assert(str._header()->nRefCount != 0);
	if (str._header()->nRefCount >= 0)
	{
		m_str = str.m_str;
		_header()->AddRef();
	}
	else
	{
		_Initialize();
	}
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>::DrxStringT(const DrxStringT<T>& str, size_type nOff, size_type nCount)
{
	_Initialize();
	assign(str, nOff, nCount);
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>::DrxStringT(const_str str)
{
	_Initialize();
	// Make a copy of C string.
	size_type nLen = _strlen(str);
	if (nLen != 0)
	{
		_AllocData(nLen);
		_copy(m_str, str, nLen);
		DRX_STRING_DEBUG(m_str)
	}
}

template<class T>
inline DrxStringT<T>::DrxStringT(const CConstCharWrapper& str)
{
	_Initialize();
	m_str = const_cast<pointer>(str.GetCharPointer());
}

template<class T>
template<size_t AnySize>
inline DrxStringT<T>::DrxStringT(const DrxStackStringT<T, AnySize>& str)
{
	_Initialize();
	const size_type nLength = str.length();
	if (nLength > 0)
	{
		_AllocData(nLength);
		_copy(m_str, str, nLength);
		DRX_STRING_DEBUG(m_str)
	}
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>::DrxStringT(const_str str, size_type nLength)
{
	_Initialize();
	if (nLength > 0)
	{
		_AllocData(nLength);
		_copy(m_str, str, nLength);
		DRX_STRING_DEBUG(m_str)
	}
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>::DrxStringT(size_type nRepeat, value_type ch)
{
	_Initialize();
	if (nRepeat > 0)
	{
		_AllocData(nRepeat);
		_set(m_str, ch, nRepeat);
		DRX_STRING_DEBUG(m_str)
	}
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>::DrxStringT(const_iterator _First, const_iterator _Last)
{
	_Initialize();
	size_type nLength = (size_type)(_Last - _First);
	if (nLength > 0)
	{
		_AllocData(nLength);
		_copy(m_str, _First, nLength);
		DRX_STRING_DEBUG(m_str)
	}
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>::~DrxStringT()
{
	_FreeData(_header());
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline typename DrxStringT<T>::size_type DrxStringT<T >::length() const
{
	return _header()->nLength;
}
template<class T>
inline typename DrxStringT<T>::size_type DrxStringT<T >::size() const
{
	return _header()->nLength;
}
template<class T>
inline typename DrxStringT<T>::size_type DrxStringT<T >::capacity() const
{
	return _header()->nAllocSize;
}

template<class T>
inline bool DrxStringT<T >::empty() const
{
	return length() == 0;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline void DrxStringT<T >::clear()
{
	if (length() == 0)
		return;
	if (_header()->nRefCount >= 0)
		_Free();
	else
		resize(0);
	assert(length() == 0);
	assert(_header()->nRefCount < 0 || capacity() == 0);
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline void DrxStringT<T >::reserve(size_type nCount)
{
	// Reserve of 0 is shrinking container to fit number of characters in it..
	if (nCount > capacity())
	{
		StrHeader* pOldData = _header();
		_AllocData(nCount);
		_copy(m_str, pOldData->GetChars(), pOldData->nLength);
		_header()->nLength = pOldData->nLength;
		m_str[pOldData->nLength] = 0;
		_FreeData(pOldData);
	}
	else if (nCount == 0)
	{
		shrink_to_fit();
	}
	DRX_STRING_DEBUG(m_str)
}

template<class T>
inline void DrxStringT<T >::shrink_to_fit()
{
	if (length() < capacity())
	{
		StrHeader* const pOldData = _header();
		_AllocData(length());
		_copy(m_str, pOldData->GetChars(), pOldData->nLength);
		_FreeData(pOldData);
	}
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::append(const_str _Ptr)
{
	*this += _Ptr;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::append(const_str _Ptr, size_type nCount)
{
	_ConcatenateInPlace(_Ptr, nCount);
	return *this;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::append(const DrxStringT<T>& _Str, size_type off, size_type nCount)
{
	size_type len = _Str.length();
	if (off > len)
		return *this;
	if (off + nCount > len)
		nCount = len - off;
	_ConcatenateInPlace(_Str.m_str + off, nCount);
	return *this;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::append(const DrxStringT<T>& _Str)
{
	*this += _Str;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::append(size_type nCount, value_type _Ch)
{
	if (nCount > 0)
	{
		if (_header()->nRefCount > 1 || length() + nCount > capacity())
		{
			StrHeader* pOldData = _header();
			_AllocData(length() + nCount);
			_copy(m_str, pOldData->GetChars(), pOldData->nLength);
			_set(m_str + pOldData->nLength, _Ch, nCount);
			_FreeData(pOldData);
		}
		else
		{
			size_type nOldLength = length();
			_set(m_str + nOldLength, _Ch, nCount);
			check_convert(_header()->nLength) = nOldLength + nCount;
			m_str[length()] = 0; // Make null terminated string.
		}
	}
	DRX_STRING_DEBUG(m_str)
	return *this;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::append(const_iterator _First, const_iterator _Last)
{
	append(_First, (size_type)(_Last - _First));
	return *this;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::assign(const_str _Ptr)
{
	*this = _Ptr;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::assign(const_str _Ptr, size_type nCount)
{
	size_type len = _strnlen(_Ptr, nCount);
	_Assign(_Ptr, len);
	return *this;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::assign(const DrxStringT<T>& _Str, size_type off, size_type nCount)
{
	size_type len = _Str.length();
	if (off > len)
		return *this;
	if (off + nCount > len)
		nCount = len - off;
	_Assign(_Str.m_str + off, nCount);
	return *this;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::assign(const DrxStringT<T>& _Str)
{
	*this = _Str;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::assign(size_type nCount, value_type _Ch)
{
	if (nCount >= 1)
	{
		_AllocData(nCount);
		_set(m_str, _Ch, nCount);
		DRX_STRING_DEBUG(m_str)
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::assign(const_iterator _First, const_iterator _Last)
{
	assign(_First, (size_type)(_Last - _First));
	return *this;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline typename DrxStringT<T>::value_type DrxStringT<T >::at(size_type index) const
{
	assert(index >= 0 && index < length());
	return m_str[index];
}

/*
   inline value_type& DrxStringT<T>::at( size_type index )
   {
   // same as GetAt
   assert( index >= 0 && index < length() );
   return m_str[index];
   }
 */

/*
   inline DrxStringT<T>::value_type DrxStringT<T>::operator[]( size_type index ) const
   {
   assert( index >= 0 && index < length() );
   return m_str[index];
   }
 */

/*
   inline value_type& DrxStringT<T>::operator[]( size_type index )
   {
   // same as GetAt
   assert( index >= 0 && index < length() );
   return m_str[index];
   }
 */

//////////////////////////////////////////////////////////////////////////
template<class T>
inline i32 DrxStringT<T >::compare(const DrxStringT<T>& _Str) const
{
	return compare(_Str.m_str);
}

template<class T>
inline i32 DrxStringT<T >::compare(size_type _Pos1, size_type _Num1, const DrxStringT<T>& _Str) const
{
	return compare(_Pos1, _Num1, _Str.m_str, npos);
}

template<class T>
inline i32 DrxStringT<T >::compare(size_type _Pos1, size_type _Num1, const DrxStringT<T>& _Str, size_type nOff, size_type nCount) const
{
	assert(nOff < _Str.length());
	return compare(_Pos1, _Num1, _Str.m_str + nOff, nCount);
}

template<class T>
inline i32 DrxStringT<T >::compare(tukk _Ptr) const
{
	return strcmp(m_str, _Ptr);
}

template<class T>
inline i32 DrxStringT<T >::compare(const wchar_t* _Ptr) const
{
	return wcscmp(m_str, _Ptr);
}

template<class T>
inline i32 DrxStringT<T >::compare(size_type _Pos1, size_type _Num1, const value_type* _Ptr, size_type _Num2) const
{
	assert(_Pos1 < length());
	if (length() - _Pos1 < _Num1)
		_Num1 = length() - _Pos1; // trim to size

	i32 res = _Num1 == 0 ? 0 : strncmp(m_str + _Pos1, _Ptr, (_Num1 < _Num2) ? _Num1 : _Num2);
	return (res != 0 ? res : _Num2 == npos && _Ptr[_Num1] == 0 ? 0 : _Num1 < _Num2 ? -1 : _Num1 == _Num2 ? 0 : +1);
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline i32 DrxStringT<T >::compareNoCase(const DrxStringT<T>& _Str) const
{
	return stricmp(m_str, _Str.m_str);
}

template<class T>
inline i32 DrxStringT<T >::compareNoCase(size_type _Pos1, size_type _Num1, const DrxStringT<T>& _Str) const
{
	return compareNoCase(_Pos1, _Num1, _Str.m_str, npos);
}

template<class T>
inline i32 DrxStringT<T >::compareNoCase(size_type _Pos1, size_type _Num1, const DrxStringT<T>& _Str, size_type nOff, size_type nCount) const
{
	assert(nOff < _Str.length());
	return compareNoCase(_Pos1, _Num1, _Str.m_str + nOff, nCount);
}

template<class T>
inline i32 DrxStringT<T >::compareNoCase(const value_type* _Ptr) const
{
	return stricmp(m_str, _Ptr);
}

template<class T>
inline i32 DrxStringT<T >::compareNoCase(size_type _Pos1, size_type _Num1, const value_type* _Ptr, size_type _Num2) const
{
	assert(_Pos1 < length());
	if (length() - _Pos1 < _Num1)
		_Num1 = length() - _Pos1; // trim to size

	i32 res = _Num1 == 0 ? 0 : strnicmp(m_str + _Pos1, _Ptr, (_Num1 < _Num2) ? _Num1 : _Num2);
	return (res != 0 ? res : _Num2 == npos && _Ptr[_Num1] == 0 ? 0 : _Num1 < _Num2 ? -1 : _Num1 == _Num2 ? 0 : +1);
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline typename DrxStringT<T>::size_type DrxStringT<T >::copy(value_type* _Ptr, size_type nCount, size_type nOff) const
{
	assert(nOff < length());
	if (nOff + nCount > length()) // trim to offset.
		nCount = length() - nOff;

	_copy(_Ptr, m_str + nOff, nCount);
	return nCount;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline void DrxStringT<T >::resize(size_type nCount, value_type _Ch)
{
	_MakeUnique();
	if (nCount > length())
	{
		size_type numToAdd = nCount - length();
		append(numToAdd, _Ch);
	}
	else if (nCount < length())
	{
		_header()->nLength = nCount;
		m_str[length()] = 0; // Make null terminated string.

	}
}

//////////////////////////////////////////////////////////////////////////
//! compare helpers
template<class T> inline bool operator==(const DrxStringT<T>& s1, const DrxStringT<T>& s2)
{ return s1.compare(s2) == 0; }
template<class T> inline bool operator==(const DrxStringT<T>& s1, const typename DrxStringT<T>::value_type* s2)
{ return s1.compare(s2) == 0; }
template<class T> inline bool operator==(const typename DrxStringT<T>::value_type* s1, const DrxStringT<T>& s2)
{ return s2.compare(s1) == 0; }
template<class T> inline bool operator!=(const DrxStringT<T>& s1, const DrxStringT<T>& s2)
{ return s1.compare(s2) != 0; }
template<class T> inline bool operator!=(const DrxStringT<T>& s1, const typename DrxStringT<T>::value_type* s2)
{ return s1.compare(s2) != 0; }
template<class T> inline bool operator!=(const typename DrxStringT<T>::value_type* s1, const DrxStringT<T>& s2)
{ return s2.compare(s1) != 0; }
template<class T> inline bool operator<(const DrxStringT<T>& s1, const DrxStringT<T>& s2)
{ return s1.compare(s2) < 0; }
template<class T> inline bool operator<(const DrxStringT<T>& s1, const typename DrxStringT<T>::value_type* s2)
{ return s1.compare(s2) < 0; }
template<class T> inline bool operator<(const typename DrxStringT<T>::value_type* s1, const DrxStringT<T>& s2)
{ return s2.compare(s1) > 0; }
template<class T> inline bool operator>(const DrxStringT<T>& s1, const DrxStringT<T>& s2)
{ return s1.compare(s2) > 0; }
template<class T> inline bool operator>(const DrxStringT<T>& s1, const typename DrxStringT<T>::value_type* s2)
{ return s1.compare(s2) > 0; }
template<class T> inline bool operator>(const typename DrxStringT<T>::value_type* s1, const DrxStringT<T>& s2)
{ return s2.compare(s1) < 0; }
template<class T> inline bool operator<=(const DrxStringT<T>& s1, const DrxStringT<T>& s2)
{ return s1.compare(s2) <= 0; }
template<class T> inline bool operator<=(const DrxStringT<T>& s1, const typename DrxStringT<T>::value_type* s2)
{ return s1.compare(s2) <= 0; }
template<class T> inline bool operator<=(const typename DrxStringT<T>::value_type* s1, const DrxStringT<T>& s2)
{ return s2.compare(s1) >= 0; }
template<class T> inline bool operator>=(const DrxStringT<T>& s1, const DrxStringT<T>& s2)
{ return s1.compare(s2) >= 0; }
template<class T> inline bool operator>=(const DrxStringT<T>& s1, const typename DrxStringT<T>::value_type* s2)
{ return s1.compare(s2) >= 0; }
template<class T> inline bool operator>=(const typename DrxStringT<T>::value_type* s1, const DrxStringT<T>& s2)
{ return s2.compare(s1) <= 0; }

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::operator=(value_type ch)
{
	_Assign(&ch, 1);
	return *this;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T> operator+(const DrxStringT<T>& string1, typename DrxStringT<T>::value_type ch)
{
	DrxStringT<T> s(string1);
	s.append(1, ch);
	return s;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T> operator+(typename DrxStringT<T>::value_type ch, const DrxStringT<T>& str)
{
	DrxStringT<T> s;
	s.reserve(str.size() + 1);
	s.append(1, ch);
	s.append(str);
	return s;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T> operator+(const DrxStringT<T>& string1, const DrxStringT<T>& string2)
{
	DrxStringT<T> s(string1);
	s.append(string2);
	return s;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T> operator+(const DrxStringT<T>& str1, const typename DrxStringT<T>::value_type* str2)
{
	DrxStringT<T> s(str1);
	s.append(str2);
	return s;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T> operator+(const typename DrxStringT<T>::value_type* str1, const DrxStringT<T>& str2)
{
	assert(str1 == NULL || DrxStringT<T>::_IsValidString(str1));
	DrxStringT<T> s;
	s.reserve(DrxStringT<T>::_strlen(str1) + str2.size());
	s.append(str1);
	s.append(str2);
	return s;
}

template<class T>
inline DrxStringT<T>& DrxStringT<T >::operator=(const DrxStringT<T>& str)
{
	if (m_str != str.m_str)
	{
		if (_header()->nRefCount < 0)
		{
			// Empty string.
			//			_Assign( str.m_str,str.length() );
			if (str._header()->nRefCount < 0)
				; // two empty strings...
			else
			{
				m_str = str.m_str;
				_header()->AddRef();
			}
		}
		else if (str._header()->nRefCount < 0)
		{
			// If source string is empty.
			_Free();
			m_str = str.m_str;
		}
		else
		{
			// Copy string reference.
			_Free();
			m_str = str.m_str;
			_header()->AddRef();
		}
	}
	return *this;
}

template<class T>
inline DrxStringT<T>& DrxStringT<T >::operator=(const_str str)
{
	assert(str == NULL || _IsValidString(str));
	_Assign(str, _strlen(str));
	return *this;
}

template<class T>
inline DrxStringT<T>& DrxStringT<T >::operator+=(const_str str)
{
	assert(str == NULL || _IsValidString(str));
	_ConcatenateInPlace(str, _strlen(str));
	return *this;
}

template<class T>
inline DrxStringT<T>& DrxStringT<T >::operator+=(value_type ch)
{
	_ConcatenateInPlace(&ch, 1);
	return *this;
}

template<class T>
inline DrxStringT<T>& DrxStringT<T >::operator+=(const DrxStringT<T>& str)
{
	_ConcatenateInPlace(str.m_str, str.length());
	return *this;
}

//! Find first single character.
template<class T>
inline typename DrxStringT<T>::size_type DrxStringT<T >::find(value_type ch, size_type pos) const
{
	if (pos > length())
	{
		return (typename DrxStringT<T>::size_type)npos;
	}
	const_str str = _strchr(m_str + pos, ch);
	// return npos if not found and index otherwise
	return (str == NULL) ? npos : (size_type)(str - m_str);
}

//! Find a sub-string (like strstr).
template<class T>
inline typename DrxStringT<T>::size_type DrxStringT<T >::find(const_str subs, size_type pos) const
{
	assert(_IsValidString(subs));
	if (pos > length())
	{
		return npos;
	}

	// find first matching substring
	const_str str = _strstr(m_str + pos, subs);

	// return npos for not found, distance from beginning otherwise
	return (str == NULL) ? npos : (size_type)(str - m_str);

}

//! Find last single character.
// \return -1 if not found, distance from beginning otherwise.
template<class T>
inline typename DrxStringT<T>::size_type DrxStringT<T >::rfind(value_type ch, size_type pos) const
{
	const_str str;
	if (pos == npos)
	{
		// find last single character
		str = _strrchr(m_str, ch);
		// return -1 if not found, distance from beginning otherwise
		return (str == NULL) ? (size_type) - 1 : (size_type)(str - m_str);
	}
	else
	{
		if (pos > length())
		{
			return npos;
		}

		value_type tmp = m_str[pos + 1];
		m_str[pos + 1] = 0;
		str = _strrchr(m_str, ch);
		m_str[pos + 1] = tmp;
	}
	return (str == NULL) ? (size_type) - 1 : (size_type)(str - m_str);
}

template<class T>
inline typename DrxStringT<T>::size_type DrxStringT<T >::rfind(const DrxStringT<T>& subs, size_type pos) const
{
	size_type res = npos;
	for (i32 i = (i32)size(); i >= (i32)pos; --i)
	{
		size_type findRes = find(subs, i);
		if (findRes != npos)
		{
			res = findRes;
			break;
		}
	}
	return res;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline typename DrxStringT<T>::size_type DrxStringT<T >::find_first_of(const DrxStringT<T>& _Str, size_type _Off) const
{
	return find_first_of(_Str.m_str, _Off);
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline typename DrxStringT<T>::size_type DrxStringT<T >::find_first_of(value_type _Ch, size_type nOff) const
{
	if (nOff > length())
	{
		return npos;
	}
	value_type charSet[2] = { _Ch, 0 };
	const_str str = strpbrk(m_str + nOff, charSet);
	return (str == NULL) ? (size_type) - 1 : (size_type)(str - m_str);
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline typename DrxStringT<T>::size_type DrxStringT<T >::find_first_of(const_str charSet, size_type nOff) const
{
	assert(_IsValidString(charSet));
	if (nOff > length())
	{
		return npos;
	}
	const_str str = strpbrk(m_str + nOff, charSet);
	return (str == NULL) ? (size_type) - 1 : (size_type)(str - m_str);
}

template<>
inline DrxStringT<wchar_t>::size_type DrxStringT<wchar_t >::find_first_of(const_str charSet, size_type nOff) const
{
	assert(_IsValidString(charSet));
	if (nOff > length())
	{
		return npos;
	}
	const_str str = wcspbrk(m_str + nOff, charSet);
	return (str == NULL) ? (size_type) - 1 : (size_type)(str - m_str);
}

//size_type find_first_not_of(const _Self& __s, size_type __pos = 0) const
//{ return find_first_not_of(__s._M_start, __pos, __s.size()); }

//size_type find_first_not_of(const _CharT* __s, size_type __pos = 0) const
//{ _STLP_FIX_LITERAL_BUG(__s) return find_first_not_of(__s, __pos, _Traits::length(__s)); }

//size_type find_first_not_of(const _CharT* __s, size_type __pos,	size_type __n) const;

//size_type find_first_not_of(_CharT __c, size_type __pos = 0) const;

//////////////////////////////////////////////////////////////////////////
template<class T>
inline typename DrxStringT<T>::size_type DrxStringT<T >::find_first_not_of(const value_type* _Ptr, size_type _Off) const
{ return find_first_not_of(_Ptr, _Off, _strlen(_Ptr)); }

//////////////////////////////////////////////////////////////////////////
template<class T>
inline typename DrxStringT<T>::size_type DrxStringT<T >::find_first_not_of(const DrxStringT<T>& _Str, size_type _Off) const
{ return find_first_not_of(_Str.m_str, _Off); }

//////////////////////////////////////////////////////////////////////////
template<class T>
inline typename DrxStringT<T>::size_type DrxStringT<T >::find_first_not_of(value_type _Ch, size_type _Off) const
{
	if (_Off > length())
		return npos;
	else
	{
		for (const value_type* str = begin() + _Off; str != end(); ++str)
			if (*str != _Ch)
				return size_type(str - begin());
		// Character found!
		return npos;
	}
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline typename DrxStringT<T>::size_type DrxStringT<T >::find_first_not_of(const value_type* _Ptr, size_type _Off, size_type _Count) const
{
	if (_Off > length())
		return npos;
	else
	{
		const value_type* charsFirst = _Ptr, * charsLast = _Ptr + _Count;
		for (const value_type* str = begin() + _Off; str != end(); ++str)
		{
			const value_type* c;
			for (c = charsFirst; c != charsLast; ++c)
			{
				if (*c == *str)
					break;
			}
			if (c == charsLast)
				return size_type(str - begin());// Current character not in char set.
		}
		return npos;
	}
}
//////////////////////////////////////////////////////////////////////////
template<class T>
inline typename DrxStringT<T>::size_type DrxStringT<T >::find_last_of(value_type _Ch, size_type _Off) const
{
	size_type nLenght(length());
	// Empty strings, always return npos (same semantic as STxt).
	if (nLenght == 0)
	{
		return npos;
	}

	// If the offset is is bigger than the size of the string
	if (_Off >= nLenght)
	{
		// We set it to the size of the string -1, so we will not do bad pointer operations nor we will
		// test the null terminating character.
		_Off = nLenght - 1;
	}

	// From the character at the offset position, going to to the direction of the first character.
	for (const value_type* str = begin() + _Off; true; --str)
	{
		// We found a character in the string which matches the input character.
		if (*str == _Ch)
		{
			return size_type(str - begin()); // Character found!
		}
		// If the next element will be begin()-1, then we should stop.
		if (str == begin())
		{
			break;
		}
	}

	// We found nothing.
	return npos;
}
//////////////////////////////////////////////////////////////////////////
//! This function is actually a convenience alias.
template<class T>
inline typename DrxStringT<T>::size_type DrxStringT<T >::find_last_of(const value_type* _Ptr, size_type _Off) const
{
	// BTW: what will happen if wchar_t is used here?
	return find_last_of(_Ptr, _Off, _strlen(_Ptr));
}
//////////////////////////////////////////////////////////////////////////
template<class T>
inline typename DrxStringT<T>::size_type DrxStringT<T >::find_last_of(const value_type* _Ptr, size_type _Off, size_type _Count) const
{
	size_type nLenght(length());
	// Empty strings, always return npos (same semantic as STxt).
	if (nLenght == 0)
	{
		return npos;
	}

	// If the offset is is bigger than the size of the string
	if (_Off >= nLenght)
	{
		// We set it to the size of the string -1, so we will not do bad pointer operations nor we will
		// test the null terminating character.
		_Off = nLenght - 1;
	}

	// From the character at the offset position, going to to the direction of the first character.
	const value_type* charsFirst = _Ptr, * charsLast = _Ptr + _Count;
	for (const value_type* str = begin() + _Off; true; --str)
	{
		const value_type* c;
		// For every character in the character set.
		for (c = charsFirst; c != charsLast; ++c)
		{
			// If the current character matches any of the charcaters in the input string...
			if (*c == *str)
			{
				// This is the value we must return.
				return size_type(str - begin());
			}
		}

		// If the next element will be begin()-1, then we should stop.
		if (str == begin())
		{
			break;
		}
	}
	// We couldn't find any character of the input string in the current string.
	return npos;
}
/////////////////////////////////////////////////////////////////////////////
template<class T>
inline typename DrxStringT<T>::size_type DrxStringT<T >::find_last_of(const _Self& strCharSet, size_type _Off) const
{
	size_type nLenght(length());
	// Empty strings, always return npos (same semantic as STxt).
	if (nLenght == 0)
	{
		return npos;
	}

	// If the offset is is bigger than the size of the string
	if (_Off >= nLenght)
	{
		// We set it to the size of the string -1, so we will not do bad pointer operations nor we will
		// test the null terminating character.
		_Off = nLenght - 1;
	}

	// From the character at the offset position, going to to the direction of the first character.
	for (const value_type* str = begin() + _Off; true; --str)
	{
		// We check every character of the input string.
		for (const value_type* strInputCharacter = strCharSet.begin(); strInputCharacter != strCharSet.end(); ++strInputCharacter)
		{
			// If any character matches.
			if (*str == *strInputCharacter)
			{
				// We return the position where we found it.
				return size_type(str - begin()); // Character found!
			}
		}
		// If the next element will be begin()-1, then we should stop.
		if (str == begin())
		{
			break;
		}
	}

	// As we couldn't find any matching character...we return the appropriate value.
	return npos;
}
//////////////////////////////////////////////////////////////////////////
template<class T>
inline typename DrxStringT<T>::size_type DrxStringT<T >::find_last_not_of(value_type _Ch, size_type _Off) const
{
	size_type nLenght(length());
	// Empty strings, always return npos (same semantic as STxt).
	if (nLenght == 0)
	{
		return npos;
	}

	// If the offset is is bigger than the size of the string
	if (_Off >= nLenght)
	{
		// We set it to the size of the string -1, so we will not do bad pointer operations nor we will
		// test the null terminating character.
		_Off = nLenght - 1;
	}

	// From the character at the offset position, going to to the direction of the first character.
	for (const value_type* str = begin() + _Off; true; --str)
	{
		// If the current character being analyzed is different of the input character.
		if (*str != _Ch)
		{
			// We found the last item which is not the input character before the given offset.
			return size_type(str - begin()); // Character found!
		}
		// If the next element will be begin()-1, then we should stop.
		if (str == begin())
		{
			break;
		}
	}

	// As we couldn't find any matching character...we return the appropriate value.
	return npos;
}
//////////////////////////////////////////////////////////////////////////
//! This function is actually a convenience alias.
template<class T>
inline typename DrxStringT<T>::size_type DrxStringT<T >::find_last_not_of(const value_type* _Ptr, size_type _Off) const
{
	// BTW: what will happen if wchar_t is used here?
	return find_last_not_of(_Ptr, _Off, _strlen(_Ptr));
}
//////////////////////////////////////////////////////////////////////////
template<class T>
inline typename DrxStringT<T>::size_type DrxStringT<T >::find_last_not_of(const value_type* _Ptr, size_type _Off, size_type _Count) const
{
	size_type nLenght(length());
	// Empty strings, always return npos (same semantic as STxt).
	if (nLenght == 0)
	{
		return npos;
	}

	// If the offset is is bigger than the size of the string
	if (_Off >= nLenght)
	{
		// We set it to the size of the string -1, so we will not do bad pointer operations nor we will
		// test the null terminating character.
		_Off = nLenght - 1;
	}

	// From the character at the offset position, going to to the direction of the first character.
	const value_type* charsFirst = _Ptr, * charsLast = _Ptr + _Count;
	for (const value_type* str = begin() + _Off; true; --str)
	{
		bool boFoundAny(false);
		const value_type* c;
		// For every character in the character set.
		for (c = charsFirst; c != charsLast; ++c)
		{
			// If the current character matches any of the charcaters in the input string...
			if (*c == *str)
			{
				// So we signal it was found and stop this search.
				boFoundAny = true;
				break;
			}
		}

		// Using a different solution of the other similar methods
		// to make it easier to read.
		// If the character being analyzed is not in the set...
		if (!boFoundAny)
		{
			//.. we return the position where we found it.
			return size_type(str - begin());
		}

		// If the next element will be begin()-1, then we should stop.
		if (str == begin())
		{
			break;
		}
	}
	// We couldn't find any character of the input string not in the character set.
	return npos;

}
//////////////////////////////////////////////////////////////////////////
template<class T>
inline typename DrxStringT<T>::size_type DrxStringT<T >::find_last_not_of(const _Self& _Str, size_type _Off) const
{
	size_type nLenght(length());
	// Empty strings, always return npos (same semantic as STxt).
	if (nLenght == 0)
	{
		return npos;
	}

	// If the offset is is bigger than the size of the string
	if (_Off >= nLenght)
	{
		// We set it to the size of the string -1, so we will not do bad pointer operations nor we will
		// test the null terminating character.
		_Off = nLenght - 1;
	}

	// From the character at the offset position, going to to the direction of the first character.
	for (const value_type* str = begin() + _Off; true; --str)
	{
		bool boFoundAny(false);
		for (const value_type* strInputCharacter = _Str.begin(); strInputCharacter != _Str.end(); ++strInputCharacter)
		{
			// The character matched one of the character set...
			if (*strInputCharacter == *str)
			{
				// So we signal it was found and stop this search.
				boFoundAny = true;
				break;
			}
		}

		// Using a different solution of the other similar methods
		// to make it easier to read.
		// If the character being analyzed is not in the set...
		if (!boFoundAny)
		{
			//.. we return the position where we found it.
			return size_type(str - begin());
		}

		// If the next element will be begin()-1, then we should stop.
		if (str == begin())
		{
			break;
		}
	}

	// As we couldn't find any matching character...we return the appropriate value.
	return npos;
}
//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T> DrxStringT<T >::substr(size_type pos, size_type count) const
{
	if (pos >= length())
	{
		return DrxStringT<T>();
	}
	if (count == npos)
	{
		count = length() - pos;
	}
	if (pos + count > length())
	{
		count = length() - pos;
	}
	return DrxStringT<T>(m_str + pos, count);
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::erase(size_type nIndex, size_type nCount)
{
	if (nCount > length() - nIndex)
	{
		nCount = length() - nIndex;
	}
	if (nIndex < length())
	{
		_MakeUnique();
		size_type nNumToCopy = length() - (nIndex + nCount) + 1;
		_move(m_str + nIndex, m_str + nIndex + nCount, nNumToCopy);
		_header()->nLength = length() - nCount;
	}

	return *this;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::insert(size_type nIndex, value_type ch)
{
	return insert(nIndex, 1, ch);
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::insert(size_type nIndex, size_type nCount, value_type ch)
{
	_MakeUnique();

	size_type nNewLength = length();
	if (nIndex > nNewLength)
		nIndex = nNewLength;
	nNewLength += nCount;

	if (capacity() < nNewLength)
	{
		StrHeader* pOldData = _header();
		const_str pstr = m_str;
		_AllocData(nNewLength);
		_copy(m_str, pstr, pOldData->nLength + 1);
		_FreeData(pOldData);
	}

	_move(m_str + nIndex + nCount, m_str + nIndex, (nNewLength - nIndex - nCount) + 1);
	_set(m_str + nIndex, ch, nCount);
	_header()->nLength = nNewLength;
	DRX_STRING_DEBUG(m_str)

	return *this;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::insert(size_type nIndex, const_str pstr, size_type nCount)
{
	const size_type nInsertLength = nCount;
	size_type nNewLength = length();
	if (nInsertLength > 0)
	{
		_MakeUnique();
		if (nIndex > nNewLength)
			nIndex = nNewLength;
		nNewLength += nInsertLength;

		if (capacity() < nNewLength)
		{
			StrHeader* pOldData = _header();
			const_str pOldStr = m_str;
			_AllocData(nNewLength);
			_copy(m_str, pOldStr, (pOldData->nLength + 1));
			_FreeData(pOldData);
		}

		_move(m_str + nIndex + nInsertLength, m_str + nIndex, (nNewLength - nIndex - nInsertLength + 1));
		_copy(m_str + nIndex, pstr, nInsertLength);
		_header()->nLength = nNewLength;
		m_str[length()] = 0;
	}

	DRX_STRING_DEBUG(m_str)

	return *this;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::insert(size_type nIndex, const_str pstr)
{
	return insert(nIndex, pstr, _strlen(pstr));
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::replace(size_type pos, size_type count, const_str strNew)
{
	return replace(pos, count, strNew, _strlen(strNew));
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::replace(size_type pos, size_type count, const_str strNew, size_type count2)
{
	erase(pos, count);
	insert(pos, strNew, count2);
	return *this;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::replace(size_type pos, size_type count, size_type nNumChars, value_type chNew)
{
	erase(pos, count);
	insert(pos, nNumChars, chNew);
	return *this;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::replace(value_type chOld, value_type chNew)
{
	if (chOld != chNew)
	{
		_MakeUnique();
		value_type* strend = m_str + length();
		for (value_type* str = m_str; str != strend; ++str)
		{
			if (*str == chOld)
			{
				*str = chNew;
			}
		}
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::replace(const_str strOld, const_str strNew)
{
	size_type nSourceLen = _strlen(strOld);
	if (nSourceLen == 0)
		return *this;
	size_type nReplacementLen = _strlen(strNew);

	size_type nCount = 0;
	value_type* strStart = m_str;
	value_type* strEnd = m_str + length();
	value_type* strTarget;
	while (strStart < strEnd)
	{
		while ((strTarget = _strstr(strStart, strOld)) != NULL)
		{
			nCount++;
			strStart = strTarget + nSourceLen;
		}
		strStart += _strlen(strStart) + 1;
	}

	if (nCount > 0)
	{
		_MakeUnique();

		size_type nOldLength = length();
		size_type nNewLength = nOldLength + (nReplacementLen - nSourceLen) * nCount;
		if (capacity() < nNewLength || _header()->nRefCount > 1)
		{
			StrHeader* pOldData = _header();
			const_str pstr = m_str;
			_AllocData(nNewLength);
			_copy(m_str, pstr, pOldData->nLength);
			_FreeData(pOldData);
		}
		strStart = m_str;
		strEnd = m_str + length();

		while (strStart < strEnd)
		{
			while ((strTarget = _strstr(strStart, strOld)) != NULL)
			{
				size_type nBalance = nOldLength - ((size_type)(strTarget - m_str) + nSourceLen);
				_move(strTarget + nReplacementLen, strTarget + nSourceLen, nBalance);
				_copy(strTarget, strNew, nReplacementLen);
				strStart = strTarget + nReplacementLen;
				strStart[nBalance] = 0;
				nOldLength += (nReplacementLen - nSourceLen);
			}
			strStart += _strlen(strStart) + 1;
		}
		_header()->nLength = nNewLength;
	}
	DRX_STRING_DEBUG(m_str)

	return *this;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline void DrxStringT<T >::swap(DrxStringT<T>& _Str)
{
	value_type* temp = _Str.m_str;
	_Str.m_str = m_str;
	m_str = temp;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::FormatV(const T* format, va_list args)
{
	static_assert(std::is_same<T, char>::value, "This function supports 'char' only");
	assert(_IsValidString(format));

	#if DRX_COMPILER_MSVC && DRX_COMPILER_VERSION <= 1700
	// Visual Studio 2012 and older don't have va_copy()
	i32k n = DrxStringUtils_Internal::compute_length_formatted_va(format, args);
	#else
	va_list argsCopy;
	va_copy(argsCopy, args);
	i32k n = DrxStringUtils_Internal::compute_length_formatted_va(format, argsCopy);
	va_end(argsCopy);
	#endif

	if (n < 0)
	{
		clear();
	}
	else
	{
		resize(n);
		drx_vsprintf(m_str, n + 1, format, args);
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::Format(const T* format, ...)
{
	static_assert(std::is_same<T, char>::value, "This function supports 'char' only");
	va_list args;
	va_start(args, format);
	FormatV(format, args);
	va_end(args);
	return *this;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::MakeLower()
{
	_MakeUnique();
	for (value_type* s = m_str; *s != 0; s++)
	{
		const value_type c = *s;
		*s = (c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c; // ASCII only, standard "C" locale
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::MakeUpper()
{
	_MakeUnique();
	for (value_type* s = m_str; *s != 0; s++)
	{
		const value_type c = *s;
		*s = (c >= 'a' && c <= 'z') ? c - 'a' + 'A' : c; // ASCII only, standard "C" locale
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::Trim()
{
	return TrimRight().TrimLeft();
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::Trim(value_type ch)
{
	_MakeUnique();
	const value_type chset[2] = { ch, 0 };
	return TrimRight(chset).TrimLeft(chset);
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::Trim(const value_type* sCharSet)
{
	_MakeUnique();
	return TrimRight(sCharSet).TrimLeft(sCharSet);
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::TrimRight(value_type ch)
{
	const value_type chset[2] = { ch, 0 };
	return TrimRight(chset);
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::TrimRight(const value_type* sCharSet)
{
	if (!sCharSet || !(*sCharSet) || length() < 1)
		return *this;

	const value_type* last = m_str + length() - 1;
	const value_type* str = last;
	while ((str != m_str) && (_strchr(sCharSet, *str) != 0))
		str--;

	if (str != last)
	{
		// Just shrink length of the string.
		size_type nNewLength = (size_type)(str - m_str) + 1; // m_str can change in _MakeUnique
		_MakeUnique();
		_header()->nLength = nNewLength;
		m_str[nNewLength] = 0;
	}

	return *this;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::TrimRight()
{
	if (length() < 1)
		return *this;

	const value_type* last = m_str + length() - 1;
	const value_type* str = last;
	while ((str != m_str) && (isspace((u8)*str) != 0))
		str--;

	if (str != last)   // something changed?
	{
		// Just shrink length of the string.
		size_type nNewLength = (size_type)(str - m_str) + 1; // m_str can change in _MakeUnique
		_MakeUnique();
		_header()->nLength = nNewLength;
		m_str[nNewLength] = 0;
	}

	return *this;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::TrimLeft(value_type ch)
{
	const value_type chset[2] = { ch, 0 };
	return TrimLeft(chset);
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::TrimLeft(const value_type* sCharSet)
{
	if (!sCharSet || !(*sCharSet))
		return *this;

	const value_type* str = m_str;
	while ((*str != 0) && (_strchr(sCharSet, *str) != 0))
		str++;

	if (str != m_str)
	{
		size_type nOff = (size_type)(str - m_str); // m_str can change in _MakeUnique
		_MakeUnique();
		size_type nNewLength = length() - nOff;
		_move(m_str, m_str + nOff, nNewLength + 1);
		_header()->nLength = nNewLength;
		m_str[nNewLength] = 0;
	}

	return *this;
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T>& DrxStringT<T >::TrimLeft()
{
	const value_type* str = m_str;
	while ((*str != 0) && (isspace((u8)*str) != 0))
		str++;

	if (str != m_str)
	{
		size_type nOff = (size_type)(str - m_str); // m_str can change in _MakeUnique
		_MakeUnique();
		size_type nNewLength = length() - nOff;
		_move(m_str, m_str + nOff, nNewLength + 1);
		_header()->nLength = nNewLength;
		m_str[nNewLength] = 0;
	}

	return *this;
}

template<class T>
inline DrxStringT<T> DrxStringT<T >::Right(size_type count) const
{
	if (count == npos)
		return DrxStringT<T>();
	else if (count > length())
		return *this;

	return DrxStringT<T>(m_str + length() - count, count);
}

template<class T>
inline DrxStringT<T> DrxStringT<T >::Left(size_type count) const
{
	if (count == npos)
		return DrxStringT<T>();
	else if (count > length())
		count = length();

	return DrxStringT<T>(m_str, count);
}

//! strspn equivalent.
template<class T>
inline DrxStringT<T> DrxStringT<T >::SpanIncluding(const_str charSet) const
{
	assert(_IsValidString(charSet));
	return Left((size_type)strspn(m_str, charSet));
}

//! strcspn equivalent.
template<class T>
inline DrxStringT<T> DrxStringT<T >::SpanExcluding(const_str charSet) const
{
	assert(_IsValidString(charSet));
	return Left((size_type)strcspn(m_str, charSet));
}

//////////////////////////////////////////////////////////////////////////
template<class T>
inline DrxStringT<T> DrxStringT<T >::Tokenize(const_str charSet, i32& nStart) const
{
	if (nStart < 0)
	{
		return DrxStringT<T>();
	}

	if (!charSet)
		return *this;

	const_str sPlace = m_str + nStart;
	const_str sEnd = m_str + length();
	if (sPlace < sEnd)
	{
		i32 nIncluding = (i32)strspn(sPlace, charSet);

		if ((sPlace + nIncluding) < sEnd)
		{
			sPlace += nIncluding;
			i32 nExcluding = (i32)strcspn(sPlace, charSet);
			i32 nFrom = nStart + nIncluding;
			nStart = nFrom + nExcluding + 1;

			return substr(nFrom, nExcluding);
		}
	}
	// Return empty string.
	nStart = -1;
	return DrxStringT<T>();
}

//! Variant of DrxStringT<T> which does not share memory with other strings.
template<class T>
class DrxStringLocalT : public DrxStringT<T>
{
public:
	typedef DrxStringT<T>                       BaseType;
	typedef typename BaseType::const_str        const_str;
	typedef typename BaseType::value_type       value_type;
	typedef typename BaseType::size_type        size_type;
	typedef typename BaseType::iterator         iterator;
	typedef typename BaseType::reverse_iterator reverse_iterator;

	DrxStringLocalT()
	{}
	DrxStringLocalT(const DrxStringLocalT& str)
		: BaseType(str.c_str())
	{}
	DrxStringLocalT(const BaseType& str)
		: BaseType(str.c_str())
	{}
	template<size_t Size>
	DrxStringLocalT(const DrxStackStringT<T, Size>& str)
		: BaseType(str.c_str())
	{}
	DrxStringLocalT(const_str str)
		: BaseType(str)
	{}
	DrxStringLocalT(const_str str, size_t len)
		: BaseType(str, len)
	{}
	DrxStringLocalT(const_str begin, const_str end)
		: BaseType(begin, end)
	{}
	DrxStringLocalT(size_type nRepeat, value_type ch)
		: BaseType(nRepeat, ch)
	{}
	DrxStringLocalT& operator=(const BaseType& str)
	{
		BaseType::operator=(str.c_str());
		return *this;
	}
	DrxStringLocalT& operator=(const DrxStringLocalT& str)
	{
		BaseType::operator=(str.c_str());
		return *this;
	}
	DrxStringLocalT& operator=(const_str str)
	{
		BaseType::operator=(str);
		return *this;
	}
	DrxStringLocalT& operator+=(const BaseType& str)
	{
		BaseType::operator+=(str.c_str());
		return *this;
	}
	DrxStringLocalT& operator+=(const DrxStringLocalT& str)
	{
		BaseType::operator+=(str.c_str());
		return *this;
	}
	DrxStringLocalT& operator+=(const_str str)
	{
		BaseType::operator+=(str);
		return *this;
	}
	DrxStringLocalT& operator+=(value_type ch)
	{
		BaseType::operator+=(ch);
		return *this;
	}
	iterator         begin() { return BaseType::m_str; }
	using BaseType::begin; // const version
	iterator         end() { return BaseType::m_str + BaseType::length(); }
	using BaseType::end; // const version
	reverse_iterator rbegin() { return reverse_iterator(end()); }
	using BaseType::rbegin; // const version
	reverse_iterator rend() { return reverse_iterator(begin()); }
	using BaseType::rend; // const version
};

//////////////////////////////////////////////////////////////////////////
//! compare helpers
template<class T> inline bool operator==(const DrxStringLocalT<T>& s1, const DrxStringLocalT<T>& s2)
{
	return s1.compare(s2) == 0;
}
template<class T> inline bool operator==(const DrxStringLocalT<T>& s1, const typename DrxStringLocalT<T>::value_type* s2)
{
	return s1.compare(s2) == 0;
}
template<class T> inline bool operator==(const typename DrxStringLocalT<T>::value_type* s1, const DrxStringLocalT<T>& s2)
{
	return s2.compare(s1) == 0;
}
template<class T> inline bool operator!=(const DrxStringLocalT<T>& s1, const DrxStringLocalT<T>& s2)
{
	return s1.compare(s2) != 0;
}
template<class T> inline bool operator!=(const DrxStringLocalT<T>& s1, const typename DrxStringLocalT<T>::value_type* s2)
{
	return s1.compare(s2) != 0;
}
template<class T> inline bool operator!=(const typename DrxStringLocalT<T>::value_type* s1, const DrxStringLocalT<T>& s2)
{
	return s2.compare(s1) != 0;
}
template<class T> inline bool operator<(const DrxStringLocalT<T>& s1, const DrxStringLocalT<T>& s2)
{
	return s1.compare(s2) < 0;
}
template<class T> inline bool operator<(const DrxStringLocalT<T>& s1, const typename DrxStringLocalT<T>::value_type* s2)
{
	return s1.compare(s2) < 0;
}
template<class T> inline bool operator<(const typename DrxStringLocalT<T>::value_type* s1, const DrxStringLocalT<T>& s2)
{
	return s2.compare(s1) > 0;
}
template<class T> inline bool operator>(const DrxStringLocalT<T>& s1, const DrxStringLocalT<T>& s2)
{
	return s1.compare(s2) > 0;
}
template<class T> inline bool operator>(const DrxStringLocalT<T>& s1, const typename DrxStringLocalT<T>::value_type* s2)
{
	return s1.compare(s2) > 0;
}
template<class T> inline bool operator>(const typename DrxStringLocalT<T>::value_type* s1, const DrxStringLocalT<T>& s2)
{
	return s2.compare(s1) < 0;
}
template<class T> inline bool operator<=(const DrxStringLocalT<T>& s1, const DrxStringLocalT<T>& s2)
{
	return s1.compare(s2) <= 0;
}
template<class T> inline bool operator<=(const DrxStringLocalT<T>& s1, const typename DrxStringLocalT<T>::value_type* s2)
{
	return s1.compare(s2) <= 0;
}
template<class T> inline bool operator<=(const typename DrxStringLocalT<T>::value_type* s1, const DrxStringLocalT<T>& s2)
{
	return s2.compare(s1) >= 0;
}
template<class T> inline bool operator>=(const DrxStringLocalT<T>& s1, const DrxStringLocalT<T>& s2)
{
	return s1.compare(s2) >= 0;
}
template<class T> inline bool operator>=(const DrxStringLocalT<T>& s1, const typename DrxStringLocalT<T>::value_type* s2)
{
	return s1.compare(s2) >= 0;
}
template<class T> inline bool operator>=(const typename DrxStringLocalT<T>::value_type* s1, const DrxStringLocalT<T>& s2)
{
	return s2.compare(s1) <= 0;
}

template<class T>
inline DrxStringLocalT<T> operator+(const DrxStringLocalT<T>& string1, typename DrxStringLocalT<T>::value_type ch)
{
	DrxStringLocalT<T> s(string1);
	s += ch;
	return s;
}
template<class T>
inline DrxStringLocalT<T> operator+(typename DrxStringLocalT<T>::value_type ch, const DrxStringLocalT<T>& str)
{
	DrxStringLocalT<T> s;
	s.reserve(str.size() + 1);
	s.append(1, ch);
	s.append(str);
	return s;
}
template<class T>
inline DrxStringLocalT<T> operator+(const DrxStringLocalT<T>& string1, const DrxStringLocalT<T>& string2)
{
	DrxStringLocalT<T> s(string1);
	s += string2;
	return s;
}
template<class T>
inline DrxStringLocalT<T> operator+(const DrxStringLocalT<T>& str1, const typename DrxStringLocalT<T>::value_type* str2)
{
	DrxStringLocalT<T> s(str1);
	s.append(str2);
	return s;
}
template<class T>
inline DrxStringLocalT<T> operator+(const typename DrxStringLocalT<T>::value_type* str1, const DrxStringLocalT<T>& str2)
{
	assert(str1 == NULL || DrxStringLocalT<T>::_IsValidString(str1));
	DrxStringLocalT<T> s;
	s.reserve(DrxStringLocalT<T>::_strlen(str1) + str2.size());
	s.append(str1);
	s.append(str2);
	return s;
}
template<class T>
inline DrxStringLocalT<T> operator+(const typename DrxStringLocalT<T>::BaseType& str1, const DrxStringLocalT<T>& str2)
{
	DrxStringLocalT<T> s(str1);
	s.append(str2);
	return s;
}
template<class T>
inline DrxStringLocalT<T> operator+(const DrxStringLocalT<T>& str1, const typename DrxStringLocalT<T>::BaseType& str2)
{
	DrxStringLocalT<T> s(str1);
	s.append(str2);
	return s;
}

typedef DrxStringLocalT<char> DrxStringLocal;

#if !defined(RESOURCE_COMPILER)
	typedef DrxStringT<char>         string;
	typedef DrxStringT<wchar_t>      wstring;
#else
	// Resource compiler spawns many threads managing strings
	// We force use of a non-reference tracked string in order to avoid freeing the same string multiple times across threads
	typedef DrxStringLocalT<char>    string;
	typedef DrxStringLocalT<wchar_t> wstring;
#endif

#else // !defined(NOT_USE_DRX_STRING)

	#include <string> // STL string
typedef STxt  string;
typedef std::wstring wstring;

#endif // !defined(NOT_USE_DRX_STRING)

