// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/BaseEnv_Prerequisites.h>

#include <drx3D/Schema2/BaseEnv_EntityTransformComponentBase.h>

namespace SchematycBaseEnv
{
	class CEntityParticleEmitterComponent : public CEntityTransformComponentBase
	{
	private:

		struct SParticleEmitterProperties
		{
			SParticleEmitterProperties();

			void Serialize(Serialization::IArchive& archive);

			sxema2::SParticleEffectName effectName;
			float                          scale;
			bool                           bDisable;
			bool                           bPrime;
			float                          countScale;
			float                          speedScale;
			float                          timeScale;
			float                          pulsePeriod;
			float                          strength;
		};

		struct SProperties
		{
			void Serialize(Serialization::IArchive& archive);

			sxema2::SEnvFunctionResult SetTransform(const sxema2::SEnvFunctionContext& context, const STransform& _transform);

			STransform                 transform;
			SParticleEmitterProperties particleEmitter;
		};

	public:

		// sxema2::IComponent
		virtual bool Init(const sxema2::SComponentParams& params) override;
		virtual void Run(sxema2::ESimulationMode simulationMode) override;
		virtual sxema2::IComponentPreviewPropertiesPtr GetPreviewProperties() const override;
		virtual void Preview(const SRendParams& params, const SRenderingPassInfo& passInfo, const sxema2::IComponentPreviewProperties& previewProperties) const override;
		virtual void Shutdown() override;
		virtual sxema2::INetworkSpawnParamsPtr GetNetworkSpawnParams() const override;
		// ~sxema2::IComponent

		sxema2::SEnvFunctionResult SetVisible(const sxema2::SEnvFunctionContext& context, bool bVisible);
		sxema2::SEnvFunctionResult IsVisible(const sxema2::SEnvFunctionContext& context, bool &bVisible) const;

		static void Register();

	private:

		void LoadParticleEmitter();

	public:

		static const sxema2::SGUID s_componentGUID;

	private:

		SProperties m_properties;
	};
}