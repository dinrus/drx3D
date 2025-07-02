// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>
#include <drx3D/Schema2/ScriptTimer.h>

#include <drx3D/CoreX/Serialization/BlackBox.h>
#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/CoreX/Serialization/STL.h>
#include <drx3D/Schema2/GUIDRemapper.h>
#include <drx3D/Schema2/IAbstractInterface.h>
#include <drx3D/Schema2/ICompiler.h>
#include <drx3D/Schema2/Deprecated/DocUtils.h>
#include <drx3D/Schema2/IEnvRegistry.h>
#include <drx3D/Schema2/ISerializationContext.h>
#include <drx3D/Schema2/SerializationUtils.h>

namespace sxema2
{
	//////////////////////////////////////////////////////////////////////////
	CScriptTimer::CScriptTimer(IScriptFile& file, const SGUID& guid, const SGUID& scopeGUID, tukk szName)
		: CScriptElementBase(EScriptElementType::Timer, file)
		, m_guid(guid)
		, m_scopeGUID(scopeGUID)
		, m_name(szName)
		, m_params(STimerDuration(1.0f), ETimerFlags::AutoStart)
	{}

	//////////////////////////////////////////////////////////////////////////
	EAccessor CScriptTimer::GetAccessor() const
	{
		return EAccessor::Private;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptTimer::GetGUID() const
	{
		return m_guid;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CScriptTimer::GetScopeGUID() const
	{
		return m_scopeGUID;
	}

	//////////////////////////////////////////////////////////////////////////
	bool CScriptTimer::SetName(tukk szName)
	{
		m_name = szName;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CScriptTimer::GetName() const
	{
		return m_name.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptTimer::EnumerateDependencies(const ScriptDependancyEnumerator& enumerator) const {}

	//////////////////////////////////////////////////////////////////////////
	void CScriptTimer::Refresh(const SScriptRefreshParams& params) {}

	//////////////////////////////////////////////////////////////////////////
	void CScriptTimer::Serialize(Serialization::IArchive& archive)
	{
		LOADING_TIME_PROFILE_SECTION;

		CScriptElementBase::Serialize(archive);

		SerializationContext::SetValidatorLink(archive, SValidatorLink(m_guid)); // #SchematycTODO : Can we set this from CScriptElementBase?
		switch(SerializationContext::GetPass(archive))
		{
		case ESerializationPass::PreLoad:
		case ESerializationPass::Save:
		case ESerializationPass::Edit:
			{
				if(!archive.isEdit())
				{
					archive(m_guid, "guid");
					archive(m_scopeGUID, "scope_guid");
					archive(m_name, "name");
				}
				const ETimerUnits prevUnits = m_params.duration.units;
				archive(m_params.duration.units, "units", "Units");
				switch(m_params.duration.units)
				{
				case ETimerUnits::Frames:
					{
						if(m_params.duration.units != prevUnits)
						{
							m_params.duration = STimerDuration(u32(1));
						}
						archive(m_params.duration.frames, "frames", "Duration");
						break;
					}
				case ETimerUnits::Seconds:
					{
						if(m_params.duration.units != prevUnits)
						{
							m_params.duration = STimerDuration(1.0f);
						}
						archive(m_params.duration.seconds, "seconds", "Duration");
						break;
					}
				case ETimerUnits::Random:
					{
						if(m_params.duration.units != prevUnits)
						{
							m_params.duration = STimerDuration(1.0f, 1.0f);
						}
						archive(m_params.duration.range.min, "min", "Minimum");
						archive(m_params.duration.range.max, "max", "^Maximum");
						break;
					}
				}
				ValidateDuration(m_params.duration, &archive, false);
				archive(GameSerialization::EnumBitFlags(m_params.flags), "flags", "Flags");
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptTimer::RemapGUIDs(IGUIDRemapper& guidRemapper)
	{
		m_guid      = guidRemapper.Remap(m_guid);
		m_scopeGUID = guidRemapper.Remap(m_scopeGUID);
	}

	//////////////////////////////////////////////////////////////////////////
	STimerParams CScriptTimer::GetParams() const
	{
		STimerParams params = m_params;
		ValidateDuration(params.duration, nullptr, true);
		return params;
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptTimer::ValidateDuration(STimerDuration& duration, Serialization::IArchive* pArchive, bool bApplyCorrections) const
	{
		switch(duration.units)
		{
		case ETimerUnits::Frames:
			{
				u32k min = 1;
				u32k max = 1000;
				if(duration.frames < min)
				{
					if(pArchive)
					{
						pArchive->warning(duration.frames, "Minimum delay is %d frames!", min);
					}
					if(bApplyCorrections)
					{
						duration.frames = min;
					}
				}
				else if(duration.frames > max)
				{
					if(pArchive)
					{
						pArchive->warning(duration.frames, "Maximum delay is %d frames!", max);
					}
					if(bApplyCorrections)
					{
						duration.frames = max;
					}
				}
				break;
			}
		case ETimerUnits::Seconds:
			{
				const float min = 0.2f;
				const float max = 1000.0f;
				if(duration.seconds < min)
				{
					if(pArchive)
					{
						pArchive->warning(duration.seconds, "Minimum delay is %f seconds!", min);
					}
					if(bApplyCorrections)
					{
						duration.seconds = min;
					}
				}
				else if(duration.seconds > max)
				{
					if(pArchive)
					{
						pArchive->warning(duration.seconds, "Maximum delay is %f seconds!", max);
					}
					if(bApplyCorrections)
					{
						duration.seconds = max;
					}
				}
				break;
			}
		case ETimerUnits::Random:
			{
				const float min = 0.2f;
				const float max = 1000.0f;
				if(duration.range.min < min)
				{
					if(pArchive)
					{
						pArchive->warning(duration.range.min, "Minimum delay is %f seconds!", min);
					}
					if(bApplyCorrections)
					{
						duration.range.min = min;
					}
				}
				if(duration.range.max > max)
				{
					if(pArchive)
					{
						pArchive->warning(duration.range.max, "Maximum delay is %f seconds!", max);
					}
					if(bApplyCorrections)
					{
						duration.range.max = max;
					}
				}
				if(duration.range.min > duration.range.max)
				{
					if(pArchive)
					{
						pArchive->warning(duration.range.max, "Minimum must never exceed maximum!");
					}
					if(bApplyCorrections)
					{
						duration.range.max = duration.range.min;
					}
				}
			}
		}
	}
}
