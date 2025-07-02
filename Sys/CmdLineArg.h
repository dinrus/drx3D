// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements the command line argument interface ICmdLineArg.

   -------------------------------------------------------------------------
   История:
   - 30:7:2004   17:43 : Created by Márcio Martins

*************************************************************************/
#ifndef __CMDLINEARG_H__
#define __CMDLINEARG_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/Sys/ICmdLine.h>

class CCmdLineArg :
	public ICmdLineArg
{
public:
	CCmdLineArg(tukk name, tukk value, ECmdLineArgType type);
	virtual ~CCmdLineArg();

	tukk           GetName() const;
	tukk           GetValue() const;
	const ECmdLineArgType GetType() const;
	const float           GetFValue() const;
	i32k             GetIValue() const;

private:

	ECmdLineArgType m_type;
	string          m_name;
	string          m_value;
};

#endif //__CMDLINEARG_H__
