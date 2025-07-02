// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <vector>
#include "Rect.h"
#include "IDrawContext.h"

struct HWND__;
typedef HWND__* HWND;

class QWidget;

class PropertyTree;
class PropertyRow;
class PropertyTreeModel;
class PropertyRowNumberField;
class PropertyRowString;

namespace property_tree {

class IMenu;
class InplaceWidget
{
public:
	InplaceWidget(PropertyTree* tree) : tree_(tree) {}
	virtual ~InplaceWidget() {}
	virtual uk actualWidget() { return 0; }
	virtual void showPopup() {}
	virtual void commit() = 0;
	virtual bool autoFocus() const { return true; }
	PropertyTree* tree() { return tree_; }
protected:
	PropertyTree* tree_;
};

struct ComboBoxClientRow {
	virtual void populateComboBox(std::vector<STxt>* strings, i32* selectedIndex, PropertyTree* tree) = 0;
	virtual bool onComboBoxSelected(tukk text, PropertyTree* tree) = 0;
};

enum Key
{
	KEY_UNKNOWN = -1,
	KEY_BACKSPACE,
	KEY_DELETE,
	KEY_DOWN,
	KEY_END,
	KEY_ESCAPE,
	KEY_F2,
	KEY_HOME,
	KEY_INSERT,
	KEY_LEFT,
	KEY_MENU,
	KEY_RETURN,
	KEY_ENTER,
	KEY_RIGHT,
	KEY_SPACE,
	KEY_UP,
	KEY_C,
	KEY_V,
	KEY_Z,
	KEY_COUNT
};

enum Modifier
{
	MODIFIER_CONTROL = 1 << 0,
	MODIFIER_SHIFT = 1 << 1,
	MODIFIER_ALT = 1 << 2
};

struct KeyEvent
{
	i32 key_;
	i32 modifiers_;

	KeyEvent() : key_(0), modifiers_(0) {}

	i32 key() const { return key_; }
	i32 modifiers() const { return modifiers_; }
};


enum Cursor
{
	CURSOR_BLANK,
	CURSOR_ARROW,
	CURSOR_HAND,
	CURSOR_SLIDE
};

struct IUIFacade
{
	virtual ~IUIFacade() {}

	// only for one-platform property-row usage
	virtual QWidget* qwidget() { return 0; }
	virtual HWND hwnd() { return 0; }
	
	virtual IMenu* createMenu() = 0;
	virtual void setCursor(Cursor cursor) = 0;
	virtual void unsetCursor() = 0;
	virtual Point cursorPosition() = 0;
	virtual i32 textWidth(tukk text, Font font) = 0;
	virtual i32 textHeight(i32 width, tukk text, Font font) = 0;

	virtual Point screenSize() = 0;

	virtual InplaceWidget* createComboBox(ComboBoxClientRow* client) = 0;
	virtual InplaceWidget* createNumberWidget(PropertyRowNumberField* row) = 0;
	virtual InplaceWidget* createStringWidget(PropertyRowString* row) = 0;
};

}
using namespace property_tree;

