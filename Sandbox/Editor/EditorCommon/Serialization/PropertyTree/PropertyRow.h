/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#if !defined(__clang__) && defined(__GNUC__) && (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 7))
// GCC got support for override keyword in 4.8
# define override
# define final
#endif

#include <typeinfo>
#include <algorithm>
#include <drx3D/CoreX/Serialization/yasli/Serializer.h>
#include <drx3D/CoreX/Serialization/yasli/StringList.h>
#include <drx3D/CoreX/Serialization/yasli/Pointers.h>
#include <drx3D/CoreX/Serialization/yasli/Config.h>
#include "Factory.h"
#include "ConstStringList.h"
#include "IDrawContext.h"
#include "IUIFacade.h"
#include "Rect.h"
#include "sigslot.h"

#ifdef _MSC_VER
#pragma warning(disable: 4264) // no override available for virtual member function from base 'PropertyRow'; function is hidden
#endif

class QPainter;
namespace property_tree { 
struct KeyEvent; 
class IMenu;
class InplaceWidget;
}
using property_tree::KeyEvent;

using std::vector;
class PropertyTree;
class PropertyRow;
class PropertyTreeModel;

enum ScanResult {
	SCAN_FINISHED, 
	SCAN_CHILDREN, 
	SCAN_SIBLINGS, 
	SCAN_CHILDREN_SIBLINGS,
};

struct PropertyRowMenuHandler : sigslot::has_slots
{
	virtual ~PropertyRowMenuHandler() {}
};

struct PropertyActivationEvent
{
	enum Reason
	{
		REASON_PRESS,
		REASON_RELEASE,
		REASON_DOUBLECLICK,
		REASON_KEYBOARD,
		REASON_NEW_ELEMENT,
		REASON_CONTEXT_MENU
	};

	PropertyTree* tree;
	Reason reason;
	bool force;
	Point clickPoint;

	PropertyActivationEvent()
	: force(false)
	, clickPoint(0, 0)
	, tree(0)
	, reason(REASON_PRESS)
	{
	}
};

struct PropertyDragEvent
{
	::PropertyTree* tree;
	Point pos;
	Point start;
	Point lastDelta;
	Point totalDelta;
	Modifier modifier;
};

struct PropertyHoverInfo
{
	property_tree::Cursor cursor;
	yasli::string toolTip;

	PropertyHoverInfo()
	: cursor(CURSOR_ARROW)
	{
	}
};

enum DragCheckBegin {
	DRAG_CHECK_IGNORE,
	DRAG_CHECK_SET,
	DRAG_CHECK_UNSET
};

class PropertyTreeTransaction;

class PROPERTY_TREE_API PropertyRow : public yasli::RefCounter
{
public:
	enum WidgetPlacement {
		WIDGET_NONE,
		WIDGET_ICON,
		WIDGET_AFTER_NAME,
		WIDGET_VALUE,
		WIDGET_AFTER_PULLED,
		WIDGET_INSTEAD_OF_TEXT
	};

	typedef std::vector< yasli::SharedPtr<PropertyRow> > Rows;
	typedef Rows::iterator iterator;
	typedef Rows::const_iterator const_iterator;

	PropertyRow();
	virtual ~PropertyRow();

	void setNames(tukk name, tukk label, tukk typeName);

	bool selected() const{ return selected_; }
	void setSelected(bool selected) { selected_ = selected; }
	bool expanded() const{ return expanded_; }
	void _setExpanded(bool expanded); // use PropertyTree::expandRow
	void setExpandedRecursive(PropertyTree* tree, bool expanded);

	void setMatchFilter(bool matchFilter) { matchFilter_ = matchFilter; }
	bool matchFilter() const { return matchFilter_; }

	void setBelongsToFilteredRow(bool belongs) { belongsToFilteredRow_ = belongs; }
	bool belongsToFilteredRow() const { return belongsToFilteredRow_; }

	bool visible(const PropertyTree* tree) const;
	bool hasVisibleChildren(const PropertyTree* tree, bool internalCall = false) const;

	const PropertyRow* hit(const PropertyTree* tree, Point point) const;
	PropertyRow* hit(const PropertyTree* tree, Point point);
	PropertyRow* parent() { return parent_; }
	const PropertyRow* parent() const{ return parent_; }
	void setParent(PropertyRow* row) { parent_ = row; }
	bool isRoot() const { return !parent_; }
	i32 level() const;

