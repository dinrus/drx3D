// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:	Dummy font implementation (dedicated server)
   -------------------------------------------------------------------------
   История:
   - Jun 30,2006:	Created by Sascha Demetrio

*************************************************************************/

#include <drx3D/Font/StdAfx.h>

#if defined(USE_NULLFONT)

	#include<drx3D/Font/NullFont.h>

CNullFont CDrxNullFont::ms_nullFont;

#endif // USE_NULLFONT
