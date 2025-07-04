// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfxBaseEnv.h>
#include <drx3D/Schema2/BaseEnv_EntityDetectionVolumeComponent.h>

#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/CoreX/Renderer/IRenderer.h>
#include <drx3D/CoreX/Renderer/IRenderAuxGeom.h>

#include <drx3D/Schema2/BaseEnv_AutoRegistrar.h>
#include <drx3D/Schema2/BaseEnv_BaseEnv.h>
#include <drx3D/Schema2/BaseEnv_EntityGeomComponent.h>

namespace SchematycBaseEnv
{
	namespace EntityDetectionVolumeComponentUtils
	{
		struct SPreviewProperties : public sxema2::IComponentPreviewProperties
		{
			SPreviewProperties()
				: gizmoLength(1.0f)
				, bShowGizmos(false)
				, bShowVolumes(false)
			{}

			// sxema2::IComponentPreviewProperties

			virtual void Serialize(Serialization::IArchive& archive) override
			{
				archive(bShowGizmos, "bShowGizmos", "Show Gizmos");
				archive(gizmoLength, "gizmoLength", "Gizmo Length");
				archive(bShowVolumes, "bShowVolumes", "Show Volumes");
			}

			// ~sxema2::IComponentPreviewProperties

			float gizmoLength;
			bool  bShowGizmos;
			bool  bShowVolumes;
		};
	}

	const sxema2::SGUID CEntityDetectionVolumeComponent::s_componentGUID   = "5F0322C0-2EB0-46C7-B3E3-56AB5F200E74";
	const sxema2::SGUID CEntityDetectionVolumeComponent::s_enterSignalGUID = "FBCA030D-7122-41B0-A3AE-BD9EA9E8965E";
	const sxema2::SGUID CEntityDetectionVolumeComponent::s_leaveSignalGUID = "8F2FFE6D-8E74-4B82-B045-051230027AA9";

	CEntityDetectionVolumeComponent::SVolumeProperties::SVolumeProperties()
		: size(1.0f)
		, radius(1.0f)
		, shape(ESpatialVolumeShape::Box)
	{}

	void CEntityDetectionVolumeComponent::SVolumeProperties::Serialize(Serialization::IArchive& archive)
	{
		archive(shape, "shape", "Shape");
		archive.doc("Shape");

		switch(shape)
		{
		case ESpatialVolumeShape::Box:
			{
				archive(size, "size", "Size");
				archive.doc("Size");
				break;
			}
		case ESpatialVolumeShape::Sphere:
			{
				archive(radius, "radius", "Radius");
				archive.doc("Radius");
				break;
			}
		}

		archive(tags, "tags", "+Tags");
		archive.doc("Tags");
		archive(monitorTags, "monitorTags", "+Monitor Tags");
		archive.doc("Signals will be sent when volumes with one or more of these tags set enter/leave this volume.");
	}

	CEntityDetectionVolumeComponent::SProperties::SProperties()
		: bEnabled(true)
	{}

	void CEntityDetectionVolumeComponent::SProperties::Serialize(Serialization::IArchive& archive)
	{
		archive(TransformDecorator<ETransformComponent::PositionAndRotation>(transform), "transform", "Transform");
		archive.doc("Transform");
		archive(volume, "volume", "Volume");
		archive.doc("Volumes");
	}

	sxema2::SEnvFunctionResult CEntityDetectionVolumeComponent::SProperties::SetTransform(const sxema2::SEnvFunctionContext& context, const STransform& _transform)
	{
		transform = _transform;
		return sxema2::SEnvFunctionResult();
	}

	CEntityDetectionVolumeComponent::SVolume::SVolume()
		: rot(IDENTITY)
		, pos(ZERO)
		, size(ZERO)
		, radius(.0f)
		, shape(ESpatialVolumeShape::None)
	{}

	CEntityDetectionVolumeComponent::CEntityDetectionVolumeComponent()
		: m_pProperties(nullptr)
		, m_bEnabled(false)
	{}

