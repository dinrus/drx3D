// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   IBreakableGlassSystem.h
//  Version:     v1.00
//  Created:     11/11/2011 by ChrisBu.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   Visual Studio 2010
//  Описание: Interface to the Breakable Glass System
////////////////////////////////////////////////////////////////////////////

#ifndef __IBREAKABLEGLASSSYSTEM_H__
#define __IBREAKABLEGLASSSYSTEM_H__
#pragma once

struct EventPhysCollision;

struct IBreakableGlassSystem
{
	virtual ~IBreakableGlassSystem() {}

	virtual void Update(const float frameTime) = 0;
	virtual bool BreakGlassObject(const EventPhysCollision& physEvent, const bool forceGlassBreak = false) = 0;
	virtual void ResetAll() = 0;
	virtual void GetMemoryUsage(IDrxSizer* pSizer) const = 0;
};

#endif // __IBREAKABLEGLASSSYSTEM_H__
