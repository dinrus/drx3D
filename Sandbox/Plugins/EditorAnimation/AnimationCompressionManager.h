// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <IBackgroundTaskManager.h>
#include <IAnimationCompressionManager.h>
#include <IEditor.h>
#include <DrxSystem/File/IFileChangeMonitor.h>

class CBackgroundTaskUpdateLocalAnimations;
class CBackgroundTaskCompressCAF;
class CAnimationCompressionManager
	: public IAnimationCompressionManager
	  , public IFileChangeListener
	  , public IEditorNotifyListener
{
public:
	CAnimationCompressionManager();
	~CAnimationCompressionManager();

	bool IsEnabled() const override;
	void UpdateLocalAnimations() override;
	void QueueAnimationCompression(tukk animationName) override;

	void OnEditorNotifyEvent(EEditorNotifyEvent ev) override;
private:
	class CBackgroundTaskCompressCAF;
	class CBackgroundTaskUpdateLocalAnimations;
	class CBackgroundTaskImportAnimation;
	class CBackgroundTaskReloadCHRPARAMS;
	class CBackgroundTaskCleanUpAnimations;

	void Start();
	void OnFileChange(tukk filename, EChangeType eType) override;

	void OnCAFCompressed(CBackgroundTaskCompressCAF* pTask);
	void OnImportTriggered(tukk animationPath);
	void OnReloadCHRPARAMSComplete(CBackgroundTaskReloadCHRPARAMS* pTask);

	struct SAnimationInWorks
	{
		CBackgroundTaskCompressCAF* pTask;
	};

	CBackgroundTaskUpdateLocalAnimations* m_pRescanTask;
	CBackgroundTaskReloadCHRPARAMS*       m_pReloadCHRPARAMSTask;
	typedef std::map<string, SAnimationInWorks> TAnimationMap;
	TAnimationMap                         m_animationsInWork;
	std::vector<SAnimationInWorks>        m_canceledAnimations;
	bool m_enabled;
};