	bool CEntityDetectionVolumeComponent::Init(const sxema2::SComponentParams& params)
	{
		if(CEntityTransformComponentBase::Init(params))
		{
			m_pProperties = params.properties.ToPtr<SProperties>();
			m_bEnabled    = m_pProperties->bEnabled;

			const EntityId entityId = CEntityComponentBase::GetEntityId();
			IEntity&       entity = CEntityComponentBase::GetEntity();
			const Matrix34 worldTM = CEntityComponentBase::GetEntity().GetWorldTM();
				
			CSpatialIndex&           spatialIndex = CBaseEnv::GetInstance().GetSpatialIndex();
			const SVolumeProperties& volumeProperties = m_pProperties->volume;

			m_volume.shape  = m_pProperties->volume.shape;
			m_volume.rot    = Matrix33::CreateRotationXYZ(m_pProperties->transform.rot);
			m_volume.size   = volumeProperties.size;
			m_volume.pos    = m_pProperties->transform.pos;
			m_volume.radius = volumeProperties.radius;

			SSpatialVolumeParams volumeParams;
			volumeParams.tags        = spatialIndex.CreateTags(volumeProperties.tags);
			volumeParams.monitorTags = spatialIndex.CreateTags(volumeProperties.monitorTags);
			volumeParams.entityId    = entityId;

			switch(m_volume.shape)
			{
			case ESpatialVolumeShape::Box:
				{
					volumeParams.bounds = CSpatialVolumeBounds::CreateOBB(worldTM, m_volume.pos, m_volume.size, m_volume.rot);
					break;
				}
			case ESpatialVolumeShape::Sphere:
				{
					volumeParams.bounds = CSpatialVolumeBounds::CreateSphere(worldTM, m_volume.pos, m_volume.radius);
					break;
				}
			case ESpatialVolumeShape::Point:
				{
					volumeParams.bounds = CSpatialVolumeBounds::CreatePoint(worldTM, m_volume.pos);
					break;
				}
			}

			m_volume.volumeId = spatialIndex.CreateVolume(volumeParams, SpatialIndexEventCallback::FromMemberFunction<CEntityDetectionVolumeComponent, &CEntityDetectionVolumeComponent::VolumeEventCallback>(*this));
			return true;
		}
		return false;
	}

	void CEntityDetectionVolumeComponent::Run(sxema2::ESimulationMode simulationMode)
	{
		switch(simulationMode)
		{
		case sxema2::ESimulationMode::Game:
			{
				gEnv->pSchematyc2->GetUpdateScheduler().Connect(m_updateScope, sxema2::UpdateCallback::FromMemberFunction<CEntityDetectionVolumeComponent, &CEntityDetectionVolumeComponent::Update>(*this), sxema2::EUpdateFrequency::EveryFrame, sxema2::EUpdatePriority::Default, m_updateFilter.Init(&CEntityComponentBase::GetEntity()));
				g_entityEventSignal.Select(CEntityComponentBase::GetEntityId()).Connect(EntityEventSignal::Delegate::FromMemberFunction<CEntityDetectionVolumeComponent, &CEntityDetectionVolumeComponent::OnEvent>(*this), m_connectionScope);
				break;
			}
		}

		if(m_pProperties->bEnabled)
		{
			Enable();
		}
	}
		
	sxema2::IComponentPreviewPropertiesPtr CEntityDetectionVolumeComponent::GetPreviewProperties() const
	{
		static sxema2::IComponentPreviewPropertiesPtr previewProperties = std::make_shared<EntityDetectionVolumeComponentUtils::SPreviewProperties>();
		return previewProperties;
	}

