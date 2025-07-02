// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/String/DrxString.h>
#include <drx3D/CoreX/Sandbox/DrxSignal.h>

#include <functional>
#include <memory>

class CRcInOrderCaller;
struct ITaskHost;

class QTemporaryDir;

namespace MeshImporter
{

class CSceneManager;

} //endns MeshImporter

namespace FbxMetaData
{

struct SMetaData;

} //endns FbxMetaData

//! CTempRcObject is used to create temporary asset files from FBX, such as CGF, SKIN, and CHR files.
class CTempRcObject
{
public:
	typedef std::function<void(const CTempRcObject*)> Finalize;

public:
	CTempRcObject(ITaskHost* pTaskHost, MeshImporter::CSceneManager* pSceneManager);
	~CTempRcObject();

	void SetMessage(const string& message);
	void ClearMessage();

	void SetMetaData(const FbxMetaData::SMetaData& metaData);
	void SetFinalize(const Finalize& finalize);

	void CreateAsync();

	string GetFilePath() const;
private:
	void RcCaller_OnAssign(CRcInOrderCaller* pCaller, const QString& filePath, uk pUserData);

	std::unique_ptr<CRcInOrderCaller> m_pRcCaller;
	std::unique_ptr<QTemporaryDir> m_pTempDir;
	MeshImporter::CSceneManager* m_pSceneManager;
	ITaskHost* m_pTaskHost;
	string m_filePath;

	string m_message;
	FbxMetaData::SMetaData m_metaData;
	Finalize m_finalize;
};

