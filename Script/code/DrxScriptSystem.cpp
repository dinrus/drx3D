// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Script/StdAfx.h>
#include <drx3D/Script/ScriptSystem.h>

// Included only once per DLL module.
#include <drx3D/CoreX/Platform/platform_impl.inl>

#include <drx3D/Sys/IEngineModule.h>
#include <drx3D/CoreX/Extension/IDrxFactory.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>

//////////////////////////////////////////////////////////////////////////
class CEngineModule_DinrusXScriptSys : public IScriptSystemEngineModule
{
	DRXINTERFACE_BEGIN()
		DRXINTERFACE_ADD(Drx::IDefaultModule)
		DRXINTERFACE_ADD(IScriptSystemEngineModule)
	DRXINTERFACE_END()

	DRXGENERATE_SINGLETONCLASS_GUID(CEngineModule_DinrusXScriptSys, "EngineModule_DinrusXScript", "d032b164-4978-4f82-a99e-7dc6b6338c5c"_drx_guid)

	virtual ~CEngineModule_DinrusXScriptSys()
	{
		SAFE_DELETE(gEnv->pScriptSystem);
	}

	//////////////////////////////////////////////////////////////////////////
	virtual tukk GetName() const override { return "DinrusXScriptSys"; };
	virtual tukk GetCategory() const override { return "drx3D"; };

	//////////////////////////////////////////////////////////////////////////
	virtual bool Initialize(SSysGlobEnv& env, const SSysInitParams& initParams) override
	{
		ISystem* pSystem = env.pSystem;

		CScriptSystem* pScriptSystem = new CScriptSystem;

		bool bStdLibs = true;
		if (!pScriptSystem->Init(pSystem, bStdLibs, 1024))
		{
			pScriptSystem->Release();
			return false;
		}

		env.pScriptSystem = pScriptSystem;
		return true;
	}
};

DRXREGISTER_SINGLETON_CLASS(CEngineModule_DinrusXScriptSys)

#if DRX_PLATFORM_WINDOWS && !defined(_LIB)
HANDLE gDLLHandle = NULL;
BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD ul_reason_for_call,
                      LPVOID lpReserved)
{
	gDLLHandle = hModule;
	return TRUE;
}
#endif

#include <drx3D/CoreX/CrtDebugStats.h>
