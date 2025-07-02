// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/ISerializationContext.h>

namespace sxema
{
class CSerializationContext : public ISerializationContext
{
public:

	CSerializationContext(const SSerializationContextParams& params);

	// ISerializationContext
	virtual ESerializationPass    GetPass() const override;
	virtual void                  SetValidatorLink(const SValidatorLink& validatorLink) override;
	virtual const SValidatorLink& GetValidatorLink() const override;
	// ~ISerializationContext

private:

	Serialization::SContext m_context;
	ESerializationPass      m_pass;
	SValidatorLink          m_validatorLink;
};
} // sxema
