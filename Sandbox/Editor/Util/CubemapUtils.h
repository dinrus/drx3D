// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#if !defined __cubemaputils_h_
#define __cubemaputils_h_

namespace CubemapUtils
{
bool GenCubemapWithPathAndSize(string& filename, i32k size, const bool dds = true);
bool GenCubemapWithObjectPathAndSize(string& filename, CBaseObject* pObject, i32k size, const bool dds);
void GenHDRCubemapTiff(const string& fileName, std::size_t size, Vec3& pos);
void RegenerateAllEnvironmentProbeCubemaps();
void GenerateCubemaps();
}

#endif //__cubemaputils_h_

