// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _AIACTIONS_H_
#define _AIACTIONS_H_

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/FlowGraph/IFlowSystem.h>
#include <drx3D/AI/IAIAction.h>
#include <drx3D/CoreX/Containers/DrxListenerSet.h>

#include <drx3D/AI/SmartObjects.h>

// Предварительная декларация.
class CAIActionUpr;

///////////////////////////////////////////////////
// CAIAction ссылается на Flow Graph - последовательность элементарных действий,
// которую следует выполнять при "использовании" смарт-объекта.
///////////////////////////////////////////////////
class CAIAction : public IAIAction
//Класс ДействиеИИ, наследует от интерфейса ИДействиеИИ.
{
protected:
	// CAIActionUpr нуждается в правах доступа и изменения этих приватных членов-данных.
	friend class CAIActionUpr;
	//Класс УпрДействиемИИ, которому даётся "верительная грамота".

	// Уникальное имя этого ДействияИИ.
	string m_Name;

	// Указывает на flow graph (граф потока), который нужно использовать для выполнения этого ДействияИИ.
	IFlowGraphPtr m_pFlowGraph;

	void NotifyListeners(EActionEvent event)
	//Метод уведомиДатчики, принимает СобытиеЕДейство.
	{
		for (TActionListeners::Notifier notifier(m_listeners); notifier.IsValid(); notifier.Next())
			notifier->OnActionEvent(event);
	}

public:
	CAIAction(const CAIAction& src) : m_listeners(1)
	//Копи-конструктор, принимающий ссылку на другое Дествие ИИ.
	{
		m_Name = src.m_Name;
		m_pFlowGraph = src.m_pFlowGraph;

		// [11/11/2010 willw] Do not copy the listeners, as they registered to listen to src not this.
		//m_listeners = src.m_listeners;
	}
	CAIAction() : m_listeners(1) {}
	//Дефолтный (безпараметрный) конструктор класса.
	virtual ~CAIAction() {}
	//Деструктор этого класса.

	// Обходит все узлы "подлежащего" flow-графа и проверяет на наличие несовместимых узлов,
	//при выполнении в контексте ДействияИИ.
	virtual bool TraverseAndValidateAction(EntityId idOfUser) const;
	//Виртуальная функция обойдиИВалидируйДействие, принимающая ИдСущности, возврзнач бул.

	// Возвращает уникальное имя этого ДействияИИ.
	virtual tukk GetName() const { return m_Name; }
	//Виртуальная функц. дайИмя, возврзнач ук на сим (ткст0).

	// Возвращает целевой пайп, выполняющий это ДействиеИИ.
	virtual IGoalPipe* GetGoalPipe() const { return NULL; };

	// Возвращает Flow Graph, ассоциированный с этим ДействиемИИ.
	virtual IFlowGraph* GetFlowGraph() const { return m_pFlowGraph; }

	// Возвращает сущность User, ассоциированную с этим ДействиемИИ.
	// Базовому классу нельзя присваивать никаких сущностей.
	virtual IEntity* GetUserEntity() const { return NULL; }

	// Возвращает сущность Object, ассоциированную с этим ДействиемИИ.
	// Базовому классу нельзя присваивать никаких сущностей.
	virtual IEntity* GetObjectEntity() const { return NULL; }

	// Завершает выполнение этого Действия ИИ.
	// Не делает ничего, так как не может быть выполнен.
	virtual void EndAction() {}

	// Отменяет выполнение этого Действия ИИ.
	// Не делает ничего, так как не может быть выполнен.
	virtual void CancelAction() {}

	// Прерывает исполнение этого Действия ИИ - запускается процедура очистки.
	virtual bool AbortAction() { AIAssert(!"Aborting inactive AI action!"); return false; }

	// Метит это Дествие ИИ как изменённое.
	virtual void Invalidate() { m_pFlowGraph = NULL; }

	// Удаляет указатели на сущности.
	virtual void        OnEntityRemove()                                                     {}

	virtual bool        IsSignaledAnimation() const                                          { return false; }
	virtual bool        IsExactPositioning() const                                           { return false; }
	virtual tukk GetAnimationName() const                                             { return NULL; }

	virtual const Vec3& GetAnimationPos() const                                              { return Vec3Constants<float>::fVec3_Zero; }
	virtual const Vec3& GetAnimationDir() const                                              { return Vec3Constants<float>::fVec3_OneY; }
	virtual bool        IsUsingAutoAssetAlignment() const                                    { return false; }

	virtual const Vec3& GetApproachPos() const                                               { return Vec3Constants<float>::fVec3_Zero; }

	virtual float       GetStartWidth() const                                                { return 0; }
	virtual float       GetStartArcAngle() const                                             { return 0; }
	virtual float       GetDirectionTolerance() const                                        { return 0; }

	virtual void        RegisterListener(IAIActionListener* eventListener, tukk name) { m_listeners.Add(eventListener, name); }
	virtual void        UnregisterListener(IAIActionListener* eventListener)                 { m_listeners.Remove(eventListener); }

private:

	typedef CListenerSet<IAIActionListener*> TActionListeners;

	TActionListeners m_listeners;

};

