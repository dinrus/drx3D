// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Interface to use serialization helpers

   -------------------------------------------------------------------------
   История:
   - 02:06:2010: Created by Kevin Kirst

*************************************************************************/

#ifndef __ISERIALIZEHELPER_H__
#define __ISERIALIZEHELPER_H__

struct ISerializedObject
{
	// <interfuscator:shuffle>
	virtual ~ISerializedObject() {}
	virtual u32 GetGUID() const = 0;
	virtual void   GetMemoryUsage(IDrxSizer* pSizer) const = 0;

	virtual void   AddRef() = 0;
	virtual void   Release() = 0;

	//! Returns true if object contains no serialized data.
	virtual bool IsEmpty() const = 0;

	//! Resets the serialized object to an initial (empty) state.
	virtual void Reset() = 0;

	//! Call to inject the serialized data into another TSerialize object.
	//! \param serialize TSerialize object to use.
	virtual void Serialize(TSerialize& serialize) = 0;
	// </interfuscator:shuffle>
};

struct ISerializeHelper
{
	typedef bool (* TSerializeFunc)(TSerialize serialize, uk pArgument);

	// <interfuscator:shuffle>
	virtual ~ISerializeHelper() {}
	virtual void GetMemoryUsage(IDrxSizer* pSizer) const = 0;

	virtual void AddRef() = 0;
	virtual void Release() = 0;

	//! Returns an ISerializedObject to be used with this helper.
	//! \param szSection Name of the serialized object's section.
	virtual _smart_ptr<ISerializedObject> CreateSerializedObject(tukk szSection) = 0;

	//! Begins the writing process using the supplied functor.
	//! \param pObject Serialization object to write with.
	//! \param serializeFunc Functor called to supply the serialization logic.
	//! \param pArgument Optional argument passed in to functor.
	//! \return true if writing occurred with given serialization object.
	virtual bool Write(ISerializedObject* pObject, TSerializeFunc serializeFunc, uk pArgument = NULL) = 0;

	//! Begins the reading process using the supplied functor.
	//! \param pObject Serialization object to read from.
	//! \param serializeFunc Functor called to supply the serialization logic.
	//! \param pArgument Optional argument passed in to functor.
	//! \return true if writing occurred with given serialization object.
	virtual bool Read(ISerializedObject* pObject, TSerializeFunc serializeFunc, uk pArgument = NULL) = 0;
	// </interfuscator:shuffle>
};

#endif //__ISERIALIZEHELPER_H__
