// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include <drx3D/CoreX/Platform/platform_impl.inl>
#include "IPlugin.h"
#include "Objects/DesignerObject.h"
#include "Objects/AreaSolidObject.h"
#include "Objects/ClipVolumeObject.h"
#include "Tools/AreaSolidTool.h"
#include "DesignerEditor.h"
#include "Tools/ClipVolumeTool.h"
#include "Util/Converter.h"
#include "Util/Exporter.h"
#include "GameExporter.h"
#include "Util/BoostPythonHelpers.h"
#include "Core/Helper.h"
#include "UVMappingEditor/UVMappingEditor.h"
#include <drx3D/CoreX/Serialization/Enum.h>
#include <drx3D/Sandbox/Editor/Plugin/QtViewPane.h>
#include "UIs/DesignerDockable.h"

DECLARE_PYTHON_MODULE(designer);
DECLARE_PYTHON_MODULE(uvmapping);

using namespace Designer;

namespace Designer
{
namespace UVMapping
{
REGISTER_VIEWPANE_FACTORY_AND_MENU(UVMappingEditor, "UV Mapping", "Designer Tool", true, "Designer Tool")
REGISTER_VIEWPANE_FACTORY_AND_MENU(DesignerDockable, "Modeling", "Designer Tool", true, "Designer Tool")
}
}

class EditorDesigner : public IPlugin, public IAutoEditorNotifyListener
{
public:

	i32       GetPluginVersion() override { return 1; }
	tukk GetPluginName() override    { return "DrxDesigner"; }
	tukk GetPluginDescription() override { return "Drxengine modeling and UV mapping tool"; }

	void        OnEditorNotifyEvent(EEditorNotifyEvent aEventId) override
	{
		// Notify the session and tool
		DesignerSession::GetInstance()->OnEditorNotifyEvent(aEventId);
		if (GetDesigner())
			GetDesigner()->OnEditorNotifyEvent(aEventId);

		switch (aEventId)
		{
		case eNotify_OnBeginExportToGame:
		case eNotify_OnBeginSceneSave:
			{
				DesignerSession::GetInstance()->EndSession();
				RemoveAllEmptyDesignerObjects();
			}
			break;

		case eNotify_OnExportBrushes:
			{
				CGameExporter* pGameExporter = CGameExporter::GetCurrentExporter();
				if (pGameExporter)
				{
					Exporter designerExporter;
					string path = PathUtil::RemoveSlash(PathUtil::GetPathWithoutFilename(pGameExporter->GetLevelPack().m_sPath)).c_str();
					designerExporter.ExportBrushes(path, pGameExporter->GetLevelPack().m_pakFile);
				}
			}
			break;
		}
	}
};


REGISTER_PLUGIN(EditorDesigner);

