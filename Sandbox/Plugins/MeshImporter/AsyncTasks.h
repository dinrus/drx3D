// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
// Async tasks used by FbxTool.
#pragma once
#include "AsyncHelper.h"
#include "FbxScene.h"
#include "FbxMetaData.h"
#include "MainDialog.h"

#include <DrxSystem/ISystem.h>
#include <DrxSystem/ILog.h>

// Async FBX scene import task
class CAsyncImportSceneTask : public CAsyncTaskBase, FbxTool::ICallbacks
{
public:
	typedef std::function<void (CAsyncImportSceneTask*)> Callback;

	CAsyncImportSceneTask();

	void                                   SetUserSettings(const FbxMetaData::SSceneUserSettings& userSettings);
	void                                   SetFilePath(const QString& filePath);
	void                                   SetCallback(const Callback& callback);
	void                                   SetUserData(uk pUserData);

	std::unique_ptr<FbxTool::CScene>       GetScene();
	uk                                  GetUserData();

	bool                                   Succeeded() const;
	const FbxMetaData::SSceneUserSettings& GetUserSettings() const;
	const QString&                         GetFilePath() const;

	// CAsyncTaskBase
	virtual bool PerformTask() override;
	virtual void FinishTask(bool bResult) override;
	// ~CAsyncTaskBase

	// ICallbacks
	virtual void OnProgressMessage(tukk szMessage) override;
	virtual void OnProgressPercentage(i32 percentage) override;
	virtual void OnWarning(tukk szMessage) override;
	virtual void OnError(tukk szMessage) override;
	// ~ICallbacks
private:
	static DrxCriticalSection m_fbxSdkMutex;

	FbxMetaData::SSceneUserSettings  m_userSettings;
	QString                          m_filePath;
	bool                             m_bSucceeded;
	std::unique_ptr<FbxTool::CScene> m_resultScene;
	QString                          m_lastProgressMessage;
	i32                              m_lastProgressPercentage;
	Callback                         m_callback;
	uk                            m_pUserData;
};

