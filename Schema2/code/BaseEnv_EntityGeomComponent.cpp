// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfxBaseEnv.h>
#include <drx3D/Schema2/BaseEnv_EntityGeomComponent.h>

#include <drx3D/Schema2/BaseEnv_AutoRegistrar.h>

namespace SchematycBaseEnv
{
	const sxema2::SGUID CEntityGeomComponent::s_componentGUID = "d2474675-c67c-42b2-af33-5c5ace2d1d8c";

	void CEntityGeomComponent::SProperties::Serialize(Serialization::IArchive& archive)
	{
		archive(fileName, "fileName", "FileName");
		archive.doc("Name of .cgf file to load");
		archive(transform, "transform", "Transform");
		archive.doc("Transform relative to parent/entity");
	}

	sxema2::SEnvFunctionResult CEntityGeomComponent::SProperties::SetFileName(const sxema2::SEnvFunctionContext& context, const sxema2::SGeometryFileName& _fileName)
	{
		fileName = _fileName;
		return sxema2::SEnvFunctionResult();
	}

	sxema2::SEnvFunctionResult CEntityGeomComponent::SProperties::SetTransform(const sxema2::SEnvFunctionContext& context, const STransform& _transform)
	{
		transform = _transform;
		return sxema2::SEnvFunctionResult();
	}

	CEntityGeomComponent::CEntityGeomComponent()
		: m_pProperties(nullptr)
	{}

	bool CEntityGeomComponent::Init(const sxema2::SComponentParams& params)
	{
		if(!CEntityTransformComponentBase::Init(params))
		{
			return false;
		}
		m_pProperties = params.properties.ToPtr<SProperties>();
		return true;
	}

	void CEntityGeomComponent::Run(sxema2::ESimulationMode simulationMode)
	{
		if(!m_pProperties->fileName.value.empty())
		{
			i32k slot = CEntityComponentBase::GetEntity().LoadGeometry(g_emptySlot, m_pProperties->fileName.value.c_str());
			CEntityTransformComponentBase::SetSlot(slot, m_pProperties->transform);
		}
	}
		
	sxema2::IComponentPreviewPropertiesPtr CEntityGeomComponent::GetPreviewProperties() const
	{
		return sxema2::IComponentPreviewPropertiesPtr();
	}

	void CEntityGeomComponent::Preview(const SRendParams& params, const SRenderingPassInfo& passInfo, const sxema2::IComponentPreviewProperties& previewProperties) const {}

	void CEntityGeomComponent::Shutdown()
	{
		CEntityTransformComponentBase::Shutdown();
	}

	sxema2::INetworkSpawnParamsPtr CEntityGeomComponent::GetNetworkSpawnParams() const
	{
		return sxema2::INetworkSpawnParamsPtr();
	}

	sxema2::SEnvFunctionResult CEntityGeomComponent::SetVisible(const sxema2::SEnvFunctionContext& context, bool bVisible)
	{
		return sxema2::SEnvFunctionResult();
	}

	sxema2::SEnvFunctionResult CEntityGeomComponent::IsVisible(const sxema2::SEnvFunctionContext& context, bool &bVisible) const
	{
		return sxema2::SEnvFunctionResult();
	}

	void CEntityGeomComponent::Register()
	{
		sxema2::IEnvRegistry& envRegistry = gEnv->pSchematyc2->GetEnvRegistry();

		{
			sxema2::IComponentFactoryPtr pComponentFactory = SXEMA2_MAKE_COMPONENT_FACTORY_SHARED(CEntityGeomComponent, SProperties, CEntityGeomComponent::s_componentGUID);
			pComponentFactory->SetName("Geom");
			pComponentFactory->SetNamespace("Base");
			pComponentFactory->SetAuthor("Dinrus");
			pComponentFactory->SetDescription("Entity geometry component");
			pComponentFactory->SetFlags(sxema2::EComponentFlags::CreatePropertyGraph);
			pComponentFactory->SetAttachmentType(sxema2::EComponentSocket::Parent, g_tranformAttachmentGUID);
			pComponentFactory->SetAttachmentType(sxema2::EComponentSocket::Child, g_tranformAttachmentGUID);
			envRegistry.RegisterComponentFactory(pComponentFactory);
		}

		// Functions

		{
			sxema2::CEnvFunctionDescriptorPtr pFunctionDescriptor = sxema2::MakeEnvFunctionDescriptorShared(&CEntityGeomComponent::SetVisible);
			pFunctionDescriptor->SetGUID("abc4938d-a631-4a36-9f10-22cf6dc9dabd");
			pFunctionDescriptor->SetName("SetVisible");
			pFunctionDescriptor->BindInput(0, 0, "Visible", "");
			envRegistry.RegisterFunction(pFunctionDescriptor);
		}

		{
			sxema2::CEnvFunctionDescriptorPtr pFunctionDescriptor = sxema2::MakeEnvFunctionDescriptorShared(&CEntityGeomComponent::IsVisible);
			pFunctionDescriptor->SetGUID("5aa5e8f0-b4f4-491d-8074-d8b129500d09");
			pFunctionDescriptor->SetName("IsVisible");
			pFunctionDescriptor->BindOutput(0, 0, "Visible", "");
			envRegistry.RegisterFunction(pFunctionDescriptor);
		}

		{
			sxema2::CEnvFunctionDescriptorPtr pFunctionDescriptor = sxema2::MakeEnvFunctionDescriptorShared(&CEntityGeomComponent::SProperties::SetFileName);
			pFunctionDescriptor->SetGUID("68a4cc56-e304-475c-a9cf-70d1a11b207d");
			pFunctionDescriptor->SetName("SetFileName");
			pFunctionDescriptor->BindInput(0, 0, "FileName", "");
			envRegistry.RegisterFunction(pFunctionDescriptor);
		}

		{
			sxema2::CEnvFunctionDescriptorPtr pFunctionDescriptor = sxema2::MakeEnvFunctionDescriptorShared(&CEntityGeomComponent::SProperties::SetTransform);
			pFunctionDescriptor->SetGUID("4f1e6b3b-1a23-45b9-a2c4-c397d8903e0e");
			pFunctionDescriptor->SetName("SetTransform");
			pFunctionDescriptor->BindInput(0, 0, "Transform", "");
			envRegistry.RegisterFunction(pFunctionDescriptor);
		}
	}
}

SXEMA2_GAME_ENV_AUTO_REGISTER(&SchematycBaseEnv::CEntityGeomComponent::Register)