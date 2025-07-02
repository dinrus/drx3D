// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

#include <drx3D/Network/ISerialize.h> // <> required for Interfuscator
#include <drx3D/Sys/TimeValue.h>

template<bool READING>
class CSimpleSerializeImpl_Reading;

template<>
class CSimpleSerializeImpl_Reading<true>
{
public:
	CSimpleSerializeImpl_Reading() : m_bCommit(true) {}
	ILINE bool IsReading() const
	{
		return true;
	}
	ILINE bool ShouldCommitValues() const
	{
		return m_bCommit;
	}
	ILINE void Update(ISerializeUpdateFunction* pFunc)
	{
		if (m_bCommit) pFunc->Execute();
	}
protected:
	bool m_bCommit;
};

template<>
class CSimpleSerializeImpl_Reading<false>
{
public:
	ILINE bool IsReading() const
	{
		return false;
	}
	ILINE bool ShouldCommitValues() const
	{
		return true;
	}
	ILINE void Update(ISerializeUpdateFunction*)
	{
	}
};

template<bool READING, ESerializationTarget TARGET>
class CSimpleSerializeImpl : public CSimpleSerializeImpl_Reading<READING>
{
public:
	CSimpleSerializeImpl() : m_failed(false) {}
	ILINE void BeginGroup(tukk szName)
	{
	}
	ILINE void EndGroup()
	{
	}
	ILINE ESerializationTarget GetSerializationTarget() const
	{
		return TARGET;
	}
	ILINE void FlagPartialRead() {}

	ILINE bool Ok() const
	{
		return !m_failed;
	}

protected:
	ILINE void Failed()
	{
		m_failed = true;
	}

private:
	bool m_failed;
};

template<class Impl>
class CSimpleSerialize : public ISerialize
{
public:
	ILINE CSimpleSerialize(Impl& impl) : m_impl(impl)
	{
	}

	void Update(ISerializeUpdateFunction* pFunc)
	{
		m_impl.Update(pFunc);
	}

	void BeginGroup(tukk szName)
	{
		m_impl.BeginGroup(szName);
	}

	bool BeginOptionalGroup(tukk szName, bool condition)
	{
		return m_impl.BeginOptionalGroup(szName, condition);
	}

	void EndGroup()
	{
		m_impl.EndGroup();
	}

	bool IsReading() const
	{
		return m_impl.IsReading();
	}

	bool ShouldCommitValues() const
	{
		return m_impl.ShouldCommitValues();
	}

	ESerializationTarget GetSerializationTarget() const
	{
		return m_impl.GetSerializationTarget();
	}

	void WriteStringValue(tukk name, SSerializeString& value, u32 policy)
	{
		m_impl.Value(name, value, policy);
	}
	void ReadStringValue(tukk name, SSerializeString& curValue, u32 policy)
	{
		m_impl.Value(name, curValue, policy);
	}

	bool Ok() const
	{
		return m_impl.Ok();
	}

	void FlagPartialRead()
	{
		m_impl.FlagPartialRead();
	}

#define SERIALIZATION_TYPE(T) \
  virtual void Value(tukk name, T & x, u32 policy) { m_impl.Value(name, x, policy); }
#include <drx3D/Network/SerializationTypes.h>
#undef SERIALIZATION_TYPE

#define SERIALIZATION_TYPE(T) \
  virtual void ValueWithDefault(tukk name, T & x, const T &defaultValue) { assert(0); }
#include <drx3D/Network/SerializationTypes.h>
	SERIALIZATION_TYPE(SSerializeString)
#undef SERIALIZATION_TYPE

	Impl * GetInnerImpl() { return &m_impl; }

protected:
	Impl& m_impl;
};

//! Support serialization with default values.
//! Requires Implementation serialization stub to have Value() method returning boolean.
template<class Impl>
class CSimpleSerializeWithDefaults : public CSimpleSerialize<Impl>
{
public:
	ILINE CSimpleSerializeWithDefaults(Impl& impl) : CSimpleSerialize<Impl>(impl) {}

#define SERIALIZATION_TYPE(T)                                                     \
  virtual void ValueWithDefault(tukk name, T & x, const T &defaultValue) { \
    if (CSimpleSerialize<Impl>::m_impl.IsReading()) {                             \
      if (!CSimpleSerialize<Impl>::m_impl.Value(name, x, 0))                      \
        x = defaultValue;                                                         \
    }                                                                             \
    else if (x != defaultValue)                                                   \
      CSimpleSerialize<Impl>::m_impl.Value(name, x, 0);                           \
  }
#include <drx3D/Network/SerializationTypes.h>
	SERIALIZATION_TYPE(SSerializeString)
#undef SERIALIZATION_TYPE
};

//! \endcond