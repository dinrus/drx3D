#include <drx3D/Geometry/b3GeometryUtil.h>

/*
  Make sure this dummy function never changes so that it
  can be used by probes that are checking whether the
  library is actually installed.
*/
extern "C"
{
	void b3BulletMathProbe();

	void b3BulletMathProbe() {}
}

bool b3GeometryUtil::isPointInsidePlanes(const b3AlignedObjectArray<b3Vec3>& planeEquations, const b3Vec3& point, b3Scalar margin)
{
	i32 numbrushes = planeEquations.size();
	for (i32 i = 0; i < numbrushes; i++)
	{
		const b3Vec3& N1 = planeEquations[i];
		b3Scalar dist = b3Scalar(N1.dot(point)) + b3Scalar(N1[3]) - margin;
		if (dist > b3Scalar(0.))
		{
			return false;
		}
	}
	return true;
}

bool b3GeometryUtil::areVerticesBehindPlane(const b3Vec3& planeNormal, const b3AlignedObjectArray<b3Vec3>& vertices, b3Scalar margin)
{
	i32 numvertices = vertices.size();
	for (i32 i = 0; i < numvertices; i++)
	{
		const b3Vec3& N1 = vertices[i];
		b3Scalar dist = b3Scalar(planeNormal.dot(N1)) + b3Scalar(planeNormal[3]) - margin;
		if (dist > b3Scalar(0.))
		{
			return false;
		}
	}
	return true;
}

bool notExist(const b3Vec3& planeEquation, const b3AlignedObjectArray<b3Vec3>& planeEquations);

bool notExist(const b3Vec3& planeEquation, const b3AlignedObjectArray<b3Vec3>& planeEquations)
{
	i32 numbrushes = planeEquations.size();
	for (i32 i = 0; i < numbrushes; i++)
	{
		const b3Vec3& N1 = planeEquations[i];
		if (planeEquation.dot(N1) > b3Scalar(0.999))
		{
			return false;
		}
	}
	return true;
}

void b3GeometryUtil::getPlaneEquationsFromVertices(b3AlignedObjectArray<b3Vec3>& vertices, b3AlignedObjectArray<b3Vec3>& planeEquationsOut)
{
	i32k numvertices = vertices.size();
	// brute force:
	for (i32 i = 0; i < numvertices; i++)
	{
		const b3Vec3& N1 = vertices[i];

		for (i32 j = i + 1; j < numvertices; j++)
		{
			const b3Vec3& N2 = vertices[j];

			for (i32 k = j + 1; k < numvertices; k++)
			{
				const b3Vec3& N3 = vertices[k];

				b3Vec3 planeEquation, edge0, edge1;
				edge0 = N2 - N1;
				edge1 = N3 - N1;
				b3Scalar normalSign = b3Scalar(1.);
				for (i32 ww = 0; ww < 2; ww++)
				{
					planeEquation = normalSign * edge0.cross(edge1);
					if (planeEquation.length2() > b3Scalar(0.0001))
					{
						planeEquation.normalize();
						if (notExist(planeEquation, planeEquationsOut))
						{
							planeEquation[3] = -planeEquation.dot(N1);

							//check if inside, and replace supportingVertexOut if needed
							if (areVerticesBehindPlane(planeEquation, vertices, b3Scalar(0.01)))
							{
								planeEquationsOut.push_back(planeEquation);
							}
						}
					}
					normalSign = b3Scalar(-1.);
				}
			}
		}
	}
}

void b3GeometryUtil::getVerticesFromPlaneEquations(const b3AlignedObjectArray<b3Vec3>& planeEquations, b3AlignedObjectArray<b3Vec3>& verticesOut)
{
	i32k numbrushes = planeEquations.size();
	// brute force:
	for (i32 i = 0; i < numbrushes; i++)
	{
		const b3Vec3& N1 = planeEquations[i];

		for (i32 j = i + 1; j < numbrushes; j++)
		{
			const b3Vec3& N2 = planeEquations[j];

			for (i32 k = j + 1; k < numbrushes; k++)
			{
				const b3Vec3& N3 = planeEquations[k];

				b3Vec3 n2n3;
				n2n3 = N2.cross(N3);
				b3Vec3 n3n1;
				n3n1 = N3.cross(N1);
				b3Vec3 n1n2;
				n1n2 = N1.cross(N2);

				if ((n2n3.length2() > b3Scalar(0.0001)) &&
					(n3n1.length2() > b3Scalar(0.0001)) &&
					(n1n2.length2() > b3Scalar(0.0001)))
				{
					//point P out of 3 plane equations:

					//	d1 ( N2 * N3 ) + d2 ( N3 * N1 ) + d3 ( N1 * N2 )
					//P =  -------------------------------------------------------------------------
					//   N1 . ( N2 * N3 )

					b3Scalar quotient = (N1.dot(n2n3));
					if (b3Fabs(quotient) > b3Scalar(0.000001))
					{
						quotient = b3Scalar(-1.) / quotient;
						n2n3 *= N1[3];
						n3n1 *= N2[3];
						n1n2 *= N3[3];
						b3Vec3 potentialVertex = n2n3;
						potentialVertex += n3n1;
						potentialVertex += n1n2;
						potentialVertex *= quotient;

						//check if inside, and replace supportingVertexOut if needed
						if (isPointInsidePlanes(planeEquations, potentialVertex, b3Scalar(0.01)))
						{
							verticesOut.push_back(potentialVertex);
						}
					}
				}
			}
		}
	}
}
