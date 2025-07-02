// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Font/StdAfx.h>
// Included only once per DLL module.
#include <drx3D/CoreX/Platform/platform_impl.inl>

#include <drx3D/Sys/IEngineModule.h>
#include <drx3D/CoreX/Extension/IDrxFactory.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>

#include <drx3D/Font/drxFont.h>
#if defined(USE_NULLFONT)
	#include <drx3D/Font/NullFont.h>
#endif

///////////////////////////////////////////////
extern "C" IDrxFont * CreateDrxFontInterface(ISystem * pSystem)
{
	if (gEnv->IsDedicated())
	{
#if defined(USE_NULLFONT)
		return new CDrxNullFont();
#else
		// The NULL font implementation must be present for all platforms
		// supporting running as a pure dedicated server.
		pSystem->GetILog()->LogError("Missing NULL font implementation for dedicated server");
		return NULL;
#endif
	}
	else
	{
#if defined(USE_NULLFONT) && defined(USE_NULLFONT_ALWAYS)
		return new CDrxNullFont();
#else
		return new CDrxFont(pSystem);
#endif
	}
}

//////////////////////////////////////////////////////////////////////////
class CEngineModule_DrxFont : public IFontEngineModule
{
	DRXINTERFACE_BEGIN()
		DRXINTERFACE_ADD(Drx::IDefaultModule)
		DRXINTERFACE_ADD(IFontEngineModule)
	DRXINTERFACE_END()

	DRXGENERATE_SINGLETONCLASS_GUID(CEngineModule_DrxFont, "EngineModule_DinrusXFont", "6758643f-4321-4957-9b92-0d898d31f434"_drx_guid)

	virtual ~CEngineModule_DrxFont()
	{
		SAFE_RELEASE(gEnv->pDrxFont);
	}

	//////////////////////////////////////////////////////////////////////////
	virtual tukk GetName() const override { return "DinrusXFont"; };
	virtual tukk GetCategory() const override { return "drx3D"; };

	//////////////////////////////////////////////////////////////////////////
	virtual bool Initialize(SSysGlobEnv& env, const SSysInitParams& initParams) override
	{
		ISystem* pSystem = env.pSystem;
		env.pDrxFont = CreateDrxFontInterface(pSystem);
		return env.pDrxFont != 0;
	}
};

DRXREGISTER_SINGLETON_CLASS(CEngineModule_DrxFont)