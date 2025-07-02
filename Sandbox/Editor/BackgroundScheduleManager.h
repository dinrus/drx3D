// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Include/IBackgroundScheduleManager.h"

namespace BackgroundScheduleManager
{

class CScheduleItem : public IBackgroundScheduleItem
{
private:
	STxt        m_name;
	volatile i32       m_refCount;

	EScheduleItemState m_state;

	typedef std::vector<IBackgroundScheduleItemWork*> TWorkItems;
	TWorkItems m_workItems;
	TWorkItems m_addedWorkItems;
	TWorkItems m_processedWorkItems;

public:
	CScheduleItem(tukk szName);
	virtual ~CScheduleItem();

	// IBackgroundScheduleItem interface
	virtual tukk                  GetDescription() const;
	virtual EScheduleItemState           GetState() const;
	virtual const float                  GetProgress() const;
	virtual u32k                 GetNumWorkItems() const;
	virtual IBackgroundScheduleItemWork* GetWorkItem(u32k index) const;
	virtual void                         AddWorkItem(IBackgroundScheduleItemWork* pWork);
	virtual void                         AddRef();
	virtual void                         Release();

	// Update schedule item
	EScheduleWorkItemStatus Update();

	// Request to stop work in this item
	void RequestStop();
};

class CSchedule : public IBackgroundSchedule
{
private:
	STxt    m_name;
	volatile i32   m_refCount;
	bool           m_bCanceled;

	EScheduleState m_state;

	typedef std::vector<CScheduleItem*> TItems;
	TItems m_items;

	u32 m_currentItem;

public:
	CSchedule(tukk szName);
	virtual ~CSchedule();

	// IBackgroundSchedule interface
	virtual tukk              GetDescription() const;
	virtual float                    GetProgress() const;
	virtual IBackgroundScheduleItem* GetProcessedItem() const;
	virtual u32k             GetNumItems() const;
	virtual IBackgroundScheduleItem* GetItem(u32k index) const;
	virtual EScheduleState           GetState() const;
	virtual void                     Cancel();
	virtual bool                     IsCanceled() const;
	virtual void                     AddItem(IBackgroundScheduleItem* pItem);
	virtual void                     AddRef();
	virtual void                     Release();

	// Update schedule item
	EScheduleWorkItemStatus Update();
};

class CScheduleManager : public IBackgroundScheduleManager, public IEditorNotifyListener
{
private:
	typedef std::vector<CSchedule*> TSchedules;
	TSchedules m_schedules;

public:
	CScheduleManager();
	virtual ~CScheduleManager();

	// IBackgroundScheduleManager interface
	virtual IBackgroundSchedule*     CreateSchedule(tukk szName);
	virtual IBackgroundScheduleItem* CreateScheduleItem(tukk szName);
	virtual void                     SubmitSchedule(IBackgroundSchedule* pSchedule);
	virtual u32k             GetNumSchedules() const;
	virtual IBackgroundSchedule*     GetSchedule(u32k index) const;
	virtual void                     Update();

	// IEditorNotifyListener interface implementation
	virtual void OnEditorNotifyEvent(EEditorNotifyEvent ev) override;
};

}