	void CEntityDetectionVolumeComponent::Preview(const SRendParams& params, const SRenderingPassInfo& passInfo, const sxema2::IComponentPreviewProperties& previewProperties) const
	{
		const EntityDetectionVolumeComponentUtils::SPreviewProperties& previewPropertiesImpl = static_cast<const EntityDetectionVolumeComponentUtils::SPreviewProperties&>(previewProperties);

		if(previewPropertiesImpl.bShowGizmos)
		{
			if(CEntityTransformComponentBase::SlotIsValid())
			{
				IRenderAuxGeom& renderAuxGeom = *gEnv->pRenderer->GetIRenderAuxGeom();
				const Matrix34  worldTM = CEntityTransformComponentBase::GetWorldTM();
				const float     lineThickness = 4.0f;

				renderAuxGeom.DrawLine(worldTM.GetTranslation(), ColorB(255, 0, 0, 255), worldTM.GetTranslation() + (worldTM.GetColumn0().GetNormalized() * previewPropertiesImpl.gizmoLength), ColorB(255, 0, 0, 255), lineThickness);
				renderAuxGeom.DrawLine(worldTM.GetTranslation(), ColorB(0, 255, 0, 255), worldTM.GetTranslation() + (worldTM.GetColumn1().GetNormalized() * previewPropertiesImpl.gizmoLength), ColorB(0, 255, 0, 255), lineThickness);
				renderAuxGeom.DrawLine(worldTM.GetTranslation(), ColorB(0, 0, 255, 255), worldTM.GetTranslation() + (worldTM.GetColumn2().GetNormalized() * previewPropertiesImpl.gizmoLength), ColorB(0, 0, 255, 255), lineThickness);
			}
		}

		if(previewPropertiesImpl.bShowVolumes)
		{
			IRenderAuxGeom& renderAuxGeom = *gEnv->pRenderer->GetIRenderAuxGeom();
			IEntity&        entity = CEntityComponentBase::GetEntity();
			const Matrix34  worldTM = entity.GetWorldTM();

			switch(m_volume.shape)
			{
			case ESpatialVolumeShape::Box:
				{
					const CSpatialVolumeBounds bounds = CSpatialVolumeBounds::CreateOBB(worldTM, m_volume.pos, m_volume.size, m_volume.rot);
					renderAuxGeom.DrawOBB(bounds.AsBox(), Matrix34(IDENTITY), false, ColorB(0, 150, 0, 255), eBBD_Faceted);
					break;
				}
			case ESpatialVolumeShape::Sphere:
				{
					SAuxGeomRenderFlags prevRenderFlags = renderAuxGeom.GetRenderFlags();
					renderAuxGeom.SetRenderFlags(e_Mode3D | e_FillModeWireframe | e_CullModeNone | e_DepthWriteOff | e_DepthTestOn);
					renderAuxGeom.DrawSphere(worldTM.TransformPoint(m_volume.pos), m_volume.radius, ColorB(0, 150, 0, 64), false);
					renderAuxGeom.SetRenderFlags(prevRenderFlags);
					break;
				}
			case ESpatialVolumeShape::Point:
				{
					renderAuxGeom.DrawSphere(worldTM.TransformPoint(m_volume.pos), 0.1f, ColorB(0, 150, 0, 64), false);
					break;
				}
			}
		}
	}

	void CEntityDetectionVolumeComponent::Shutdown()
	{
		CSpatialIndex& spatialIndex = CBaseEnv::GetInstance().GetSpatialIndex();
		spatialIndex.DestroyVolume(m_volume.volumeId);

		CEntityTransformComponentBase::Shutdown();
	}

	sxema2::INetworkSpawnParamsPtr CEntityDetectionVolumeComponent::GetNetworkSpawnParams() const
	{
		return sxema2::INetworkSpawnParamsPtr();
	}

