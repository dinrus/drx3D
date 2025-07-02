// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfxBaseEnv.h>
#include <drx3D/Schema2/BaseEnv_EntityFoundationProperties.h>

namespace SchematycBaseEnv
{
	SEntityFoundationProperties::SEntityFoundationProperties()
		: icon("editor/objecticons/schematyc.bmp")
		, bHideInEditor(false)
		, bTriggerAreas(true)
	{}

	void SEntityFoundationProperties::Serialize(Serialization::IArchive& archive)
	{
		archive(Serialization::ObjectIconPath(icon), "icon", "Icon");
		archive.doc("Icon");
		archive(bHideInEditor, "bHideInEditor", "Hide In Editor");
		archive.doc("Hide entity class in editor");
		archive(bTriggerAreas, "bTriggerAreas", "Trigger Areas");
		archive.doc("Entity can enter and trigger areas");
	}
}