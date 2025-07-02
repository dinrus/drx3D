// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Audio/ATLEntities.h>

namespace DrxAudio
{
class CAudioListenerUpr;
class CATLAudioObject;
class CAudioTranslationLayer;

namespace Impl
{
struct IImpl;
} //endns Impl

struct SInternalControls
{
	using SwitchState = std::pair<ControlId const, SwitchStateId const>;

	std::map<SwitchState, IAudioSwitchStateImpl const*> m_switchStates;
	std::map<ControlId, CATLTriggerImpl const*>         m_triggerConnections;
	std::map<ControlId, IParameterImpl const*>          m_parameterConnections;
};

class COcclusionObstructionState final : public IAudioSwitchStateImpl
{
public:

	explicit COcclusionObstructionState(SwitchStateId const stateId, CAudioListenerUpr const& audioListenerUpr, CATLAudioObject const& globalAudioObject);

	// IAudioSwitchStateImpl
	virtual ERequestStatus Set(CATLAudioObject& audioObject) const override;
	// ~IAudioSwitchStateImpl

	COcclusionObstructionState(COcclusionObstructionState const&) = delete;
	COcclusionObstructionState(COcclusionObstructionState&&) = delete;
	COcclusionObstructionState& operator=(COcclusionObstructionState const&) = delete;
	COcclusionObstructionState& operator=(COcclusionObstructionState&&) = delete;

private:

	SwitchStateId const          m_stateId;
	CAudioListenerUpr const& m_audioListenerUpr;
	CATLAudioObject const&       m_globalAudioObject;
};

class CRelativeVelocityTrackingState final : public IAudioSwitchStateImpl
{
public:

	explicit CRelativeVelocityTrackingState(SwitchStateId const stateId, CATLAudioObject const& globalAudioObject);

	// IAudioSwitchStateImpl
	virtual ERequestStatus Set(CATLAudioObject& audioObject) const override;
	// ~IAudioSwitchStateImpl

	CRelativeVelocityTrackingState(CRelativeVelocityTrackingState const&) = delete;
	CRelativeVelocityTrackingState(CRelativeVelocityTrackingState&&) = delete;
	CRelativeVelocityTrackingState& operator=(CRelativeVelocityTrackingState const&) = delete;
	CRelativeVelocityTrackingState& operator=(CRelativeVelocityTrackingState&&) = delete;

private:

	SwitchStateId const    m_stateId;
	CATLAudioObject const& m_globalAudioObject;
};

class CAbsoluteVelocityTrackingState final : public IAudioSwitchStateImpl
{
public:

	explicit CAbsoluteVelocityTrackingState(SwitchStateId const stateId, CATLAudioObject const& globalAudioObject);

	// IAudioSwitchStateImpl
	virtual ERequestStatus Set(CATLAudioObject& audioObject) const override;
	// ~IAudioSwitchStateImpl

	CAbsoluteVelocityTrackingState(CAbsoluteVelocityTrackingState const&) = delete;
	CAbsoluteVelocityTrackingState(CAbsoluteVelocityTrackingState&&) = delete;
	CAbsoluteVelocityTrackingState& operator=(CAbsoluteVelocityTrackingState const&) = delete;
	CAbsoluteVelocityTrackingState& operator=(CAbsoluteVelocityTrackingState&&) = delete;

private:

	SwitchStateId const    m_stateId;
	CATLAudioObject const& m_globalAudioObject;
};

class CDoNothingTrigger final : public CATLTriggerImpl
{
public:

	explicit CDoNothingTrigger(TriggerImplId const id);

	// CATLTriggerImpl
	virtual ERequestStatus Execute(Impl::IObject* const pImplObject, Impl::IEvent* const pImplEvent) const override;
	// ~CATLTriggerImpl
};

class CLoseFocusTrigger final : public CATLTriggerImpl
{
public:

	explicit CLoseFocusTrigger(TriggerImplId const id, Impl::IImpl& iimpl);

	// CATLTriggerImpl
	virtual ERequestStatus Execute(Impl::IObject* const pImplObject, Impl::IEvent* const pImplEvent) const override;
	// ~CATLTriggerImpl

private:

	Impl::IImpl& m_iimpl;
};

class CGetFocusTrigger final : public CATLTriggerImpl
{
public:

	explicit CGetFocusTrigger(TriggerImplId const id, Impl::IImpl& iimpl);

	// CATLTriggerImpl
	virtual ERequestStatus Execute(Impl::IObject* const pImplObject, Impl::IEvent* const pImplEvent) const override;
	// ~CATLTriggerImpl

private:

	Impl::IImpl& m_iimpl;
};

class CMuteAllTrigger final : public CATLTriggerImpl
{
public:

	explicit CMuteAllTrigger(TriggerImplId const id, Impl::IImpl& iimpl);

	// CATLTriggerImpl
	virtual ERequestStatus Execute(Impl::IObject* const pImplObject, Impl::IEvent* const pImplEvent) const override;
	// ~CATLTriggerImpl

private:

	Impl::IImpl& m_iimpl;
};

class CUnmuteAllTrigger final : public CATLTriggerImpl
{
public:

	explicit CUnmuteAllTrigger(TriggerImplId const id, Impl::IImpl& iimpl);

	// CATLTriggerImpl
	virtual ERequestStatus Execute(Impl::IObject* const pImplObject, Impl::IEvent* const pImplEvent) const override;
	// ~CATLTriggerImpl

private:
	Impl::IImpl& m_iimpl;
};

class CPauseAllTrigger final : public CATLTriggerImpl
{
public:

	explicit CPauseAllTrigger(TriggerImplId const id, Impl::IImpl& iimpl);

	// CATLTriggerImpl
	virtual ERequestStatus Execute(Impl::IObject* const pImplObject, Impl::IEvent* const pImplEvent) const override;
	// ~CATLTriggerImpl

private:

	Impl::IImpl& m_iimpl;
};

class CResumeAllTrigger final : public CATLTriggerImpl
{
public:

	explicit CResumeAllTrigger(TriggerImplId const id, Impl::IImpl& iimpl);

	// CATLTriggerImpl
	virtual ERequestStatus Execute(Impl::IObject* const pImplObject, Impl::IEvent* const pImplEvent) const override;
	// ~CATLTriggerImpl

private:

	Impl::IImpl& m_iimpl;
};

class CAbsoluteVelocityParameter final : public IParameterImpl
{
public:

	explicit CAbsoluteVelocityParameter() = default;

	// IParameterImpl
	virtual ERequestStatus Set(CATLAudioObject& audioObject, float const value) const override;
	// ~IParameterImpl
};

class CRelativeVelocityParameter final : public IParameterImpl
{
public:

	explicit CRelativeVelocityParameter() = default;

	// IParameterImpl
	virtual ERequestStatus Set(CATLAudioObject& audioObject, float const value) const override;
	// ~IParameterImpl
};
} //endns DrxAudio
