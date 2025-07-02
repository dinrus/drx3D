// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfxBaseEnv.h>
#include <drx3D/Schema2/BaseEnv_Prerequisites.h>

#include <drx3D/Schema2/BaseEnv_AutoRegistrar.h>
#include <drx3D/Schema2/BaseEnv_EntityLightComponent.h>
#include <drx3D/Schema2/BaseEnv_EntityActionBase.h>

namespace SchematycBaseEnv
{
	class CEntityLightSwitchAction : public CEntityActionBase
	{
	private:

		struct SProperties
		{
			SProperties()
				: bOn(true)
			{}

			void Serialize(Serialization::IArchive& archive)
			{
				archive(bOn, "on", "On");
			}

			bool bOn;
		};

	public:

		CEntityLightSwitchAction()
			: m_pProperties(nullptr)
			, m_pLightComponent(nullptr)
			, m_bPrevOn(false)
		{}

		// IAction

		virtual bool Init(const sxema2::SActionParams& params) override
		{
			if(!CEntityActionBase::Init(params))
			{
				return false;
			}
			m_pProperties     = params.properties.ToPtr<SProperties>();
			m_pLightComponent = static_cast<CEntityLightComponent*>(params.pComponent);
			return true;
		}

		virtual void Start() override
		{
			m_bPrevOn = m_pLightComponent->Switch(m_pProperties->bOn);
		}

		virtual void Stop() override
		{
			m_pLightComponent->Switch(m_bPrevOn);
		}

		virtual void Shutdown() override {}

		// ~IAction

		static void Register()
		{
			sxema2::IActionFactoryPtr pActionFactory = SXEMA2_MAKE_COMPONENT_ACTION_FACTORY_SHARED(CEntityLightSwitchAction, SProperties, CEntityLightSwitchAction::s_actionGUID, CEntityLightComponent::s_componentGUID);
			pActionFactory->SetName("Switch");
			pActionFactory->SetAuthor("Dinrus");
			pActionFactory->SetDescription("Switch light on/off");
			gEnv->pSchematyc2->GetEnvRegistry().RegisterActionFactory(pActionFactory);
		}

	public:

		static const sxema2::SGUID s_actionGUID;

	private:

		const SProperties*     m_pProperties;
		CEntityLightComponent* m_pLightComponent;
		bool                   m_bPrevOn;
	};

	const sxema2::SGUID CEntityLightSwitchAction::s_actionGUID = "09c39bb0-4fb6-4ad3-b0ba-ca1fc87c7d9d";
}

SXEMA2_GAME_ENV_AUTO_REGISTER(&SchematycBaseEnv::CEntityLightSwitchAction::Register)
