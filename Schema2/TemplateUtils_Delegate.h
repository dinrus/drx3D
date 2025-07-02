// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// TODO : Consider renaming CDelegateContextAllocator to CAbstractAllocator and moving to separate file, it might be useful.
// TODO : Can we avoid heap allocation when using lambda functions?

#pragma once

#include <drx3D/Schema2/TemplateUtils_PreprocessorUtils.h>
#include <drx3D/Schema2/TemplateUtils_TypeUtils.h>

#define DELEGATE_FASTCALL __fastcall

namespace TemplateUtils
{
	// Delegate class.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename SIGNATURE> class CDelegate;

	// Helper for constructing delegates.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename FUNCTION_PTR_TYPE, FUNCTION_PTR_TYPE FUNCTION_PTR> struct SMakeDelegate;

	// Delegate context allocator. This class abstracts the allocation, cloning and destruction of
	// objects of any type.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CDelegateContextAllocator
	{
	public:

		inline CDelegateContextAllocator()
			: m_pStub(nullptr)
		{}

		inline CDelegateContextAllocator(const CDelegateContextAllocator& rhs)
			: m_pStub(rhs.m_pStub)
		{}

		template <typename TYPE> inline void SetType()
		{
			m_pStub = Stub<TYPE>;
		}

		inline uk Clone(ukk pContext) const
		{
			if(m_pStub)
			{
				return m_pStub(ECommand::Clone, const_cast<uk>(pContext));
			}
			return nullptr;
		}

		inline void Delete(uk pContext) const
		{
			if(m_pStub)
			{
				m_pStub(ECommand::Delete, pContext);
			}
		}

		inline operator bool () const
		{
			return m_pStub != nullptr;
		}

	private:

		enum class ECommand
		{
			Clone,
			Delete
		};

		typedef uk (*StubPtr)(ECommand, uk );

		template <typename TYPE> static uk Stub(ECommand command, uk pContext)
		{
			switch(command)
			{
			case ECommand::Clone:
				{
					return new (DrxModuleMalloc(sizeof(TYPE))) TYPE(*static_cast<const TYPE*>(pContext));
				}
			case ECommand::Delete:
				{
					static_cast<TYPE*>(pContext)->~TYPE();
					DrxModuleFree(pContext);
					break;
				}
			}
			return nullptr;
		}

		StubPtr m_pStub;
	};
}

// Delegate template specializations.
////////////////////////////////////////////////////////////////////////////////////////////////////
#define INCLUDING_FROM_TEMPLATE_UTILS_DELEGATE_HEADER

#define PARAM_COUNT 0
#include <drx3D/Schema2/TemplateUtils_DelegateImpl.h>
#undef PARAM_COUNT

#define PARAM_COUNT 1
#include <drx3D/Schema2/TemplateUtils_DelegateImpl.h>
#undef PARAM_COUNT

#define PARAM_COUNT 2
#include <drx3D/Schema2/TemplateUtils_DelegateImpl.h>
#undef PARAM_COUNT

#define PARAM_COUNT 3
#include <drx3D/Schema2/TemplateUtils_DelegateImpl.h>
#undef PARAM_COUNT

#define PARAM_COUNT 4
#include <drx3D/Schema2/TemplateUtils_DelegateImpl.h>
#undef PARAM_COUNT

#define PARAM_COUNT 5
#include <drx3D/Schema2/TemplateUtils_DelegateImpl.h>
#undef PARAM_COUNT

#define PARAM_COUNT 6
#include <drx3D/Schema2/TemplateUtils_DelegateImpl.h>
#undef PARAM_COUNT

#define PARAM_COUNT 7
#include <drx3D/Schema2/TemplateUtils_DelegateImpl.h>
#undef PARAM_COUNT

#define PARAM_COUNT 8
#include <drx3D/Schema2/TemplateUtils_DelegateImpl.h>
#undef PARAM_COUNT

#undef INCLUDING_FROM_TEMPLATE_UTILS_DELEGATE_HEADER