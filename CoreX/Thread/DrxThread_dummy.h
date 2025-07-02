// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

//////////////////////////////////////////////////////////////////////////
DrxEvent::DrxEvent() {}
DrxEvent::~DrxEvent() {}
void DrxEvent::Reset()                                {}
void DrxEvent::Set()                                  {}
void DrxEvent::Wait() const                           {}
bool DrxEvent::Wait(u32k timeoutMillis) const {}
typedef DrxEvent DrxEventTimed;

//////////////////////////////////////////////////////////////////////////
class _DummyLock
{
public:
	_DummyLock();

	void Lock();
	bool TryLock();
	void Unlock();

#ifndef _RELEASE
	bool IsLocked();
#endif
};

template<>
class DrxLock<DRXLOCK_FAST>
	: public _DummyLock
{
	DrxLock(const DrxLock<DRXLOCK_FAST>&);
	void operator=(const DrxLock<DRXLOCK_FAST>&);

public:
	DrxLock();
};

template<>
class DrxLock<DRXLOCK_RECURSIVE>
	: public _DummyLock
{
	DrxLock(const DrxLock<DRXLOCK_RECURSIVE>&);
	void operator=(const DrxLock<DRXLOCK_RECURSIVE>&);

public:
	DrxLock();
};

template<>
class DrxCondLock<DRXLOCK_FAST>
	: public DrxLock<DRXLOCK_FAST>
{
};

template<>
class DrxCondLock<DRXLOCK_RECURSIVE>
	: public DrxLock<DRXLOCK_FAST>
{
};

template<>
class DrxCond<DrxLock<DRXLOCK_FAST>>
{
	typedef DrxLock<DRXLOCK_FAST> LockT;
	DrxCond(const DrxCond<LockT>&);
	void operator=(const DrxCond<LockT>&);

public:
	DrxCond();

	void Notify();
	void NotifySingle();
	void Wait(LockT&);
	bool TimedWait(LockT &, u32);
};

template<>
class DrxCond<DrxLock<DRXLOCK_RECURSIVE>>
{
	typedef DrxLock<DRXLOCK_RECURSIVE> LockT;
	DrxCond(const DrxCond<LockT>&);
	void operator=(const DrxCond<LockT>&);

public:
	DrxCond();

	void Notify();
	void NotifySingle();
	void Wait(LockT&);
	bool TimedWait(LockT &, u32);
};

class _DummyRWLock
{
public:
	_DummyRWLock() {}

	void RLock();
	bool TryRLock();
	void WLock();
	bool TryWLock();
	void Lock()    { WLock(); }
	bool TryLock() { return TryWLock(); }
	void Unlock();
};