///////////////////////////////////////////////////
// CAnimationAction ссылается на целевой пайп,
// который нужно выполнить, чтобы "использовать" смарт-объект.
///////////////////////////////////////////////////
class CAnimationAction : public IAIAction
{
protected:
	// Параметры анимации.
	bool   bSignaledAnimation;
	bool   bExactPositioning;
	string sAnimationName;

	// Параметры приближения.
	Vec3  vApproachPos;
	float fApproachSpeed;
	i32   iApproachStance;

	// Параметры точного позиционирования.
	Vec3  vAnimationPos;
	Vec3  vAnimationDir;
	float fStartWidth;
	float fDirectionTolerance;
	float fStartArcAngle;
	bool  bAutoTarget;

	void NotifyListeners(EActionEvent event)
	{
		for (TActionListeners::Notifier notifier(m_listeners); notifier.IsValid(); notifier.Next())
			notifier->OnActionEvent(event);
	}

public:
	CAnimationAction(
	  tukk animationName,
	  bool signaled,
	  float approachSpeed,
	  i32 approachStance,
	  float startWidth,
	  float directionTolerance,
	  float startArcAngle)
		: sAnimationName(animationName)
		, bSignaledAnimation(signaled)
		, fApproachSpeed(approachSpeed)
		, iApproachStance(approachStance)
		, fStartWidth(startWidth)
		, fDirectionTolerance(directionTolerance)
		, fStartArcAngle(startArcAngle)
		, bExactPositioning(true)
		, vApproachPos(ZERO)
		, bAutoTarget(false)
		, m_listeners(1)
	{}
	CAnimationAction(
	  tukk animationName,
	  bool signaled)
		: sAnimationName(animationName)
		, bSignaledAnimation(signaled)
		, fApproachSpeed(0.0f)
		, iApproachStance(0)
		, fStartWidth(0.0f)
		, fDirectionTolerance(0.0f)
		, fStartArcAngle(0.0f)
		, bExactPositioning(false)
		, vApproachPos(ZERO)
		, bAutoTarget(false)
		, m_listeners(1)
	{}
	virtual ~CAnimationAction() {}

	virtual bool TraverseAndValidateAction(EntityId idOfUser) const { return true; }  // CAnimationAction doesn't deal with flow graph, hence no "incompatible" nodes

	// Возвращает уникальное имя этого ДействияИИ.
	virtual tukk GetName() const { return NULL; }

	// Возвращает целевой пайп, выполняющий это ДействиеИИ.
	virtual IGoalPipe* GetGoalPipe() const;

