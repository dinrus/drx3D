// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// DinrusXAISys.cpp : Определяет точку входа в приложение DLL.
//

#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/DrxAISys.h>
#include <drx3D/CoreX/Platform/platform_impl.inl>
#include <drx3D/AI/CAISystem.h>
#include <drx3D/AI/AILog.h>
#include <drx3D/Sys/ISystem.h>

#include <drx3D/Sys/IEngineModule.h>
#include <drx3D/CoreX/Extension/IDrxFactory.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>

CAISystem* g_pAISystem;

/*
   //////////////////////////////////////////////////////////////////////////
   // Pointer to Global ISystem.
   static ISystem* gISystem = 0;
   ISystem* GetISystem()
   {
   return gISystem;
   }
 */

//////////////////////////////////////////////////////////////////////////
class CEngineModule_DinrusXAISys : public IAIEngineModule
{
	DRXINTERFACE_BEGIN()
		DRXINTERFACE_ADD(Drx::IDefaultModule)
		DRXINTERFACE_ADD(IAIEngineModule)
	DRXINTERFACE_END()

	DRXGENERATE_SINGLETONCLASS_GUID(CEngineModule_DinrusXAISys, "EngineModule_DinrusXAI", "6b8e79a7-8400-4f44-97db-7614428ad251"_drx_guid)

	virtual ~CEngineModule_DinrusXAISys()
	{
		DrxUnregisterFlowNodes();
		SAFE_RELEASE(gEnv->pAISystem);
	}

	//////////////////////////////////////////////////////////////////////////
	virtual tukk GetName()  const override { return "DinrusXAI"; };
	virtual tukk GetCategory()  const override { return "drx3D"; };

	//////////////////////////////////////////////////////////////////////////
	virtual bool Initialize(SSysGlobEnv& env, const SSysInitParams& initParams) override
	{
		ISystem* pSystem = env.pSystem;

		AIInitLog(pSystem);

		g_pAISystem = new CAISystem(pSystem);
		env.pAISystem = g_pAISystem;

		DrxRegisterFlowNodes();

		return true;
	}
};

DRXREGISTER_SINGLETON_CLASS(CEngineModule_DinrusXAISys)

//////////////////////////////////////////////////////////////////////////
#include <drx3D/CoreX/CrtDebugStats.h>

#ifndef _LIB
	#include <drx3D/CoreX/Common_TypeInfo.h>
#endif

#include <drx3D/CoreX/TypeInfo_impl.h>
#include <drx3D/AI/AutoTypeStructs_info.h>
