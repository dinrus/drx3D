// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Renderer/IFlares.h>

class COpticsFactory
{

private:
	COpticsFactory(){}

public:
	static COpticsFactory* GetInstance()
	{
		static COpticsFactory instance;
		return &instance;
	}

	IOpticsElementBase* Create(EFlareType type) const;
};
