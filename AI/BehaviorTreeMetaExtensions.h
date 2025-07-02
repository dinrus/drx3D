// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/IBehaviorTree.h>

namespace BehaviorTree
{
class MetaExtensionFactory : public IMetaExtensionFactory
{
public:

	MetaExtensionFactory();

	// IMetaExtensionFactory
	virtual void RegisterMetaExtensionCreator(MetaExtensionCreator metaExtensionCreator) override;
	virtual void CreateMetaExtensions(MetaExtensionTable& metaExtensionTable) override;
	// ~IExtensionFactory

private:

	typedef DynArray<MetaExtensionCreator> MetaExtensionCreatorArray;

	MetaExtensionCreatorArray m_metaExtensionCreators;
};
}
