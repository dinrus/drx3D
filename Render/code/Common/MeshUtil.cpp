// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/RendElements/MeshUtil.h>
#include <drx3D/CoreX/Renderer/VertexFormats.h>
#include <drx3D/CoreX/Math/MTPseudoRandom.h>

using namespace stable_rand;

CMTRand_int32 randGen;
void  stable_rand::setSeed(u32 seed)  { randGen.seed(seed); }
float stable_rand::randUnit()            { return randGen.GenerateFloat() * 2.f - 1.f; }
float stable_rand::randPositive()        { return randGen.GenerateFloat(); }
float stable_rand::randBias(float noise) { return 1.f + randUnit() * noise; }

float computeFade(float current, float start, float end, float fadingLength)
{
	if (current < start - fadingLength)
		return 0;
	else if (current < start)
		return 1 - (start - current) / fadingLength;
	else if (current > end + fadingLength)
		return 0;
	else if (current > end)
		return 1 - (current - end) / fadingLength;
	else
		return 1;
}

Vec2 rotate(float x, float y, float rad)
{
	Vec2 ret;
	float cosRad = cos(rad);
	float sinRad = sin(rad);
	ret.x = x * cosRad - y * sinRad;
	ret.y = y * cosRad + x * sinRad;
	return ret;
}

void MeshUtil::GenDisk(float radius, i32 polySides, i32 ringCount, bool capInnerHole, const ColorF& clr, float* ringPosArray, std::vector<SVF_P3F_C4B_T2F>& vertOut, std::vector<u16>& idxOut)
{
	bool gCollectPosArray = false;
	if (ringPosArray == NULL)
	{
		// create a uniformly spaced setting
		ringPosArray = new float[ringCount];
		float ringCountf = (float)ringCount;
		for (i32 i = 0; i < ringCount; i++)
			ringPosArray[i] = (i + 1) / ringCountf;
		gCollectPosArray = true;
	}

	// Current strategy: whether or not capping, center pivot is always generated only not indexed in later case.
	// UV mapping follows polar representation: u for thou, v for theta
	DWORD dclr = clr.pack_argb8888();
	vertOut.resize(polySides * ringCount + 1);  // rings and the center

	SVF_P3F_C4B_T2F& centerV = vertOut[0];
	centerV.xyz = Vec3(0, 0, 0);
	centerV.st = Vec2(0, 0);
	centerV.color.dcolor = dclr;

	// Generate all vertices starting from inner rings:
	for (i32 i = 0; i < polySides; i++)
	{
		float theta = i / (float)polySides;
		float angle = 2 * PI * theta;
		float x = cos(angle);
		float y = sin(angle);
		//float noise = randUnit() * noiseStrength;

		for (i32 r = 0; r < ringCount; r++)
		{
			float curRingRadius = ringPosArray[r];
			float thou = (r + 1) / (float)(ringCount);
			//curRingRadius += noise; // apply translation noise

			SVF_P3F_C4B_T2F& vert = vertOut[i * ringCount + r + 1];
			vert.xyz.Set(x * curRingRadius, y * curRingRadius, 0);
			vert.st.set(thou, theta);

			vert.color.dcolor = dclr;
		}
	}

	// Rings' Indices;
	i32 holeCapperIdxCount = 0;
	if (capInnerHole)
		holeCapperIdxCount = polySides * 3;
	idxOut.resize(polySides * (ringCount - 1) * 6 + holeCapperIdxCount);

	for (i32 i = 0; i < polySides; i++)
	{
		i32 baseVertIdx0 = i * ringCount;
		for (i32 r = 0; r < ringCount - 1; r++)
		{
			// counter clock wise:
			i32 a = baseVertIdx0 + r;
			i32 b = a + 1;
			i32 c = (a + ringCount + 1) % vertOut.size();
			i32 d = (c - 1) % vertOut.size();

			i32 baseIdx = (i * (ringCount - 1) + r) * 6 + holeCapperIdxCount;
			idxOut[baseIdx] = b;
			idxOut[baseIdx + 1] = a;
			idxOut[baseIdx + 2] = c;

			idxOut[baseIdx + 3] = a;
			idxOut[baseIdx + 4] = d;
			idxOut[baseIdx + 5] = c;
		}
	}

	// indices for the capper:
	if (capInnerHole)
	{
		// inner circle:
		// vert: [nPolySide][nRing]
		// index: [nPolySide][3]
		for (i32 i = 0; i < polySides; i++)
		{
			i32 baseIdx = i * 3;
			idxOut[baseIdx] = 0;
			idxOut[baseIdx + 1] = (i != (polySides - 1)) ? (2 + i * ringCount) : 1;
			idxOut[baseIdx + 2] = 1 + i * ringCount;
		}
	}

	if (gCollectPosArray)
		delete[] ringPosArray;
}

