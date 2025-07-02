// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "EntityObject.h"

#include <DrxSchematyc/Env/IEnvRegistry.h>
#include <DrxSchematyc/Env/Elements/EnvComponent.h>

class CEntityObjectWithComponent : public CEntityObject
{
public:
	DECLARE_DYNCREATE(CEntityObjectWithComponent)

	// CEntityObject
	virtual bool Init(CBaseObject* prev, const string& file) override;
	virtual bool CreateGameObject() override;
	// ~CEntityObject

protected:
	DrxGUID m_componentGUID;
};

/*!
* Class Description of Entity with a default component
*/
class CEntityWithComponentClassDesc : public CObjectClassDesc
{
public:
	ObjectType          GetObjectType() { return OBJTYPE_ENTITY; }
	tukk         ClassName() { return "EntityWithComponent"; }
	tukk         Category() { return "Components"; }
	CRuntimeClass*      GetRuntimeClass() { return RUNTIME_CLASS(CEntityObjectWithComponent); }
	tukk         GetFileSpec() { return ""; }
	virtual tukk GetDataFilesFilterString() override { return ""; }
	void                EnumerateObjects(IObjectEnumerator* pEnumerator)
	{
		Schematyc::IEnvRegistry& registry = gEnv->pSchematyc->GetEnvRegistry();

		auto visitComponentsLambda = [pEnumerator](const Schematyc::IEnvComponent& component)
		{
			if (strlen(component.GetDesc().GetLabel()) == 0)
			{
				return Schematyc::EVisitStatus::Continue;
			}

			if (component.GetDesc().GetComponentFlags().Check(EEntityComponentFlags::HideFromInspector))
			{
				return Schematyc::EVisitStatus::Continue;
			}

			tukk szCategory = component.GetDesc().GetEditorCategory();
			if (szCategory == nullptr || szCategory[0] == '\0')
			{
				szCategory = "General";
			}

			char buffer[37];
			component.GetDesc().GetGUID().ToString(buffer);

			string sLabel = szCategory;
			sLabel.append("/");
			sLabel.append(component.GetDesc().GetLabel());

			pEnumerator->AddEntry(sLabel, buffer);

			return Schematyc::EVisitStatus::Continue;
		};

		registry.VisitComponents(visitComponentsLambda);
	}
};
