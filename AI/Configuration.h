// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   Configuration.h
//  Created:     02/08/2008 by Matthew
//  Описание: Simple struct for storing fundamental AI system settings
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __AICONFIGURATION
#define __AICONFIGURATION

#pragma once

// These values should not change and are carefully chosen but arbitrary
enum EConfigCompatabilityMode
{
	ECCM_NONE    = 0,
	ECCM_DRXSIS  = 2,
	ECCM_GAME04  = 4,
	ECCM_WARFACE = 7,
	ECCM_DRXSIS2 = 8
};

struct SConfiguration
{
	SConfiguration()
		: eCompatibilityMode(ECCM_NONE)
	{
	}

	EConfigCompatabilityMode eCompatibilityMode;

	// Should probably include logging and debugging flags
};

#endif // __AICONFIGURATION
