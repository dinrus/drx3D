// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Entity/IEntityClass.h>

#include <drx3D/Schema/IRuntimeRegistry.h>

//////////////////////////////////////////////////////////////////////////
// Описание:
//    Standard implementation of the IEntityClass interface.
//////////////////////////////////////////////////////////////////////////
class CEntityClass : public IEntityClass
{
public:
	CEntityClass();
	virtual ~CEntityClass();

	//////////////////////////////////////////////////////////////////////////
	// IEntityClass interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual void                      Release() override                { delete this; };

	virtual u32                    GetFlags() const override         { return m_nFlags; };
	virtual void                      SetFlags(u32 nFlags) override  { m_nFlags = nFlags; };

	virtual tukk               GetName() const override          { return m_sName.c_str(); }
	virtual DrxGUID                   GetGUID() const final             { return m_guid; };
	virtual tukk               GetScriptFile() const override    { return m_sScriptFile.c_str(); }

	virtual IEntityScript*            GetIEntityScript() const override { return m_pEntityScript; }
	virtual IScriptTable*             GetScriptTable() const override;
	virtual bool                      LoadScript(bool bForceReload) override;
	virtual UserProxyCreateFunc       GetUserProxyCreateFunc() const override { return m_pfnUserProxyCreate; };
	virtual uk                     GetUserProxyData() const override       { return m_pUserProxyUserData; };

	virtual IEntityEventHandler*      GetEventHandler() const override;
	virtual IEntityScriptFileHandler* GetScriptFileHandler() const override;

	virtual const SEditorClassInfo&  GetEditorClassInfo() const override;
	virtual void                     SetEditorClassInfo(const SEditorClassInfo& editorClassInfo) override;

	virtual i32                      GetEventCount() override;
	virtual IEntityClass::SEventInfo GetEventInfo(i32 nIndex) override;
	virtual bool                     FindEventInfo(tukk sEvent, SEventInfo& event) override;

	virtual const OnSpawnCallback&   GetOnSpawnCallback() const override { return m_onSpawnCallback; };

	//////////////////////////////////////////////////////////////////////////

	void                             SetClassDesc(const IEntityClassRegistry::SEntityClassDesc& classDesc);

	void                             SetName(tukk sName);
	void                             SetGUID(const DrxGUID& guid);
	void                             SetScriptFile(tukk sScriptFile);
	void                             SetEntityScript(IEntityScript* pScript);

	void                             SetUserProxyCreateFunc(UserProxyCreateFunc pFunc, uk pUserData = NULL);
	void                             SetEventHandler(IEntityEventHandler* pEventHandler);
	void                             SetScriptFileHandler(IEntityScriptFileHandler* pScriptFileHandler);

	void                             SetOnSpawnCallback(const OnSpawnCallback& callback);

	sxema::IRuntimeClassConstPtr GetSchematycRuntimeClass() const;

	void                             GetMemoryUsage(IDrxSizer* pSizer) const override
	{
		pSizer->AddObject(m_sName);
		pSizer->AddObject(m_sScriptFile);
		pSizer->AddObject(m_pEntityScript);
		pSizer->AddObject(m_pEventHandler);
		pSizer->AddObject(m_pScriptFileHandler);
	}
private:
	u32                                   m_nFlags;
	string                                   m_sName;
	DrxGUID                                  m_guid;
	string                                   m_sScriptFile;
	IEntityScript*                           m_pEntityScript;

	UserProxyCreateFunc                      m_pfnUserProxyCreate;
	uk                                    m_pUserProxyUserData;

	bool                                     m_bScriptLoaded;

	IEntityEventHandler*                     m_pEventHandler;
	IEntityScriptFileHandler*                m_pScriptFileHandler;

	SEditorClassInfo                         m_EditorClassInfo;

	OnSpawnCallback                          m_onSpawnCallback;
	DrxGUID                                  m_schematycRuntimeClassGuid;

	IFlowNodeFactory*                        m_pIFlowNodeFactory = nullptr;

	mutable sxema::IRuntimeClassConstPtr m_pSchematycRuntimeClass = nullptr;
};