	// Возвращает Flow Graph, ассоциированный с этим ДействиемИИ.
	virtual IFlowGraph* GetFlowGraph() const { return NULL; }

	// Возвращает сущность User, ассоциированную с этим ДействиемИИ.
	// Базовому классу нельзя присваивать никаких сущностей.
	virtual IEntity* GetUserEntity() const { return NULL; }

	// Возвращает сущность Object, ассоциированную с этим ДействиемИИ.
	// Базовому классу нельзя присваивать никаких сущностей.
	virtual IEntity* GetObjectEntity() const { return NULL; }

	// Заканчивает выполнение этого ДействияИИ.
	// Не делает ничего, так как это действие невыполнимо.
	virtual void EndAction() {}

	// Отменяет выполнение этого ДействияИИ.
	// Не делает ничего, так как это действие невыполнимо.
	virtual void CancelAction() {}

	// Прерывает выполнение этого ДейстияИИ - запускает процедуру очистки.
	virtual bool AbortAction() { AIAssert(!"Aborting inactive AI action!"); return false; }

	// Отмечает это ДействиеИИ как изменённое.
	virtual void Invalidate() {}

	// Удаляет указатели на сущности.
	virtual void OnEntityRemove() {}

	void         SetTarget(const Vec3& position, const Vec3& direction)
	{
		vAnimationPos = position;
		vAnimationDir = direction;
	}

	void SetAutoTarget(const Vec3& position, const Vec3& direction)
	{
		vAnimationPos = position;
		vAnimationDir = direction;
		bAutoTarget = true;
	}

	void SetApproachPos(const Vec3& position)
	{
		vApproachPos = position;
	}

	virtual bool        IsSignaledAnimation() const                                          { return bSignaledAnimation; }
	virtual bool        IsExactPositioning() const                                           { return bExactPositioning; }
	virtual tukk GetAnimationName() const                                             { return sAnimationName; }

	virtual const Vec3& GetAnimationPos() const                                              { return vAnimationPos; }
	virtual const Vec3& GetAnimationDir() const                                              { return vAnimationDir; }
	virtual bool        IsUsingAutoAssetAlignment() const                                    { return bAutoTarget; }

	virtual const Vec3& GetApproachPos() const                                               { return vApproachPos; }

	virtual float       GetStartWidth() const                                                { return fStartWidth; }
	virtual float       GetStartArcAngle() const                                             { return fStartArcAngle; }
	virtual float       GetDirectionTolerance() const                                        { return fDirectionTolerance; }

	virtual void        RegisterListener(IAIActionListener* eventListener, tukk name) { m_listeners.Add(eventListener, name); }
	virtual void        UnregisterListener(IAIActionListener* eventListener)                 { m_listeners.Remove(eventListener); }

private:

	typedef CListenerSet<IAIActionListener*> TActionListeners;

	TActionListeners m_listeners;

};

