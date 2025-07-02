#ifndef DRX3D_IDEBUG_DRAW__H
#define DRX3D_IDEBUG_DRAW__H

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/Transform2.h>

///The IDebugDraw interface class allows hooking up a debug renderer to visually debug simulations.
///Typical use case: create a debug drawer object, and assign it to a CollisionWorld or DynamicsWorld using setDebugDrawer and call debugDrawWorld.
///A class that implements the IDebugDraw interface will need to provide non-empty implementations of the the drawLine and getDebugMode methods at a minimum.
///For color arguments the X,Y,Z components refer to Red, Green and Blue each in the range [0..1]
class IDebugDraw
{
public:
	ATTRIBUTE_ALIGNED16(struct)
	DefaultColors
	{
		Vec3 m_activeObject;
		Vec3 m_deactivatedObject;
		Vec3 m_wantsDeactivationObject;
		Vec3 m_disabledDeactivationObject;
		Vec3 m_disabledSimulationObject;
		Vec3 m_aabb;
		Vec3 m_contactPoint;

		DefaultColors()
			: m_activeObject(1, 1, 1),
			  m_deactivatedObject(0, 1, 0),
			  m_wantsDeactivationObject(0, 1, 1),
			  m_disabledDeactivationObject(1, 0, 0),
			  m_disabledSimulationObject(1, 1, 0),
			  m_aabb(1, 0, 0),
			  m_contactPoint(1, 1, 0)
		{
		}
	};

	enum DebugDrawModes
	{
		DBG_NoDebug = 0,
		DBG_DrawWireframe = 1,
		DBG_DrawAabb = 2,
		DBG_DrawFeaturesText = 4,
		DBG_DrawContactPoints = 8,
		DBG_NoDeactivation = 16,
		DBG_NoHelpText = 32,
		DBG_DrawText = 64,
		DBG_ProfileTimings = 128,
		DBG_EnableSatComparison = 256,
		DBG_DisableBulletLCP = 512,
		DBG_EnableCCD = 1024,
		DBG_DrawConstraints = (1 << 11),
		DBG_DrawConstraintLimits = (1 << 12),
		DBG_FastWireframe = (1 << 13),
		DBG_DrawNormals = (1 << 14),
		DBG_DrawFrames = (1 << 15),
		DBG_MAX_DEBUG_DRAW_MODE
	};

	virtual ~IDebugDraw(){};

	virtual DefaultColors getDefaultColors() const
	{
		DefaultColors colors;
		return colors;
	}
	///the default implementation for setDefaultColors has no effect. A derived class can implement it and store the colors.
	virtual void setDefaultColors(const DefaultColors& /*colors*/) {}

	virtual void drawLine(const Vec3& from, const Vec3& to, const Vec3& color) = 0;

	virtual void drawLine(const Vec3& from, const Vec3& to, const Vec3& fromColor, const Vec3& toColor)
	{
		(void)toColor;
		drawLine(from, to, fromColor);
	}

	virtual void drawSphere(Scalar radius, const Transform2& transform, const Vec3& color)
	{
		Vec3 center = transform.getOrigin();
		Vec3 up = transform.getBasis().getColumn(1);
		Vec3 axis = transform.getBasis().getColumn(0);
		Scalar minTh = -SIMD_HALF_PI;
		Scalar maxTh = SIMD_HALF_PI;
		Scalar minPs = -SIMD_HALF_PI;
		Scalar maxPs = SIMD_HALF_PI;
		Scalar stepDegrees = 30.f;
		drawSpherePatch(center, up, axis, radius, minTh, maxTh, minPs, maxPs, color, stepDegrees, false);
		drawSpherePatch(center, up, -axis, radius, minTh, maxTh, minPs, maxPs, color, stepDegrees, false);
	}

	virtual void drawSphere(const Vec3& p, Scalar radius, const Vec3& color)
	{
		Transform2 tr;
		tr.setIdentity();
		tr.setOrigin(p);
		drawSphere(radius, tr, color);
	}

	virtual void drawTriangle(const Vec3& v0, const Vec3& v1, const Vec3& v2, const Vec3& /*n0*/, const Vec3& /*n1*/, const Vec3& /*n2*/, const Vec3& color, Scalar alpha)
	{
		drawTriangle(v0, v1, v2, color, alpha);
	}
	virtual void drawTriangle(const Vec3& v0, const Vec3& v1, const Vec3& v2, const Vec3& color, Scalar /*alpha*/)
	{
		drawLine(v0, v1, color);
		drawLine(v1, v2, color);
		drawLine(v2, v0, color);
	}

