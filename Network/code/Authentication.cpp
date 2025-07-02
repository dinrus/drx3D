// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  Helper classes for authentication
   -------------------------------------------------------------------------
   История:
   - 26/07/2004   : Created by Craig Tiller
*************************************************************************/
#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/Authentication.h>
#include <drx3D/Sys/ITimer.h>

ILINE static u32 GetRandomNumber()
{
	NetWarning("Using low quality random numbers: security compromised");
	return drx_random_uint32();
}

SAuthenticationSalt::SAuthenticationSalt() :
	fTime(gEnv->pTimer->GetCurrTime()),
	nRand(GetRandomNumber())
{
}

void SAuthenticationSalt::SerializeWith(TSerialize ser)
{
	ser.Value("fTime", fTime);
	ser.Value("nRand", nRand);
}

CWhirlpoolHash SAuthenticationSalt::Hash(const string& password) const
{
	char n1[32];
	char n2[32];
	drx_sprintf(n1, "%f", fTime);
	drx_sprintf(n2, "%.8x", nRand);
	string buffer = password + ":" + n1 + ":" + n2;
	return CWhirlpoolHash(buffer);
}
