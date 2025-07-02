// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "SharedData.h"

#include <drx3D/CoreX/Platform/platform.h>
#include <drx3D/CoreX/String/DrxString.h>

namespace ACE
{
namespace Impl
{
struct IItem
{
	//! \cond INTERNAL
	virtual ~IItem() = default;
	//! \endcond

	//! Returns id of the item.
	virtual ControlId GetId() const = 0;

	//! Returns name of the item.
	virtual string const& GetName() const = 0;

	//! Returns radius of the item.
	//! The radius is used to calculate the activity radius of the connected audio system trigger.
	virtual float GetRadius() const = 0;

	//! Returns the number of children.
	virtual size_t GetNumChildren() const = 0;

	//! Returns a pointer to the child item at the given index.
	//! \param index Index of the child slot.
	virtual IItem* GetChildAt(size_t const index) const = 0;

	//! Returns a pointer to the parent item.
	virtual IItem* GetParent() const = 0;

	//! Returns flags of the item.
	virtual EItemFlags GetFlags() const = 0;
};
} //endns Impl
} //endns ACE
