// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfxBaseEnv.h>
#include <drx3D/Schema2/BaseEnv_BaseEnv.h>

#include <drx3D/Schema2/BaseEnv_AutoRegistrar.h>
//#include <drx3D/Schema2/BaseEnv_EntityClassRegistrar.h>
//#include <drx3D/Schema2/BaseEnv_EntityMap.h>
#include <drx3D/Schema2/BaseEnv_SpatialIndex.h>

#include <drx3D/Sys/IConsole.h>

namespace SchematycBaseEnv
{
	namespace UpdateFlags
	{
		i32k s_None = 0;
		i32k s_StagePrePhysics = eUpdateFlags_StagePrePhysics;
		i32k s_StageDefaultAndPost = eUpdateFlags_StageDefaultAndPost;
		i32k s_Timers = eUpdateFlags_Timers;
		i32k s_SpatialIndex = eUpdateFlags_SpatialIndex;
		i32k s_StagePostDebug = eUpdateFlags_StagePostDebug;

		static i32k s_GameDefaultUpdate = eUpdateFlags_GameDefaultUpdate;
		static i32k s_EditorGameDefaultUpdate = eUpdateFlags_EditorGameDefaultUpdate;

		static i32k s_Default = eUpdateFlags_Default;
	}

	CBaseEnv* CBaseEnv::ms_pInstance = nullptr;

	CBaseEnv::CBaseEnv()
		: m_pSpatialIndex(new CSpatialIndex())
		, m_editorGameDefaultUpdateMask(UpdateFlags::s_EditorGameDefaultUpdate)
		, m_gameDefaultUpdateMask(UpdateFlags::s_GameDefaultUpdate)
	{
		// TODO : Shouldn't log recording, loading and compiling be handled automatically by the sxema framework?

		DRX_ASSERT(!ms_pInstance);
		ms_pInstance = this;

		if (gEnv->IsEditor())
		{
			gEnv->pSchematyc2->GetLogRecorder().Begin();
		}

		m_gameEntityClassRegistrar.Init();

		gEnv->pSchematyc2->Signals().envRefresh.Connect(sxema2::EnvRefreshSignal::Delegate::FromMemberFunction<CBaseEnv, &CBaseEnv::Refresh>(*this), m_connectionScope);
		Refresh(); // TODO : This should be called elsewhere!!!

		gEnv->pSchematyc2->RefreshLogFileSettings();	// TODO: moving this line?

		if (gEnv->IsEditor())
		{
			gEnv->pSchematyc2->GetLogRecorder().End();
		}

		REGISTER_CVAR(sc_Update, UpdateFlags::s_Default, VF_DEV_ONLY | VF_BITFIELD,
			"Selects which update stages are performed by the sxema update manager. Possible values:\n"
			"\t0 - all updates disabled;\n"
			"\t1 - default update flags (usually, same as 'qwe' flags in the pure game)\n"
			"Flags:\n"
			"\tq - PrePhysics stage update;\n"
			"\tw - Default and Post stage update\n"
			"\te - schematyc timers update\n"
			"\tr - PostDebug stage update\n"
		);
	}

	CBaseEnv::~CBaseEnv()
	{
		if (IConsole* pConsole = gEnv->pConsole)
		{
			pConsole->UnregisterVariable("sc_Update");
		}
	}

	i32 CBaseEnv::GetUpdateFlags() const
	{
		IF_LIKELY (sc_Update == UpdateFlags::s_Default)
		{
			return GetDefaultUpdateFlags();
		}
		else
		{
			return sc_Update;
		}
	}

	void CBaseEnv::PrePhysicsUpdate()
	{
		i32k updateFlags = GetUpdateFlags();
		if(!gEnv->pGameFramework->IsGamePaused() && !gEnv->IsEditing() && ((updateFlags & UpdateFlags::s_StagePrePhysics) != 0))
		{
			sxema2::IUpdateScheduler& updateScheduler = gEnv->pSchematyc2->GetUpdateScheduler();
			updateScheduler.BeginFrame(gEnv->pTimer->GetFrameTime());
			updateScheduler.Update(sxema2::EUpdateStage::PrePhysics | sxema2::EUpdateDistribution::Earliest, sxema2::EUpdateStage::PrePhysics | sxema2::EUpdateDistribution::End);
		}
	}

	void CBaseEnv::Update(sxema2::CUpdateRelevanceContext* pRelevanceContext)
	{
		if(!gEnv->pGameFramework->IsGamePaused())
		{
			DRX_PROFILE_FUNCTION(PROFILE_GAME);

			sxema2::IUpdateScheduler& updateScheduler = gEnv->pSchematyc2->GetUpdateScheduler();
			if(!updateScheduler.InFrame())
			{
				updateScheduler.BeginFrame(gEnv->pTimer->GetFrameTime());
			}

			i32k updateFlags = GetUpdateFlags();

			if(gEnv->IsEditing())
			{
				updateScheduler.Update(sxema2::EUpdateStage::Editing | sxema2::EUpdateDistribution::Earliest, sxema2::EUpdateStage::Editing | sxema2::EUpdateDistribution::End, pRelevanceContext);
			}
			else
			{
				if ((updateFlags & UpdateFlags::s_Timers) != 0)
				{
					gEnv->pSchematyc2->GetTimerSystem().Update();
				}
				
				if ((updateFlags & UpdateFlags::s_SpatialIndex) != 0)
				{
					m_pSpatialIndex->Update();
				}

				if ((updateFlags & UpdateFlags::s_StageDefaultAndPost) != 0)
				{
					updateScheduler.Update(sxema2::EUpdateStage::Default | sxema2::EUpdateDistribution::Earliest, sxema2::EUpdateStage::Post | sxema2::EUpdateDistribution::End, pRelevanceContext);
				}
			}

			if ((updateFlags & UpdateFlags::s_StagePostDebug) != 0)
			{
				updateScheduler.Update(sxema2::EUpdateStage::PostDebug | sxema2::EUpdateDistribution::Earliest, sxema2::EUpdateStage::PostDebug | sxema2::EUpdateDistribution::End, pRelevanceContext);
			}

			updateScheduler.EndFrame();

			gEnv->pSchematyc2->GetLog().Update();
		}
	}

	CBaseEnv& CBaseEnv::GetInstance()
	{
		DRX_ASSERT(ms_pInstance);
		return *ms_pInstance;
	}

	CSpatialIndex& CBaseEnv::GetSpatialIndex()
	{
		return *m_pSpatialIndex;
	}

/*
	CEntityClassRegistrar& CBaseEnv::GetGameEntityClassRegistrar()
	{
		return m_gameEntityClassRegistrar;
	}*/

	CEntityMap& CBaseEnv::GetGameEntityMap()
	{
		return m_gameEntityMap;
	}

	void CBaseEnv::UpdateSpatialIndex()
	{
		return m_pSpatialIndex->Update();
	}

	i32 CBaseEnv::GetDefaultUpdateFlags() const
	{
		return gEnv->IsEditor()
			? m_editorGameDefaultUpdateMask
			: m_gameDefaultUpdateMask;
	}

	void CBaseEnv::Refresh()
	{
		m_gameEntityClassRegistrar.Refresh();
		CAutoRegistrar::Process();
		gEnv->pSchematyc2->GetEnvRegistry().Validate();
	}
}
