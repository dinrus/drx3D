// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/AI/StdAfx.h>

#include <drx3D/AI/BehaviorTreeMetaExtensions.h>

#include <drx3D/AI/IBehaviorTree.h>

namespace BehaviorTree
{

MetaExtensionFactory::MetaExtensionFactory()
	: m_metaExtensionCreators()
{
}

void MetaExtensionFactory::RegisterMetaExtensionCreator(MetaExtensionCreator metaExtensionCreator)
{
	m_metaExtensionCreators.push_back(metaExtensionCreator);
}

void MetaExtensionFactory::CreateMetaExtensions(MetaExtensionTable& metaExtensionTable)
{
	for (MetaExtensionCreator& metaExtensionCreator : m_metaExtensionCreators)
	{
		metaExtensionTable.AddMetaExtension(metaExtensionCreator());
	}
}

}
