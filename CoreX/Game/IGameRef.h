// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Game/IGame.h>

struct IGameRef
{
	DRX_DEPRECATED_GAME_DLL IGameRef() : m_ptr(0) {}
	IGameRef(IGame** ptr) : m_ptr(ptr) {};
	~IGameRef() {};

	IGame*    operator->() const     { return m_ptr ? *m_ptr : 0; };
	operator IGame*() const { return m_ptr ? *m_ptr : 0; };
	IGameRef& operator=(IGame** ptr) { m_ptr = ptr; return *this; };

private:
	IGame** m_ptr;
};