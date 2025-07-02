// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Serialization/PropertyTree/IDrawContext.h"
#include "Serialization/PropertyTree/PropertyRowField.h"
#include "Serialization/QPropertyTree/QPropertyTree.h"
#include "Serialization/PropertyTree/PropertyTreeModel.h"
#include "Serialization/PropertyTree/Imenu.h"
#include "Serialization.h"

//TODO : this file is in editor package, breaking intended package dependencies
#include "Serialization/Decorators/EntityLink.h"

using Serialization::PrefabLink;

class CPrefabObject;

class PropertyRowPrefabLink : public PropertyRowField
{
public:
	~PropertyRowPrefabLink();

	void                        pick(PropertyTree* tree);
	void                        select();
	void                        clear();

	bool                        isLeaf() const override   { return true; }
	bool                        isStatic() const override { return false; }

	void                        setValueAndContext(const Serialization::SStruct& ser, Serialization::IArchive& ar) override;
	bool                        assignTo(const Serialization::SStruct& ser) const override;
	bool                        onActivate(const PropertyActivationEvent& e) override;

	virtual bool                usePathEllipsis() const override { return true; }
	string                      valueAsString() const;
	void                        serializeValue(Serialization::IArchive& ar);
	ukk                 searchHandle() const override { return handle_; }
	Serialization::TypeID       typeId() const override       { return type_; }
	const DrxGUID& GetGUID()                     { return guid_; }
	CPrefabObject* GetOwner()                    { return owner_; }

	bool           onContextMenu(IMenu& menu, PropertyTree* tree) override;

private:
	struct Picker;

	DrxGUID                 guid_;
	string                  name_;
	std::unique_ptr<Picker> picker_;
	DrxGUID                 owner_guid_;
	CPrefabObject*          owner_;

	Serialization::TypeID   type_;
	ukk             handle_;
};

struct PrefabLinkMenuHandler : PropertyRowMenuHandler
{
public:
	PropertyTree*          tree;
	PropertyRowPrefabLink* self;

	PrefabLinkMenuHandler(PropertyTree* tree, PropertyRowPrefabLink* container);

	void onMenuPick();
	void onMenuSelect();
	void onMenuDelete();
	void onMenuClone();
	void onMenuExtract();
};