	void add(PropertyRow* row);
	void addAfter(PropertyRow* row, PropertyRow* after);
	void addBefore(PropertyRow* row, PropertyRow* before);

	template<class Op> bool scanChildren(Op& op);
	template<class Op> bool scanChildren(Op& op, PropertyTree* tree);
	template<class Op> bool scanChildrenReverse(Op& op, PropertyTree* tree);
	template<class Op> bool scanChildrenBottomUp(Op& op, PropertyTree* tree);

	PropertyRow* childByIndex(i32 index);
	const PropertyRow* childByIndex(i32 index) const;
	i32 childIndex(const PropertyRow* row) const;
	bool isChildOf(const PropertyRow* row) const;

	bool empty() const{ return children_.empty(); }
	iterator find(PropertyRow* row) { return std::find(children_.begin(), children_.end(), row); }
	PropertyRow* findFromIndex(i32* outIndex, tukk name, tukk typeName, i32 startIndex) const;
	PropertyRow* findByAddress(ukk handle);
	virtual ukk searchHandle() const;
	iterator begin() { return children_.begin(); }
	iterator end() { return children_.end(); }
	const_iterator begin() const{ return children_.begin(); }
	const_iterator end() const{ return children_.end(); }
	std::size_t count() const{ return children_.size(); }
	iterator erase(iterator it){ return children_.erase(it); }
	void clear(){ children_.clear(); }
	void erase(PropertyRow* row);
	void swapChildren(PropertyRow* row, PropertyTreeModel* model);

	void assignRowState(const PropertyRow& row, bool recurse);
	void assignRowProperties(PropertyRow* row);
	void replaceAndPreserveState(PropertyRow* oldRow, PropertyRow* newRow, PropertyTreeModel* model);

	tukk name() const{ return name_; }
	void setName(tukk name) { name_ = name; }
	tukk label() const { return label_; }
	tukk labelUndecorated() const { return labelUndecorated_; }
	void setLabel(tukk label);
	void setLabelChanged();
	void setTooltip(tukk tooltip);
	bool setValidatorEntry(i32 index, i32 count);
	i32 validatorCount() const{ return validatorCount_; }
	i32 validatorIndex() const{ return validatorIndex_; }
	void resetValidatorIcons();
	void addValidatorIcons(bool hasWarnings, bool hasErrors);
	tukk tooltip() const { return tooltip_; }
	void setLayoutChanged();
	void setLabelChangedToChildren();
	void setLayoutChangedToChildren();
	void setHideChildren(bool hideChildren) { hideChildren_ = hideChildren; }
	bool hideChildren() const { return hideChildren_; }
	void updateLabel(const PropertyTree* tree, i32 index, bool parentHidesNonInlineChildren);
	void updateTextSizeInitial(const PropertyTree* tree, i32 index, bool force);
	virtual void labelChanged() {}
	void parseControlCodes(const PropertyTree* tree, tukk label, bool changeLabel);
	tukk typeName() const{ return typeName_; }
	virtual tukk typeNameForFilter(PropertyTree* tree) const;
	void setTypeName(tukk typeName) { YASLI_ASSERT(strlen(typeName)); typeName_ = typeName; }
	tukk rowText(char (&containerLabelBuffer)[16], const PropertyTree* tree, i32 rowIndex) const;

	PropertyRow* findSelected();
	PropertyRow* find(tukk name, tukk nameAlt, tukk typeName);
	const PropertyRow* find(tukk name, tukk nameAlt, tukk typeName) const;
	void intersect(const PropertyRow* row);

	i32 verticalIndex(PropertyTree* tree, PropertyRow* row);
	PropertyRow* rowByVerticalIndex(PropertyTree* tree, i32 index);
	i32 horizontalIndex(PropertyTree* tree, PropertyRow* row);
	PropertyRow* rowByHorizontalIndex(PropertyTree* tree, i32 index);

	virtual bool assignToPrimitive(uk object, size_t size) const{ return false; }
	virtual bool assignTo(const yasli::Serializer& ser) const{ return false; }
	virtual bool assignToByPointer(uk instance, const yasli::TypeID& type) const{ return assignTo(yasli::Serializer(type, instance, type.sizeOf(), 0)); }
	virtual void setValueAndContext(const yasli::Serializer& ser, yasli::Archive& ar) { serializer_ = ser; }
	virtual void handleChildrenChange() {}
	virtual yasli::string valueAsString() const;
	virtual yasli::wstring valueAsWString() const;

	i32 height() const{ return size_.y(); }

