// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Move to script folder.

#pragma once

#include <drx3D/Schema2/IScriptGraph.h>

namespace sxema2
{
	struct IDomainContext;
	struct IScriptGraphNodeCreationMenu;

	struct IScriptGraphNodeCreator
	{
		virtual ~IScriptGraphNodeCreator() {}

		virtual SGUID GetTypeGUID() const = 0;
		virtual IScriptGraphNodePtr CreateNode() = 0; // #SchematycTODO : Do we really need to pass graph parameter here?
		virtual void PopulateNodeCreationMenu(IScriptGraphNodeCreationMenu& nodeCreationMenu, const IDomainContext& domainContext, const IScriptGraphExtension& graph) = 0;
	};

	DECLARE_SHARED_POINTERS(IScriptGraphNodeCreator)

	class CScriptGraphNodeFactory
	{
	public:

		bool RegisterCreator(const IScriptGraphNodeCreatorPtr& pCreator);
		IScriptGraphNodeCreator* GetCreator(const SGUID& typeGUID);
		IScriptGraphNodePtr CreateNode(const SGUID& typeGUID); // #SchematycTODO : Do we really need to pass graph parameter here?
		void PopulateNodeCreationMenu(IScriptGraphNodeCreationMenu& nodeCreationMenu, const IDomainContext& domainContext, const IScriptGraphExtension& graph);

	private:

		typedef std::unordered_map<SGUID, IScriptGraphNodeCreatorPtr> Creators;

		Creators m_creators;
	};
}