	virtual void drawContactPoint(const Vec3& PointOnB, const Vec3& normalOnB, Scalar distance, i32 lifeTime, const Vec3& color) = 0;

	virtual void reportErrorWarning(tukk warningString) = 0;

	virtual void draw3dText(const Vec3& location, tukk textString) = 0;

	virtual void setDebugMode(i32 debugMode) = 0;

	virtual i32 getDebugMode() const = 0;

	virtual void drawAabb(const Vec3& from, const Vec3& to, const Vec3& color)
	{
		Vec3 halfExtents = (to - from) * 0.5f;
		Vec3 center = (to + from) * 0.5f;
		i32 i, j;

		Vec3 edgecoord(1.f, 1.f, 1.f), pa, pb;
		for (i = 0; i < 4; i++)
		{
			for (j = 0; j < 3; j++)
			{
				pa = Vec3(edgecoord[0] * halfExtents[0], edgecoord[1] * halfExtents[1],
							   edgecoord[2] * halfExtents[2]);
				pa += center;

				i32 othercoord = j % 3;
				edgecoord[othercoord] *= -1.f;
				pb = Vec3(edgecoord[0] * halfExtents[0], edgecoord[1] * halfExtents[1],
							   edgecoord[2] * halfExtents[2]);
				pb += center;

				drawLine(pa, pb, color);
			}
			edgecoord = Vec3(-1.f, -1.f, -1.f);
			if (i < 3)
				edgecoord[i] *= -1.f;
		}
	}
	virtual void drawTransform2(const Transform2& transform, Scalar orthoLen)
	{
		Vec3 start = transform.getOrigin();
		drawLine(start, start + transform.getBasis() * Vec3(orthoLen, 0, 0), Vec3(Scalar(1.), Scalar(0.3), Scalar(0.3)));
		drawLine(start, start + transform.getBasis() * Vec3(0, orthoLen, 0), Vec3(Scalar(0.3), Scalar(1.), Scalar(0.3)));
		drawLine(start, start + transform.getBasis() * Vec3(0, 0, orthoLen), Vec3(Scalar(0.3), Scalar(0.3), Scalar(1.)));
	}

	virtual void drawArc(const Vec3& center, const Vec3& normal, const Vec3& axis, Scalar radiusA, Scalar radiusB, Scalar minAngle, Scalar maxAngle,
						 const Vec3& color, bool drawSect, Scalar stepDegrees = Scalar(10.f))
	{
		const Vec3& vx = axis;
		Vec3 vy = normal.cross(axis);
		Scalar step = stepDegrees * SIMD_RADS_PER_DEG;
		i32 nSteps = (i32)Fabs((maxAngle - minAngle) / step);
		if (!nSteps) nSteps = 1;
		Vec3 prev = center + radiusA * vx * Cos(minAngle) + radiusB * vy * Sin(minAngle);
		if (drawSect)
		{
			drawLine(center, prev, color);
		}
		for (i32 i = 1; i <= nSteps; i++)
		{
			Scalar angle = minAngle + (maxAngle - minAngle) * Scalar(i) / Scalar(nSteps);
			Vec3 next = center + radiusA * vx * Cos(angle) + radiusB * vy * Sin(angle);
			drawLine(prev, next, color);
			prev = next;
		}
		if (drawSect)
		{
			drawLine(center, prev, color);
		}
	}
	virtual void drawSpherePatch(const Vec3& center, const Vec3& up, const Vec3& axis, Scalar radius,
								 Scalar minTh, Scalar maxTh, Scalar minPs, Scalar maxPs, const Vec3& color, Scalar stepDegrees = Scalar(10.f), bool drawCenter = true)
	{
		Vec3 vA[74];
		Vec3 vB[74];
		Vec3 *pvA = vA, *pvB = vB, *pT;
		Vec3 npole = center + up * radius;
		Vec3 spole = center - up * radius;
		Vec3 arcStart;
		Scalar step = stepDegrees * SIMD_RADS_PER_DEG;
		const Vec3& kv = up;
		const Vec3& iv = axis;
		Vec3 jv = kv.cross(iv);
		bool drawN = false;
		bool drawS = false;
		if (minTh <= -SIMD_HALF_PI)
		{
			minTh = -SIMD_HALF_PI + step;
			drawN = true;
		}
		if (maxTh >= SIMD_HALF_PI)
		{
			maxTh = SIMD_HALF_PI - step;
			drawS = true;
		}
		if (minTh > maxTh)
		{
			minTh = -SIMD_HALF_PI + step;
			maxTh = SIMD_HALF_PI - step;
			drawN = drawS = true;
		}
		i32 n_hor = (i32)((maxTh - minTh) / step) + 1;
		if (n_hor < 2) n_hor = 2;
		Scalar step_h = (maxTh - minTh) / Scalar(n_hor - 1);
		bool isClosed = false;
		if (minPs > maxPs)
		{
			minPs = -SIMD_PI + step;
			maxPs = SIMD_PI;
			isClosed = true;
		}
		else if ((maxPs - minPs) >= SIMD_PI * Scalar(2.f))
		{
			isClosed = true;
		}
		else
		{
			isClosed = false;
		}
		i32 n_vert = (i32)((maxPs - minPs) / step) + 1;
		if (n_vert < 2) n_vert = 2;
		Scalar step_v = (maxPs - minPs) / Scalar(n_vert - 1);
		for (i32 i = 0; i < n_hor; i++)
		{
			Scalar th = minTh + Scalar(i) * step_h;
			Scalar sth = radius * Sin(th);
			Scalar cth = radius * Cos(th);
			for (i32 j = 0; j < n_vert; j++)
			{
				Scalar psi = minPs + Scalar(j) * step_v;
				Scalar sps = Sin(psi);
				Scalar cps = Cos(psi);
				pvB[j] = center + cth * cps * iv + cth * sps * jv + sth * kv;
				if (i)
				{
					drawLine(pvA[j], pvB[j], color);
				}
				else if (drawS)
				{
					drawLine(spole, pvB[j], color);
				}
				if (j)
				{
					drawLine(pvB[j - 1], pvB[j], color);
				}
				else
				{
					arcStart = pvB[j];
				}
				if ((i == (n_hor - 1)) && drawN)
				{
					drawLine(npole, pvB[j], color);
				}

				if (drawCenter)
				{
					if (isClosed)
					{
						if (j == (n_vert - 1))
						{
							drawLine(arcStart, pvB[j], color);
						}
					}
					else
					{
						if (((!i) || (i == (n_hor - 1))) && ((!j) || (j == (n_vert - 1))))
						{
							drawLine(center, pvB[j], color);
						}
					}
				}
			}
			pT = pvA;
			pvA = pvB;
			pvB = pT;
		}
	}

