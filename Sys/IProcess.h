// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//
//	File:IProcess.h
//	Описание: Process common interface
//
//	История:
//	-September 03,2001:Created by Marco Corbetta
//
//////////////////////////////////////////////////////////////////////

#ifndef IPROCESS_H
#define IPROCESS_H

#if _MSC_VER > 1000
	#pragma once
#endif

// forward declaration
struct    SRenderingPassInfo;
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
struct IProcess
{
	// <interfuscator:shuffle>
	virtual ~IProcess(){}
	virtual bool Init() = 0;
	virtual void Update() = 0;
	virtual void RenderWorld(i32k nRenderFlags, const SRenderingPassInfo& passInfo, tukk szDebugName) = 0;
	virtual void ShutDown() = 0;
	virtual void SetFlags(i32 flags) = 0;
	virtual i32  GetFlags(void) = 0;
	// </interfuscator:shuffle>
};

#endif
