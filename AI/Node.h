// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

	#include <drx3D/AI/IBehaviorTree.h>

	#include <drx3D/AI/SerializationSupport.h>

namespace BehaviorTree
{
class Node : public INode
{
public:
	//! Когда вы "тыкаете" узел, прилагаются следующие правила:
	//! 1. Если до этого узел не выполнялся, вначале надо узел инициализовать,
	//!  а затем сразу же обновить.
	//! 2. Если же узел уже выполнялся, но уже нет, то он будет
	//!  терминирован (terminated).
	virtual Status Tick(const UpdateContext& unmodifiedContext) override
	{
		DRX_PROFILE_FUNCTION(PROFILE_AI);

	#ifdef DEBUG_MODULAR_BEHAVIOR_TREE
		DebugTree* debugTree = unmodifiedContext.debugTree;
		if (debugTree)
			debugTree->Push(this);
	#endif // DEBUG_MODULAR_BEHAVIOR_TREE

		UpdateContext context = unmodifiedContext;

		const RuntimeDataID runtimeDataID = MakeRuntimeDataID(context.entityId, m_id);
		context.runtimeData = GetCreator()->GetRuntimeData(runtimeDataID);
		const bool nodeNeedsToBeInitialized = (context.runtimeData == NULL);
		if (nodeNeedsToBeInitialized)
		{
			context.runtimeData = GetCreator()->AllocateRuntimeData(runtimeDataID);
		}

		if (nodeNeedsToBeInitialized)
		{
	#ifdef USING_BEHAVIOR_TREE_LOG
			if (!m_startLog.empty())
				context.behaviorLog.AddMessage(m_startLog.c_str());
	#endif // USING_BEHAVIOR_TREE_LOG
			OnInitialize(context);
		}

		const Status status = Update(context);

		if (status != Running)
		{
			OnTerminate(context);
			GetCreator()->FreeRuntimeData(runtimeDataID);
			context.runtimeData = NULL;

	#ifdef USING_BEHAVIOR_TREE_LOG
			if (status == Success && !m_successLog.empty())
				context.behaviorLog.AddMessage(m_successLog.c_str());
			else if (status == Failure && !m_failureLog.empty())
				context.behaviorLog.AddMessage(m_failureLog.c_str());
	#endif // USING_BEHAVIOR_TREE_LOG
		}

	#ifdef DEBUG_MODULAR_BEHAVIOR_TREE
		if (debugTree)
			debugTree->Pop(status);
	#endif // DEBUG_MODULAR_BEHAVIOR_TREE

		return status;
	}

	//! Вызывайте, чтобы явно терминировать узел.
	//! Узел сам позаботится о зачистке памяти.
	//! Ничего страшного, если вызвать Terminate над уже терминированным узлом,
	// хотя, конечно же, это повторняк ("redundant" - "редудик").
	virtual void Terminate(const UpdateContext& unmodifiedContext) override
	{
		UpdateContext context = unmodifiedContext;
		const RuntimeDataID runtimeDataID = MakeRuntimeDataID(context.entityId, m_id);
		context.runtimeData = GetCreator()->GetRuntimeData(runtimeDataID);
		if (context.runtimeData != NULL)
		{
			OnTerminate(context);
			GetCreator()->FreeRuntimeData(runtimeDataID);
			context.runtimeData = NULL;
		}
	}

	//! Послать событие узлу.
	//! Событие отдиспетчируется корректному методу HandleEvent с валидированными рантаймными данными.
	//! Никогда не переписывате (override) this!
	virtual void SendEvent(const EventContext& unmodifiedContext, const Event& event) override
	{
		uk runtimeData = GetCreator()->GetRuntimeData(MakeRuntimeDataID(unmodifiedContext.entityId, m_id));
		if (runtimeData)
		{
			EventContext context = unmodifiedContext;
			context.runtimeData = runtimeData;
			HandleEvent(context, event);
		}
	}

