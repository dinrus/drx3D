// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>

#include <QString>
#include <DrxIcon.h>

namespace DrxSchematycEditor {

enum EObjectStructureItemType : i32
{
	eObjectItemType_Unset,
	eObjectItemType_Interface,
	eObjectItemType_Graph,
	eObjectItemType_State,
	eObjectItemType_StateMachine
};

class CAbstractObjectStructureModel;

class CAbstractObjectStructureModelItem
{
public:
	CAbstractObjectStructureModelItem(CAbstractObjectStructureModel& model)
		: m_model(model)
	{}
	virtual ~CAbstractObjectStructureModelItem() {}

	CAbstractObjectStructureModel&             GetModel() const                                                       { return m_model; }

	QString                                    GetName() const                                                        { return m_name; }
	virtual void                               SetName(QString name)                                                  { m_name = name; }

	virtual i32                              GetType() const                                                        { return eObjectItemType_Unset; }

	virtual QString                            GetToolTip() const                                                     { return QString(); }
	virtual const DrxIcon*                     GetIcon() const                                                        { return nullptr; }
	virtual CAbstractObjectStructureModelItem* GetParentItem() const                                                  { return nullptr; }

	virtual u32                             GetNumChildItems() const                                               { return 0; }
	virtual CAbstractObjectStructureModelItem* GetChildItemByIndex(u32 index) const                                { return nullptr; }
	virtual u32                             GetChildItemIndex(const CAbstractObjectStructureModelItem& item) const { return 0xffffffff; }
	virtual bool                               RemoveChild(CAbstractObjectStructureModelItem& item)                   { return false; }

	virtual u32                             GetIndex() const                                                       { return 0xffffffff; }
	virtual void                               Serialize(Serialization::IArchive& archive)                            {}

	virtual bool                               AllowsRenaming() const                                                 { return false; }

protected:
	CAbstractObjectStructureModel& m_model;
	QString                        m_name;
};

}

