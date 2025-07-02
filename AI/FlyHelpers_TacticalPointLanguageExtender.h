// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __FLY_HELPERS__TACTICAL_POINT_LANGUAGE_EXTENDER__H__
#define __FLY_HELPERS__TACTICAL_POINT_LANGUAGE_EXTENDER__H__

#include <drx3D/AI/ITacticalPointSystem.h>

namespace FlyHelpers
{

class CTacticalPointLanguageExtender
	: public ITacticalPointLanguageExtender
{
public:
	void         Initialize();
	void         Deinitialize();
	virtual bool GeneratePoints(TGenerateParameters& parameters, SGenerateDetails& details, TObjectType object, const Vec3& objectPos, TObjectType auxObject, const Vec3& auxObjectPos) const;

private:
	void RegisterWithTacticalPointSystem();
	void RegisterQueries();
	void UnregisterFromTacticalPointSystem();
};

}

#endif
