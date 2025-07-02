// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

struct SRenderContext;
struct SKeyEvent;
struct SMouseEvent;

class QViewportConsumer
{
public:
	virtual void OnViewportRender(const SRenderContext& rc) {}
	virtual void OnViewportKey(const SKeyEvent& ev)         {}
	virtual void OnViewportMouse(const SMouseEvent& ev)     {}
};

