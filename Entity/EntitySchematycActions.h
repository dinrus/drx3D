// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : These actions are for test purposes only and should either be removed or polished up and moved to separate files.

#include <drx3D/CoreX/Renderer/IRenderAuxGeom.h>

#include  <drx3D/Schema/SharedString.h>
#include  <drx3D/Schema/ITimerSystem.h>
#include  <drx3D/Schema/IUpdateScheduler.h>
#include  <drx3D/Schema/Action.h>
#include  <drx3D/Schema/EnvAction.h>
#include  <drx3D/Schema/EnvSignal.h>

#include  <drx3D/Schema/IEnvRegistrar.h>

namespace sxema
{

bool Serialize(Serialization::IArchive& archive, STimerDuration& value, tukk szName, tukk szLabel) // #SchematycTODO : Move to ITimerSystem.h?
{
	// #SchematycTODO : Validate!!!
	const ETimerUnits prevUnits = value.units;
	archive(value.units, "units", "Units");
	switch (value.units)
	{
	case ETimerUnits::Frames:
		{
			if (value.units != prevUnits)
			{
				value = STimerDuration(u32(1));
			}
			archive(value.frames, "frames", "Duration");
			break;
		}
	case ETimerUnits::Seconds:
		{
			if (value.units != prevUnits)
			{
				value = STimerDuration(1.0f);
			}
			archive(value.seconds, "seconds", "Duration");
			break;
		}
	case ETimerUnits::Random:
		{
			if (value.units != prevUnits)
			{
				value = STimerDuration(1.0f, 1.0f);
			}
			archive(value.range.min, "min", "Minimum");
			archive(value.range.max, "max", "Maximum");
			break;
		}
	}
	return true;
}

class CEntityTimerAction final : public CAction
{
public:

	struct STickSignal
	{
		static void ReflectType(CTypeDesc<STickSignal>& desc)
		{
			desc.SetGUID("57f19e5f-23be-40a5-aff9-98042f6a63f8"_drx_guid);
			desc.SetLabel("Tick");
		}
	};

public:

	// CAction

	virtual bool Init() override
	{
		return true;
	}

	virtual void Start() override
	{
		STimerParams timerParams(m_duration);
		if (m_bAutoStart)
		{
			timerParams.flags.Add(ETimerFlags::AutoStart);
		}
		if (m_bRepeat)
		{
			timerParams.flags.Add(ETimerFlags::Repeat);
		}

		m_timerId = gEnv->pSchematyc->GetTimerSystem().CreateTimer(timerParams, SXEMA_MEMBER_DELEGATE(&CEntityTimerAction::OnTimer, *this));
	}

	virtual void Stop() override
	{
		if (m_timerId != TimerId::Invalid)
		{
			gEnv->pSchematyc->GetTimerSystem().DestroyTimer(m_timerId);
			m_timerId = TimerId::Invalid;
		}
	}

	virtual void Shutdown() override {}

	// ~CAction

	static void ReflectType(CTypeDesc<CEntityTimerAction>& desc)
	{
		desc.SetGUID("6937eddc-f25c-44dc-a759-501d2e5da0df"_drx_guid);
		desc.SetIcon("icons:schematyc/entity_timer_action.png");
		desc.AddMember(&CEntityTimerAction::m_duration, 'dur', "duration", "Duration", "Timer duration", STimerDuration(0.0f));
		desc.AddMember(&CEntityTimerAction::m_bAutoStart, 'auto', "bAutoStart", "AutoStart", "Start timer automatically", true);
		desc.AddMember(&CEntityTimerAction::m_bRepeat, 'rep', "bRepeat", "Repeat", "Repeat timer", false);
	}

	static void Register(IEnvRegistrar& registrar)
	{
		CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			CEnvRegistrationScope actionScope = scope.Register(SXEMA_MAKE_ENV_ACTION(CEntityTimerAction));
			{
				actionScope.Register(SXEMA_MAKE_ENV_SIGNAL(STickSignal));
			}
		}
	}

private:

	void OnTimer()
	{
		CAction::OutputSignal(STickSignal());
		if (!m_bRepeat)
		{
			CAction::GetObject().StopAction(*this);
		}
	}

private:

	STimerDuration m_duration = STimerDuration(0.0f);
	bool           m_bAutoStart = true;
	bool           m_bRepeat = false;

	TimerId        m_timerId = TimerId::Invalid;
};

class CEntityDebugTextAction final : public CAction
{
public:

	// CAction

	virtual bool Init() override
	{
		return true;
	}

	virtual void Start() override
	{
		gEnv->pSchematyc->GetUpdateScheduler().Connect(SUpdateParams(SXEMA_MEMBER_DELEGATE(&CEntityDebugTextAction::Update, *this), m_connectionScope));
	}

	virtual void Stop() override
	{
		m_connectionScope.Release();
	}

	virtual void Shutdown() override {}

	// ~CAction

	static void ReflectType(CTypeDesc<CEntityDebugTextAction>& desc)
	{
		desc.SetGUID("8ea4441b-e080-4cca-8b3e-973e017404d3"_drx_guid);
		desc.SetIcon("icons:schematyc/entity_debug_text_action.png");
		desc.AddMember(&CEntityDebugTextAction::m_text, 'txt', "text", "Text", "Text to display", CSharedString());
		desc.AddMember(&CEntityDebugTextAction::m_pos, 'pos', "pos", "Pos", "Text position", Vec2(ZERO));
		desc.AddMember(&CEntityDebugTextAction::m_color, 'col', "color", "Color", "Text color", Col_White);
	}

	static void Register(IEnvRegistrar& registrar)
	{
		CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			scope.Register(SXEMA_MAKE_ENV_ACTION(CEntityDebugTextAction));
		}
	}

private:

	void Update(const SUpdateContext& updateContext)
	{
		IRenderAuxText::Draw2dLabel(m_pos.x, m_pos.y, 2.0f, m_color, false, "%s", m_text.c_str());
	}

private:

	CSharedString    m_text;
	Vec2             m_pos = Vec2(ZERO);
	ColorF           m_color = Col_White;

	CConnectionScope m_connectionScope;
};

} // sxema
