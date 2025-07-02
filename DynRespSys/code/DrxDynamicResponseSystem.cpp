// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/DynRespSys/stdafx.h>
#include <drx3D/CoreX/Platform/platform_impl.inl>
#include <drx3D/Sys/IEngineModule.h>
#include <drx3D/CoreX/Memory/BucketAllocatorImpl.h>
#include <drx3D/CoreX/Extension/IDrxFactory.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>

#include <drx3D/DynRespSys/ResponseSystem.h>
#include <drx3D/DynRespSys/VariableCollection.h>

//////////////////////////////////////////////////////////////////////////
class CEngineModule_drx3DDynRespSys : public DRS::IDynamicResponseSystemEngineModule
{
	DRXINTERFACE_BEGIN()
		DRXINTERFACE_ADD(Drx::IDefaultModule)
		DRXINTERFACE_ADD(DRS::IDynamicResponseSystemEngineModule)
	DRXINTERFACE_END()

	DRXGENERATE_SINGLETONCLASS_GUID(CEngineModule_drx3DDynRespSys, "EngineModule_drx3DDynRespSys", "d1ed34dd-a44c-4c17-959a-46df79af5db3"_drx_guid)

	virtual ~CEngineModule_drx3DDynRespSys() override
	{
		SAFE_DELETE(gEnv->pDynamicResponseSystem);
	}

	virtual tukk GetName() const override { return "drx3DDynRespSys"; }
	virtual tukk GetCategory() const override { return "drx3D"; }

	virtual bool        Initialize(SSysGlobEnv& env, const SSysInitParams& initParams) override
	{
		DrxDRS::CResponseSystem* pResponseSystem = new DrxDRS::CResponseSystem();
		env.pDynamicResponseSystem = pResponseSystem;

		pResponseSystem->CreateVariableCollection(DrxDRS::CVariableCollection::s_globalCollectionName);

		return true;
	}
};

DRXREGISTER_SINGLETON_CLASS(CEngineModule_drx3DDynRespSys)

#include <drx3D/CoreX/CrtDebugStats.h>
