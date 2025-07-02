// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/FundamentalTypes.h>
#include <drx3D/Schema/EnvElementBase.h>
#include <drx3D/Schema/IEnvRegistry.h>
#include <drx3D/Schema/IEnvClass.h>
#include <drx3D/Schema/Any.h>
#include <drx3D/Schema/TypeName.h>

#define SXEMA_MAKE_ENV_CLASS(guid, name) sxema::EnvClass::MakeShared(guid, name, SXEMA_SOURCE_FILE_INFO)

namespace sxema
{

class CEnvClass : public CEnvElementBase<IEnvClass>
{
private:

	typedef std::vector<DrxGUID> Interfaces;
	typedef std::vector<DrxGUID> Components;

public:

	inline CEnvClass(const DrxGUID& guid, tukk szName, const SSourceFileInfo& sourceFileInfo)
		: CEnvElementBase(guid, szName, sourceFileInfo)
	{}

	// IEnvElement

	virtual bool IsValidScope(IEnvElement& scope) const override
	{
		switch (scope.GetType())
		{
		case EEnvElementType::Module:
			{
				return true;
			}
		default:
			{
				return false;
			}
		}
	}

	// ~IEnvElement

	// IEnvClass

	virtual u32 GetInterfaceCount() const override
	{
		return m_interfaces.size();
	}

	virtual DrxGUID GetInterfaceGUID(u32 interfaceIdx) const override
	{
		return interfaceIdx < m_interfaces.size() ? m_interfaces[interfaceIdx] : DrxGUID();
	}

	virtual u32 GetComponentCount() const override
	{
		return m_components.size();
	}

	virtual DrxGUID GetComponentTypeGUID(u32 componentIdx) const override
	{
		return componentIdx < m_components.size() ? m_components[componentIdx] : DrxGUID();
	}

	virtual CAnyConstPtr GetProperties() const override
	{
		return m_pProperties.get();
	}

	virtual IObjectPreviewer* GetPreviewer() const override
	{
		return m_pPreviewer.get();
	}

	// ~IEnvClass

	inline void AddInterface(const DrxGUID& guid)
	{
		m_interfaces.push_back(guid);
	}

	inline void AddComponent(const DrxGUID& guid)
	{
		m_components.push_back(guid);
	}

	template<typename TYPE> inline void SetProperties(const TYPE& properties)
	{
		m_pProperties = CAnyValue::MakeShared(properties);
	}

	template<typename TYPE> inline void SetPreviewer(const TYPE& previewer)
	{
		m_pPreviewer.reset(new TYPE(previewer));
	}
	
private:

	Interfaces                        m_interfaces;
	Components                        m_components;
	CAnyValuePtr                      m_pProperties;
	std::unique_ptr<IObjectPreviewer> m_pPreviewer;
};

namespace EnvClass
{

inline std::shared_ptr<CEnvClass> MakeShared(const DrxGUID& guid, tukk szName, const SSourceFileInfo& sourceFileInfo)
{
	return std::make_shared<CEnvClass>(guid, szName, sourceFileInfo);
}

} // EnvClass
} // sxema
