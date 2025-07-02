// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/Action.h>
#include <drx3D/Schema/TypeDesc.h>
#include <drx3D/Schema/EnumFlags.h>

namespace sxema
{

enum class EActionFlags
{
	None                    = 0,
	Singleton               = BIT(0), // ает выполняться только одному экземпляру этого действия на один объект в одно и то же время.
	ServerOnly              = BIT(1), // Действие может быть активно только на сервере.
	ClientOnly              = BIT(2), // Действие может быть активно только на клиентах.
	NetworkReplicateServer  = BIT(3), //Действие должно реплицироваться на сервере.
	NetworkReplicateClients = BIT(4)  // Действие должно реплицироваться на клиентах.
};

typedef CEnumFlags<EActionFlags> ActionFlags;

class CActionDesc : public CCustomClassDesc
{
public:

	inline void SetIcon(tukk szIcon)
	{
		m_icon = szIcon;
	}

	inline tukk GetIcon() const
	{
		return m_icon.c_str();
	}

	inline void SetActionFlags(const ActionFlags& flags)
	{
		m_flags = flags;
	}

	inline ActionFlags GetActionFlags() const
	{
		return m_flags;
	}

private:

	string      m_icon;
	ActionFlags m_flags;
};

SXEMA_DECLARE_CUSTOM_CLASS_DESC(CAction, CActionDesc, CClassDescInterface)

} // sxema