//Generate a hoop which consists of specified number of inscribe circles
void MeshUtil::GenHoop(float radius, i32 polySides, float thickness, i32 ringCount, const ColorF& clr, float noiseStrength, i32 noiseSeed, float startAngle, float endAngle, float fadeAngle, std::vector<SVF_P3F_C4B_T2F>& vertOut, std::vector<u16>& idxOut)
{
	// use inscribe circles. Tangent point is at the origin:
	setSeed(noiseSeed);

	polySides *= 2;

	if (ringCount < 2)
	{
		// Log("Warning: Illegal ring count");
		ringCount = 2;
	}

	if (endAngle < startAngle)
	{
		float tmp = endAngle;
		endAngle = startAngle;
		startAngle = tmp;
	}

	float startAngleRad = startAngle / 180 * PI;
	float endAngleRad = endAngle / 180 * PI;
	float fadeAngleRad = fadeAngle / 180 * PI;

	// Verts:
	vertOut.resize(polySides * ringCount);

	float innerRadius = radius - thickness;
	float deltaRadius = thickness / (float)(ringCount - 1);
	float deltaTheta = 1 / (float)polySides;

	for (i32 i = 0; i < polySides; i++)
	{
		float spikeThetaWidth = 0.18f * deltaTheta;
		float thetaLinear = i / (float)polySides + randUnit() * 0.5f * deltaTheta - spikeThetaWidth;  // old linear mapping
		float k = 2.6f * thetaLinear - 1.3f;
		float theta = 0.5f + 0.5f * tanf(k) / tanf(1.3f);
		float angle = 2 * PI * theta;
		float x = cos(angle);
		float y = sin(angle);
		float fade = computeFade(angle, startAngleRad, endAngleRad, fadeAngleRad);
		float noise = randUnit() * noiseStrength;

		for (i32 r = 0; r < ringCount; r++)
		{
			float curRingRadius = innerRadius + r * deltaRadius;
			float thou = r / (float)(ringCount - 1);
			curRingRadius += noise; // apply translation noise

			SVF_P3F_C4B_T2F& vert = vertOut[i * ringCount + r];
			vert.xyz.Set((x - 1) * curRingRadius, y * curRingRadius, 0);
			vert.st.set(thou, theta);

			ColorF c(clr.r, clr.g, clr.b, clr.a * fade);
			vert.color.dcolor = c.pack_argb8888();
		}

		// generate spikes
		i++;
		theta += 2 * spikeThetaWidth;
		angle = 2 * PI * theta;
		x = cos(angle);
		y = sin(angle);
		for (i32 r = 0; r < ringCount; r++)
		{
			float curRingRadius = innerRadius + r * deltaRadius;
			float thou = r / (float)(ringCount - 1);
			curRingRadius += noise; // apply translation noise

			SVF_P3F_C4B_T2F& vert = vertOut[i * ringCount + r];
			vert.xyz.Set((x - 1) * curRingRadius, y * curRingRadius, 0);
			vert.st.set(thou, theta);

			ColorF c(clr.r, clr.g, clr.b, clr.a * fade);
			vert.color.dcolor = c.pack_argb8888();
		}
	}

	// Indices;
	idxOut.resize(polySides * (ringCount - 1) * 6);
	for (i32 i = 0; i < polySides; i++)
	{
		i32 baseVertIdx0 = i * ringCount;
		for (i32 r = 0; r < ringCount - 1; r++)
		{
			// counter clock wise:
			i32 a = baseVertIdx0 + r;
			i32 b = a + 1;
			i32 c = (a + ringCount + 1) % vertOut.size();
			i32 d = (c - 1) % vertOut.size();

			i32 baseIdx = (i * (ringCount - 1) + r) * 6;
			idxOut[baseIdx] = b;
			idxOut[baseIdx + 2] = c;
			idxOut[baseIdx + 1] = a;

			idxOut[baseIdx + 3] = a;
			idxOut[baseIdx + 5] = c;
			idxOut[baseIdx + 4] = d;
		}
	}
}

