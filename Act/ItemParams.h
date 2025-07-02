// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Item parameters structure.

   -------------------------------------------------------------------------
   История:
   - 7:10:2005   14:20 : Created by Márcio Martins

*************************************************************************/
#ifndef __ITEMPARAMS_H__
#define __ITEMPARAMS_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include "IItemSystem.h"
#include <drx3D/CoreX/StlUtils.h>

template<class F, class T>
struct SItemParamConversion
{
	static ILINE bool ConvertValue(const F& from, T& to)
	{
		to = (T)from;
		return true;
	};
};

// taken from IFlowSystem.h and adapted...
#define ITEMSYSTEM_STRING_CONVERSION(type, fmt)                   \
  template<>                                                      \
  struct SItemParamConversion<type, string>                       \
  {                                                               \
    static ILINE bool ConvertValue(const type &from, string & to) \
    {                                                             \
      to.Format(fmt, from);                                       \
      return true;                                                \
    }                                                             \
  };                                                              \
  template<>                                                      \
  struct SItemParamConversion<string, type>                       \
  {                                                               \
    static ILINE bool ConvertValue(const string &from, type & to) \
    {                                                             \
      return 1 == sscanf(from.c_str(), fmt, &to);                 \
    }                                                             \
  };

ITEMSYSTEM_STRING_CONVERSION(i32, "%d");
ITEMSYSTEM_STRING_CONVERSION(float, "%f");

#undef ITEMSYSTEM_STRING_CONVERSION

template<>
struct SItemParamConversion<Vec3, bool>
{
	static ILINE bool ConvertValue(const Vec3& from, float& to)
	{
		to = from.GetLengthSquared() > 0;
		return true;
	}
};

template<>
struct SItemParamConversion<bool, Vec3>
{
	static ILINE bool ConvertValue(const float& from, Vec3& to)
	{
		to = Vec3(from ? 1.f : 0.f, from ? 1.f : 0.f, from ? 1.f : 0.f);
		return true;
	}
};

template<>
struct SItemParamConversion<Vec3, float>
{
	static ILINE bool ConvertValue(const Vec3& from, float& to)
	{
		to = from.x;
		return true;
	}
};

template<>
struct SItemParamConversion<float, Vec3>
{
	static ILINE bool ConvertValue(const float& from, Vec3& to)
	{
		to = Vec3(from, from, from);
		return true;
	}
};

template<>
struct SItemParamConversion<Vec3, i32>
{
	static ILINE bool ConvertValue(const Vec3& from, i32& to)
	{
		to = (i32)from.x;
		return true;
	}
};

template<>
struct SItemParamConversion<i32, Vec3>
{
	static ILINE bool ConvertValue(i32k& from, Vec3& to)
	{
		to = Vec3((float)from, (float)from, (float)from);
		return true;
	}
};

template<>
struct SItemParamConversion<Vec3, string>
{
	static ILINE bool ConvertValue(const Vec3& from, string& to)
	{
		to.Format("%s,%s,%s", from.x, from.y, from.z);
		return true;
	}
};

template<>
struct SItemParamConversion<string, Vec3>
{
	static ILINE bool ConvertValue(const string& from, Vec3& to)
	{
		return sscanf(from.c_str(), "%f,%f,%f", &to.x, &to.y, &to.z) == 3;
	}
};

typedef TFlowInputData TItemParamValue;

class CItemParamsNode : public IItemParamsNode
{
public:

	enum EXMLFilterType
	{
		eXMLFT_none   = 0,
		eXMLFT_add    = 1,
		eXMLFT_remove = 2
	};

	CItemParamsNode();
	virtual ~CItemParamsNode();

	virtual void                   AddRef() const      { ++m_refs; };
	virtual u32                 GetRefCount() const { return m_refs; };
	virtual void                   Release() const     { if (!--m_refs) delete this; };

	virtual void                   GetMemoryUsage(IDrxSizer* pSizer) const;

