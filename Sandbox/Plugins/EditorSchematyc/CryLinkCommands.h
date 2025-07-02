// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma  once

struct IConsole;

namespace Schematyc
{
class CDrxLinkCommands
{
public:

	void                     Register(IConsole* pConsole);
	void                     Unregister();

	static CDrxLinkCommands& GetInstance();

private:

	CDrxLinkCommands();

	~CDrxLinkCommands();

private:

	IConsole*               m_pConsole;
	bool                    m_bRegistered;

	static CDrxLinkCommands ms_instance;
};
} // Schematyc

