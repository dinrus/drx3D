#ifndef OVERLAPPING_PAIR_CALLBACK_H
#define OVERLAPPING_PAIR_CALLBACK_H

class Dispatcher;
struct BroadphasePair;

///The OverlappingPairCallback class is an additional optional broadphase user callback for adding/removing overlapping pairs, similar interface to OverlappingPairCache.
class OverlappingPairCallback
{
protected:
	OverlappingPairCallback() {}

public:
	virtual ~OverlappingPairCallback()
	{
	}

	virtual BroadphasePair* addOverlappingPair(BroadphaseProxy* proxy0, BroadphaseProxy* proxy1) = 0;

	virtual uk removeOverlappingPair(BroadphaseProxy* proxy0, BroadphaseProxy* proxy1, Dispatcher* dispatcher) = 0;

	virtual void removeOverlappingPairsContainingProxy(BroadphaseProxy* proxy0, Dispatcher* dispatcher) = 0;
};

#endif  //OVERLAPPING_PAIR_CALLBACK_H