	void CEntityDetectionVolumeComponent::Register()
	{
		sxema2::IEnvRegistry& envRegistry = gEnv->pSchematyc2->GetEnvRegistry();

		{
			sxema2::IComponentFactoryPtr pComponentFactory = SXEMA2_MAKE_COMPONENT_FACTORY_SHARED(CEntityDetectionVolumeComponent, SProperties, CEntityDetectionVolumeComponent::s_componentGUID);
			pComponentFactory->SetName("DetectionVolume");
			pComponentFactory->SetNamespace("Base");
			pComponentFactory->SetAuthor("Dinrus");
			pComponentFactory->SetDescription("Entity detection volume component");
			pComponentFactory->SetFlags(sxema2::EComponentFlags::CreatePropertyGraph);
			pComponentFactory->SetAttachmentType(sxema2::EComponentSocket::Parent, g_tranformAttachmentGUID);
			pComponentFactory->SetAttachmentType(sxema2::EComponentSocket::Child, g_tranformAttachmentGUID);
			envRegistry.RegisterComponentFactory(pComponentFactory);
		}

		// Functions

		{
			sxema2::CEnvFunctionDescriptorPtr pFunctionDescriptor = sxema2::MakeEnvFunctionDescriptorShared(&CEntityDetectionVolumeComponent::SProperties::SetTransform);
			pFunctionDescriptor->SetGUID("D363B5BC-75BD-4582-8BD8-AA6EC1CD8DAE");
			pFunctionDescriptor->SetName("SetTransform");
			pFunctionDescriptor->BindInput(0, 0, "Transform", "");
			envRegistry.RegisterFunction(pFunctionDescriptor);
		}

		{
			sxema2::IComponentMemberFunctionPtr pFunction = SXEMA2_MAKE_COMPONENT_MEMBER_FUNCTION_SHARED(CEntityDetectionVolumeComponent::SetVolumeSize, "C926DA55-97A2-4C97-A7E6-C1416DD6284A", CEntityDetectionVolumeComponent::s_componentGUID);
			pFunction->SetAuthor("Dinrus");
			pFunction->SetDescription("Sets the size of the volume.");
			pFunction->BindInput(0, "X", "Size X", .0f);
			pFunction->BindInput(1, "Y", "Size Y", .0f);
			pFunction->BindInput(2, "Z", "Size Z", .0f);
			envRegistry.RegisterComponentMemberFunction(pFunction);
		}

		{
			sxema2::IComponentMemberFunctionPtr pFunction = SXEMA2_MAKE_COMPONENT_MEMBER_FUNCTION_SHARED(CEntityDetectionVolumeComponent::GetVolumeSize, "E8E87E86-C96B-4948-A298-0FE010E04F2E", CEntityDetectionVolumeComponent::s_componentGUID);
			pFunction->SetAuthor("Dinrus");
			pFunction->SetDescription("Gets the size of the volume.");
			pFunction->BindOutput(0, "X", "Size X");
			pFunction->BindOutput(1, "Y", "Size Y");
			pFunction->BindOutput(2, "Z", "Size Z");
			envRegistry.RegisterComponentMemberFunction(pFunction);
		}

		{
			sxema2::IComponentMemberFunctionPtr pFunction = SXEMA2_MAKE_COMPONENT_MEMBER_FUNCTION_SHARED(CEntityDetectionVolumeComponent::SetVolumeRadius, "17FB678F-2256-4F7B-AB46-E8EAACD2497C", CEntityDetectionVolumeComponent::s_componentGUID);
			pFunction->SetAuthor("Dinrus");
			pFunction->SetDescription("Sets the radius of the volume.");
			pFunction->BindInput(0, "Radius", "Radius", .0f);
			envRegistry.RegisterComponentMemberFunction(pFunction);
		}

		{
			sxema2::IComponentMemberFunctionPtr pFunction = SXEMA2_MAKE_COMPONENT_MEMBER_FUNCTION_SHARED(CEntityDetectionVolumeComponent::GetVolumeRadius, "3950621F-8D47-4BE7-836C-35F211B41FBB", CEntityDetectionVolumeComponent::s_componentGUID);
			pFunction->SetAuthor("Dinrus");
			pFunction->SetDescription("Gets the radius of the volume.");
			pFunction->BindOutput(0, "Radius", "Radius");
			envRegistry.RegisterComponentMemberFunction(pFunction);
		}

		// Signals

		{
			sxema2::ISignalPtr pSignal = envRegistry.CreateSignal(CEntityDetectionVolumeComponent::s_enterSignalGUID);
			pSignal->SetSenderGUID(CEntityDetectionVolumeComponent::s_componentGUID);
			pSignal->SetName("EnterVolume");
			pSignal->SetFileName(SXEMA2_FILE_NAME, "Code");
			pSignal->SetAuthor("Dinrus");
			pSignal->SetDescription("Sent when an entity enters the volume.");
			pSignal->AddInput("Volume", "Name of the volume", sxema2::CPoolString());
			pSignal->AddInput("Entity", "Entity entering volume", sxema2::ExplicitEntityId());
		}

		{
			sxema2::ISignalPtr pSignal = envRegistry.CreateSignal(CEntityDetectionVolumeComponent::s_leaveSignalGUID);
			pSignal->SetSenderGUID(CEntityDetectionVolumeComponent::s_componentGUID);
			pSignal->SetName("LeaveVolume");
			pSignal->SetFileName(SXEMA2_FILE_NAME, "Code");
			pSignal->SetAuthor("Dinrus");
			pSignal->SetDescription("Sent when an entity leaves the volume.");
			pSignal->AddInput("Volume", "Name of the volume", sxema2::CPoolString());
			pSignal->AddInput("Entity", "Entity leaving volume", sxema2::ExplicitEntityId());
		}
	}

