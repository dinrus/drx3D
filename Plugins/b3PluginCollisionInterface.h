#ifndef D3_PLUGIN_COLLISION_INTERFACE_H
#define D3_PLUGIN_COLLISION_INTERFACE_H

enum b3PluginCollisionFilterModes
{
	D3_FILTER_GROUPAMASKB_AND_GROUPBMASKA = 0,
	D3_FILTER_GROUPAMASKB_OR_GROUPBMASKA
};

struct b3PluginCollisionInterface
{
	virtual void setBroadphaseCollisionFilter(
		i32 objectUniqueIdA, i32 objectUniqueIdB,
		i32 linkIndexA, i32 linkIndexB,
		bool enableCollision) = 0;

	virtual void removeBroadphaseCollisionFilter(
		i32 objectUniqueIdA, i32 objectUniqueIdB,
		i32 linkIndexA, i32 linkIndexB) = 0;

	virtual i32 getNumRules() const = 0;

	virtual void resetAll() = 0;

	virtual i32 needsBroadphaseCollision(i32 objectUniqueIdA, i32 linkIndexA,
										 i32 collisionFilterGroupA, i32 collisionFilterMaskA,
										 i32 objectUniqueIdB, i32 linkIndexB,
										 i32 collisionFilterGroupB, i32 collisionFilterMaskB,
										 i32 filterMode) = 0;
};

#endif  //D3_PLUGIN_COLLISION_INTERFACE_H