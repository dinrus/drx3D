// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$

   -------------------------------------------------------------------------
   История:
   - 2:8:2004   15:20 : Created by Márcio Martins

*************************************************************************/
#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/CmdLineArg.h>

CCmdLineArg::CCmdLineArg(tukk name, tukk value, ECmdLineArgType type)
{
	m_name = name;
	m_value = value;
	m_type = type;
}

CCmdLineArg::~CCmdLineArg()
{
}

tukk CCmdLineArg::GetName() const
{
	return m_name.c_str();
}
tukk CCmdLineArg::GetValue() const
{
	return m_value.c_str();
}
const ECmdLineArgType CCmdLineArg::GetType() const
{
	return m_type;
}
const float CCmdLineArg::GetFValue() const
{
	return (float)atof(m_value.c_str());
}
i32k CCmdLineArg::GetIValue() const
{
	return atoi(m_value.c_str());
}
