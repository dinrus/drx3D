#include "../gwenUserInterface.h"
#include "../gwenInternalData.h"
#include <X/Gwen/Controls/ImagePanel.h>
#include <X/Gwen/Controls/ColorPicker.h>
#include <drx/Core/Core.h>

class MyGraphWindow* graphWindow = 0;

GwenUserInterface::GwenUserInterface()
{
	m_data = new GwenInternalData();
	m_data->m_toggleButtonCallback = 0;
	m_data->m_comboBoxCallback = 0;
}

class MyMenuItems : public Gwen::Controls::Base
{
public:
	b3FileOpenCallback m_fileOpenCallback;
	b3QuitCallback m_quitCallback;

	MyMenuItems() : Gwen::Controls::Base(0), m_fileOpenCallback(0)
	{
	}
	void myQuitApp(Gwen::Controls::Base* pControl)
	{
		if (m_quitCallback)
		{
			(*m_quitCallback)();
		}
	}
	void fileOpen(Gwen::Controls::Base* pControl)
	{
		if (m_fileOpenCallback)
		{
			(*m_fileOpenCallback)();
		}
	}
};

struct MyTestMenuBar : public Gwen::Controls::MenuStrip
{
	Gwen::Controls::MenuItem* m_fileMenu;
	Gwen::Controls::MenuItem* m_viewMenu;
	MyMenuItems* m_menuItems;

	MyTestMenuBar(Gwen::Controls::Base* pParent)
		: Gwen::Controls::MenuStrip(pParent)
	{
		//		Gwen::Controls::MenuStrip* menu = new Gwen::Controls::MenuStrip( pParent );
		{
			m_menuItems = new MyMenuItems();
			m_menuItems->m_fileOpenCallback = 0;
			m_menuItems->m_quitCallback = 0;

			m_fileMenu = AddWItem("Файл");

			m_fileMenu->GetMenu()->AddWItem("Открыть", m_menuItems, (Gwen::Event::Handler::Function)&MyMenuItems::fileOpen);
			m_fileMenu->GetMenu()->AddWItem("Выйти", m_menuItems, (Gwen::Event::Handler::Function)&MyMenuItems::myQuitApp);
			m_viewMenu = AddWItem("Вид");
		}
	}
	virtual ~MyTestMenuBar()
	{
		delete m_menuItems;
	}
};

void GwenUserInterface::exit()
{
	//m_data->m_menubar->RemoveAllChildren();
	delete m_data->m_tab;
	delete m_data->m_windowRight;
	delete m_data->m_leftStatusBar;
	delete m_data->m_TextOutput;
	delete m_data->m_rightStatusBar;
	delete m_data->m_bar;
	delete m_data->m_menubar;

	m_data->m_menubar = 0;
	delete m_data->pCanvas;
	m_data->pCanvas = 0;
}

GwenUserInterface::~GwenUserInterface()
{
	for (i32 i = 0; i < m_data->m_handlers.size(); i++)
	{
		delete m_data->m_handlers[i];
	}

	m_data->m_handlers.clear();

	delete m_data;
}

void GwenUserInterface::resize(i32 width, i32 height)
{
	m_data->pCanvas->SetSize(width, height);
}

struct MyComboBoxHander : public Gwen::Event::Handler
{
	GwenInternalData* m_data;
	i32 m_buttonId;

	MyComboBoxHander(GwenInternalData* data, i32 buttonId)
		: m_data(data),
		  m_buttonId(buttonId)
	{
	}

	void onSelect(Gwen::Controls::Base* pControl)
	{
		Gwen::Controls::ComboBox* but = (Gwen::Controls::ComboBox*)pControl;

		Gwen::Txt str = (but->GetSelectedItem()->GetText()).ToTxt();

		if (m_data->m_comboBoxCallback)
			(*m_data->m_comboBoxCallback)(m_buttonId, str.ToStd().c_str());
	}
};

struct MyButtonHander : public Gwen::Event::Handler
{
	GwenInternalData* m_data;
	i32 m_buttonId;

	MyButtonHander(GwenInternalData* data, i32 buttonId)
		: m_data(data),
		  m_buttonId(buttonId)
	{
	}