	virtual i32 widgetSizeMin(const PropertyTree*) const { return userWidgetSize() >= 0 ? userWidgetSize() : 0; }
	virtual i32 widgetSizeMax(const PropertyTree*) const { return INT_MAX; }
	virtual i32 floorHeight() const{ return 0; }

	void calcPulledRows(i32* minTextSize, i32* freePulledChildren, i32* minimalWidth, const PropertyTree* tree, i32 index);
	void calculateMinimalSize(const PropertyTree* tree, i32 posX, i32 availableWidth, bool force, i32* extraSizeRemainder, i32* _extraSize, i32 index);
	void setTextSize(const PropertyTree* tree, i32 rowIndex, float multiplier);
	void calculateTotalSizes(i32* minTextSize);
	void adjustVerticalPosition(const PropertyTree* tree, i32& totalHeight);

	virtual bool isWidgetFixed() const{ return userFixedWidget_ || (widgetPlacement() != WIDGET_VALUE && widgetPlacement() != WIDGET_INSTEAD_OF_TEXT); }

	virtual WidgetPlacement widgetPlacement() const{ return WIDGET_NONE; }

	Rect rect() const{ return Rect(pos_.x(), pos_.y(), size_.x(), size_.y()); }
	Rect rectIncludingChildren(const PropertyTree* tree) const;
	Rect textRect(const PropertyTree* tree) const;
	Rect widgetRect(const PropertyTree* tree) const;
	Rect plusRect(const PropertyTree* tree) const;
	Rect floorRect(const PropertyTree* tree) const;
	Rect validatorRect(const PropertyTree* tree) const;
	Rect validatorWarningIconRect(const PropertyTree* tree) const;
	Rect validatorErrorIconRect(const PropertyTree* tree) const;
	Rect titleIconRect(const PropertyTree* tree) const;
	void adjustHoveredRect(Rect& hoveredRect);
	i32 heightIncludingChildren() const{ return heightIncludingChildren_; }
	property_tree::Font rowFont(const PropertyTree* tree) const;

	void drawRow(IDrawContext& x, const PropertyTree* tree, i32 rowIndex, bool selectionPass);

	virtual void redraw(IDrawContext& context);
	virtual property_tree::InplaceWidget* createWidget(PropertyTree* tree) { return 0; }

	virtual bool isContainer() const{ return false; }
	virtual bool isPointer() const{ return false; }
	virtual bool isObject() const{ return false; }

	virtual bool isLeaf() const{ return false; }
	virtual void closeNonLeaf(const yasli::Serializer& ser, yasli::Archive& ar) {}
	virtual bool isStatic() const{ return pulledContainer_ == 0; }
	virtual bool isSelectable() const{ return (!userReadOnly() && !userReadOnlyRecurse()) || (!pulledUp() && !pulledBefore()); }
	virtual bool activateOnAdd() const{ return false; }
	virtual bool inlineInShortArrays() const{ return false; }

	bool canBeToggled(const PropertyTree* tree) const;
	bool canBeDragged() const;
	bool canBeDroppedOn(const PropertyRow* parentRow, const PropertyRow* beforeChild, const PropertyTree* tree) const;
	void dropInto(PropertyRow* parentRow, PropertyRow* cursorRow, PropertyTree* tree, bool before);
	virtual bool getHoverInfo(PropertyHoverInfo* hit, const Point& cursorPos, const PropertyTree* tree) const { 
		hit->toolTip = tooltip_;
		return true; 
	}

	virtual bool onShowToolTip() { return false; }
	virtual void onHideToolTip() { }
	virtual bool onActivate(const PropertyActivationEvent& e);
	virtual bool onKeyDown(PropertyTree* tree, const KeyEvent* ev);
	virtual bool onMouseDown(PropertyTree* tree, Point point, bool& changed) { return false; }
	virtual void onMouseDrag(const PropertyDragEvent& e) {}
	virtual void onMouseStill(const PropertyDragEvent& e) {}
	virtual void onMouseUp(PropertyTree* tree, Point point) {}
	// "drag check" allows you to "paint" with the mouse through checkboxes to set all values at once
	virtual DragCheckBegin onMouseDragCheckBegin() { return DRAG_CHECK_IGNORE; }
	virtual bool onMouseDragCheck(PropertyTree* tree, bool value) { return false; }
	virtual bool onContextMenu(IMenu &menu, PropertyTree* tree);

	virtual bool onDataDragMove(PropertyTree* tree);
	virtual bool onDataDrop(PropertyTree* tree);

