// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

struct EventPhys;

//! Базовый класс для всех различаемых физических событий.
//! В основном класс работает как future.
//! Start() стартует вычисление (частично в основном потоке, большей частью в task/job).
//! Result() синхронизует операцию задания и возвращает итог.
struct IDeferredPhysicsEvent
{
	//! Перечневый список всех типов различаемых событий.
	enum DeferredEventType
	{
		PhysCallBack_OnCollision
	};

	IDeferredPhysicsEvent(){}
	// <interfuscator:shuffle>
	virtual ~IDeferredPhysicsEvent(){}

	// == Интерфейс, похожий на "future" == //

	//! Стартует исполнение данного события.
	virtual void Start() = 0;

	//! Выполняет событие.
	virtual void Execute() = 0;

	//! Выполнение события через job manager.
	virtual void ExecuteAsJob() = 0;

	//! Синхронизует событие и выполняет всю необходимую постобработку,
	//! затем возвращает итог.
	virtual i32 Result(EventPhys* pOrigEvent = 0) = 0;

	//! Дожидаться окончания события.
	virtual void Sync() = 0;

	//! Проверить, окончена ли часть асинхронного вычисления.
	virtual bool HasFinished() = 0;

	//! Получить конкретный тип Type этого различаемого события.
	virtual DeferredEventType GetType() const = 0;

	//! \returns Указатель на исходное событие физики.
	virtual EventPhys* PhysicsEvent() = 0;
	// </interfuscator:shuffle>
};

//! Класс управления для различаемых событий физики.
struct IDeferredPhysicsEventUpr
{
	//! Тип функции создания, используемой для создания необходимых различаемых событий
	//! в функции HandleEvent.
	typedef IDeferredPhysicsEvent*(* CreateEventFunc)(const EventPhys* pEvent);

	IDeferredPhysicsEventUpr(){}
	// <interfuscator:shuffle>
	virtual ~IDeferredPhysicsEventUpr(){}

	//! Диспетчирует (отправляет) различаемое событие в поток задачи.
	virtual void DispatchDeferredEvent(IDeferredPhysicsEvent* pEvent) = 0;

	//! Инкапсулирует общую логику для различаемых событий;
	//! должна вызываться из обратных вызовов физики.
	//! Управляет переменными cvar, а также созданием различаемых событий.
	virtual i32 HandleEvent(const EventPhys* pEvent, IDeferredPhysicsEventUpr::CreateEventFunc, IDeferredPhysicsEvent::DeferredEventType) = 0;

	//! Регистрирует и Отрегистрирует Различаемые события в управлении.
	virtual void RegisterDeferredEvent(IDeferredPhysicsEvent* pDeferredEvent) = 0;
	virtual void UnRegisterDeferredEvent(IDeferredPhysicsEvent* pDeferredEvent) = 0;

	//! Удаляет все различамые события in flight, use only when also clearing the physics event queue
	//! or else this call results in dangling points, mostly used for save/load.
	virtual void                   ClearDeferredEvents() = 0;

	virtual void                   Update() = 0;

	virtual IDeferredPhysicsEvent* GetLastCollisionEventForEntity(IPhysicalEntity* pPhysEnt) = 0;
	// </interfuscator:shuffle>
};

//! \endcond