	void onButtonA(Gwen::Controls::Base* pControl)
	{
		Gwen::Controls::Button* but = (Gwen::Controls::Button*)pControl;
		i32 tog = but->GetToggleState();
		if (m_data->m_toggleButtonCallback)
			(*m_data->m_toggleButtonCallback)(m_buttonId, tog);
	}
};

void GwenUserInterface::textOutput(tukk message)
{
	Gwen::WTxt msg = Gwen::WTxt(message);
	m_data->m_TextOutput->AddWItem(msg);
	m_data->m_TextOutput->Scroller()->ScrollToBottom();
}

void GwenUserInterface::setExampleDescription(tukk message)
{
	//Gwen apparently doesn't have text/word wrap, so do rudimentary brute-force implementation here.

	STxt wrapmessage = message;
	i32 startPos = 0;

	STxt lastFit = "";
	bool hasSpace = false;
	STxt lastFitSpace = "";
	i32 spacePos = 0;

	m_data->m_exampleInfoTextOutput->Clear();
	i32 fixedWidth = m_data->m_exampleInfoTextOutput->GetBounds().w - 25;
	i32 wrapLen = i32(wrapmessage.length());
	for (i32 endPos = 0; endPos <= wrapLen; endPos++)
	{
		STxt sub = wrapmessage.substr(startPos, (endPos - startPos));
		Gwen::Point pt = m_data->pRenderer->MeasureText(m_data->pCanvas->GetSkin()->GetDefaultFont(), sub);

		if (pt.x <= fixedWidth)
		{
			lastFit = sub;

			if (message[endPos] == ' ' || message[endPos] == '.' || message[endPos] == ',')
			{
				hasSpace = true;
				lastFitSpace = sub;
				spacePos = endPos;
			}
		}
		else
		{
			//submit and
			if (hasSpace)
			{
				endPos = spacePos + 1;
				hasSpace = false;
				lastFit = lastFitSpace;
				startPos = endPos;
			}
			else
			{
				startPos = endPos - 1;
			}
			Gwen::WTxt msg = drx::Txt(lastFit).ToWTxt();

			m_data->m_exampleInfoTextOutput->AddWItem(msg);
			m_data->m_exampleInfoTextOutput->Scroller()->ScrollToBottom();
		}
	}

	if (lastFit.length())
	{
		Gwen::WTxt msg = drx::Txt(lastFit).ToWTxt();
		m_data->m_exampleInfoTextOutput->AddWItem(msg);
		m_data->m_exampleInfoTextOutput->Scroller()->ScrollToBottom();
	}
}

void GwenUserInterface::setStatusBarMessage(tukk message, bool isLeft)
{
	Gwen::WTxt msg = Gwen::WTxt(message);
	if (isLeft)
	{
		m_data->m_leftStatusBar->SetWText(msg);
	}
	else
	{
		m_data->m_rightStatusBar->SetWText(msg);
	}
}

void GwenUserInterface::registerFileOpenCallback(b3FileOpenCallback callback)
{
	m_data->m_menuItems->m_fileOpenCallback = callback;
}

void GwenUserInterface::registerQuitCallback(b3QuitCallback callback)
{
	m_data->m_menuItems->m_quitCallback = callback;
}

