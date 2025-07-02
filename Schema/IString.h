// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace sxema
{
struct IString
{
	virtual ~IString() {}

	virtual tukk c_str() const = 0;
	virtual u32      length() const = 0;
	virtual bool        empty() const = 0;

	virtual void        clear() = 0;
	virtual void        assign(tukk szInput) = 0;
	virtual void        assign(tukk szInput, u32 offset, u32 length) = 0;
	virtual void        append(tukk szInput) = 0;
	virtual void        append(tukk szInput, u32 offset, u32 length) = 0;
	virtual void        insert(u32 offset, tukk szInput) = 0;
	virtual void        insert(u32 offset, tukk szInput, u32 length) = 0;

	virtual void        Format(tukk szFormat, ...) = 0;
	virtual void        TrimLeft(tukk szChars) = 0;
	virtual void        TrimRight(tukk szChars) = 0;
};
} // sxema
