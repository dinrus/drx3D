// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Create doc graph node factory?

#pragma once

#include <drx3D/Schema2/IScriptFile.h>

#include <drx3D/Schema2/ScriptElementBase.h>
#include <drx3D/Schema2/ScriptVariableDeclaration.h>

namespace sxema2
{
	typedef std::vector<CScriptVariableDeclaration> ScriptVariableDeclarationVector;

	struct SDocGraphNode // N.B. This is a workaround for issues with the serialization system causing serialization function from ScriptGraphBase.cpp to be called.
	{
		SDocGraphNode();
		SDocGraphNode(const IScriptGraphNodePtr& _pNode);

		IScriptGraphNodePtr pNode;
	};

	typedef std::map<SGUID, SDocGraphNode> DocGraphNodeMap;

	class CScriptGraphLink : public IScriptGraphLink
	{
	public:

		CScriptGraphLink(const SGUID& srcNodeGUID = SGUID(), tukk szSrcOutputName = nullptr, const SGUID& dstGUID = SGUID(), tukk szDstInputName = nullptr);

		// IScriptGraphLink
		virtual void SetSrcNodeGUID(const SGUID& guid) override;
		virtual SGUID GetSrcNodeGUID() const override;
		virtual tukk GetSrcOutputName() const override;
		virtual void SetDstNodeGUID(const SGUID& guid) override;
		virtual SGUID GetDstNodeGUID() const override;
		virtual tukk GetDstInputName() const override;
		virtual void Serialize(Serialization::IArchive& archive) override;
		// ~IScriptGraphLink

	private:

		SGUID  m_srcNodeGUID;
		string m_srcOutputName;
		SGUID  m_dstNodeGUID;
		string m_dstInputName;
	};

	DECLARE_SHARED_POINTERS(CScriptGraphLink)

	class CDocGraphBase : public CScriptElementBase<IDocGraph>
	{
	public:

		CDocGraphBase(IScriptFile& file, const SGUID& guid = SGUID(), const SGUID& scopeGUID = SGUID(), tukk szName = nullptr, EScriptGraphType type = EScriptGraphType::Unknown, const SGUID& contextGUID = SGUID());

		// IScriptElement
		virtual SGUID GetGUID() const override;
		virtual SGUID GetScopeGUID() const override;
		virtual bool SetName(tukk szName) override;
		virtual tukk GetName() const override;
		virtual void EnumerateDependencies(const ScriptDependancyEnumerator& enumerator) const override;
		virtual void Refresh(const SScriptRefreshParams& params) override;
		virtual void Serialize(Serialization::IArchive& archive) override; // #SchematycTODO : Split this up into two or more distinct functions which must be explicitly called by derived classes?
		virtual void RemapGUIDs(IGUIDRemapper& guidRemapper) override;
		// ~IScriptElement

