// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptGraph.h>

namespace sxema
{
struct IScriptView;
struct IScriptGraphNodeCreationMenu;

struct IScriptGraphNodeCreator
{
	virtual ~IScriptGraphNodeCreator() {}

	virtual DrxGUID             GetTypeGUID() const = 0;
	virtual IScriptGraphNodePtr CreateNode(const DrxGUID& guid) = 0;
	virtual void                PopulateNodeCreationMenu(IScriptGraphNodeCreationMenu& nodeCreationMenu, const IScriptView& scriptView, const IScriptGraph& graph) = 0;
};

DECLARE_SHARED_POINTERS(IScriptGraphNodeCreator)

class CScriptGraphNodeFactory
{
public:

	bool                     RegisterCreator(const IScriptGraphNodeCreatorPtr& pCreator);
	bool                     CanCreateNode(const DrxGUID& typeGUID, const IScriptView& scriptView, const IScriptGraph& graph) const;
	IScriptGraphNodeCreator* GetCreator(const DrxGUID& typeGUID);
	IScriptGraphNodePtr      CreateNode(const DrxGUID& typeGUID, const DrxGUID& guid = DrxGUID());
	void                     PopulateNodeCreationMenu(IScriptGraphNodeCreationMenu& nodeCreationMenu, const IScriptView& scriptView, const IScriptGraph& graph);

private:

	typedef std::unordered_map<DrxGUID, IScriptGraphNodeCreatorPtr> Creators;

	Creators m_creators;
};
}
