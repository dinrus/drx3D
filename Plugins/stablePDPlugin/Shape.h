#pragma once

#include <drxtypes.h>
#include <string>

class cShape
{
public:
	enum eShape
	{
		eShapeNull,
		eShapeBox,
		eShapeCapsule,
		eShapeSphere,
		eShapeCylinder,
		eShapePlane,
		eShapeMax,
	};

	static bool ParseShape(const STxt& str, cShape::eShape& out_shape);
};