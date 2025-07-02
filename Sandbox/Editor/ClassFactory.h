// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "IEditorClassFactory.h"
#include <drx3D/CoreX/ToolsHelpers/GuidUtil.h>

//! Class factory is a common repository of all registered plugin classes,
//! Classes here can found by their class ID or all classes of given system class retrieved
class CClassFactory : public IEditorClassFactory
{
public:
	CClassFactory();
	~CClassFactory();

	//! Access class factory singleton.
	static CClassFactory* Instance();
	//! Register a new class to the factory
	void                  RegisterClass(IClassDesc* pClassDesc);
	//! Unregister the class from the factory.
	void                  UnregisterClass(IClassDesc* pClassDesc);
	//! Find class in the factory by class name
	IClassDesc*           FindClass(tukk className) const;
	//! Get classes matching specific requirements ordered alphabetically by name.
	void                  GetClassesBySystemID(ESystemClassID aSystemClassID, std::vector<IClassDesc*>& rOutClasses);
	void                  GetClassesByCategory(tukk pCategory, std::vector<IClassDesc*>& rOutClasses);

private:
	typedef std::map<string, IClassDesc*>                   TNameMap;

	TNameMap                 m_nameToClass;
	std::vector<IClassDesc*> m_classes;
	static CClassFactory*    s_pInstance;
};