	virtual void drawBox(const Vec3& bbMin, const Vec3& bbMax, const Vec3& color)
	{
		drawLine(Vec3(bbMin[0], bbMin[1], bbMin[2]), Vec3(bbMax[0], bbMin[1], bbMin[2]), color);
		drawLine(Vec3(bbMax[0], bbMin[1], bbMin[2]), Vec3(bbMax[0], bbMax[1], bbMin[2]), color);
		drawLine(Vec3(bbMax[0], bbMax[1], bbMin[2]), Vec3(bbMin[0], bbMax[1], bbMin[2]), color);
		drawLine(Vec3(bbMin[0], bbMax[1], bbMin[2]), Vec3(bbMin[0], bbMin[1], bbMin[2]), color);
		drawLine(Vec3(bbMin[0], bbMin[1], bbMin[2]), Vec3(bbMin[0], bbMin[1], bbMax[2]), color);
		drawLine(Vec3(bbMax[0], bbMin[1], bbMin[2]), Vec3(bbMax[0], bbMin[1], bbMax[2]), color);
		drawLine(Vec3(bbMax[0], bbMax[1], bbMin[2]), Vec3(bbMax[0], bbMax[1], bbMax[2]), color);
		drawLine(Vec3(bbMin[0], bbMax[1], bbMin[2]), Vec3(bbMin[0], bbMax[1], bbMax[2]), color);
		drawLine(Vec3(bbMin[0], bbMin[1], bbMax[2]), Vec3(bbMax[0], bbMin[1], bbMax[2]), color);
		drawLine(Vec3(bbMax[0], bbMin[1], bbMax[2]), Vec3(bbMax[0], bbMax[1], bbMax[2]), color);
		drawLine(Vec3(bbMax[0], bbMax[1], bbMax[2]), Vec3(bbMin[0], bbMax[1], bbMax[2]), color);
		drawLine(Vec3(bbMin[0], bbMax[1], bbMax[2]), Vec3(bbMin[0], bbMin[1], bbMax[2]), color);
	}
	virtual void drawBox(const Vec3& bbMin, const Vec3& bbMax, const Transform2& trans, const Vec3& color)
	{
		drawLine(trans * Vec3(bbMin[0], bbMin[1], bbMin[2]), trans * Vec3(bbMax[0], bbMin[1], bbMin[2]), color);
		drawLine(trans * Vec3(bbMax[0], bbMin[1], bbMin[2]), trans * Vec3(bbMax[0], bbMax[1], bbMin[2]), color);
		drawLine(trans * Vec3(bbMax[0], bbMax[1], bbMin[2]), trans * Vec3(bbMin[0], bbMax[1], bbMin[2]), color);
		drawLine(trans * Vec3(bbMin[0], bbMax[1], bbMin[2]), trans * Vec3(bbMin[0], bbMin[1], bbMin[2]), color);
		drawLine(trans * Vec3(bbMin[0], bbMin[1], bbMin[2]), trans * Vec3(bbMin[0], bbMin[1], bbMax[2]), color);
		drawLine(trans * Vec3(bbMax[0], bbMin[1], bbMin[2]), trans * Vec3(bbMax[0], bbMin[1], bbMax[2]), color);
		drawLine(trans * Vec3(bbMax[0], bbMax[1], bbMin[2]), trans * Vec3(bbMax[0], bbMax[1], bbMax[2]), color);
		drawLine(trans * Vec3(bbMin[0], bbMax[1], bbMin[2]), trans * Vec3(bbMin[0], bbMax[1], bbMax[2]), color);
		drawLine(trans * Vec3(bbMin[0], bbMin[1], bbMax[2]), trans * Vec3(bbMax[0], bbMin[1], bbMax[2]), color);
		drawLine(trans * Vec3(bbMax[0], bbMin[1], bbMax[2]), trans * Vec3(bbMax[0], bbMax[1], bbMax[2]), color);
		drawLine(trans * Vec3(bbMax[0], bbMax[1], bbMax[2]), trans * Vec3(bbMin[0], bbMax[1], bbMax[2]), color);
		drawLine(trans * Vec3(bbMin[0], bbMax[1], bbMax[2]), trans * Vec3(bbMin[0], bbMin[1], bbMax[2]), color);
	}

