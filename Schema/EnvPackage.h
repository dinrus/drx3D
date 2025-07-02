// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IEnvPackage.h>

#define SXEMA_MAKE_ENV_PACKAGE(guid, name, author, description, callback) stl::make_unique<sxema::CEnvPackage>(guid, name, author, description, callback)

namespace sxema
{

typedef std::unique_ptr<IEnvPackage> IEnvPackagePtr;

class CEnvPackage : public IEnvPackage
{
public:

	inline CEnvPackage(const DrxGUID& guid, tukk szName, tukk szAuthor, tukk szDescription, const EnvPackageCallback& callback)
		: m_guid(guid)
		, m_name(szName)
		, m_author(szAuthor)
		, m_description(szDescription)
		, m_callback(callback)
	{}

	// IEnvPackage

	virtual DrxGUID GetGUID() const override
	{
		return m_guid;
	}

	virtual tukk GetName() const override
	{
		return m_name.c_str();
	}

	virtual tukk GetAuthor() const override
	{
		return m_author.c_str();
	}

	virtual tukk GetDescription() const override
	{
		return m_description.c_str();
	}

	virtual EnvPackageCallback GetCallback() const override
	{
		return m_callback;
	}

	// ~IEnvPackage

private:

	DrxGUID              m_guid;
	string             m_name;
	string             m_author;
	string             m_description;
	EnvPackageCallback m_callback;
};

} // sxema
