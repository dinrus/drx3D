// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/IAny.h>
#include <drx3D/Schema2/ISerializationContext.h>

namespace sxema2
{
	class CSerializationContext : public ISerializationContext
	{
	public:

		CSerializationContext(const SSerializationContextParams& params);

		// ISerializationContext
		virtual IScriptFile* GetScriptFile() const override;
		virtual ESerializationPass GetPass() const override;
		virtual void SetValidatorLink(const SValidatorLink& validatorLink) override;
		virtual const SValidatorLink& GetValidatorLink() const override;
		virtual void AddDependency(ukk pDependency, const EnvTypeId& typeId) override;
		// ~ISerializationContext

	protected:

		virtual ukk GetDependency_Protected(const EnvTypeId& typeId) const override;

	private:

		typedef std::unordered_map<EnvTypeId, ukk> DependencyMap;

		Serialization::SContext m_context;
		IScriptFile*            m_pScriptFile;
		ESerializationPass      m_pass;
		SValidatorLink          m_validatorLink;
		DependencyMap           m_dependencies;
	};
}
