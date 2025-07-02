// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class EDITOR_COMMON_API CScopedRenderLock
{
public:
	CScopedRenderLock();
	~CScopedRenderLock();
	explicit operator bool();

	static void Lock();
	static void Unlock();

private:
	bool m_bOwnsLock;

	static i32 s_renderLock;
};