///////////////////////////////////////////////////
// CActiveAction представлят собой единичное активное CAIAction.
///////////////////////////////////////////////////
class CActiveAction
	: public CAIAction
{
protected:
	// CAIActionUpr нуждается в доступе к приватным членам этого класса для изменения данных.
	friend class CAIActionUpr;

	// Сущности, участвующие в этом ИИ Действии.
	IEntity* m_pUserEntity;
	IEntity* m_pObjectEntity;

	// AI Action is suspended if this counter isn't zero
	// it shows by how many other AI Actions this was suspended
	i32 m_SuspendCounter;

	// id of goal pipe used for tracking
	i32 m_idGoalPipe;

	// alertness threshold level
	i32 m_iThreshold;

	// next user's and object's states
	string m_nextUserState;
	string m_nextObjectState;

	// canceled user's and object's states
	string m_canceledUserState;
	string m_canceledObjectState;

	// set to true if action should be deleted on next update
	bool m_bDeleted;

	// set to true if action is high priority
	bool m_bHighPriority;

	// exact positioning parameters
	bool   m_bSignaledAnimation;
	bool   m_bExactPositioning;
	string m_sAnimationName;
	Vec3   m_vAnimationPos;
	Vec3   m_vAnimationDir;
	Vec3   m_vApproachPos;
	bool   m_bAutoTarget;
	float  m_fStartWidth;
	float  m_fDirectionTolerance;
	float  m_fStartArcAngle;

public:
	CActiveAction()
		: m_vAnimationPos(ZERO)
		, m_vAnimationDir(ZERO)
		, m_vApproachPos(ZERO)
		, m_bSignaledAnimation(0)
		, m_bExactPositioning(0)
		, m_bAutoTarget(0)
		, m_fStartWidth(0)
		, m_fStartArcAngle(0)
		, m_fDirectionTolerance(0)
		, m_pUserEntity(NULL)
		, m_pObjectEntity(NULL)
		, m_SuspendCounter(0)
		, m_idGoalPipe(0)
		, m_iThreshold(0)
		, m_bDeleted(false)
		, m_bHighPriority(false)
	{}

	CActiveAction(const CActiveAction& src)
		: CAIAction()
	{
		*this = src;
	}

	CActiveAction& operator=(const CActiveAction& src)
	{
		m_Name = src.m_Name;
		m_pFlowGraph = src.m_pFlowGraph;

		m_pUserEntity = src.m_pUserEntity;
		m_pObjectEntity = src.m_pObjectEntity;
		m_SuspendCounter = src.m_SuspendCounter;
		m_idGoalPipe = src.m_idGoalPipe;
		m_iThreshold = src.m_iThreshold;
		m_nextUserState = src.m_nextUserState;
		m_nextObjectState = src.m_nextObjectState;
		m_canceledUserState = src.m_canceledUserState;
		m_canceledObjectState = src.m_canceledObjectState;
		m_bDeleted = src.m_bDeleted;
		m_bHighPriority = src.m_bHighPriority;
		m_bSignaledAnimation = src.m_bSignaledAnimation;
		m_bExactPositioning = src.m_bExactPositioning;
		m_sAnimationName = src.m_sAnimationName;
		m_vAnimationPos = src.m_vAnimationPos;
		m_vAnimationDir = src.m_vAnimationDir;
		m_vApproachPos = src.m_vApproachPos;
		m_bAutoTarget = src.m_bAutoTarget;
		m_fStartWidth = src.m_fStartWidth;
		m_fStartArcAngle = src.m_fStartArcAngle;
		m_fDirectionTolerance = src.m_fDirectionTolerance;
		return *this;
	}

	// returns the User entity associated to this AI Action
	virtual IEntity* GetUserEntity() const { return m_pUserEntity; }

	// returns the Object entity associated to this AI Action
	virtual IEntity* GetObjectEntity() const { return m_pObjectEntity; }

	// returns true if action is active and marked as high priority
	virtual bool IsHighPriority() const { return m_bHighPriority; }

	// ends execution of this AI Action
	virtual void EndAction();

	// cancels execution of this AI Action
	virtual void CancelAction();

	// aborts execution of this AI Action - will start clean up procedure
	virtual bool AbortAction();

	// removes pointers to entities
	virtual void        OnEntityRemove() { m_pUserEntity = m_pObjectEntity = NULL; }

	bool                operator==(const CActiveAction& other) const;

	virtual bool        IsSignaledAnimation() const       { return m_bSignaledAnimation; }
	virtual bool        IsExactPositioning() const        { return m_bExactPositioning; }
	virtual tukk GetAnimationName() const          { return m_sAnimationName; }

	virtual const Vec3& GetAnimationPos() const           { return m_vAnimationPos; }
	virtual const Vec3& GetAnimationDir() const           { return m_vAnimationDir; }
	virtual bool        IsUsingAutoAssetAlignment() const { return m_bAutoTarget; }

	virtual const Vec3& GetApproachPos() const            { return m_vApproachPos; }

	virtual float       GetStartWidth() const             { return m_fStartWidth; }
	virtual float       GetStartArcAngle() const          { return m_fStartArcAngle; }
	virtual float       GetDirectionTolerance() const     { return m_fDirectionTolerance; }

	void                Serialize(TSerialize ser);
};