void GwenUserInterface::init(i32 width, i32 height, Gwen::Renderer::Base* renderer, float retinaScale)
{
	m_data->m_curYposition = 20;
	//m_data->m_primRenderer = new GLPrimitiveRenderer(width,height);
	m_data->pRenderer = renderer;  //new GwenOpenGL3CoreRenderer(m_data->m_primRenderer,stash,width,height,retinaScale);

	m_data->skin.SetRender(m_data->pRenderer);

	m_data->pCanvas = new Gwen::Controls::Canvas(&m_data->skin);
	m_data->pCanvas->SetSize(width, height);
	m_data->pCanvas->SetDrawBackground(false);
	m_data->pCanvas->SetBackgroundColor(Gwen::Color(150, 170, 170, 255));

	MyTestMenuBar* menubar = new MyTestMenuBar(m_data->pCanvas);
	m_data->m_viewMenu = menubar->m_viewMenu;
	m_data->m_menuItems = menubar->m_menuItems;
	m_data->m_menubar = menubar;

	Gwen::Controls::StatusBar* bar = new Gwen::Controls::StatusBar(m_data->pCanvas);
	m_data->m_bar = bar;

	m_data->m_rightStatusBar = new Gwen::Controls::Label(bar);

	m_data->m_rightStatusBar->SetWidth(width / 2);
	//m_data->m_rightStatusBar->SetText( L"Label Added to Right" );
	bar->AddControl(m_data->m_rightStatusBar, true);

	m_data->m_TextOutput = new Gwen::Controls::ListBox(m_data->pCanvas);

	m_data->m_TextOutput->Dock(Gwen::Pos::Bottom);
	m_data->m_TextOutput->SetHeight(100);

	m_data->m_leftStatusBar = new Gwen::Controls::Label(bar);

	//m_data->m_leftStatusBar->SetText( L"Label Added to Left" );
	m_data->m_leftStatusBar->SetWidth(width / 2);
	bar->AddControl(m_data->m_leftStatusBar, false);

	//Gwen::KeyboardFocus
	/*Gwen::Controls::GroupBox* box = new Gwen::Controls::GroupBox(m_data->pCanvas);
	box->SetText("text");
	box->SetName("name");
	box->SetHeight(500);
	*/
	Gwen::Controls::ScrollControl* windowRight = new Gwen::Controls::ScrollControl(m_data->pCanvas);
	windowRight->Dock(Gwen::Pos::Right);
	windowRight->SetWidth(250);
	windowRight->SetHeight(250);
	windowRight->SetScroll(false, true);
	m_data->m_windowRight = windowRight;

	//windowLeft->SetSkin(
	Gwen::Controls::TabControl* tab = new Gwen::Controls::TabControl(windowRight);
	m_data->m_tab = tab;

	//tab->SetHeight(300);
	tab->SetWidth(240);
	tab->SetHeight(13250);
	//tab->Dock(Gwen::Pos::Left);
	tab->Dock(Gwen::Pos::Fill);
	//tab->SetMargin( Gwen::Margin( 2, 2, 2, 2 ) );

	Gwen::WTxt str1("Параметры");
	m_data->m_demoPage = tab->AddPage(str1);

	//	Gwen::UnicodeString str2(L"OpenCL");
	//	tab->AddPage(str2);
	//Gwen::UnicodeString str3(L"page3");
	//	tab->AddPage(str3);

	//but->onPress.Add(handler, &MyHander::onButtonA);

	//box->Dock(Gwen::Pos::Left);

	/*Gwen::Controls::WindowControl* windowBottom = new Gwen::Controls::WindowControl(m_data->pCanvas);
	windowBottom->SetHeight(100);
	windowBottom->Dock(Gwen::Pos::Bottom);
	windowBottom->SetTitle("bottom");
	*/
	//	Gwen::Controls::Property::Text* prop = new Gwen::Controls::Property::Text(m_data->pCanvas);
	//prop->Dock(Gwen::Pos::Bottom);
	/*Gwen::Controls::SplitterBar* split = new Gwen::Controls::SplitterBar(m_data->pCanvas);
	split->Dock(Gwen::Pos::Center);
	split->SetHeight(300);
	split->SetWidth(300);
	*/
	/*


	*/

	Gwen::Controls::ScrollControl* windowLeft = new Gwen::Controls::ScrollControl(m_data->pCanvas);
	windowLeft->Dock(Gwen::Pos::Left);
	//	windowLeft->SetTitle("title");
	windowLeft->SetScroll(false, false);
	windowLeft->SetWidth(250);
	windowLeft->SetPos(50, 50);
	windowLeft->SetHeight(500);
	//windowLeft->SetClosable(false);
	//	windowLeft->SetShouldDrawBackground(true);
	windowLeft->SetTabable(true);

	Gwen::Controls::TabControl* explorerTab = new Gwen::Controls::TabControl(windowLeft);

	//tab->SetHeight(300);
	//	explorerTab->SetWidth(230);
	explorerTab->SetHeight(250);
	//tab->Dock(Gwen::Pos::Left);
	explorerTab->Dock(Gwen::Pos::Fill);

	//m_data->m_exampleInfoTextOutput->SetBounds(2, 10, 236, 400);

	//windowRight

	Gwen::WTxt explorerStr1("Обозреватель");
	m_data->m_explorerPage = explorerTab->AddPage(explorerStr1);
	Gwen::WTxt shapesStr1("Тест");
	Gwen::Controls::TabButton* shapes = explorerTab->AddPage(shapesStr1);

	///todo(erwincoumans) figure out why the HSV color picker is extremely slow
	//Gwen::Controls::HSVColorPicker* color = new Gwen::Controls::HSVColorPicker(shapes->GetPage());
	Gwen::Controls::ColorPicker* color = new Gwen::Controls::ColorPicker(shapes->GetPage());
	color->SetKeyboardInputEnabled(true);

	Gwen::Controls::TreeControl* ctrl = new Gwen::Controls::TreeControl(m_data->m_explorerPage->GetPage());
	m_data->m_explorerTreeCtrl = ctrl;
	ctrl->SetKeyboardInputEnabled(true);
	ctrl->Focus();
	ctrl->SetBounds(2, 10, 236, 300);

	m_data->m_exampleInfoGroupBox = new Gwen::Controls::Label(m_data->m_explorerPage->GetPage());
	m_data->m_exampleInfoGroupBox->SetPos(2, 314);
	m_data->m_exampleInfoGroupBox->SetHeight(15);
	m_data->m_exampleInfoGroupBox->SetWidth(234);
	m_data->m_exampleInfoGroupBox->SetWText("Описание Примера");

	m_data->m_exampleInfoTextOutput = new Gwen::Controls::ListBox(m_data->m_explorerPage->GetPage());

	//m_data->m_exampleInfoTextOutput->Dock( Gwen::Pos::Bottom );
	m_data->m_exampleInfoTextOutput->SetPos(2, 332);
	m_data->m_exampleInfoTextOutput->SetHeight(150);
	m_data->m_exampleInfoTextOutput->SetWidth(233);
}

