// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/SerializationContext.h>

#include <drx3D/Schema/Assert.h>

namespace sxema
{
CSerializationContext::CSerializationContext(const SSerializationContextParams& params)
	: m_context(params.archive, static_cast<ISerializationContext*>(nullptr))
	, m_pass(params.pass)
{
	SXEMA_CORE_ASSERT(!params.archive.context<ISerializationContext>());
	m_context.set(static_cast<ISerializationContext*>(this));
}

ESerializationPass CSerializationContext::GetPass() const
{
	return m_pass;
}

void CSerializationContext::SetValidatorLink(const SValidatorLink& validatorLink)
{
	m_validatorLink = validatorLink;
}

const SValidatorLink& CSerializationContext::GetValidatorLink() const
{
	return m_validatorLink;
}
}