		// IDocGraph
		virtual EScriptGraphType GetType() const override;
		virtual SGUID GetContextGUID() const override;
		virtual void SetPos(Vec2 pos) override;
		virtual Vec2 GetPos() const override;
		virtual size_t GetInputCount() const override;
		virtual tukk GetInputName(size_t inputIdx) const override;
		virtual IAnyConstPtr GetInputValue(size_t inputIdx) const override;
		virtual size_t GetOutputCount() const override;
		virtual tukk GetOutputName(size_t outputIdx) const override;
		virtual IAnyConstPtr GetOutputValue(size_t outputIdx) const override;
		virtual size_t GetAvailableNodeCount() override;
		virtual tukk GetAvailableNodeName(size_t availableNodeIdx) const override;
		virtual EScriptGraphNodeType GetAvailableNodeType(size_t availableNodeIdx) const override;
		virtual SGUID GetAvailableNodeContextGUID(size_t availableNodeIdx) const override;
		virtual SGUID GetAvailableNodeRefGUID(size_t availableNodeIdx) const override;
		virtual IScriptGraphNode* AddNode(EScriptGraphNodeType type, const SGUID& contextGUID = SGUID(), const SGUID& refGUID = SGUID(), Vec2 pos = Vec2()) override;
		virtual void RemoveNode(const SGUID& guid) override;
		virtual IScriptGraphNode* GetNode(const SGUID& guid) override;
		virtual const IScriptGraphNode* GetNode(const SGUID& guid) const override;
		virtual void VisitNodes(const ScriptGraphNodeVisitor& visitor) override;
		virtual void VisitNodes(const ScriptGraphNodeConstVisitor& visitor) const override;
		virtual bool CanAddLink(const SGUID& srcNodeGUID, tukk szScOutputName, const SGUID& dstNodeGUID, tukk szDstInputName) const override;
		virtual IScriptGraphLink* AddLink(const SGUID& srcNodeGUID, tukk szScOutputName, const SGUID& dstNodeGUID, tukk szDstInputName) override;
		virtual void RemoveLink(size_t linkIdx) override;
		virtual void RemoveLinks(const SGUID& nodeGUID) override;
		virtual size_t GetLinkCount() const override;
		virtual size_t FindLink(const SGUID& srcGUID, tukk szSrcOutputName, const SGUID& dstNodeGUID, tukk szDstInputName) const override;
		virtual IScriptGraphLink* GetLink(size_t linkIdx) override;
		virtual const IScriptGraphLink* GetLink(size_t linkIdx) const override;
		virtual void VisitLinks(const ScriptGraphLinkVisitor& visitor) override;
		virtual void VisitLinks(const ScriptGraphLinkConstVisitor& visitor) const override;
		virtual void RemoveBrokenLinks() override;
		virtual SScriptGraphSignals& Signals() override;
		// ~IDocGraph

		void SerializeNodeMapValue(Serialization::IArchive& archive, std::pair<SGUID, SDocGraphNode>& value);
		void AddAvailableNode(tukk szName, EScriptGraphNodeType type, const SGUID& contextGUID, const SGUID& refGUID);
		void ClearAvailablNodes();

	protected:

		ScriptVariableDeclarationVector& GetInputs();
		ScriptVariableDeclarationVector& GetOutputs();

		virtual IScriptGraphNodePtr CreateNode(const SGUID& guid, EScriptGraphNodeType type, const SGUID& contextGUID, const SGUID& refGUID, Vec2 pos) = 0;

	private:

		typedef std::vector<CScriptGraphLinkPtr> LinkVector;

		struct SInfoSerializer
		{
			SInfoSerializer(CDocGraphBase& _graph);

			void Serialize(Serialization::IArchive& archive);

			CDocGraphBase& graph;
		};

		struct SDataSerializer
		{
			SDataSerializer(CDocGraphBase& _graph);

			void Serialize(Serialization::IArchive& archive);

			CDocGraphBase& graph;
		};

		struct SNodeAndLinkSerializer
		{
			SNodeAndLinkSerializer(CDocGraphBase& _graph);

			void Serialize(Serialization::IArchive& archive);

			CDocGraphBase& graph;
		};

		struct SAvailableNode
		{
			SAvailableNode(tukk _szName, EScriptGraphNodeType _type, const SGUID& _contextGUID, const SGUID& _refGUID);

			string               name;
			EScriptGraphNodeType type;
			SGUID                contextGUID;
			SGUID                refGUID;
		};

		typedef std::vector<SAvailableNode> AvailableNodeVector;

		bool IsBrokenLink(const CScriptGraphLink& link) const;

		SGUID                           m_guid;
		SGUID                           m_scopeGUID;
		string                          m_name;
		EScriptGraphType                m_type;
		SGUID                           m_contextGUID;
		Vec2                            m_pos;
		ScriptVariableDeclarationVector m_inputs;
		ScriptVariableDeclarationVector m_outputs;
		DocGraphNodeMap                 m_nodes;
		LinkVector                      m_links;
		AvailableNodeVector             m_availableNodes;
		SScriptGraphSignals             m_signals;
	};
}

namespace std // In order to resolve ADL, std type serialization overrides must be placed in std namespace.
{
	inline bool Serialize(Serialization::IArchive& archive, shared_ptr<sxema2::CScriptGraphLink>& value, tukk szName, tukk szLabel)
	{
		if(!value)
		{
			value = std::make_shared<sxema2::CScriptGraphLink>();
		}
		archive(*value, szName, szLabel);
		return true;
	}
}
