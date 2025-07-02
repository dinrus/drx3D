
#ifndef DRX3D_COLLISION_MARGIN_H
#define DRX3D_COLLISION_MARGIN_H

///The CONVEX_DISTANCE_MARGIN is a default collision margin for convex collision shapes derived from ConvexInternalShape.
///This collision margin is used by Gjk and some other algorithms
///Note that when creating small objects, you need to make sure to set a smaller collision margin, using the 'setMargin' API
#define CONVEX_DISTANCE_MARGIN Scalar(0.04)  // Scalar(0.1)//;/Scalar(0.01)

#endif  //DRX3D_COLLISION_MARGIN_H
