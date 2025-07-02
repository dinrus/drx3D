// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptGraph.h>

#include <drx3D/Schema/ScriptElementBase.h>

namespace sxema
{

class CScriptGraphLink : public IScriptGraphLink
{
public:

	CScriptGraphLink();
	CScriptGraphLink(const DrxGUID& srcNodeGUID, const CUniqueId& srcOutputId, const DrxGUID& dstNodeGUID, const CUniqueId& dstInputId);
	CScriptGraphLink(const DrxGUID& srcNodeGUID, tukk szSrcOutputName, const DrxGUID& dstNodeGUID, tukk szDstInputName);

	// IScriptGraphLink
	virtual void      SetSrcNodeGUID(const DrxGUID& guid) override;
	virtual DrxGUID   GetSrcNodeGUID() const override;
	virtual CUniqueId GetSrcOutputId() const override;
	virtual void      SetDstNodeGUID(const DrxGUID& guid) override;
	virtual DrxGUID   GetDstNodeGUID() const override;
	virtual CUniqueId GetDstInputId() const override;
	virtual void      Serialize(Serialization::IArchive& archive) override;
	// ~IScriptGraphLink

	void        SetSrcOutputId(const CUniqueId& id);
	tukk GetSrcOutputName() const;
	void        SetDstInputId(const CUniqueId& id);
	tukk GetDstInputName() const;

private:

	DrxGUID   m_srcNodeGUID;
	CUniqueId m_srcOutputId;
	string    m_srcOutputName;
	DrxGUID   m_dstNodeGUID;
	CUniqueId m_dstInputId;
	string    m_dstInputName;
};

DECLARE_SHARED_POINTERS(CScriptGraphLink)

class CScriptGraph : public IScriptGraph
{
private:

	typedef std::map<DrxGUID, IScriptGraphNodePtr> Nodes;
	typedef std::vector<CScriptGraphLinkPtr>       Links;

	struct SSignals
	{
		ScriptGraphLinkRemovedSignal linkRemoved;
	};

public:

	CScriptGraph(IScriptElement& element, EScriptGraphType type);

	// IScriptExtension
	virtual void EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const override;
	virtual void RemapDependencies(IGUIDRemapper& guidRemapper) override;
	virtual void ProcessEvent(const SScriptEvent& event) override;
	virtual void Serialize(Serialization::IArchive& archive) override;
	// ~IScriptExtension

	// IScriptGraph
	virtual EScriptGraphType                     GetType() const override;
	virtual IScriptElement&                      GetElement() override;
	virtual const IScriptElement&                GetElement() const override;

	virtual void                                 SetPos(Vec2 pos) override;
	virtual Vec2                                 GetPos() const override;

	virtual void                                 PopulateNodeCreationMenu(IScriptGraphNodeCreationMenu& nodeCreationMenu) override;
	virtual bool                                 AddNode(const IScriptGraphNodePtr& pNode) override;
	virtual IScriptGraphNodePtr                  AddNode(const DrxGUID& typeGUID) override;
	virtual void                                 RemoveNode(const DrxGUID& guid) override;
	virtual u32                               GetNodeCount() const override;
	virtual IScriptGraphNode*                    GetNode(const DrxGUID& guid) override;
	virtual const IScriptGraphNode*              GetNode(const DrxGUID& guid) const override;
	virtual void                                 VisitNodes(const ScriptGraphNodeVisitor& visitor) override;
	virtual void                                 VisitNodes(const ScriptGraphNodeConstVisitor& visitor) const override;

	virtual bool                                 CanAddLink(const DrxGUID& srcNodeGUID, const CUniqueId& srcOutputId, const DrxGUID& dstNodeGUID, const CUniqueId& dstInputId) const override;
	virtual IScriptGraphLink*                    AddLink(const DrxGUID& srcNodeGUID, const CUniqueId& srcOutputId, const DrxGUID& dstNodeGUID, const CUniqueId& dstInputId) override;
	virtual void                                 RemoveLink(u32 linkIdx) override;
	virtual void                                 RemoveLinks(const DrxGUID& nodeGUID) override;
	virtual u32                               GetLinkCount() const override;
	virtual IScriptGraphLink*                    GetLink(u32 linkIdx) override;
	virtual const IScriptGraphLink*              GetLink(u32 linkIdx) const override;
	virtual u32                               FindLink(const DrxGUID& srcNodeGUID, const CUniqueId& srcOutputId, const DrxGUID& dstNodeGUID, const CUniqueId& dstInputId) const override;
	virtual EVisitResult                         VisitLinks(const ScriptGraphLinkVisitor& visitor) override;
	virtual EVisitResult                         VisitLinks(const ScriptGraphLinkConstVisitor& visitor) const override;
	virtual EVisitResult                         VisitInputLinks(const ScriptGraphLinkVisitor& visitor, const DrxGUID& dstNodeGUID, const CUniqueId& dstInputId) override;
	virtual EVisitResult                         VisitInputLinks(const ScriptGraphLinkConstVisitor& visitor, const DrxGUID& dstNodeGUID, const CUniqueId& dstInputId) const override;
	virtual EVisitResult                         VisitOutputLinks(const ScriptGraphLinkVisitor& visitor, const DrxGUID& srcNodeGUID, const CUniqueId& srcOutputId) override;
	virtual EVisitResult                         VisitOutputLinks(const ScriptGraphLinkConstVisitor& visitor, const DrxGUID& srcNodeGUID, const CUniqueId& srcOutputId) const override;
	virtual bool                                 GetLinkSrc(const IScriptGraphLink& link, IScriptGraphNode*& pNode, u32& outputIdx) override;
	virtual bool                                 GetLinkSrc(const IScriptGraphLink& link, const IScriptGraphNode*& pNode, u32& outputIdx) const override;
	virtual bool                                 GetLinkDst(const IScriptGraphLink& link, IScriptGraphNode*& pNode, u32& inputIdx) override;
	virtual bool                                 GetLinkDst(const IScriptGraphLink& link, const IScriptGraphNode*& pNode, u32& inputIdx) const override;

	virtual void                                 RemoveBrokenLinks() override;
	virtual ScriptGraphLinkRemovedSignal::Slots& GetLinkRemovedSignalSlots() override;

	virtual void                                 FixMapping(IScriptGraphNode& node) override;
	// ~IScriptGraph

private:

	template<typename PREDICATE> inline void RemoveLinks(PREDICATE predicate)
	{
		for (u32 linkIdx = 0, linkCount = m_links.size(); linkIdx < linkCount; )
		{
			const CScriptGraphLink& link = *m_links[linkIdx];
			if (predicate(link))
			{
				m_signals.linkRemoved.Send(link);
				m_links.erase(m_links.begin() + linkIdx);
				--linkCount;
			}
			else
			{
				++linkIdx;
			}
		}
	}

	void PatchLinks();

private:

	IScriptElement&  m_element;
	EScriptGraphType m_type;
	Vec2             m_pos;
	Nodes            m_nodes;
	Links            m_links;
	SSignals         m_signals;
};

DECLARE_SHARED_POINTERS(CScriptGraph)

} // sxema
