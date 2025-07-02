// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/IBaseEnv.h>

#include <drx3D/Schema2/BaseEnv_Prerequisites.h>

#include <drx3D/Schema2/BaseEnv_EntityClassRegistrar.h> // TODO : Remove!!!
#include <drx3D/Schema2/BaseEnv_EntityMap.h> // TODO : Remove!!!

namespace sxema2
{
	class CUpdateRelevanceContext;
}

namespace SchematycBaseEnv
{
	class CSpatialIndex;

//	class CEntityClassRegistrar;
//	class CEntityMap;

	class CBaseEnv : public SchematycBaseEnv::IBaseEnv
	{
	public:

		explicit CBaseEnv();
		~CBaseEnv();

		void PrePhysicsUpdate();
		void Update(sxema2::CUpdateRelevanceContext* pRelevanceContext = nullptr);

		static CBaseEnv& GetInstance();

		CSpatialIndex& GetSpatialIndex();
		CEntityMap& GetGameEntityMap();

		// SchematycBaseEnv::IBaseEnv
		virtual void UpdateSpatialIndex() override;

		virtual i32 GetEditorGameDefaultUpdateMask() const override { return m_editorGameDefaultUpdateMask; }
		virtual void SetEditorGameDefaultUpdateMask(i32k mask) override { m_editorGameDefaultUpdateMask = mask; }
		virtual i32 GetGameDefaultUpdateMask() const override { return m_gameDefaultUpdateMask; }
		virtual void SetGameDefaultUpdateMask(i32k mask) override { m_gameDefaultUpdateMask = mask; }
		// ~SchematycBaseEnv::IBaseEnv

	private:
		void Refresh();
		i32 GetUpdateFlags() const;
		i32 GetDefaultUpdateFlags() const;

	private:
		static CBaseEnv* ms_pInstance;

		std::shared_ptr<CSpatialIndex>  m_pSpatialIndex; // TODO : Why can't we use std::unique_ptr?
		CEntityClassRegistrar           m_gameEntityClassRegistrar;
		CEntityMap                      m_gameEntityMap;
		TemplateUtils::CConnectionScope m_connectionScope;

		i32                             m_editorGameDefaultUpdateMask;
		i32                             m_gameDefaultUpdateMask;
		i32                             sc_Update;
	};
}