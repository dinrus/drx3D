// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:
   Allows creation of text mode displays the for dedicated server
   -------------------------------------------------------------------------
   История:
   - Nov 22,2006:	Created by Craig Tiller

*************************************************************************/

#pragma once

struct ITextModeConsole
{
	// <interfuscator:shuffle>
	virtual ~ITextModeConsole() {}
	virtual Vec2_tpl<i32> BeginDraw() = 0;
	virtual void          PutText(i32 x, i32 y, tukk msg) = 0;
	virtual void          EndDraw() = 0;
	virtual void          OnShutdown() = 0;

	virtual void          SetTitle(tukk title)    {}
	virtual void          SetHeader(tukk pHeader) {}
	// </interfuscator:shuffle>
};