	//! Подгрузить узел "дерева поведения" с инфой из узла xml.
	virtual LoadResult LoadFromXml(const XmlNodeRef& xml, const struct LoadContext& context) override
	{

	#ifdef USING_BEHAVIOR_TREE_LOG
		m_startLog = xml->getAttr("_startLog");
		m_successLog = xml->getAttr("_successLog");
		m_failureLog = xml->getAttr("_failureLog");
	#endif // USING_BEHAVIOR_TREE_LOG

	#ifdef USING_BEHAVIOR_TREE_COMMENTS
		m_comment = xml->getAttr("_comment");
	#endif // USING_BEHAVIOR_TREE_COMMENTS

		return LoadSuccess;
	}

	#ifdef USING_BEHAVIOR_TREE_XML_DESCRIPTION_CREATION
	virtual XmlNodeRef CreateXmlDescription() override
	{
		XmlNodeRef node = GetISystem()->CreateXmlNode("Node");

		auto setAttrOptional = [this, &node](tukk szKey, const string& value)
		{
			if (!value.empty())
			{
				node->setAttr(szKey, value);
			}
		};
		#ifdef USING_BEHAVIOR_TREE_LOG
		setAttrOptional("_startLog", m_startLog);
		setAttrOptional("_successLog", m_successLog);
		setAttrOptional("_failureLog", m_failureLog);
		#endif // USING_BEHAVIOR_TREE_LOG
		#ifdef USING_BEHAVIOR_TREE_COMMENTS
		setAttrOptional("_comment", m_comment);
		#endif // USING_BEHAVIOR_TREE_COMMENTS

		return node;
	}
	#endif

	#ifdef USING_BEHAVIOR_TREE_SERIALIZATION
	virtual void Serialize(Serialization::IArchive& archive) override
	{
		#ifdef DEBUG_MODULAR_BEHAVIOR_TREE
		HandleXmlLineNumberSerialization(archive, m_xmlLine);
		#endif

		#ifdef USING_BEHAVIOR_TREE_COMMENTS
		HandleCommentSerialization(archive, m_comment);
		#endif // USING_BEHAVIOR_TREE_COMMENTS
	}
	#endif

	void                SetCreator(INodeCreator* creator) { m_creator = creator; }
	INodeCreator*       GetCreator()                      { return m_creator; }
	const INodeCreator* GetCreator() const                { return m_creator; }

	#ifdef DEBUG_MODULAR_BEHAVIOR_TREE
	void   SetXmlLine(u32k line) { m_xmlLine = line; }
	u32 GetXmlLine() const            { return m_xmlLine; }
	#endif // DEBUG_MODULAR_BEHAVIOR_TREE

	#ifdef USING_BEHAVIOR_TREE_NODE_CUSTOM_DEBUG_TEXT
	virtual void GetCustomDebugText(const UpdateContext& updateContext, stack_string& debugText) const {}
	#endif // USING_BEHAVIOR_TREE_NODE_CUSTOM_DEBUG_TEXT

	template<typename RuntimeDataType>
	RuntimeDataType& GetRuntimeData(const EventContext& context)
	{
		assert(context.runtimeData);
		return *reinterpret_cast<RuntimeDataType*>(context.runtimeData);
	}

	template<typename RuntimeDataType>
	RuntimeDataType& GetRuntimeData(const UpdateContext& context)
	{
		assert(context.runtimeData);
		return *reinterpret_cast<RuntimeDataType*>(context.runtimeData);
	}

	template<typename RuntimeDataType>
	const RuntimeDataType& GetRuntimeData(const UpdateContext& context) const
	{
		assert(context.runtimeData);
		return *reinterpret_cast<const RuntimeDataType*>(context.runtimeData);
	}

	NodeID GetNodeID() const { return m_id; }

	NodeID m_id;     //!< TODO: Сделать это доступным только создателю.

protected:

	Node()
		: m_id(0)
		, m_creator(NULL)

	#ifdef DEBUG_MODULAR_BEHAVIOR_TREE
		, m_xmlLine(0)
	#endif //DEBUG_MODULAR_BEHAVIOR_TREE
	{
	}
#ifndef SWIG
	//! Вызывается перед первым вызовом Update.
	virtual void OnInitialize(const UpdateContext& context) {}

	//! Вызывается при терминации узла. Уберитесь за собой тутось!
	//! Не путайте с Terminate, который вызываете, когда надо терминировать узел.
	// :) Смутило? Почитайте ещё разок.
	//! OnTerminate вызывается в одном из следщ случаев:
	//! a) Сам узел вернул Success/Failure при Update.
	//! b) Другой узел попросил этот to Terminate, в то время как этот выполнялся.
	virtual void OnTerminate(const UpdateContext& context) {}

