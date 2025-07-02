// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/BaseEnv_Prerequisites.h>

#include <drx3D/Schema2/BaseEnv_EntityTransformComponentBase.h>

namespace SchematycBaseEnv
{
	class CEntityLightComponent : public CEntityTransformComponentBase
	{
	private:

		struct SProperties
		{
			SProperties();

			void Serialize(Serialization::IArchive& archive);

			sxema2::SEnvFunctionResult SetTransform(const sxema2::SEnvFunctionContext& context, const STransform& _transform);

			STransform transform;

			bool   bOn;
			ColorF color;
			float  diffuseMultiplier;
			float  specularMultiplier;
			float  hdrDynamicMultiplier;
			float  radius;
			float  frustumAngle;
			float  attenuationBulbSize;
		};

	public:

		CEntityLightComponent();

		// sxema2::IComponent
		virtual bool Init(const sxema2::SComponentParams& params) override;
		virtual void Run(sxema2::ESimulationMode simulationMode) override;
		virtual sxema2::IComponentPreviewPropertiesPtr GetPreviewProperties() const override;
		virtual void Preview(const SRendParams& params, const SRenderingPassInfo& passInfo, const sxema2::IComponentPreviewProperties& previewProperties) const override;
		virtual void Shutdown() override;
		virtual sxema2::INetworkSpawnParamsPtr GetNetworkSpawnParams() const override;
		// ~sxema2::IComponent

		bool Switch(bool bOn);

		static void Register();

	private:

		void SwitchOn();
		void SwitchOff();

	public:

		static const sxema2::SGUID s_componentGUID;

	private:

		const SProperties* m_pProperties;
		bool               m_bOn;
	};
}