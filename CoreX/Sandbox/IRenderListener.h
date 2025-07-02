// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

struct DisplayContext;

struct IRenderListener
{
	virtual void Render(DisplayContext& rDisplayContext) = 0;
};


