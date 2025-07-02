// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Audio/ATLEntityData.h>

namespace DrxAudio
{
namespace Impl
{
namespace PortAudio
{
class CListener final : public IListener
{
public:

	CListener() = default;
	CListener(CListener const&) = delete;
	CListener(CListener&&) = delete;
	CListener& operator=(CListener const&) = delete;
	CListener& operator=(CListener&&) = delete;

	// DrxAudio::Impl::IListener
	virtual ERequestStatus Set3DAttributes(SObject3DAttributes const& attributes) override { return ERequestStatus::Success; }
	// ~DrxAudio::Impl::IListener
};

class CParameter final : public IParameter
{
public:

	CParameter() = default;
	CParameter(CParameter const&) = delete;
	CParameter(CParameter&&) = delete;
	CParameter& operator=(CParameter const&) = delete;
	CParameter& operator=(CParameter&&) = delete;
};

class CSwitchState final : public ISwitchState
{
public:

	CSwitchState() = default;
	CSwitchState(CSwitchState const&) = delete;
	CSwitchState(CSwitchState&&) = delete;
	CSwitchState& operator=(CSwitchState const&) = delete;
	CSwitchState& operator=(CSwitchState&&) = delete;
};

class CEnvironment final : public IEnvironment
{
public:

	CEnvironment() = default;
	CEnvironment(CEnvironment const&) = delete;
	CEnvironment(CEnvironment&&) = delete;
	CEnvironment& operator=(CEnvironment const&) = delete;
	CEnvironment& operator=(CEnvironment&&) = delete;
};

class CFile final : public IFile
{
public:

	CFile() = default;
	CFile(CFile const&) = delete;
	CFile(CFile&&) = delete;
	CFile& operator=(CFile const&) = delete;
	CFile& operator=(CFile&&) = delete;
};

class CStandaloneFile final : public IStandaloneFile
{
public:

	CStandaloneFile() = default;
	CStandaloneFile(CStandaloneFile const&) = delete;
	CStandaloneFile(CStandaloneFile&&) = delete;
	CStandaloneFile& operator=(CStandaloneFile const&) = delete;
	CStandaloneFile& operator=(CStandaloneFile&&) = delete;
};
} //endns PortAudio
} //endns Impl
} //endns DrxAudio
