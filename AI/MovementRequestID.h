// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

//! When you queue a movement request you will get a request id back.
//! This ID is used when you want to cancel a queued request.
//! (Wrapper around 'u32' but provides type safety.)
//! An ID of 0 represents an invalid/uninitialized ID.
struct MovementRequestID
{
	u32 id;

	MovementRequestID() : id(0) {}
	MovementRequestID(u32 _id) : id(_id) {}
	MovementRequestID& operator++()                           { ++id; return *this; }
	bool               operator==(u32 otherID) const { return id == otherID; }
	operator u32() const { return id; }
	bool IsInvalid() const { return id == 0; }

	static MovementRequestID Invalid() { return MovementRequestID(); }
};

//! \endcond