	void CEntityDetectionVolumeComponent::SetVolumeSize(float sizeX, float sizeY, float sizeZ)
	{
		if(m_volume.shape == ESpatialVolumeShape::Box)
		{
			m_volume.size.x = sizeX;
			m_volume.size.y = sizeY;
			m_volume.size.z = sizeZ;
		}
	}

	void CEntityDetectionVolumeComponent::GetVolumeSize(float& sizeX, float& sizeY, float& sizeZ) const
	{
		if(m_volume.shape == ESpatialVolumeShape::Box)
		{
			sizeX = m_volume.size.x;
			sizeY = m_volume.size.y;
			sizeZ = m_volume.size.z;
		}
		else
		{
			sizeX = sizeY = sizeZ = .0f;
		}
	}

	void CEntityDetectionVolumeComponent::SetVolumeRadius(float radius)
	{
		if(m_volume.shape == ESpatialVolumeShape::Sphere)
		{
			m_volume.radius = radius;
		}
	}

	float CEntityDetectionVolumeComponent::GetVolumeRadius() const
	{
		if(m_volume.shape == ESpatialVolumeShape::Sphere)
		{
			return m_volume.radius;
		}
		return 0.0f;
	}

	void CEntityDetectionVolumeComponent::VolumeEventCallback(const SSpatialIndexEvent& event)
	{
		const SSpatialVolumeParams eventVolumeParams = CBaseEnv::GetInstance().GetSpatialIndex().GetVolumeParams(event.volumeIds[1]);
		if(eventVolumeParams.entityId != CEntityComponentBase::GetEntityId())
		{
			tukk szVolumeName = GetVolumeName(event.volumeIds[0]);
			if(szVolumeName)
			{
				switch(event.id)
				{
				case ESpatialIndexEventId::Entering:
					{
						const IEntity* pEntity = gEnv->pEntitySystem->GetEntity(eventVolumeParams.entityId);
						DRX_ASSERT(pEntity);
						if(pEntity)
						{
							EntityIds::iterator itEntityInArea = std::find(m_entityIds.begin(), m_entityIds.end(), eventVolumeParams.entityId);
							if(itEntityInArea == m_entityIds.end())
							{
								m_entityIds.push_back(eventVolumeParams.entityId);

								sxema2::CInPlaceStack stack;
								sxema2::Push(stack, szVolumeName);
								sxema2::Push(stack, sxema2::ExplicitEntityId(eventVolumeParams.entityId));
								CEntityComponentBase::GetObject().ProcessSignal(s_enterSignalGUID, stack);
							}
						}
						break;
					}
				case ESpatialIndexEventId::Leaving:
					{
						EntityIds::iterator itEntityInArea = std::find(m_entityIds.begin(), m_entityIds.end(), eventVolumeParams.entityId);
						if(itEntityInArea != m_entityIds.end())
						{
							sxema2::CInPlaceStack stack;
							sxema2::Push(stack, szVolumeName);
							sxema2::Push(stack, sxema2::ExplicitEntityId(eventVolumeParams.entityId));
							CEntityComponentBase::GetObject().ProcessSignal(s_leaveSignalGUID, stack);

							m_entityIds.erase(itEntityInArea);
						}
						break;
					}
				}
			}
		}
	}

