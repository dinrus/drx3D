// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/BaseEnv_Prerequisites.h>

#include <drx3D/Schema2/BaseEnv_EntityTransformComponentBase.h>
#include <drx3D/Schema2/BaseEnv_EntitySignals.h>
#include <drx3D/Schema2/BaseEnv_EntityUpdateFilters.h>
#include <drx3D/Schema2/BaseEnv_SpatialIndex.h>

namespace SchematycBaseEnv
{
	extern const sxema2::SGUID s_detectionVolumeComponentGUID;

	class CEntityDetectionVolumeComponent : public CEntityTransformComponentBase
	{
	private:

		struct SVolumeProperties
		{
			SVolumeProperties();

			void Serialize(Serialization::IArchive& archive);

			SpatialVolumeTagNames tags;
			SpatialVolumeTagNames monitorTags;
			Vec3                  size;
			float                 radius;
			ESpatialVolumeShape   shape;
		};

		struct SProperties
		{
			SProperties();

			void Serialize(Serialization::IArchive& archive);

			// PropertyGraphs
			sxema2::SEnvFunctionResult SetTransform(const sxema2::SEnvFunctionContext& context, const STransform& _transform);
			// ~PropertyGraphs

			STransform        transform;
			SVolumeProperties volume;
			bool              bEnabled;
		};

		struct SVolume
		{
			SVolume();

			Matrix33            rot;
			Vec3                pos;
			Vec3                size;
			float               radius;
			SpatialVolumeId     volumeId;
			ESpatialVolumeShape shape;
		};

		typedef std::vector<EntityId> EntityIds;

	public:

		CEntityDetectionVolumeComponent();

		// sxema2::IComponent
		virtual bool Init(const sxema2::SComponentParams& params) override;
		virtual void Run(sxema2::ESimulationMode simulationMode) override;
		virtual sxema2::IComponentPreviewPropertiesPtr GetPreviewProperties() const override;
		virtual void Preview(const SRendParams& params, const SRenderingPassInfo& passInfo, const sxema2::IComponentPreviewProperties& previewProperties) const override;
		virtual void Shutdown() override;
		virtual sxema2::INetworkSpawnParamsPtr GetNetworkSpawnParams() const override;
		// ~sxema2::IComponent

		bool SetEnabled(bool bEnabled);

		static void Register();

	private:

		void SetVolumeSize(float sizeX, float sizeY, float sizeZ);
		void GetVolumeSize(float& sizeX, float& sizeY, float& sizeZ) const;

		void SetVolumeRadius(float radius);
		float GetVolumeRadius() const;

		tukk GetVolumeName(SpatialVolumeId volumeId) const;

		void VolumeEventCallback(const SSpatialIndexEvent& event);
		void OnEvent(const SEntityEvent& event);

		void Update(const sxema2::SUpdateContext& updateContext);

		void Enable();
		void Disable();

	public:

		static const sxema2::SGUID s_componentGUID;
		static const sxema2::SGUID s_enterSignalGUID;
		static const sxema2::SGUID s_leaveSignalGUID;

	private:

		const SProperties*              m_pProperties;
		SVolume                         m_volume;
		EntityIds                       m_entityIds;
		bool                            m_bEnabled;
		EntityNotHiddenUpdateFilter     m_updateFilter;
		sxema2::CUpdateScope         m_updateScope;
		TemplateUtils::CConnectionScope m_connectionScope;
	};
}