	virtual void drawCapsule(Scalar radius, Scalar halfHeight, i32 upAxis, const Transform2& transform, const Vec3& color)
	{
		i32 stepDegrees = 30;

		Vec3 capStart(0.f, 0.f, 0.f);
		capStart[upAxis] = -halfHeight;

		Vec3 capEnd(0.f, 0.f, 0.f);
		capEnd[upAxis] = halfHeight;

		// Draw the ends
		{
			Transform2 childTransform2 = transform;
			childTransform2.getOrigin() = transform * capStart;
			{
				Vec3 center = childTransform2.getOrigin();
				Vec3 up = childTransform2.getBasis().getColumn((upAxis + 1) % 3);
				Vec3 axis = -childTransform2.getBasis().getColumn(upAxis);
				Scalar minTh = -SIMD_HALF_PI;
				Scalar maxTh = SIMD_HALF_PI;
				Scalar minPs = -SIMD_HALF_PI;
				Scalar maxPs = SIMD_HALF_PI;

				drawSpherePatch(center, up, axis, radius, minTh, maxTh, minPs, maxPs, color, Scalar(stepDegrees), false);
			}
		}

		{
			Transform2 childTransform2 = transform;
			childTransform2.getOrigin() = transform * capEnd;
			{
				Vec3 center = childTransform2.getOrigin();
				Vec3 up = childTransform2.getBasis().getColumn((upAxis + 1) % 3);
				Vec3 axis = childTransform2.getBasis().getColumn(upAxis);
				Scalar minTh = -SIMD_HALF_PI;
				Scalar maxTh = SIMD_HALF_PI;
				Scalar minPs = -SIMD_HALF_PI;
				Scalar maxPs = SIMD_HALF_PI;
				drawSpherePatch(center, up, axis, radius, minTh, maxTh, minPs, maxPs, color, Scalar(stepDegrees), false);
			}
		}

		// Draw some additional lines
		Vec3 start = transform.getOrigin();

		for (i32 i = 0; i < 360; i += stepDegrees)
		{
			capEnd[(upAxis + 1) % 3] = capStart[(upAxis + 1) % 3] = Sin(Scalar(i) * SIMD_RADS_PER_DEG) * radius;
			capEnd[(upAxis + 2) % 3] = capStart[(upAxis + 2) % 3] = Cos(Scalar(i) * SIMD_RADS_PER_DEG) * radius;
			drawLine(start + transform.getBasis() * capStart, start + transform.getBasis() * capEnd, color);
		}
	}