#undef LoadLibrary
///////////////////////////////////////////////////
// CAIActionUpr отслеживает все AIActions
///////////////////////////////////////////////////
class CAIActionUpr : public IAIActionUpr
{
private:
	// Библиотека со всеми определёнными ДействиямиИИ.
	typedef std::map<string, CAIAction> TActionsLib;
	TActionsLib m_ActionsLib;

	// Список всех активных ДействийИИ (включая подвешенные).
	typedef std::list<CActiveAction> TActiveActions;
	TActiveActions m_ActiveActions;

protected:
	// Подвешивает все активные ДействияИИ, в которые вовлечена сущность.
	// Примечание: безопасно передавать pEntity == NULL
	i32 SuspendActionsOnEntity(IEntity* pEntity, i32 goalPipeId, const IAIAction* pAction, bool bHighPriority, i32& numHigherPriority);

	// Возобновляет все активные ДействияИИ, в которые вовлечена сущность(возобновление зависит
	// от того, сколько раз оно подвешивалось).
	// Примечание: безопасно передавать pEntity == NULL
	void ResumeActionsOnEntity(IEntity* pEntity, i32 goalPipeId);

	// Реализует интерфейс IGoalPipeListener.
	virtual void OnGoalPipeEvent(IPipeUser* pPipeUser, EGoalPipeEvent event, i32 goalPipeId, bool& unregisterListenerAfterEvent);

public:
	CAIActionUpr();
	virtual ~CAIActionUpr();

	// Останавливает все активные действия.
	void Reset();

	// Возвращает существующее ДействиеИИ из библиотеки, либо NULL, если оно не найдено.
	virtual IAIAction* GetAIAction(tukk sName);

	// Возвращает существующее ДействиеИИ по индексу из библиотеки, либо NULL, если индекс вне
	// диапазона.
	virtual IAIAction* GetAIAction(size_t index);

	virtual void       ReloadActions();

	// Возвращает существующее ДействиеИИ по имени, указанному в правиле, любо создаёт новое
	// временное действие для воспроизведения анимации, указанной в правиле.
	IAIAction* GetAIAction(const CCondition* pRule);

	// adds an AI Action in the list of active actions
	void ExecuteAIAction(const IAIAction* pAction, IEntity* pUser, IEntity* pObject, i32 maxAlertness, i32 goalPipeId,
	                     tukk userState, tukk objectState, tukk userCanceledState, tukk objectCanceledState, IAIAction::IAIActionListener* pListener = NULL);

	/// Looks up the action by name and returns a handle to it.
	void ExecuteAIAction(tukk sActionName, IEntity* pUser, IEntity* pObject, i32 maxAlertness, i32 goalPipeId, IAIAction::IAIActionListener* pListener = NULL);

	// aborts specific AI Action (specified by goalPipeId) or all AI Actions (goalPipeId == 0) in which pEntity is a user
	virtual void AbortAIAction(IEntity* pEntity, i32 goalPipeId = 0);

	// finishes specific AI Action (specified by goalPipeId) for the pEntity as a user
	virtual void FinishAIAction(IEntity* pEntity, i32 goalPipeId);

	// marks AI Action from the list of active actions as deleted
	void ActionDone(CActiveAction& action, bool bRemoveAction = true);

	// removes deleted AI Action from the list of active actions
	void Update();

	// loads the library of AI Action Flow Graphs
	void LoadLibrary(tukk sPath);

	// notification sent by smart objects system
	void OnEntityRemove(IEntity* pEntity);

	void Serialize(TSerialize ser);
};

#endif
