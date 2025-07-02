// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Entity/IEntityComponent.h>
#include <drx3D/AI/INavigationSystem.h>

#include <drx3D/AI/NavigationSystemSchematyc.h>

class CAINavigationMarkupShapeComponent final 
	: public IEntityComponent
	, public INavigationSystem::INavigationSystemListener
{
public:
	static const DrxGUID& IID()
	{
		static DrxGUID id = "3ECDA25A-CCD6-4A2F-A0E4-31E3EB61E74E"_drx_guid;
		return id;
	}

	// IEntityComponent
	virtual void          Initialize() override;
	virtual void          OnShutDown() override;
	virtual uint64        GetEventMask() const override;
	virtual void          ProcessEvent(const SEntityEvent& event) override;
	virtual IEntityComponentPreviewer* GetPreviewer() override;
	// ~IEntityComponent

	// INavigationSystem::INavigationSystemListener
	virtual void OnNavigationEvent(const INavigationSystem::ENavigationEvent event) override;
	// ~INavigationSystem::INavigationSystemListener

	static void           ReflectType(sxema::CTypeDesc<CAINavigationMarkupShapeComponent>& desc);
	static void           Register(sxema::IEnvRegistrar& registrar);

	void RenderVolume() const;

public:
	struct SMarkupShapeProperties
	{
		typedef sxema::CStringHashWrapper<sxema::CFastStringHash, sxema::CEmptyStringConversion, string> Name;
		
		static void ReflectType(sxema::CTypeDesc<SMarkupShapeProperties>& desc);
		void Serialize(Serialization::IArchive& archive);
		bool operator==(const SMarkupShapeProperties& other) const
		{
			return position == other.position
				&& position == other.rotation
				&& areaTypeId == other.areaTypeId
				&& affectedAgentTypesMask == other.affectedAgentTypesMask
				&& size == other.size
				&& bIsStatic == other.bIsStatic
				&& bExpandByAgentRadius == other.bExpandByAgentRadius
				&& name == other.name;
		}

		Vec3 position = ZERO;
		Ang3 rotation = ZERO;
		NavigationAreaTypeID areaTypeId;
		NavigationComponentHelpers::SAgentTypesMask affectedAgentTypesMask;
		Vec3 size = Vec3(2.0f, 2.0f, 2.0f);
		bool bIsStatic = false;
		bool bExpandByAgentRadius = false;
		Name name;
	};

	struct SShapesArray
	{
		static void ReflectType(sxema::CTypeDesc<SShapesArray>& typeInfo)
		{
			typeInfo.SetGUID("AFCBE3EC-7A62-4233-8E88-BB5768498DF2"_drx_guid);
		}
		bool operator==(const SShapesArray& other) const { return shapes == other.shapes; }

		std::vector<SMarkupShapeProperties> shapes;
	};

private:
	struct SRuntimeData
	{
		MNM::AreaAnnotation m_defaultAreaAnotation;
		MNM::AreaAnnotation m_currentAreaAnotation;

		NavigationVolumeID m_volumeId;
	};

	void RegisterAndUpdateComponent();
	void UpdateAnnotations();
	void UpdateVolumes();

	void SetAnnotationFlag(const sxema::CSharedString& shapeName, const NavigationAreaFlagID& flagId, bool bEnable);

	void ToggleAnnotationFlags(const sxema::CSharedString& shapeName, const NavigationComponentHelpers::SAnnotationFlagsMask& flags);
	void ResetAnotations();

	SShapesArray m_shapesProperties;
	bool m_isGeometryIgnoredInNavMesh = true;

	std::vector<SRuntimeData> m_runtimeData;
};

inline bool Serialize(Serialization::IArchive& archive, CAINavigationMarkupShapeComponent::SShapesArray& value, tukk szName, tukk szLabel)
{
	return archive(value.shapes, szName, szLabel);
}