void GwenUserInterface::forceUpdateScrollBars()
{
	drx3DAssert(m_data);
	drx3DAssert(m_data->m_explorerTreeCtrl);
	if (m_data && m_data->m_explorerTreeCtrl)
	{
		m_data->m_explorerTreeCtrl->ForceUpdateScrollBars();
	}
}

void GwenUserInterface::setFocus()
{
	drx3DAssert(m_data);
	drx3DAssert(m_data->m_explorerTreeCtrl);
	if (m_data && m_data->m_explorerTreeCtrl)
	{
		m_data->m_explorerTreeCtrl->Focus();
	}
}

b3ToggleButtonCallback GwenUserInterface::getToggleButtonCallback()
{
	return m_data->m_toggleButtonCallback;
}

void GwenUserInterface::setToggleButtonCallback(b3ToggleButtonCallback callback)
{
	m_data->m_toggleButtonCallback = callback;
}
void GwenUserInterface::registerToggleButton2(i32 buttonId, tukk name)
{
	assert(m_data);
	assert(m_data->m_demoPage);

	Gwen::Controls::Button* but = new Gwen::Controls::Button(m_data->m_demoPage->GetPage());

	///some heuristic to find the button location
	i32 ypos = m_data->m_curYposition;
	but->SetPos(10, ypos);
	but->SetWidth(200);
	//but->SetBounds( 200, 30, 300, 200 );

	MyButtonHander* handler = new MyButtonHander(m_data, buttonId);
	m_data->m_handlers.push_back(handler);
	m_data->m_curYposition += 22;
	but->onToggle.Add(handler, &MyButtonHander::onButtonA);
	but->SetIsToggle(true);
	but->SetToggleState(false);
	but->SetText(name);
}

void GwenUserInterface::setComboBoxCallback(b3ComboBoxCallback callback)
{
	m_data->m_comboBoxCallback = callback;
}

