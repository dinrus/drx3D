// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef _DOUBLE_MAGAZINE_H_
#define _DOUBLE_MAGAZINE_H_


#include <drx3D/Game/Accessory.h>


class CDoubleMagazine : public CAccessory
{
public:
	CDoubleMagazine();

	virtual void OnAttach(bool attach);
	virtual void OnParentReloaded();
	virtual void SetAccessoryReloadTags(CTagState& fragTags);

private:
	bool m_reloadFaster;
};



#endif
