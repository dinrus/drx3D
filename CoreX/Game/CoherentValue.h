// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: a value which can tell whether it's changed

   -------------------------------------------------------------------------
   История:
   - 06:05:2007  : Created by Marco Koegler

*************************************************************************/
#ifndef __COHERENTVALUE_H__
#define __COHERENTVALUE_H__

#pragma once

#include <drx3D/Script/IScriptSystem.h> // <> required for Interfuscator

template<typename T>
class CCoherentValue
{
public:
	ILINE CCoherentValue()
		: m_dirty(false)
	{
	}

	ILINE CCoherentValue(const T& val)
		: m_val(val)
		, m_dirty(true)
	{
	}

	ILINE CCoherentValue(const CCoherentValue& other)
	{
		if (m_val != other.m_val)
		{
			m_dirty = true;
		}
		m_val = other.m_val;
	}

	ILINE CCoherentValue& operator=(const CCoherentValue& rhs)
	{
		if (this != &rhs)
		{
			if (m_val != rhs.m_val)
			{
				m_dirty = true;
			}
			m_val = rhs.m_val;
		}
		return *this;
	}

	ILINE CCoherentValue operator+=(const CCoherentValue& rhs)
	{
		m_dirty = true;
		m_val += rhs.m_val;

		return *this;
	}

	ILINE bool IsDirty() const
	{
		return m_dirty;
	}

	ILINE void Clear()
	{
		m_dirty = false;
	}

	ILINE const T& Value() const
	{
		return m_val;
	}

	ILINE void SetDirtyValue(CScriptSetGetChain& chain, tukk name)
	{
		if (IsDirty())
		{
			chain.SetValue(name, m_val);
			Clear();
		}
	}

	ILINE void Serialize(TSerialize ser, tukk name)
	{
		ser.Value(name, m_val);
		if (ser.IsReading())
		{
			m_dirty = true;
		}
	}

	ILINE operator T() const
	{
		return m_val;
	}
private:
	T    m_val;
	bool m_dirty;
};

#endif//__COHERENTVALUE_H__
