#ifndef GWEN_INTERNAL_DATA_H
#define GWEN_INTERNAL_DATA_H

#include <drx3D/OpenGLWindow/GwenOpenGL3CoreRenderer.h>
#include <drx3D/OpenGLWindow/GLPrimitiveRenderer.h>
#include <X/Gwen/Platform.h>
#include <X/Gwen/Controls/TreeControl.h>
#include <X/Gwen/Controls/RadioButtonController.h>
#include <X/Gwen/Controls/VerticalSlider.h>
#include <X/Gwen/Controls/HorizontalSlider.h>
#include <X/Gwen/Controls/GroupBox.h>
#include <X/Gwen/Controls/CheckBox.h>
#include <X/Gwen/Controls/StatusBar.h>
#include <X/Gwen/Controls/Button.h>
#include <X/Gwen/Controls/ComboBox.h>
#include <X/Gwen/Controls/MenuStrip.h>
#include <X/Gwen/Controls/Slider.h>
#include <X/Gwen/Controls/Property/Text.h>
#include <X/Gwen/Controls/SplitterBar.h>
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <X/Gwen/Gwen.h>
#include <X/Gwen/Align.h>
#include <X/Gwen/Utility.h>
#include <X/Gwen/Controls/WindowControl.h>
#include <X/Gwen/Controls/TabControl.h>
#include <X/Gwen/Controls/ListBox.h>
#include <X/Gwen/Skins/Simple.h>
#include "gwenUserInterface.h"

struct GwenInternalData
{
	//struct sth_stash;
	//class GwenOpenGL3CoreRenderer*	pRenderer;
	Gwen::Renderer::Base* pRenderer;
	Gwen::Skin::Simple skin;
	Gwen::Controls::Canvas* pCanvas;
	//GLPrimitiveRenderer* m_primRenderer;
	Gwen::Controls::TabButton* m_demoPage;
	Gwen::Controls::TabButton* m_explorerPage;
	Gwen::Controls::TreeControl* m_explorerTreeCtrl;
	Gwen::Controls::MenuItem* m_viewMenu;
	class MyMenuItems* m_menuItems;
	Gwen::Controls::ListBox* m_TextOutput;
	Gwen::Controls::Label* m_exampleInfoGroupBox;
	Gwen::Controls::ListBox* m_exampleInfoTextOutput;
	struct MyTestMenuBar* m_menubar;
	Gwen::Controls::StatusBar* m_bar;
	Gwen::Controls::ScrollControl* m_windowRight;
	Gwen::Controls::TabControl* m_tab;

	i32 m_curYposition;

	Gwen::Controls::Label* m_rightStatusBar;
	Gwen::Controls::Label* m_leftStatusBar;
	b3AlignedObjectArray<class Gwen::Event::Handler*> m_handlers;
	b3ToggleButtonCallback m_toggleButtonCallback;
	b3ComboBoxCallback m_comboBoxCallback;
};

#endif  //GWEN_INTERNAL_DATA_H
