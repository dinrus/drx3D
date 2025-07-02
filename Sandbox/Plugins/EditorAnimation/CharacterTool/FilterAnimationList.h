// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <DrxSystem/File/IFileChangeMonitor.h>
#include "../Shared/AnimationFilter.h"

namespace CharacterTool
{

// Maintains list of files that are used by AnimationFilter users to preview their filters.
class FilterAnimationList : public IFileChangeListener
{
public:
	FilterAnimationList();
	~FilterAnimationList();

	void                         Populate();

	const SAnimationFilterItems& Animations() const { return m_items; }
	i32                          Revision() const   { return m_revision; }
protected:
	void                         OnFileChange(tukk filename, EChangeType eType) override;
private:
	void                         UpdateItem(tukk filename);
	i32                          RemoveItem(tukk filename);

	SAnimationFilterItems m_items;
	i32                   m_revision;
};

}

