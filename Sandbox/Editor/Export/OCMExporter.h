// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __COCMExporter_h__
#define __COCMExporter_h__
#pragma once

#include <IExportManager.h>

class CFileEndianWriter;

struct SOCMeshInfo
{
	Matrix44 m_OBBMat;
	u32   m_Offset;
	size_t   m_MeshHash;
	bool operator==(const SOCMeshInfo& rOther) const { return m_MeshHash == rOther.m_MeshHash; }
};
typedef std::vector<SOCMeshInfo> tdMeshOffset;

class COCMExporter : public IExporter
{
public:
	virtual tukk GetExtension() const;
	virtual tukk GetShortDescription() const;
	virtual bool        ExportToFile(tukk filename, const SExportData* pExportData);
	virtual bool        ImportFromFile(tukk filename, SExportData* pData) { return false; };
	virtual void        Release();

private:
	tukk TrimFloat(float fValue) const;
	void        SaveInstance(CFileEndianWriter& rWriter, const SExportObject* pInstance, const SOCMeshInfo& rMeshInfo);
	size_t      SaveMesh(CFileEndianWriter& rWriter, const SExportObject* pMesh, Matrix44& rOBBMat);

	void        Extends(const Matrix44& rTransform, const SExportObject* pMesh, f32& rMinX, f32& rMaxX, f32& rMinY, f32& rMaxY, f32& rMinZ, f32& rMaxZ) const;
	Matrix44    CalcOBB(const SExportObject* pMesh);
};

#endif // __COCMExporter_h__

