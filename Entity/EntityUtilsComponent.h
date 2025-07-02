// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace sxema
{

// Forward declare interfaces.
struct IEnvRegistrar;
// Forward declare structures.
struct SUpdateContext;

class CEntityUtilsComponent final : public IEntityComponent
{
public:

	ExplicitEntityId         GetEntityId() const;

	void                     SetTransform(const DrxTransform::CTransform& transform);
	DrxTransform::CTransform GetTransform();
	void                     SetRotation(const DrxTransform::CRotation& rotation);
	DrxTransform::CRotation  GetRotation();

	void                     SetVisible(bool bVisible);
	bool                     IsVisible() const;

	static void              ReflectType(CTypeDesc<CEntityUtilsComponent>& desc);
	static void              Register(IEnvRegistrar& registrar);
};

} // sxema