	void CEntityDetectionVolumeComponent::OnEvent(const SEntityEvent& event)
	{
		switch(event.event)
		{
		case ENTITY_EVENT_XFORM:
			{
				IEntity&       entity = CEntityComponentBase::GetEntity();
				const Matrix34 worldTM = entity.GetWorldTM();

				CSpatialVolumeBounds bounds;
				switch(m_volume.shape)
				{
				case ESpatialVolumeShape::Box:
					{
						bounds = CSpatialVolumeBounds::CreateOBB(worldTM, m_volume.pos, m_volume.size, m_volume.rot);
						break;
					}
				case ESpatialVolumeShape::Sphere:
					{
						bounds = CSpatialVolumeBounds::CreateSphere(worldTM, m_volume.pos, m_volume.radius);
						break;
					}
				case ESpatialVolumeShape::Point:
					{
						bounds = CSpatialVolumeBounds::CreatePoint(worldTM, m_volume.pos);
						break;
					}
				}

				CSpatialIndex& spatialIndex = CBaseEnv::GetInstance().GetSpatialIndex();
				spatialIndex.UpdateVolumeBounds(m_volume.volumeId, bounds);
			}
			break;
		}
	}

	void CEntityDetectionVolumeComponent::Update(const sxema2::SUpdateContext& updateContext)
	{
		DRX_PROFILE_FUNCTION_ARG(PROFILE_GAME, GetEntity().GetName());

		IEntity&       entity = CEntityComponentBase::GetEntity();
		const Matrix34 worldTM = entity.GetWorldTM();

		CSpatialVolumeBounds bounds;
		switch(m_volume.shape)
		{
		case ESpatialVolumeShape::Box:
			{
				bounds = CSpatialVolumeBounds::CreateOBB(worldTM, m_volume.pos, m_volume.size, m_volume.rot);
				break;
			}
		case ESpatialVolumeShape::Sphere:
			{
				bounds = CSpatialVolumeBounds::CreateSphere(worldTM, m_volume.pos, m_volume.radius);
				break;
			}
		case ESpatialVolumeShape::Point:
			{
				bounds = CSpatialVolumeBounds::CreatePoint(worldTM, m_volume.pos);
				break;
			}
		}

		CSpatialIndex& spatialIndex = CBaseEnv::GetInstance().GetSpatialIndex();
		spatialIndex.UpdateVolumeBounds(m_volume.volumeId, bounds);
	}

	tukk CEntityDetectionVolumeComponent::GetVolumeName(SpatialVolumeId volumeId) const
	{
		// TODO: Retrieve and return the name of the component instance.
		return nullptr;
	}

	bool CEntityDetectionVolumeComponent::SetEnabled(bool bEnable)
	{
		if(bEnable != m_bEnabled)
		{
			if(bEnable)
			{
				Enable();
			}
			else
			{
				Disable();
			}
		}
		return m_bEnabled;
	}

	void CEntityDetectionVolumeComponent::Enable()
	{
		m_bEnabled = true;
	}

	void CEntityDetectionVolumeComponent::Disable()
	{
		m_bEnabled = false;
	}
}

SXEMA2_GAME_ENV_AUTO_REGISTER(&SchematycBaseEnv::CEntityDetectionVolumeComponent::Register)
