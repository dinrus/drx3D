// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QPropertyTree/ContextList.h>
#include <DrxSchematyc/Env/IEnvRegistry.h>
#include <DrxSchematyc/Services/ISettingsManager.h>
#include <DrxSchematyc/Utils/Signal.h>

namespace DrxSchematycEditor {

enum class EDetailItemType
{
	Empty,
	Settings,
	ScriptElement,
	GraphViewNode
};

struct IDetailItem
{
	virtual ~IDetailItem() {}

	virtual EDetailItemType              GetType() const = 0;
	virtual DrxGUID             GetGUID() const = 0;
	virtual Serialization::CContextList* GetContextList() = 0;
	virtual void                         Serialize(Serialization::IArchive& archive) = 0;
};

DECLARE_SHARED_POINTERS(IDetailItem)

class CSettingsDetailItem : public IDetailItem
{
public:
	CSettingsDetailItem(const Schematyc::ISettingsPtr& pSettings);

	// IDetailItem
	virtual EDetailItemType              GetType() const override;
	virtual DrxGUID             GetGUID() const override;
	virtual Serialization::CContextList* GetContextList() override;
	virtual void                         Serialize(Serialization::IArchive& archive) override;
	// ~IDetailItem

private:
	Schematyc::ISettingsPtr m_pSettings;
};
DECLARE_SHARED_POINTERS(CSettingsDetailItem)

class CScriptElementDetailItem : public IDetailItem
{
public:
	CScriptElementDetailItem(Schematyc::IScriptElement* pScriptElement);

	// IDetailItem
	virtual EDetailItemType              GetType() const override;
	virtual DrxGUID             GetGUID() const override;
	virtual Serialization::CContextList* GetContextList() override;
	virtual void                         Serialize(Serialization::IArchive& archive) override;
	// ~IDetailItem

private:
	Schematyc::IScriptElement*                   m_pScriptElement;
	std::unique_ptr<Serialization::CContextList> m_pContextList;
};
DECLARE_SHARED_POINTERS(CScriptElementDetailItem)

}