	virtual bool isFullRow(const PropertyTree* tree) const;

	// User states.
	// Assigned using control codes (characters in the beginning of label)
	// fixed widget doesn't expand automatically to occupy all available place
	bool userFixedWidget() const{ return userFixedWidget_; }
	bool userFullRow() const { return userFullRow_; }
	bool userReadOnly() const { return userReadOnly_; }
	void propagateFlagsTopToBottom();
	bool userReadOnlyRecurse() const { return userReadOnlyRecurse_; }
	bool userWidgetToContent() const { return userWidgetToContent_; }
	i32 userWidgetSize() const{ return userWidgetSize_; }

	// multiValue is used to edit properties of multiple objects simulateneously
	bool multiValue() const { return multiValue_; }
	void setMultiValue(bool multiValue) { multiValue_ = multiValue; }

	// pulledRow - is the one that is pulled up to the parents row
	// (created with ^ in the beginning of label)
	bool pulledUp() const { return pulledUp_; }
	bool pulledBefore() const { return pulledBefore_; }
	bool hasPulled() const { return hasPulled_; }
	bool packedAfterPreviousRow() const { return packedAfterPreviousRow_; }
	bool pulledSelected() const;
	bool isGroupParent(const PropertyTree* tree) const;
	PropertyRow* nonPulledParent();
	void setPulledContainer(PropertyRow* container){ pulledContainer_ = container; }
	PropertyRow* pulledContainer() { return pulledContainer_; }
	const PropertyRow* pulledContainer() const{ return pulledContainer_; }

	yasli::SharedPtr<PropertyRow> clone(ConstStringList* constStrings) const;

	yasli::Serializer serializer() const{ return serializer_; }
	virtual yasli::TypeID typeId() const{ return serializer_.type(); }
	void setSerializer(const yasli::Serializer& ser) { serializer_ = ser; }
	virtual void serializeValue(yasli::Archive& ar) {}
	void YASLI_SERIALIZE_METHOD(yasli::Archive& ar);

	void setCallback(yasli::CallbackInterface* callback);
	yasli::CallbackInterface* callback() { return callback_; }

	void setTitleIcon( tukk icon );

	static void setConstStrings(ConstStringList* constStrings){ constStrings_ = constStrings; }

protected:
	void init(tukk name, tukk nameAlt, tukk typeName);

	tukk name_;
	tukk label_;
	tukk labelUndecorated_;
	tukk typeName_;
	yasli::Serializer serializer_;
	PropertyRow* parent_;
	yasli::CallbackInterface* callback_;
	tukk tooltip_;
	Rows children_;

	u32 textHash_;

	// do we really need Point here? 
	Point pos_;
	Point size_;
	i16 textPos_;
	i16 textSizeInitial_;
	i16 textSize_;
	i16 widgetPos_; // widget == icon!
	i16 widgetSize_;
	i16 userWidgetSize_;
	unsigned short heightIncludingChildren_;
	unsigned short validatorIndex_;	
	unsigned short validatorsHeight_;
	u8 validatorCount_;
	u8 plusSize_;
	Icon titleIcon_;

	bool visible_ : 1;
	bool matchFilter_ : 1;
	bool belongsToFilteredRow_ : 1;
	bool expanded_ : 1;
	bool selected_ : 1;
	bool labelChanged_ : 1;
	bool layoutChanged_ : 1;
	bool userReadOnly_ : 1;
	bool userReadOnlyRecurse_ : 1;
	bool userFixedWidget_ : 1;
	bool userFullRow_ : 1;
	bool userHideChildren_ : 1;
	bool userPackCheckboxes_ : 1;
	bool userWidgetToContent_ : 1;
	bool pulledUp_ : 1;
	bool pulledBefore_ : 1;
	bool packedAfterPreviousRow_ : 1;
	bool hasPulled_ : 1;
	bool multiValue_ : 1;
	bool hideChildren_ : 1;
	bool validatorHasErrors_ : 1;
	bool validatorHasWarnings_ : 1;
	bool hasTitleIcon_ : 1;

	yasli::SharedPtr<PropertyRow> pulledContainer_;
	static ConstStringList* constStrings_;
	friend class PropertyOArchive;
	friend class PropertyIArchive;
};

inline u32 calculateHash(tukk str, unsigned hash = 5381)
{
	while(*str)
		hash = hash * 33 + (u8)*str++;
	return hash;
}

template<class T>
inline u32 calculateHash(const T& t, unsigned hash = 5381)
{
	for (i32 i = 0; i < sizeof(T); i++)
		hash = hash * 33 + ((u8*)&t)[i];
	return hash;
}

