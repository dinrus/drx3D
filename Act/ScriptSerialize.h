// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Helper classes for implementing serialization in script

   -------------------------------------------------------------------------
   История:
   - 24:11:2004   11:30 : Created by Craig Tiller

*************************************************************************/
#ifndef __SCRIPTSERIALIZE_H__
#define __SCRIPTSERIALIZE_H__

#pragma once

#include <vector>

enum EScriptSerializeType
{
	eSST_Bool        = 'B',
	eSST_Float       = 'f',
	eSST_Int8        = 'b',
	eSST_Int16       = 's',
	eSST_Int32       = 'i',
	eSST_String      = 'S',
	eSST_EntityId    = 'E',
	eSST_Void        = '0',
	eSST_Vec3        = 'V',
	eSST_StringTable = 'T'
};

class CScriptSerialize
{
public:
	bool ReadValue(tukk name, EScriptSerializeType type, TSerialize, IScriptTable*);
	bool WriteValue(tukk name, EScriptSerializeType type, TSerialize, IScriptTable*);

private:
	string m_tempString;
};

class CScriptSerializeAny
{
public:
	CScriptSerializeAny()
		: m_type(eSST_Void)
	{
		m_buffer[0] = 0;
	}
	CScriptSerializeAny(EScriptSerializeType type);
	CScriptSerializeAny(const CScriptSerializeAny&);
	~CScriptSerializeAny();
	CScriptSerializeAny& operator=(const CScriptSerializeAny&);

	void SerializeWith(TSerialize, tukk name = 0);
	bool   SetFromFunction(IFunctionHandler* pFH, i32 param);
	void   PushFuncParam(IScriptSystem* pSS) const;
	bool   CopyFromTable(SmartScriptTable& tbl, tukk name);
	void   CopyToTable(SmartScriptTable& tbl, tukk name);
	string DebugInfo() const;

private:
	static const size_t  BUFFER_SIZE = sizeof(string) > sizeof(Vec3) ? sizeof(string) : sizeof(Vec3);
	char                 m_buffer[BUFFER_SIZE];
	EScriptSerializeType m_type;

	template<class T>
	T* Ptr()
	{
		DRX_ASSERT(BUFFER_SIZE >= sizeof(T));
		return reinterpret_cast<T*>(m_buffer);
	}
	template<class T>
	const T* Ptr() const
	{
		DRX_ASSERT(BUFFER_SIZE >= sizeof(T));
		return reinterpret_cast<const T*>(m_buffer);
	}
};

// this class helps provide a bridge between script and ISerialize
class CScriptRMISerialize : public ISerializable
{
public:
	CScriptRMISerialize(tukk format);

	void SerializeWith(TSerialize);
	bool   SetFromFunction(IFunctionHandler* pFH, i32 firstParam);
	void   PushFunctionParams(IScriptSystem* pSS);
	string DebugInfo();

private:
	typedef std::vector<CScriptSerializeAny> TValueVec;
	TValueVec m_values;
};

#endif
