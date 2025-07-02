#ifndef _GWEN_USER_INTERFACE_H
#define _GWEN_USER_INTERFACE_H

#include <drxtypes.h>

struct GwenInternalData;

typedef void (*b3ComboBoxCallback)(i32 combobox, tukk item);
typedef void (*b3ToggleButtonCallback)(i32 button, i32 state);
typedef void (*b3FileOpenCallback)();
typedef void (*b3QuitCallback)();

namespace Gwen
{
namespace Renderer
{
class Base;
};
};  // namespace Gwen
class GwenUserInterface
{
	GwenInternalData* m_data;

public:
	GwenUserInterface();

	virtual ~GwenUserInterface();

	void init(i32 width, i32 height, Gwen::Renderer::Base* gwenRenderer, float retinaScale);
	void exit();
	void setFocus();
	void forceUpdateScrollBars();

	void draw(i32 width, i32 height);

	void resize(i32 width, i32 height);

	bool mouseMoveCallback(float x, float y);
	bool mouseButtonCallback(i32 button, i32 state, float x, float y);
	bool keyboardCallback(i32 key, i32 state);

	void setToggleButtonCallback(b3ToggleButtonCallback callback);
	b3ToggleButtonCallback getToggleButtonCallback();

	void registerToggleButton2(i32 buttonId, tukk name);

	void setComboBoxCallback(b3ComboBoxCallback callback);
	b3ComboBoxCallback getComboBoxCallback();
	void registerComboBox2(i32 buttonId, i32 numItems, tukk* items, i32 startItem = 0);

	void setStatusBarMessage(tukk message, bool isLeft = true);

	void textOutput(tukk msg);
	void setExampleDescription(tukk msg);

	void registerFileOpenCallback(b3FileOpenCallback callback);
	void registerQuitCallback(b3QuitCallback callback);

	GwenInternalData* getInternalData()
	{
		return m_data;
	}
};

#endif  //_GWEN_USER_INTERFACE_H
