// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// TODO : Replace all instances of CSignalScope with CConnectionScope!

#pragma once

#include <drx3D/Schema2/TemplateUtils_Delegate.h>
#include <drx3D/Schema2/TemplateUtils_PreprocessorUtils.h>

namespace TemplateUtils
{
	// Signal scope class.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <class SIGNAL> class CSignalScope
	{
	public:

		//friend SIGNAL;	// Temporary workaround for Linux Clang error: non-class friend type 'SIGNAL' is a C++11 extension.

		CSignalScope()
			: m_pSignal(nullptr)
		{}

		~CSignalScope()
		{
			Disconnect();
		}

		bool IsConnected() const
		{
			return m_pSignal != nullptr;
		}

		void Disconnect()
		{
			if(m_pSignal != nullptr)
			{
				m_pSignal->Disconnect(*this);
				m_pSignal = nullptr;
			}
		}

	//private:	// Temporary workaround for Linux Clang error: non-class friend type 'SIGNAL' is a C++11 extension.

		SIGNAL*	m_pSignal;
	};

	// Signal parameters structure.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename SIGNATURE>	struct SSignalParams;

	// Signal queue class.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename SIGNATURE, typename KEY = void, class ELEMENT = SSignalParams<SIGNATURE> >	class CSignalQueue;

	// Signal interface.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename SIGNATURE, typename KEY = void, class KEY_COMPARE = std::less<KEY> > class ISignal;

	// Signal class.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename SIGNATURE, typename KEY = void, class KEY_COMPARE = std::less<KEY> > class CSignal;

	// Signal constants.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	static u8k SIGNAL_MAX_RECURSION_DEPTH = 16;
}

// Signal template specializations.
////////////////////////////////////////////////////////////////////////////////////////////////////
#define INCLUDING_FROM_TEMPLATE_UTILS_SIGNAL_HEADER

#define PARAM_COUNT 0
#include <drx3D/Schema2/TemplateUtils_SignalImpl.h>
#undef PARAM_COUNT

#define PARAM_COUNT 1
#include <drx3D/Schema2/TemplateUtils_SignalImpl.h>
#undef PARAM_COUNT

#define PARAM_COUNT 2
#include <drx3D/Schema2/TemplateUtils_SignalImpl.h>
#undef PARAM_COUNT

#define PARAM_COUNT 3
#include <drx3D/Schema2/TemplateUtils_SignalImpl.h>
#undef PARAM_COUNT

#define PARAM_COUNT 4
#include <drx3D/Schema2/TemplateUtils_SignalImpl.h>
#undef PARAM_COUNT

#define PARAM_COUNT 5
#include <drx3D/Schema2/TemplateUtils_SignalImpl.h>
#undef PARAM_COUNT

#define PARAM_COUNT 6
#include <drx3D/Schema2/TemplateUtils_SignalImpl.h>
#undef PARAM_COUNT

#define PARAM_COUNT 7
#include <drx3D/Schema2/TemplateUtils_SignalImpl.h>
#undef PARAM_COUNT

#define PARAM_COUNT 8
#include <drx3D/Schema2/TemplateUtils_SignalImpl.h>
#undef PARAM_COUNT

#undef INCLUDING_FROM_TEMPLATE_UTILS_SIGNAL_HEADER