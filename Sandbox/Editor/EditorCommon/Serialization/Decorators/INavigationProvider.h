// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace Serialization
{

struct SNavigationContext
{
	string path;
};

struct INavigationProvider
{
	virtual tukk GetIcon(tukk type, tukk path) const = 0;
	virtual tukk GetFileSelectorMaskForType(tukk type) const = 0;
	virtual bool        IsSelected(tukk type, tukk path, i32 index) const = 0;
	virtual bool        IsActive(tukk type, tukk path, i32 index) const = 0;
	virtual bool        IsModified(tukk type, tukk path, i32 index) const = 0;
	virtual bool        Select(tukk type, tukk path, i32 index) const = 0;
	virtual bool        CanSelect(tukk type, tukk path, i32 index) const { return false; }
	virtual bool        CanPickFile(tukk type, i32 index) const                 { return true; }
	virtual bool        CanCreate(tukk type, i32 index) const                   { return false; }
	virtual bool        Create(tukk type, tukk path, i32 index) const    { return false; }
	virtual bool        IsRegistered(tukk type) const                           { return false; }
};

}

