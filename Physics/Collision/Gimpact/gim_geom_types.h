#ifndef GIM_GEOM_TYPES_H_INCLUDED
#define GIM_GEOM_TYPES_H_INCLUDED

#include <drx3D/Physics/Collision/Gimpact/gim_math.h>

//! Short Integer vector 2D
typedef GSHORT vec2s[2];
//! Integer vector 3D
typedef GSHORT vec3s[3];
//! Integer vector 4D
typedef GSHORT vec4s[4];

//! Short Integer vector 2D
typedef GUSHORT vec2us[2];
//! Integer vector 3D
typedef GUSHORT vec3us[3];
//! Integer vector 4D
typedef GUSHORT vec4us[4];

//! Integer vector 2D
typedef GINT vec2i[2];
//! Integer vector 3D
typedef GINT vec3i[3];
//! Integer vector 4D
typedef GINT vec4i[4];

//! Unsigned Integer vector 2D
typedef GUINT vec2ui[2];
//! Unsigned Integer vector 3D
typedef GUINT vec3ui[3];
//! Unsigned Integer vector 4D
typedef GUINT vec4ui[4];

//! Float vector 2D
typedef GREAL vec2f[2];
//! Float vector 3D
typedef GREAL vec3f[3];
//! Float vector 4D
typedef GREAL vec4f[4];

//! Double vector 2D
typedef GREAL2 vec2d[2];
//! Float vector 3D
typedef GREAL2 vec3d[3];
//! Float vector 4D
typedef GREAL2 vec4d[4];

//! Matrix 2D, row ordered
typedef GREAL mat2f[2][2];
//! Matrix 3D, row ordered
typedef GREAL mat3f[3][3];
//! Matrix 4D, row ordered
typedef GREAL mat4f[4][4];

//! Quat
typedef GREAL quatf[4];

//typedef struct _aabb3f aabb3f;

#endif  // GIM_GEOM_TYPES_H_INCLUDED
