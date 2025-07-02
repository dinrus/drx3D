// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/BaseEnv_Prerequisites.h>

#include <drx3D/CoreX/StaticInstanceList.h>

#define SXEMA2_GAME_ENV_AUTO_REGISTER(function) static SchematycBaseEnv::CAutoRegistrar PP_JOIN_XY(schematycGameEnvAutoRegistrar, __COUNTER__)(function);

namespace SchematycBaseEnv
{
	typedef void (*AutoRegistrarFunctionPtr)(); 

	class CAutoRegistrar : public CStaticInstanceList<CAutoRegistrar>
	{
	public:

		CAutoRegistrar(AutoRegistrarFunctionPtr pFunction);

		static void Process();

	private:

		AutoRegistrarFunctionPtr m_pFunction;
	};
}