	virtual void drawCylinder(Scalar radius, Scalar halfHeight, i32 upAxis, const Transform2& transform, const Vec3& color)
	{
		Vec3 start = transform.getOrigin();
		Vec3 offsetHeight(0, 0, 0);
		offsetHeight[upAxis] = halfHeight;
		i32 stepDegrees = 30;
		Vec3 capStart(0.f, 0.f, 0.f);
		capStart[upAxis] = -halfHeight;
		Vec3 capEnd(0.f, 0.f, 0.f);
		capEnd[upAxis] = halfHeight;

		for (i32 i = 0; i < 360; i += stepDegrees)
		{
			capEnd[(upAxis + 1) % 3] = capStart[(upAxis + 1) % 3] = Sin(Scalar(i) * SIMD_RADS_PER_DEG) * radius;
			capEnd[(upAxis + 2) % 3] = capStart[(upAxis + 2) % 3] = Cos(Scalar(i) * SIMD_RADS_PER_DEG) * radius;
			drawLine(start + transform.getBasis() * capStart, start + transform.getBasis() * capEnd, color);
		}
		// Drawing top and bottom caps of the cylinder
		Vec3 yaxis(0, 0, 0);
		yaxis[upAxis] = Scalar(1.0);
		Vec3 xaxis(0, 0, 0);
		xaxis[(upAxis + 1) % 3] = Scalar(1.0);
		drawArc(start - transform.getBasis() * (offsetHeight), transform.getBasis() * yaxis, transform.getBasis() * xaxis, radius, radius, 0, SIMD_2_PI, color, false, Scalar(10.0));
		drawArc(start + transform.getBasis() * (offsetHeight), transform.getBasis() * yaxis, transform.getBasis() * xaxis, radius, radius, 0, SIMD_2_PI, color, false, Scalar(10.0));
	}

	virtual void drawCone(Scalar radius, Scalar height, i32 upAxis, const Transform2& transform, const Vec3& color)
	{
		i32 stepDegrees = 30;
		Vec3 start = transform.getOrigin();

		Vec3 offsetHeight(0, 0, 0);
		Scalar halfHeight = height * Scalar(0.5);
		offsetHeight[upAxis] = halfHeight;
		Vec3 offsetRadius(0, 0, 0);
		offsetRadius[(upAxis + 1) % 3] = radius;
		Vec3 offset2Radius(0, 0, 0);
		offset2Radius[(upAxis + 2) % 3] = radius;

		Vec3 capEnd(0.f, 0.f, 0.f);
		capEnd[upAxis] = -halfHeight;

		for (i32 i = 0; i < 360; i += stepDegrees)
		{
			capEnd[(upAxis + 1) % 3] = Sin(Scalar(i) * SIMD_RADS_PER_DEG) * radius;
			capEnd[(upAxis + 2) % 3] = Cos(Scalar(i) * SIMD_RADS_PER_DEG) * radius;
			drawLine(start + transform.getBasis() * (offsetHeight), start + transform.getBasis() * capEnd, color);
		}

		drawLine(start + transform.getBasis() * (offsetHeight), start + transform.getBasis() * (-offsetHeight + offsetRadius), color);
		drawLine(start + transform.getBasis() * (offsetHeight), start + transform.getBasis() * (-offsetHeight - offsetRadius), color);
		drawLine(start + transform.getBasis() * (offsetHeight), start + transform.getBasis() * (-offsetHeight + offset2Radius), color);
		drawLine(start + transform.getBasis() * (offsetHeight), start + transform.getBasis() * (-offsetHeight - offset2Radius), color);

		// Drawing the base of the cone
		Vec3 yaxis(0, 0, 0);
		yaxis[upAxis] = Scalar(1.0);
		Vec3 xaxis(0, 0, 0);
		xaxis[(upAxis + 1) % 3] = Scalar(1.0);
		drawArc(start - transform.getBasis() * (offsetHeight), transform.getBasis() * yaxis, transform.getBasis() * xaxis, radius, radius, 0, SIMD_2_PI, color, false, 10.0);
	}

	virtual void drawPlane(const Vec3& planeNormal, Scalar planeConst, const Transform2& transform, const Vec3& color)
	{
		Vec3 planeOrigin = planeNormal * planeConst;
		Vec3 vec0, vec1;
		PlaneSpace1(planeNormal, vec0, vec1);
		Scalar vecLen = 100.f;
		Vec3 pt0 = planeOrigin + vec0 * vecLen;
		Vec3 pt1 = planeOrigin - vec0 * vecLen;
		Vec3 pt2 = planeOrigin + vec1 * vecLen;
		Vec3 pt3 = planeOrigin - vec1 * vecLen;
		drawLine(transform * pt0, transform * pt1, color);
		drawLine(transform * pt2, transform * pt3, color);
	}

	virtual void clearLines()
	{
	}

	virtual void flushLines()
	{
	}
};

#endif  //DRX3D_IDEBUG_DRAW__H