void MeshUtil::GenTrapezoidFan(i32 numSideVert, float radius, float startAngleDegree, float endAngleDegree, float centerThickness, const ColorF& clr, std::vector<SVF_P3F_C4B_T2F>& vertOut, std::vector<u16>& idxOut)
{
	if (endAngleDegree < startAngleDegree)
	{
		float tmp = endAngleDegree;
		endAngleDegree = startAngleDegree;
		startAngleDegree = tmp;
	}

	float startAngle = startAngleDegree / 180 * PI;
	float endAngle = endAngleDegree / 180 * PI;
	float midAngle = 0.5f * (startAngle + endAngle);
	float halfAngleRange = 0.5f * (endAngle - startAngle);
	float angleDelta = (endAngle - startAngle) / (numSideVert - 1);

	vertOut.resize(numSideVert * 2);
	float dirX = cos(midAngle);
	float dirY = sin(midAngle);
	DWORD dclr = clr.pack_argb8888();
	for (i32 i = 0; i < numSideVert; i++)
	{
		float relativeAngle = i * angleDelta;
		float theta = startAngle + relativeAngle;
		float x = cos(theta);
		float y = sin(theta);

		float angleRatio = (theta - midAngle) / halfAngleRange;
		float u = i / (float)(numSideVert - 1);

		SVF_P3F_C4B_T2F& vert = vertOut[i * 2];
		float yRela = angleRatio;
		vert.xyz.Set((-yRela * dirY) * centerThickness, (yRela * dirX) * centerThickness, 0);  // swap x,y and negate and translate
		vert.st.set(u, 0);
		vert.color.dcolor = dclr;

		// top:
		SVF_P3F_C4B_T2F& vert2 = vertOut[i * 2 + 1];
		vert2.xyz.Set(x * radius, y * radius, 0);
		vert2.st.set(u, 1);
		vert2.color.dcolor = dclr;
	}

	idxOut.resize((numSideVert - 1) * 6);
	for (i32 i = 0; i < numSideVert - 1; i++)
	{
		i32 baseVertIdx = i;
		i32 a = baseVertIdx * 2;
		i32 b = a + 2;
		i32 c = b + 1;
		i32 d = a + 1;

		i32 baseIdx = i * 6;

		idxOut[baseIdx] = b;
		idxOut[baseIdx + 1] = c;
		idxOut[baseIdx + 2] = a;

		idxOut[baseIdx + 3] = a;
		idxOut[baseIdx + 4] = c;
		idxOut[baseIdx + 5] = d;
	}
}

void MeshUtil::GenFan(i32 numSideVert, float radius, float startAngleDegree, float endAngleDegree, const ColorF& clr, std::vector<SVF_P3F_C4B_T2F>& vertOut, std::vector<u16>& idxOut)
{
	if (endAngleDegree < startAngleDegree)
	{
		float tmp = endAngleDegree;
		endAngleDegree = startAngleDegree;
		startAngleDegree = tmp;
	}

	float startAngle = startAngleDegree / 180 * PI;
	float endAngle = endAngleDegree / 180 * PI;
	float angleDelta = (endAngle - startAngle) / (numSideVert - 1);

	vertOut.resize(numSideVert + 1);
	DWORD dclr = clr.pack_argb8888();

	// center
	SVF_P3F_C4B_T2F& centerVert = vertOut[0];
	centerVert.xyz.Set(0, 0, 0);
	centerVert.st.set(0, 0);
	centerVert.color.dcolor = dclr;

	for (i32 i = 0; i < numSideVert; i++)
	{
		float relativeAngle = i * angleDelta;
		float theta = startAngle + relativeAngle;
		float x = cos(theta);
		float y = sin(theta);
		float xLocal = cos(relativeAngle);
		float yLocal = sin(relativeAngle);

		// top:
		SVF_P3F_C4B_T2F& vert2 = vertOut[i + 1];
		vert2.xyz.Set(x * radius, y * radius, 0);
		vert2.st.set(xLocal, yLocal);
		vert2.color.dcolor = dclr;
	}

	idxOut.resize((numSideVert - 1) * 3);
	for (i32 i = 0; i < numSideVert - 1; i++)
	{
		i32 baseVertIdx = i + 1;

		i32 a = 0;
		i32 b = baseVertIdx;
		i32 c = baseVertIdx + 1;

		i32 baseIdx = i * 3;

		idxOut[baseIdx] = a;
		idxOut[baseIdx + 1] = c;
		idxOut[baseIdx + 2] = b;
	}
}