b3ComboBoxCallback GwenUserInterface::getComboBoxCallback()
{
	return m_data->m_comboBoxCallback;
}
void GwenUserInterface::registerComboBox2(i32 comboboxId, i32 numItems, tukk* items, i32 startItem)
{
	Gwen::Controls::ComboBox* combobox = new Gwen::Controls::ComboBox(m_data->m_demoPage->GetPage());
	MyComboBoxHander* handler = new MyComboBoxHander(m_data, comboboxId);
	m_data->m_handlers.push_back(handler);

	combobox->onSelection.Add(handler, &MyComboBoxHander::onSelect);
	i32 ypos = m_data->m_curYposition;
	combobox->SetPos(10, ypos);
	combobox->SetWidth(100);
	//box->SetPos(120,130);
	for (i32 i = 0; i < numItems; i++)
	{
		Gwen::Controls::MenuItem* item = combobox->AddItem((drx::WTxt(items[i])));
		if (i == startItem)
			combobox->OnItemSelected(item);
	}

	m_data->m_curYposition += 22;
}

void GwenUserInterface::draw(i32 width, i32 height)
{
	//	printf("width = %d, height=%d\n", width,height);
	if (m_data->pCanvas)
	{
		m_data->pCanvas->SetSize(width, height);
		//m_data->m_primRenderer->setScreenSize(width,height);
		m_data->pRenderer->Resize(width, height);
		m_data->pCanvas->RenderCanvas();
		//restoreOpenGLState();
	}
}

bool GwenUserInterface::mouseMoveCallback(float x, float y)
{
	bool handled = false;

	static i32 m_lastmousepos[2] = {0, 0};
	static bool isInitialized = false;
	if (m_data->pCanvas)
	{
		if (!isInitialized)
		{
			isInitialized = true;
			m_lastmousepos[0] = x + 1;
			m_lastmousepos[1] = y + 1;
		}
		handled = m_data->pCanvas->InputMouseMoved(x, y, m_lastmousepos[0], m_lastmousepos[1]);
	}
	return handled;
}
#include <drx3D/Common/Interfaces/CommonWindowInterface.h>

bool GwenUserInterface::keyboardCallback(i32 bulletKey, i32 state)
{
	i32 gwenKey = -1;
	if (m_data->pCanvas)
	{
		//convert 'drx3D' keys into 'Gwen' keys
		switch (bulletKey)
		{
			case B3G_RETURN:
			{
				gwenKey = Gwen::Key::Return;
				break;
			}
			case B3G_LEFT_ARROW:
			{
				gwenKey = Gwen::Key::Left;
				break;
			}
			case B3G_RIGHT_ARROW:
			{
				gwenKey = Gwen::Key::Right;
				break;
			}
			case B3G_UP_ARROW:
			{
				gwenKey = Gwen::Key::Up;
				break;
			}
			case B3G_DOWN_ARROW:
			{
				gwenKey = Gwen::Key::Down;
				break;
			}
			case B3G_BACKSPACE:
			{
				gwenKey = Gwen::Key::Backspace;
				break;
			}
			case B3G_DELETE:
			{
				gwenKey = Gwen::Key::Delete;
				break;
			}
			case B3G_HOME:
			{
				gwenKey = Gwen::Key::Home;
				break;
			}
			case B3G_END:
			{
				gwenKey = Gwen::Key::End;
				break;
			}
			case B3G_SHIFT:
			{
				gwenKey = Gwen::Key::Shift;
				break;
			}
			case B3G_CONTROL:
			{
				gwenKey = Gwen::Key::Control;
				break;
			}

			default:
			{
			}
		};

		if (gwenKey >= 0)
		{
			return m_data->pCanvas->InputKey(gwenKey, state == 1);
		}
		else
		{
			if (bulletKey < 256 && state)
			{
				Gwen::UnicodeChar c = (Gwen::UnicodeChar)bulletKey;
				return m_data->pCanvas->InputCharacter(c);
			}
		}
	}
	return false;
}

bool GwenUserInterface::mouseButtonCallback(i32 button, i32 state, float x, float y)
{
	bool handled = false;
	if (m_data->pCanvas)
	{
		handled = m_data->pCanvas->InputMouseMoved(x, y, x, y);

		if (button >= 0)
		{
			handled = m_data->pCanvas->InputMouseButton(button, (bool)state);
			if (handled)
			{
				//if (!state)
				//	return false;
			}
		}
	}
	return handled;
}