struct RowWidthCache
{
	u32 valueHash;
	i32 width;

	RowWidthCache() : valueHash(0), width(-1) {}
	i32 getOrUpdate(const PropertyTree* tree, const PropertyRow* rowForValue, i32 extraSpace);
};

typedef vector<yasli::SharedPtr<PropertyRow> > PropertyRows;

template<bool value>
struct StaticBool{
	enum { Value = value };
};

struct LessStrCmpPR
{
	bool operator()(tukk a, tukk b) const {
		return strcmp(a, b) < 0;
	}
};

typedef Factory<tukk , PropertyRow, Constructor0<PropertyRow>, LessStrCmpPR> PropertyRowFactory;

template<class Op>
bool PropertyRow::scanChildren(Op& op)
{
	Rows::iterator it;

	for(it = children_.begin(); it != children_.end(); ++it){
		ScanResult result = op(*it);
		if(result == SCAN_FINISHED)
			return false;
		if(result == SCAN_CHILDREN || result == SCAN_CHILDREN_SIBLINGS){
			if(!(*it)->scanChildren(op))
				return false;
			if(result == SCAN_CHILDREN)
				return false;
		}
	}
	return true;
}

template<class Op>
bool PropertyRow::scanChildren(Op& op, PropertyTree* tree)
{
	i32 numChildren = (i32)children_.size();
	for(i32 index = 0; index < numChildren; ++index){
		PropertyRow* child = children_[index];
		ScanResult result = op(child, tree, index);
		if(result == SCAN_FINISHED)
			return false;
		if(result == SCAN_CHILDREN || result == SCAN_CHILDREN_SIBLINGS){
			if(!child->scanChildren(op, tree))
				return false;
			if(result == SCAN_CHILDREN)
				return false;
		}
	}
	return true;
}

template<class Op>
bool PropertyRow::scanChildrenReverse(Op& op, PropertyTree* tree)
{
	i32 numChildren = (i32)children_.size();
	for(i32 index = numChildren - 1; index >= 0; --index){
		PropertyRow* child = children_[index];
		ScanResult result = op(child, tree, index);
		if(result == SCAN_FINISHED)
			return false;
		if(result == SCAN_CHILDREN || result == SCAN_CHILDREN_SIBLINGS){
			if(!child->scanChildrenReverse(op, tree))
				return false;
			if(result == SCAN_CHILDREN)
				return false;
		}
	}
	return true;
}

template<class Op>
bool PropertyRow::scanChildrenBottomUp(Op& op, PropertyTree* tree)
{
	size_t numChildren = children_.size();
	for(size_t i = 0; i < numChildren; ++i)
	{
		PropertyRow* child = children_[i];
		if(!child->scanChildrenBottomUp(op, tree))
			return false;
		ScanResult result = op(child, tree);
		if(result == SCAN_FINISHED)
			return false;
	}
	return true;
}

template<class T>
inline void visitPulledRows(PropertyRow* row, T& drawFunc) 
{
	i32 count = (i32)row->count();
	for (i32 i = 0; i < count; ++i) {
		PropertyRow* child = row->childByIndex(i);
		if (child->pulledUp() || child->pulledBefore()) {
			drawFunc(child);
			visitPulledRows(child, drawFunc);
		}
	}
};

PROPERTY_TREE_API PropertyRowFactory& globalPropertyRowFactory();
PROPERTY_TREE_API yasli::ClassFactory<PropertyRow>& globalPropertyRowClassFactory();

struct PropertyRowPtrSerializer : yasli::SharedPtrSerializer<PropertyRow>
{
	PropertyRowPtrSerializer(yasli::SharedPtr<PropertyRow>& ptr) : SharedPtrSerializer(ptr) {}
	yasli::ClassFactory<PropertyRow>* factory() const override { return &globalPropertyRowClassFactory(); }
};

inline bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, yasli::SharedPtr<PropertyRow>& ptr, tukk name, tukk label)
{
	PropertyRowPtrSerializer serializer(ptr);
	return ar(static_cast<yasli::PointerInterface&>(serializer), name, label);
}

#define REGISTER_PROPERTY_ROW(DataType, RowType) \
	REGISTER_IN_PROPERTY_ROW_FACTORY(yasli::TypeID::get<DataType>().name(), RowType); \
	YASLI_CLASS_NAME(PropertyRow, RowType, #RowType, #DataType);