// A shaft/falloff-fan is a simple coarse fan-shaped mesh with odd number of side-vertices.
// It's UV mapping spans a strict rectangular shape (x:[0,1,0]  y:[0,1]).
// On top of the fan shape, there's a concentric beam which simulates the spike effect.
void MeshUtil::GenShaft(float radius, float centerThickness, i32 complexity, float startAngleDegree, float endAngleDegree, const ColorF& clr, std::vector<SVF_P3F_C4B_T2F>& vertOut, std::vector<u16>& idxOut)
{
	if (complexity <= 0)
		return;

	i32 numSideVert = complexity;
	GenTrapezoidFan(numSideVert, radius, startAngleDegree, endAngleDegree, centerThickness, clr, vertOut, idxOut);

	float midAngleDegree = 0.5f * (startAngleDegree + endAngleDegree);
	float halfAngleRangeDegree = 0.5f * (endAngleDegree - startAngleDegree);
	const float beamWidthFactor = 0.1f;
	float beamHalfAngle = halfAngleRangeDegree * beamWidthFactor;
	std::vector<SVF_P3F_C4B_T2F> beamVertOut;
	std::vector<u16> beamIdxOut;
	ColorF clr2 = clr * 1.1f;

	GenTrapezoidFan(2, radius, midAngleDegree - beamHalfAngle, midAngleDegree + beamHalfAngle, centerThickness, clr2, beamVertOut, beamIdxOut);
}

void MeshUtil::GenStreak(float dir, float radius, float thickness, const ColorF& clr, std::vector<SVF_P3F_C4B_T2F>& vertOut, std::vector<u16>& idxOut)
{
	static i32k polySides = 24;

	Matrix33 rotMx;
	rotMx.SetRotationAA(dir * PI, Vec3(0, 0, 1));

	DWORD dclr = clr.pack_argb8888();
	vertOut.resize(polySides + 1);

	SVF_P3F_C4B_T2F& centerV = vertOut[0];
	centerV.xyz = Vec3(0, 0, 0);
	centerV.st = Vec2(0, 0);
	centerV.color.dcolor = dclr;

	// Generate all vertices
	for (i32 i = 0; i < polySides; i++)
	{
		float theta = i / (float)polySides;
		float angle = 2 * PI * theta;
		float x = cos(angle);
		float y = sin(angle) * thickness;

		Vec2 scale(x, y);
		scale = scale * rotMx;

		x = scale.x;
		y = scale.y;

		SVF_P3F_C4B_T2F& vert = vertOut[i + 1];
		vert.xyz.Set(x * radius, y * radius, 0);
		vert.st.set(1.0f, theta);
		vert.color.dcolor = dclr;
	}

	// indices;
	i32 centerIdxCount = polySides * 3;
	idxOut.resize(centerIdxCount);

	for (i32 i = 0; i < polySides; i++)
	{
		i32 baseIdx = i * 3;
		idxOut[baseIdx] = 0;
		idxOut[baseIdx + 1] = (i != (polySides - 1)) ? (2 + i) : 1;
		idxOut[baseIdx + 2] = 1 + i;
	}
}

