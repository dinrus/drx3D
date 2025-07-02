// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"

#include <drx3D/CoreX/Platform/platform_impl.inl>
#include "IEditorClassFactory.h"
#include <drx3D/Sandbox/Editor/Plugin/ICommandManager.h>
#include "IPlugin.h"
#include "IResourceSelectorHost.h"
#include "AnimationCompressionManager.h"
#include "CharacterTool/CharacterToolForm.h"
#include <drx3D/Sandbox/Editor/Plugin/QtViewPane.h>

// just for CGFContent:
#include <drx3D/CoreX/Renderer/VertexFormats.h>
#include <drx3D/CoreX/Math/Drx_Geo.h>
#include <drx3D/CoreX/TypeInfo_impl.h>
#include <drx3D/CoreX/Common_TypeInfo.h>
#include <Drx3DEngine/IIndexedMesh_info.h>
#include <Drx3DEngine/CGF/CGFContent_info.h>
// ^^^

#include "Serialization.h"

#include "CharacterTool/CharacterToolForm.h"
#include "CharacterTool/CharacterToolSystem.h"


void Log(tukk format, ...)
{
	va_list args;
	va_start(args, format);
	GetIEditor()->GetSystem()->GetILog()->LogV(ILog::eAlways, format, args);
	va_end(args);
}


CharacterTool::System* g_pCharacterToolSystem;

// ---------------------------------------------------------------------------

class CEditorAnimationPlugin : public IPlugin
{
public:
	CEditorAnimationPlugin()
	{
		g_pCharacterToolSystem = new CharacterTool::System();
		g_pCharacterToolSystem->Initialize();

		const ICVar* pUseImgCafCVar = gEnv->pConsole->GetCVar("ca_UseIMG_CAF");
		const bool useImgCafSet = (pUseImgCafCVar && pUseImgCafCVar->GetIVal());
		if (useImgCafSet)
		{
			DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_WARNING, "[EditorAnimation] Animation compilation disabled: 'ca_UseIMG_CAF' should be set to zero at load time for compilation to work.");
		}
		else
		{
			g_pCharacterToolSystem->animationCompressionManager.reset(new CAnimationCompressionManager());
		}
	}

	~CEditorAnimationPlugin()
	{
		if (g_pCharacterToolSystem)
		{
			delete g_pCharacterToolSystem;
			g_pCharacterToolSystem = 0;
		}
	}

	// implements IPlugin
	i32       GetPluginVersion()                        { return 0x01; }
	tukk GetPluginName()                           { return "Editor Animation"; }
	tukk GetPluginDescription()					  { return "Animation tools and Character Tool"; }
};

//////////////////////////////////////////////////////////////////////////-

REGISTER_PLUGIN(CEditorAnimationPlugin);