	//! Do you node's work here.
	//! - Note that OnInitialize will have been automatically called for you
	//! before you get your first update.
	//! - If you return Success or Failure the node will automatically
	//! get OnTerminate called on itself.
	virtual Status Update(const UpdateContext& context) { return Running; }
#endif

private:

	//! This is where you would put your event handling code for your node.
	//! It's always called with valid data. This method should never be called directly.
	virtual void HandleEvent(const EventContext& context, const Event& event) = 0;

	INodeCreator* m_creator;

	#ifdef USING_BEHAVIOR_TREE_LOG
	string m_startLog;
	string m_successLog;
	string m_failureLog;
	#endif // USING_BEHAVIOR_TREE_LOG

	#ifdef DEBUG_MODULAR_BEHAVIOR_TREE
	u32 m_xmlLine;
	#endif // DEBUG_MODULAR_BEHAVIOR_TREE

	#ifdef USING_BEHAVIOR_TREE_COMMENTS
	string m_comment;
	#endif // USING_BEHAVIOR_TREE_COMMENTS

};

DECLARE_SHARED_POINTERS(Node);

//! An object of this class will help out when reporting warnings and errors from within the modular behavior tree code.
//! If possible, it automatically adds the node type, xml line and the name of the agent
//! using the node before routing the message along to gEnv->pLog.
class ErrorReporter
{
public:
	ErrorReporter(const Node& node, const LoadContext& context)
	{

	#ifdef DEBUG_MODULAR_BEHAVIOR_TREE
		u32k xmlLine = node.GetXmlLine();
		tukk nodeTypeName = node.GetCreator()->GetTypeName();
		m_prefixString.Format("%s(%d) [Tree='%s']", nodeTypeName, xmlLine, context.treeName);
	#else // DEBUG_MODULAR_BEHAVIOR_TREE
		m_prefixString.Format("[Tree='%s']", context.treeName);
	#endif // DEBUG_MODULAR_BEHAVIOR_TREE

	}

	ErrorReporter(const Node& node, const UpdateContext& context)
	{
		const IEntity* entity = gEnv->pEntitySystem->GetEntity(context.entityId);
		tukk agentName = entity ? entity->GetName() : "(null)";

	#ifdef DEBUG_MODULAR_BEHAVIOR_TREE
		u32k xmlLine = node.GetXmlLine();
		tukk nodeTypeName = node.GetCreator()->GetTypeName();
		m_prefixString.Format("%s(%d) [%s]", nodeTypeName, xmlLine, agentName);
	#else // DEBUG_MODULAR_BEHAVIOR_TREE
		m_prefixString.Format("[%s]", agentName);
	#endif // DEBUG_MODULAR_BEHAVIOR_TREE

	}

	//! Formats a warning message with node type, xml line and the name of the agent using the node.
	//! This information is not accessible in all configurations and is thus compiled in only when possible.
	//! The message is routed through gEnv->pLog->LogWarning().
	void LogWarning(tukk format, ...) const
	{
		va_list argumentList;
		char formattedLog[512];

		va_start(argumentList, format);
		drx_vsprintf(formattedLog, format, argumentList);
		va_end(argumentList);

		stack_string finalMessage;
		finalMessage.Format("%s %s", m_prefixString.c_str(), formattedLog);
		gEnv->pLog->LogWarning("%s", finalMessage.c_str());
	}

	//! Formats a error message with node type, xml line and the name of the agent using the node.
	//! This information is not accessible in all configurations and is thus compiled in only when possible.
	//! The message is routed through gEnv->pLog->LogError().
	void LogError(tukk format, ...) const
	{
		va_list argumentList;
		char formattedLog[512];

		va_start(argumentList, format);
		drx_vsprintf(formattedLog, format, argumentList);
		va_end(argumentList);

		stack_string finalMessage;
		finalMessage.Format("%s %s", m_prefixString.c_str(), formattedLog);
		gEnv->pLog->LogError("%s", finalMessage.c_str());
	}

private:
	DrxFixedStringT<128> m_prefixString;
};
}

//! \endcond