void MeshUtil::GenSprites(std::vector<SpritePoint>& spriteList, float aspectRatio, bool packPivotPos, std::vector<SVF_P3F_C4B_T2F>& vertOut, std::vector<u16>& idxOut)
{
	vertOut.resize(spriteList.size() * 4);

	for (u32 i = 0; i < spriteList.size(); i++)
	{
		SpritePoint& sprite = spriteList[i];
		Vec2& pivot = sprite.pos;

		float size = sprite.size;
		float rot = sprite.rotation;
		ColorF clr = sprite.color;

		float radius = size;
		DWORD dclr = clr.pack_argb8888();

		i32 vertIdx = i * 4;

		SVF_P3F_C4B_T2F& vert0 = vertOut[vertIdx];
		Vec2 p = rotate(-radius, -radius, rot);

		float zComp;
		if (packPivotPos)
			zComp = (float)(((i32)floor((pivot.x * 0.5f + 0.5f) * 4095) << 12) | ((i32)floor((pivot.y * 0.5f + 0.5f) * 4095))) / (4096 * 4096);
		else
			zComp = rot;

		vert0.xyz.Set(p.x / aspectRatio + pivot.x, p.y + pivot.y, zComp);
		vert0.color.dcolor = dclr;
		vert0.st.set(0, 0);

		SVF_P3F_C4B_T2F& vert1 = vertOut[vertIdx + 1];
		p = rotate(radius, -radius, rot);
		vert1.xyz.Set(p.x / aspectRatio + pivot.x, p.y + pivot.y, zComp);
		vert1.color.dcolor = dclr;
		vert1.st.set(1, 0);

		SVF_P3F_C4B_T2F& vert2 = vertOut[vertIdx + 2];
		p = rotate(radius, radius, rot);
		vert2.xyz.Set(p.x / aspectRatio + pivot.x, p.y + pivot.y, zComp);
		vert2.color.dcolor = dclr;
		vert2.st.set(1, 1);

		SVF_P3F_C4B_T2F& vert3 = vertOut[vertIdx + 3];
		p = rotate(-radius, radius, rot);
		vert3.xyz.Set(p.x / aspectRatio + pivot.x, p.y + pivot.y, zComp);
		vert3.color.dcolor = dclr;
		vert3.st.set(0, 1);
	}
}

void MeshUtil::TrianglizeQuadIndices(i32 quadCount, std::vector<u16>& idxOut)
{
	idxOut.resize(quadCount * 6);
	for (i32 i = 0; i < quadCount; i++)
	{
		i32 baseVertIdx = i * 4;
		i32 a = baseVertIdx;
		i32 b = a + 1;
		i32 c = b + 1;
		i32 d = c + 1;

		i32 baseIdx = i * 6;

		idxOut[baseIdx] = b;
		idxOut[baseIdx + 1] = c;
		idxOut[baseIdx + 2] = a;

		idxOut[baseIdx + 3] = c;
		idxOut[baseIdx + 4] = d;
		idxOut[baseIdx + 5] = a;
	}
}

void MeshUtil::GenScreenTile(float x0, float y0, float x1, float y1, ColorF clr, i32 rowCount, i32 columnCount, stl::aligned_vector<SVF_P3F_C4B_T2F, DRX_PLATFORM_ALIGNMENT>& vertOut, stl::aligned_vector<u16, DRX_PLATFORM_ALIGNMENT>& idxOut)
{
	i32 yCount = rowCount + 1;
	i32 xCount = columnCount + 1;
	vertOut.resize(xCount * yCount);

	DWORD dclr = clr.pack_argb8888();

	float xSpan = x1 - x0;
	float ySpan = y1 - y0;
	float xDelta = xSpan / columnCount;
	float yDelta = ySpan / rowCount;
	float uDelta = 1.f / columnCount;
	float vDelta = 1.f / rowCount;

	for (i32 j = 0; j < yCount; j++)
	{
		for (i32 i = 0; i < xCount; i++)
		{
			SVF_P3F_C4B_T2F& vert = vertOut[i + j * xCount];
			vert.xyz.Set(x0 + xDelta * i, y0 + yDelta * j, 0);
			vert.st.set(uDelta * i, vDelta * j);
			vert.color.dcolor = dclr;
		}
	}

	idxOut.resize(rowCount * columnCount * 6);
	for (i32 j = 0; j < rowCount; j++)
	{
		for (i32 i = 0; i < columnCount; i++)
		{
			i32 a = i + j * xCount;
			i32 b = a + 1;
			i32 c = (i + 1) + (j + 1) * xCount;
			i32 d = c - 1;

			i32 baseIdx = (i + j * columnCount) * 6;
			idxOut[baseIdx + 0] = b;
			idxOut[baseIdx + 1] = c;
			idxOut[baseIdx + 2] = a;

			idxOut[baseIdx + 3] = a;
			idxOut[baseIdx + 4] = c;
			idxOut[baseIdx + 5] = d;
		}
	}
}
