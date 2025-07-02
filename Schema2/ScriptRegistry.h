// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/IScriptRegistry.h>

#include <drx3D/Schema2/ScriptFile.h>

namespace sxema2
{
	class CScriptRoot;

	//////////////////////////////////////////////////

	class CNewScriptFile : public INewScriptFile // #SchematycTODO : Move to separate file?
	{
	private:

		typedef std::vector<IScriptElement*> Elements;

	public:

		CNewScriptFile(const SGUID& guid, tukk szName);

		// INewScriptFile
		virtual SGUID GetGUID() const override;
		virtual tukk GetName() const override;
		virtual const CTimeValue& GetTimeStamp() const override;
		virtual void AddElement(IScriptElement& element) override;
		virtual void RemoveElement(IScriptElement& element) override;
		virtual u32 GetElementCount() const override;
		// ~INewScriptFile

	private:

		SGUID      m_guid;
		string     m_name;
		CTimeValue m_timeStamp;
		Elements   m_elements;
	};

	DECLARE_SHARED_POINTERS(CNewScriptFile)

	//////////////////////////////////////////////////

	class CScriptRegistry : public IScriptRegistry
	{
	public:

		CScriptRegistry();

		// IScriptRegistry

		// Compatibility interface.
		//////////////////////////////////////////////////
		virtual IScriptFile* LoadFile(tukk szFileName) override;
		virtual IScriptFile* CreateFile(tukk szFileName, EScriptFileFlags flags = EScriptFileFlags::None) override;
		virtual IScriptFile* GetFile(const SGUID& guid) override;
		virtual IScriptFile* GetFile(tukk szFileName) override;
		virtual void VisitFiles(const ScriptFileVisitor& visitor, tukk szFilePath = nullptr) override;
		virtual void VisitFiles(const ScriptFileConstVisitor& visitor, tukk szFilePath = nullptr) const override;
		virtual void RefreshFiles(const SScriptRefreshParams& params) override;
		virtual bool Load() override;
		virtual void Save(bool bAlwaysSave = false) override;

		// New interface.
		//////////////////////////////////////////////////
		virtual IScriptModule* AddModule(tukk szName, IScriptElement* pScope = nullptr) override;
		virtual IScriptEnumeration* AddEnumeration(tukk szName, IScriptElement* pScope = nullptr) override;
		virtual IScriptFunction* AddFunction(tukk szName, IScriptElement* pScope = nullptr) override;
		virtual IScriptClass* AddClass(tukk szName, const SGUID& foundationGUID, IScriptElement* pScope = nullptr) override;
		virtual IScriptElement* GetElement(const SGUID& guid) override;
		virtual void RemoveElement(const SGUID& guid) override;
		virtual EVisitStatus VisitElements(const ScriptElementVisitor& visitor, IScriptElement* pScope = nullptr, EVisitFlags flags = EVisitFlags::None) override;
		virtual EVisitStatus VisitElements(const ScriptElementConstVisitor& visitor, const IScriptElement* pScope = nullptr, EVisitFlags flags = EVisitFlags::None) const override;
		virtual bool IsElementNameUnique(tukk szName, IScriptElement* pScope = nullptr) const override;
		virtual SScriptRegistrySignals& Signals() override;

		// ~IScriptRegistry

	private:

		typedef std::unordered_map<u32, CScriptFilePtr>   FilesByCRC;
		typedef std::unordered_map<SGUID, CScriptFilePtr>    FilesByGUID;
		typedef std::unordered_map<SGUID, CNewScriptFilePtr> NewFiles;
		typedef std::unordered_map<SGUID, IScriptElementPtr> Elements; // #SchematycTODO : Would it make more sense to store by raw pointer here and make ownership exclusive to script file?

		CScriptFilePtr CreateFile(tukk szFileName, const SGUID& guid, EScriptFileFlags flags);
		void FormatFileName(string& fileName);
		void EnumFile(tukk szFileName, unsigned attributes);
		void RegisterFileByGuid(const CScriptFilePtr& pFile);
		
		INewScriptFile* CreateNewFile(tukk szFileName, const SGUID& guid);
		INewScriptFile* CreateNewFile(tukk szFileName);
		INewScriptFile* GetNewFile(const SGUID& guid);

		void AddElement(const IScriptElementPtr& pElement, IScriptElement& scope);
		void RemoveElement(IScriptElement& element);
		void SaveElementRecursive(IScriptElement& element, tukk szPath);

		FilesByCRC                   m_filesByCRC;
		FilesByGUID                  m_filesByGUID;
		NewFiles                     m_newFiles;
		Elements                     m_elements;
		std::shared_ptr<CScriptRoot> m_pRoot; // #SchematycTODO : Why can't we use std::unique_ptr?
		SScriptRegistrySignals       m_signals;
	};
}