	virtual i32                    GetAttributeCount() const;
	virtual tukk            GetAttributeName(i32 i) const;
	virtual tukk            GetAttribute(i32 i) const;
	virtual bool                   GetAttribute(i32 i, Vec3& attr) const;
	virtual bool                   GetAttribute(i32 i, Ang3& attr) const;
	virtual bool                   GetAttribute(i32 i, float& attr) const;
	virtual bool                   GetAttribute(i32 i, i32& attr) const;
	virtual i32                    GetAttributeType(i32 i) const;

	virtual tukk            GetAttribute(tukk name) const;
	virtual bool                   GetAttribute(tukk name, Vec3& attr) const;
	virtual bool                   GetAttribute(tukk name, Ang3& attr) const;
	virtual bool                   GetAttribute(tukk name, float& attr) const;
	virtual bool                   GetAttribute(tukk name, i32& attr) const;
	virtual i32                    GetAttributeType(tukk name) const;

	virtual tukk            GetAttributeSafe(tukk name) const;

	virtual tukk            GetNameAttribute() const;

	virtual i32                    GetChildCount() const;
	virtual tukk            GetChildName(i32 i) const;
	virtual const IItemParamsNode* GetChild(i32 i) const;
	virtual const IItemParamsNode* GetChild(tukk name) const;

	EXMLFilterType                 ShouldConvertNodeFromXML(const XmlNodeRef& xmlNode, tukk keepWithThisAttrValue) const;

	virtual void                   SetAttribute(tukk name, tukk attr);
	virtual void                   SetAttribute(tukk name, const Vec3& attr);
	virtual void                   SetAttribute(tukk name, float attr);
	virtual void                   SetAttribute(tukk name, i32 attr);

	virtual void                   SetName(tukk name) { m_name = name; };
	virtual tukk            GetName() const           { return m_name.c_str(); };

	virtual IItemParamsNode*       InsertChild(tukk name);
	virtual void                   ConvertFromXML(const XmlNodeRef& root);
	virtual bool                   ConvertFromXMLWithFiltering(const XmlNodeRef& root, tukk keepWithThisAttrValue);

private:
	struct SAttribute
	{
		CDrxName        first; // Using DrxName to save memory on duplicate strings
		TItemParamValue second;

		SAttribute() {}
		SAttribute(tukk key, const TItemParamValue& val)
			: first(key)
			, second(val)
		{
		}

		void GetMemoryUsage(IDrxSizer* pSizer) const
		{
			pSizer->AddObject(second);
		}
	};

	typedef DynArray<SAttribute>       TAttributeMap;
	typedef DynArray<CItemParamsNode*> TChildVector;

	template<typename MT>
	typename MT::const_iterator GetConstIterator(const MT& m, i32 i) const
	{
		typename MT::const_iterator it = m.begin();
		//std::advance(it, i);
		it += i;
		return it;
	};
	TAttributeMap::const_iterator FindAttrIterator(const TAttributeMap& m, tukk name) const
	{
		TAttributeMap::const_iterator it;
		for (it = m.begin(); it != m.end(); ++it)
		{
			if (stricmp(it->first.c_str(), name) == 0)
				return it;
		}
		return m.end();
	};
	TAttributeMap::iterator FindAttrIterator(TAttributeMap& m, tukk name) const
	{
		TAttributeMap::iterator it;
		for (it = m.begin(); it != m.end(); ++it)
		{
			if (stricmp(it->first.c_str(), name) == 0)
				return it;
		}
		return m.end();
	};
	void AddAttribute(tukk name, const TItemParamValue& val)
	{
		TAttributeMap::iterator it = FindAttrIterator(m_attributes, name);
		if (it == m_attributes.end())
		{
			m_attributes.push_back(SAttribute(name, val));
		}
		else
			it->second = val;
	}

	CDrxName       m_name;
	string         m_nameAttribute;
	TAttributeMap  m_attributes;
	TChildVector   m_children;

	mutable u32 m_refs;
};

#endif
