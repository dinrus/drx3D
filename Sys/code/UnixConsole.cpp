// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:
   Console implementation for UNIX systems, based on curses/ncurses.
   -------------------------------------------------------------------------
   История:
   - Jul 04,2006:	Created by Sascha Demetrio

*************************************************************************/

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/System.h>
#include <drx3D/Sys/UnixConsole.h>

#include <drx3D/CoreX/Thread/IThreadUpr.h>

#if defined(USE_DEDICATED_SERVER_CONSOLE)

	#if defined(USE_UNIXCONSOLE)

		#if defined(_MSC_VER)
LINK_SYSTEM_LIBRARY("pdcurses.lib")
		#endif // _MSC_VER

		#if !DRX_PLATFORM_WINDOWS
			#include <sys/types.h>
			#include <sys/select.h>
			#include <unistd.h>
			#include <signal.h>
			#if DRX_PLATFORM_MAC
				#include <util.h>
				#include <sys/ioctl.h>
			#else
				#include <pty.h>
			#endif // DRX_PLATFORM_MAC
			#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_MAC
				#include <syslog.h>
			#endif
		#endif

		#if defined(UC_ENABLE_PLAYER_COUNT)
// TODO: The player count feature currently only works on Linux, because we're
// calling across DLL boundaries.  The GetNumPlayers() functionality _should_
// be exposed through INetwork.
			#include <drx3D/Script/IScriptSystem.h>
			#include <drx3D/Network/INetwork.h>
			#include <drx3D/Act/DinrusAction.h>
			#include <drx3D/Act/GameServerNub.h>
		#endif

		#include <drx3D/CoreX/Game/IGameFramework.h>

		#define UNIXConsole_MORE_LEFT    "<<"
		#define UNIXConsole_MORE_RIGHT   ">>"
		#define UNIXConsole_MORE_COLOR   (3) // Colorpair index
		#define UNIXConsole_PROMPT       "] "
		#define UNIXConsole_PROMPT_COLOR (2) // Colorpair index
		#define UNIXConsole_WRAP_CHAR    '\\'
		#define UNIXConsole_WRAP_COLOR   (4)
		#define UNIXConsole_MIN_WIDTH    (10)
		#define UNIXConsole_MAX_LINES    (1000)
		#define UNIXConsole_MAX_HISTORY  (100)

		#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_MAC
// Should find a better test for ncurses...
			#define NCURSES 1
		#endif

// macro which check if we should process console function
// used for disable console on Linux
		#define IS_SHOW_CONSOLE if (!m_bShowConsole) return;
		#define IS_SHOW_CONSOLE_RET(ret) if (!m_bShowConsole) return ret;

DrxCriticalSectionNonRecursive CUNIXConsole::m_cleanupLock;

class CUNIXConsoleInputThread : public IThread
{
	CUNIXConsole& m_UNIXConsole;
		#if !DRX_PLATFORM_WINDOWS
	i32           m_IntrPipe[2];
		#else
	HANDLE        m_IntrEvent;
		#endif
	bool          m_Cancelled;

public:
	CUNIXConsoleInputThread(CUNIXConsole& UNIXConsole)
		: m_UNIXConsole(UNIXConsole), m_Cancelled(false)
	{
		#if !DRX_PLATFORM_WINDOWS
		pipe(m_IntrPipe);
		#else
		m_IntrEvent = CreateEvent(NULL, true, false, NULL);
		#endif
	}

	~CUNIXConsoleInputThread()
	{
		#if !DRX_PLATFORM_WINDOWS
		close(m_IntrPipe[0]);
		close(m_IntrPipe[1]);
		#else
		CloseHandle(m_IntrEvent);
		#endif
	}

	// Start accepting work on thread
	virtual void ThreadEntry();

	// Signals the thread that it should not accept anymore work and exit
	void SignalStopWork() { m_Cancelled = true; Interrupt(); }

	void Interrupt()
	{
		#if !DRX_PLATFORM_WINDOWS
		write(m_IntrPipe[1], "", 1);
		#else
		SetEvent(m_IntrEvent);
		#endif
	}
};

class CUNIXConsoleSignalHandler
{
	friend class CUNIXConsole;

	static CUNIXConsole* m_pUNIXConsole;
	static void Handler(i32 signum);
};

CUNIXConsole::CUNIXConsole()
	: m_HistoryIndex(-1)
	, m_PromptResponseChars{0}
	, m_PromptResponse(0)
	, m_pSystem(NULL)
	, m_pConsole(NULL)
	, m_pTimer(NULL)
	, m_OnUpdateCalled(false)
	, m_LastUpdateTime(0.0f)
	, m_svMap(NULL)
	, m_svGameRules(NULL)
	, m_Width(~0)
	, m_Height(~0)
	, m_HeaderHeight(1)
	, m_StatusHeight(1)
	, m_CmdHeight(2)
	, m_Color(DEFAULT_COLOR)
	, m_DefaultColorPair(-1)
	, m_EnableColor(true)
	, m_WindowResized(false)
	, m_OnShutdownCalled(false)
	, m_Initialized(false)
	, m_RequireDedicatedServer(false)
	, m_ScrollUp(0)
	, m_ColorPair{0}
	, m_pInputThread(NULL)
	, m_CursorPosition(0)
	, m_ScrollPosition(0)
	, m_fsMode(false)
	, m_bShowConsole(true)
{}

CUNIXConsole::~CUNIXConsole()
{
	Cleanup();
	m_Lock.Lock();
}

void CUNIXConsole::SetRequireDedicatedServer(bool value)
{
	assert(!m_Initialized);
	m_RequireDedicatedServer = value;
}

		#if DRX_PLATFORM_WINDOWS
void ResizeConBufAndWindow(HANDLE hConsole, SHORT xSize, SHORT ySize)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi; /* hold current console buffer info */
	BOOL bSuccess;
	SMALL_RECT srWindowRect; /* hold the new console size */
	COORD coordScreen;

	bSuccess = GetConsoleScreenBufferInfo(hConsole, &csbi);
	/* get the largest size we can size the console window to */
	coordScreen = GetLargestConsoleWindowSize(hConsole);
	/* define the new console window size and scroll position */
	srWindowRect.Right = (SHORT) (min(xSize, coordScreen.X) - 1);
	srWindowRect.Bottom = (SHORT) (min(ySize, coordScreen.Y) - 1);
	srWindowRect.Left = srWindowRect.Top = (SHORT) 0;
	/* define the new console buffer size */
	coordScreen.X = xSize;
	coordScreen.Y = ySize;
	/* if the current buffer is larger than what we want, resize the */
	/* console window first, then the buffer */
	if ((DWORD) csbi.dwSize.X * csbi.dwSize.Y > (DWORD) xSize * ySize)
	{
		bSuccess = SetConsoleWindowInfo(hConsole, TRUE, &srWindowRect);
		bSuccess = SetConsoleScreenBufferSize(hConsole, coordScreen);
		bSuccess = SetConsoleWindowInfo(hConsole, TRUE, &srWindowRect);
		bSuccess = SetConsoleScreenBufferSize(hConsole, coordScreen);
	}
	/* if the current buffer is smaller than what we want, resize the */
	/* buffer first, then the console window */
	if ((DWORD) csbi.dwSize.X * csbi.dwSize.Y < (DWORD) xSize * ySize)
	{
		bSuccess = SetConsoleScreenBufferSize(hConsole, coordScreen);
		bSuccess = SetConsoleWindowInfo(hConsole, TRUE, &srWindowRect);
	}
	/* if the current buffer *is* the size we want, don't do anything! */
	return;
}

BOOL WINAPI CtrlHandler(DWORD evt)
{
	switch (evt)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
		return TRUE;
	case CTRL_CLOSE_EVENT:
		gEnv->pSystem->Quit();
		return TRUE;
	}
	return FALSE;
}
		#endif

void CUNIXConsole::Init(tukk headerString)
{
	assert(!m_Initialized);

	if (headerString != NULL)
		m_HeaderString = headerString;

		#if DRX_PLATFORM_WINDOWS
	// Allocate a console for the process. We don't care if this is successful
	// or not, because failure indicates that the process already has a
	// console, which is fine.
	AllocConsole();
	HANDLE hConsOut = GetStdHandle(STD_OUTPUT_HANDLE);
	ResizeConBufAndWindow(hConsOut, 120, 60);
	SetConsoleCtrlHandler(CtrlHandler, TRUE);
		#endif

	// Initialize curses.
	initscr();
	cbreak();
	noecho();
	nonl();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	scrollok(stdscr, TRUE);
	idcok(stdscr, TRUE);
	idlok(stdscr, TRUE);
	nodelay(stdscr, TRUE);

	// Enable color output.
	if (m_EnableColor)
	{
		if (start_color() != OK)
			m_EnableColor = false;
	}

	if (m_EnableColor)
	{
		#if defined(NCURSES)
		// Setup the color table.
		short colorPair = 0;
		attr_t attr;
		attr_get(&attr, &colorPair, NULL);
		m_DefaultColorPair = colorPair;
		m_ColorPair[0] = m_DefaultColorPair;
		m_ColorPair[1] = m_DefaultColorPair;
		short color_fg = 0, color_bg = 0;
		pair_content(m_DefaultColorPair, &color_fg, &color_bg);
		short pair = 0;
		init_pair(++pair, COLOR_BLUE, color_bg);
		m_ColorPair[2] = pair;
		init_pair(++pair, COLOR_GREEN, color_bg);
		m_ColorPair[3] = pair;
		init_pair(++pair, COLOR_RED, color_bg);
		m_ColorPair[4] = pair;
		init_pair(++pair, COLOR_CYAN, color_bg);
		m_ColorPair[5] = pair;
		init_pair(++pair, COLOR_YELLOW, COLOR_BLACK);
		m_ColorPair[6] = pair;
		init_pair(++pair, COLOR_MAGENTA, color_bg);
		m_ColorPair[7] = pair;
		init_pair(++pair, COLOR_RED, color_bg);
		m_ColorPair[8] = pair;
		init_pair(++pair, COLOR_BLACK, COLOR_WHITE);
		m_ColorPair[9] = pair;
		#else
		// Color output supported only for ncurses.
		m_EnableColor = false;
		m_DefaultColorPair = 0;
		#endif
	}
	else
		m_DefaultColorPair = 0;

	// Set the screen size and draw the initial screen.
	SetSize(COLS, LINES);

	m_syslogStats.Init();

	m_Initialized = true;
}

void CUNIXConsole::Cleanup()
{
	m_cleanupLock.Lock();
	if (m_Initialized)
	{
		// Kill the input thread.
		if (m_pInputThread != NULL)
		{
			m_pInputThread->SignalStopWork();
			gEnv->pThreadUpr->JoinThread(m_pInputThread, eJM_Join);
			delete m_pInputThread;
			m_pInputThread = NULL;
		}

		// Curses cleanup.
		clear();
		endwin();

		m_Initialized = false;
	}
	m_cleanupLock.Unlock();
}

void CUNIXConsole::SetSize(unsigned width, unsigned height)
{
	bool repaint = false;

	assert(IsLocked());
	if (width != m_Width)
	{
		m_Width = width;
		FixCursorPosition();
		repaint = true;
	}
	if (height != m_Height)
	{
		m_Height = height;
		repaint = true;
	}
	if (repaint)
		Repaint();
}

bool CUNIXConsole::IsTooSmall()
{
	assert(IsLocked());
	if (m_Height < m_HeaderHeight + m_StatusHeight + m_CmdHeight + 1)
		return true;
	if (m_Width < UNIXConsole_MIN_WIDTH)
		return true;
	return false;
}

void CUNIXConsole::CheckResize()
{
	assert(IsLocked());
		#if defined(NCURSES)
	i32 lines, cols;

	IS_SHOW_CONSOLE

	if (m_WindowResized)
	{
		m_WindowResized = false;
		// Get the new window size from the terminal driver.
		winsize ws;
		memset(&ws, 0, sizeof ws);
		ioctl(1, TIOCGWINSZ, &ws);
		lines = ws.ws_row;
		cols = ws.ws_col;
		if (m_Width != lines || m_Height != cols)
		{
			resizeterm(lines, cols);
			SetSize(cols, lines);
			Repaint();
		}
	}
		#endif
}

void CUNIXConsole::NewLine()
{
	char lineBuf[3] = "\1X";
	bool doScroll = !m_LineBuffer.empty();

	assert(IsLocked());
	lineBuf[1] = m_Color + '0';

	if (m_LineBuffer.size() == UNIXConsole_MAX_LINES)
		m_LineBuffer.pop_front();
	m_LineBuffer.push_back(lineBuf);

	if (doScroll)
		ScrollLog();
	else
	{
		unsigned row = m_Height - m_CmdHeight - m_StatusHeight - 1;
		move(row, 0);
	}
	if (m_ScrollUp > 0)
	{
		const string& lastDisplayLine
		  = m_LineBuffer[m_LineBuffer.size() - m_ScrollUp - 1];
		DrawLogLine(lastDisplayLine);
	}
}

void CUNIXConsole::ContinueLine()
{
	IS_SHOW_CONSOLE

	if (m_LineBuffer.empty())
	{
		NewLine();
		return;
	}
	const string& lastLine = *m_LineBuffer.rbegin();
	unsigned column = DrawLogLine(lastLine, true /* noOutput */);
	SetColorRaw(m_Color);
	const unsigned row = m_Height - m_CmdHeight - m_StatusHeight - 1;
	move(row, column);
}

void CUNIXConsole::SetColor(i32 color)
{
	assert(IsLocked());
	m_Color = color;
	SetColorRaw(color);
	if (m_LineBuffer.empty())
		m_LineBuffer.push_back("");
	const TLineBuffer::const_reverse_iterator it = m_LineBuffer.rbegin();
	string& lastLine = const_cast<string&>(*it);
	lastLine.push_back(1);
	lastLine.push_back('0' + color);
}

void CUNIXConsole::SetColorRaw(i32 color)
{
	attr_t colorAttr = 0;

	assert(IsLocked());
	if (color == DEFAULT_COLOR)
		color = 0;
	if (m_EnableColor)
		colorAttr = COLOR_PAIR(m_ColorPair[color]);
	switch (color)
	{
	case 0:
		attrset(A_NORMAL | colorAttr);
		break;
	case 1:
		attrset(A_REVERSE | colorAttr);
		break;
	case 2:
	case 3:
		attrset(A_NORMAL | colorAttr);
		break;
	case 4:
		attrset(A_BOLD | colorAttr);
		break;
	case 5:
		attrset(A_NORMAL | colorAttr);
		break;
	case 6:
		attrset(A_BOLD | colorAttr);
		break;
	case 7:
	case 8:
	case 9:
		attrset(A_NORMAL | colorAttr);
		break;
	default:
		abort();
	}
}

void CUNIXConsole::Put(i32 c)
{
	assert(IsLocked());

	// Get the last buffer line.
	if (m_LineBuffer.empty())
		NewLine();
	const TLineBuffer::const_reverse_iterator it = m_LineBuffer.rbegin();
	string& lastLine = const_cast<string&>(*it);
	assert(c >= 0x20);

	// Wrap the line if required.
	unsigned row = m_Height - m_CmdHeight - m_StatusHeight - 1;
	unsigned column = 0;
	GetLineHeight(lastLine, &column);
	assert(column <= m_Width);
	lastLine.push_back(char(c));

	if (m_ScrollUp == 0)
	{
		// Output the line wrap character.
		if (column == m_Width - 1)
		{
			move(row, m_Width - 1);
			attr_t colorAttr = 0;
			if (m_EnableColor)
				colorAttr = COLOR_PAIR(UNIXConsole_WRAP_COLOR);
			attrset(A_NORMAL | colorAttr);
			addch(UNIXConsole_WRAP_CHAR);
			ScrollLog();
			move(row, 0);
		}

		// Output the character.
		SetColorRaw(m_Color);
		addch(c);
	}
}

void CUNIXConsole::Put(tukk s)
{
	assert(IsLocked());
	while (*s)
	{
		if (*s == '\n')
			NewLine();
		else
			Put(*s);
		++s;
	}
}

unsigned CUNIXConsole::GetLineLength(const string& line)
{
	unsigned length = 0;

	for (string::const_iterator it = line.begin(), itEnd = line.end();
	     it != itEnd;
	     ++it)
	{
		char c = *it;
		if (c == 1)
		{
			++it;
			assert(it != itEnd);
			continue;
		}
		++length;
	}
	return length;
}

char CUNIXConsole::GetLastCharacter(const string& line, i32* color)
{
	char c0 = 0, c1 = 0;
	char lastChar = 0;

	if (!line.empty())
	{
		for (i32 i = line.size() - 1; i >= 0; i--)
		{
			c1 = c0;
			c0 = line[i];
			if (c0 == 1)
			{
				assert(lastChar);
				if (color)
					*color = c1 - '0';
				return lastChar;
			}
			else if (c0 && c1 != 1 && !lastChar)
				lastChar = c0;
		}

	}

	*color = DEFAULT_COLOR;
	if (lastChar)
		return lastChar;
	assert(c0 && c0 != 1);
	return c0;
}

unsigned CUNIXConsole::GetLineHeight(const string& line, unsigned* column)
{
	unsigned lineLength = GetLineLength(line);
	unsigned height = 1;

	assert(IsLocked());
	while (lineLength > m_Width)
	{
		lineLength -= m_Width - 1;
		++height;
	}
	if (column != NULL)
		*column = lineLength;
	return height;
}

void CUNIXConsole::ScrollLog()
{
	if (m_fsMode)
		return;

	// Scroll the log window.  We'll do that by defining a software scrolling
	// region.  The constructor has set scrollok() and idlok(), so the output
	// routing will use the hardware scrolling region (if available).
	unsigned top = m_HeaderHeight;
	unsigned bottom = m_Height - m_CmdHeight - m_StatusHeight;

	assert(IsLocked());
	// Some curses implementations (pdcurses) require the current position to
	// be within the defined scrolling region.
	move(top, 0);
	if (setscrreg(top, bottom) == OK)
	{
		move(bottom, 0);
		addch('\n');
		setscrreg(0, m_Height - 1);
		DrawStatus(1);
	}
	else
	{
		// Scrolling regions not supported.  We'll scroll the entire window and
		// the repaint everything except for the log window.
		scroll(stdscr);
		move(bottom - 1, 0);
		attrset(A_NORMAL);
		clrtobot();
		DrawHeader();
		DrawStatus();
		DrawCmd();
	}
	move(bottom - 1, 0);
}

bool CUNIXConsole::FixCursorPosition()
{
	IS_SHOW_CONSOLE_RET(false)

	bool repaint = false;

	assert(IsLocked());

	// Clip the cursor position.
	if (m_CursorPosition > (i32)m_InputLine.size())
		m_CursorPosition = m_InputLine.size();

	// Trivial scroll position fixes.
	if (m_CursorPosition < (i32)m_Width / 2 && m_ScrollPosition > 0)
	{
		m_ScrollPosition = 0;
		repaint = true;
	}
	else if (m_CursorPosition < m_ScrollPosition)
	{
		m_ScrollPosition = m_CursorPosition - m_Width / 4;
		repaint = true;
	}

	assert(m_ScrollPosition <= m_CursorPosition);
	// The method may be called after the cursor has been moved to the
	// right, so we may have to scroll the input line.
	i32 displayLenth = m_Width * m_CmdHeight;
	displayLenth -= strlen(UNIXConsole_PROMPT);
	// If the cursor is at the end of the input line, then we only must leave
	// one space for the cursor itself, otherwise we must leave space for the
	// right scroll indicator.
	if (m_CursorPosition == m_InputLine.size())
		displayLenth -= 1;
	else
		displayLenth -= strlen(UNIXConsole_MORE_RIGHT);
	if (m_ScrollPosition < m_CursorPosition - displayLenth)
	{
		m_ScrollPosition = m_CursorPosition - displayLenth;
		repaint = true;
	}

	return repaint;
}

		#if defined(UC_ENABLE_PLAYER_COUNT)
i32 CUNIXConsole::GetNumPlayers()
{
	assert(IsLocked());

	if (m_OnShutdownCalled)
		return 0;
	if (m_pSystem->IsQuitting())
		return 0;

	IGameFramework* pGameFramework = gEnv->pGameFramework;
	CGameServerNub* pGameServerNub = NULL;

	if (pGameFramework == NULL)
		return 0;
	pGameServerNub = reinterpret_cast<CDrxAction*>(pGameFramework)->GetGameServerNub();
	if (pGameServerNub == NULL)
		return 0;

	return pGameServerNub->GetPlayerCount();
}
		#endif

void CUNIXConsole::OnEdit()
{
	assert(IsLocked());
	m_SavedInputLine.clear();
	m_HistoryIndex = -1;
	if (m_pConsole != NULL)
		m_pConsole->ResetAutoCompletion();
}

void CUNIXConsole::KeyEnter()
{
	bool redrawAll = false;
	bool pushCommand = false;

	assert(IsLocked());

	// Scroll the log window to the bottom.
	if (m_ScrollUp > 0)
	{
		m_ScrollUp = 0;
		redrawAll = true;
	}

	// Process the input line.
	while (!m_InputLine.empty() && m_InputLine[0] == '\\')
		m_InputLine.erase(0, 1);
	if (!m_InputLine.empty())
	{
		pushCommand = true;

		#if defined(UC_ENABLE_MAGIC_COMMANDS)
		// Enable some magic commands intercepted by the console.  All magic
		// commands start with an '@' character.
		{
			tukk const command = m_InputLine.c_str();
			if (!stricmp(command, "@quit"))
			{
				// We're called from the input thread, hence we can't join it.  We
				// have to prevent Cleanup() (called via atexit()) from trying to
				// join.
				CUNIXConsoleInputThread* inputThread = m_pInputThread;
				m_pInputThread = NULL;
				Unlock();
				if (m_pSystem != NULL)
				{
					inputThread->SignalStopWork();
					gEnv->pThreadUpr->JoinThread(m_pInputThread, eJM_Join);

					m_pSystem->Quit();
				}
				exit(0);
				// Not reached.
				abort();
			}
			// Add other magic commands here.
		}
		#endif
	}

	if (pushCommand)
	{
		CSystem* pSystem = static_cast<CSystem*>(gEnv->pSystem);
		#if defined(CVARS_WHITELIST)
		ICVarsWhitelist* pCVarsWhitelist = pSystem->GetCVarsWhiteList();
		bool execute = (pCVarsWhitelist) ? pCVarsWhitelist->IsWhiteListed(m_InputLine, false) : true;
		if (execute)
		#endif // defined(CVARS_WHITELIST)
		{
			m_CommandQueue.push_back(m_InputLine);
		}
	}

	if (!m_InputLine.empty())
	{
		m_CommandHistory.push_back(m_InputLine);
		while (m_CommandHistory.size() > UNIXConsole_MAX_HISTORY)
			m_CommandHistory.pop_front();
		m_HistoryIndex = -1;
		m_InputLine.clear();
		m_SavedInputLine.clear();
		m_CursorPosition = 0;
		m_ScrollPosition = 0;
		if (!redrawAll)
		{
			DrawCmd();
			refresh();
		}
	}

	if (redrawAll)
		Repaint();
}
void CUNIXConsole::KeyUp()
{
	i32k historySize = m_CommandHistory.size();

	assert(IsLocked());
	if (m_HistoryIndex < historySize - 1)
	{
		if (m_HistoryIndex == -1)
			m_SavedInputLine = m_InputLine;
		m_HistoryIndex += 1;
		m_InputLine = m_CommandHistory[historySize - m_HistoryIndex - 1];
		m_CursorPosition = m_InputLine.size();
		FixCursorPosition();
		DrawCmd();
		refresh();
	}
}

void CUNIXConsole::KeyDown()
{
	i32k historySize = m_CommandHistory.size();

	assert(IsLocked());
	if (m_HistoryIndex > -1)
	{
		m_HistoryIndex -= 1;
		if (m_HistoryIndex == -1)
		{
			m_InputLine = m_SavedInputLine;
			m_SavedInputLine.clear();
		}
		else
			m_InputLine = m_CommandHistory[historySize - m_HistoryIndex - 1];
		m_CursorPosition = m_InputLine.size();
		FixCursorPosition();
		DrawCmd();
		refresh();
	}
}

void CUNIXConsole::KeyLeft()
{
	assert(IsLocked());
	if (m_CursorPosition > 0)
	{
		m_CursorPosition -= 1;
		DrawCmd(!FixCursorPosition());
	}
}

void CUNIXConsole::KeyRight()
{
	assert(IsLocked());
	if (m_CursorPosition < (i32)m_InputLine.size())
	{
		m_CursorPosition += 1;
		DrawCmd(!FixCursorPosition());
	}
}

void CUNIXConsole::KeyHome(bool ctrl)
{
	assert(IsLocked());
	if (ctrl)
	{
		i32k logHeight = GetLogHeight();
		i32 maxUp = m_LineBuffer.size() - logHeight;
		if (m_ScrollUp != maxUp)
		{
			m_ScrollUp = maxUp;
			Repaint();
		}
	}
	else if (m_CursorPosition != 0)
	{
		m_CursorPosition = 0;
		DrawCmd(!FixCursorPosition());
	}
}

void CUNIXConsole::KeyEnd(bool ctrl)
{
	assert(IsLocked());
	if (ctrl)
	{
		i32k logHeight = GetLogHeight();
		i32 maxUp = m_LineBuffer.size() - logHeight;
		if (m_ScrollUp != 0)
		{
			m_ScrollUp = 0;
			Repaint();
		}
	}
	else if (m_CursorPosition < (i32)m_InputLine.size())
	{
		m_CursorPosition = m_InputLine.size();
		DrawCmd(!FixCursorPosition());
	}
}

void CUNIXConsole::KeyBackspace()
{
	assert(IsLocked());
	if (m_CursorPosition > 0)
	{
		m_InputLine.erase(m_CursorPosition - 1, 1);
		m_CursorPosition -= 1;
		FixCursorPosition();
		OnEdit();
		DrawCmd();
	}
}

void CUNIXConsole::KeyDelete()
{
	assert(IsLocked());
	if (m_CursorPosition < (i32)m_InputLine.size())
	{
		m_InputLine.erase(m_CursorPosition, 1);
		FixCursorPosition();
		OnEdit();
		DrawCmd();
	}
}

void CUNIXConsole::KeyDeleteWord()
{
	assert(IsLocked());
	if (m_CursorPosition > 0)
	{
		tukk const inputLine = m_InputLine.c_str();
		tukk p = inputLine + m_CursorPosition - 1;

		while (p > inputLine && *p == ' ')
			--p;
		while (p > inputLine && *p != ' ')
			--p;
		m_InputLine.erase(p - inputLine, m_CursorPosition);
		m_CursorPosition = (u32)(p - inputLine);
		FixCursorPosition();
		OnEdit();
		DrawCmd();

		/* Old STxt based code, kept for reference.
		   size_t wordIndex;
		   wordIndex = m_InputLine.find_last_of(" ", m_CursorPosition - 1);
		   if (wordIndex != string::npos)
		   {
		   wordIndex = m_InputLine.find_last_not_of(" ", wordIndex);
		   if (wordIndex != string::npos)
		    wordIndex += 1;
		   }
		   if (wordIndex == string::npos)
		   {
		   m_InputLine.erase(0, m_CursorPosition);
		   m_CursorPosition = 0;
		   }
		   else
		   {
		   m_InputLine.erase(wordIndex, m_CursorPosition - wordIndex);
		   m_CursorPosition = wordIndex;
		   }
		   FixCursorPosition();
		   OnEdit();
		   DrawCmd();
		 */
	}
}

void CUNIXConsole::KeyKill()
{
	assert(IsLocked());
	if (m_CursorPosition < (i32)m_InputLine.size())
	{
		m_InputLine.resize(m_CursorPosition);
		FixCursorPosition();
		OnEdit();
		DrawCmd();
	}
}

void CUNIXConsole::KeyRepaint()
{
	assert(IsLocked());
	Repaint();
}

void CUNIXConsole::KeyTab()
{
	tukk result;

	assert(IsLocked());
	if (m_OnShutdownCalled)
		return;
	string Tmp(m_InputLine);
	Unlock();
	result = m_pConsole->ProcessCompletion(Tmp.c_str());
	Lock();
	if (result != NULL)
	{
		if (result[0] == '\\')
			++result;
		m_InputLine = result;
		m_CursorPosition = m_InputLine.size();
		FixCursorPosition();
		m_SavedInputLine.clear();
		m_HistoryIndex = -1;
		DrawCmd();
		refresh();
	}
}

void CUNIXConsole::KeyPgUp(bool ctrl)
{
	i32k logHeight = GetLogHeight();
	// i32 logStep = logHeight - 2;
	i32 logStep = ctrl ? 10 : 1;
	i32 maxUp = m_LineBuffer.size() - logHeight;
	i32 prevScrollUp = m_ScrollUp;

	assert(IsLocked());
	if (logStep < 1)
		logStep = 1;
	if (maxUp < 0)
		maxUp = 0;
	m_ScrollUp += logStep;
	if (m_ScrollUp > maxUp)
		m_ScrollUp = maxUp;
	if (m_ScrollUp != prevScrollUp)
		Repaint();
}

void CUNIXConsole::KeyPgDown(bool ctrl)
{
	i32k logHeight = GetLogHeight();
	// i32 logStep = logHeight - 2;
	i32 logStep = ctrl ? 10 : 1;
	i32 prevScrollUp = m_ScrollUp;

	assert(IsLocked());
	if (logStep < 1)
		logStep = 1;
	if (m_ScrollUp > 0)
		m_ScrollUp -= logStep;
	if (m_ScrollUp < 0)
		m_ScrollUp = 0;
	if (m_ScrollUp != prevScrollUp)
		Repaint();
}

void CUNIXConsole::Key(i32 c)
{
	assert(IsLocked());
	assert(c >= 0x20 && c <= 0xff);
	assert(m_CursorPosition <= (i32)m_InputLine.size());
	m_InputLine.insert(m_CursorPosition, 1, (char)c);
	m_CursorPosition += 1;
	FixCursorPosition();
	OnEdit();
	DrawCmd();
	refresh();
}

void CUNIXConsole::Repaint()
{
	IS_SHOW_CONSOLE

	  assert(IsLocked());
	clear();
	DrawHeader();
	if (m_fsMode)
		DrawFullscreen();
	else
		DrawLog();
	DrawStatus();
	DrawCmd();
	refresh();
}

void CUNIXConsole::Flush()
{
	IS_SHOW_CONSOLE

	  assert(IsLocked());
	refresh();
}

void CUNIXConsole::InputIdle()
{
	IS_SHOW_CONSOLE

	if (m_pTimer == NULL)
		return;

	Lock();

	CTimeValue now = m_pTimer->GetAsyncTime();
	float timePassed = (now - m_LastUpdateTime).GetSeconds();

	// If more than 0.2 sec have passed since the last OnUpdate() call, then
	// we'll start painting dots to the status line.
	if (timePassed > 0.2f)
	{
		i32 nDots = (i32)(timePassed + 0.5) / 3;
		if (nDots > (i32) m_Width - 2)
			nDots = (i32)m_Width - 2;
		if (m_ProgressStatus.length() != nDots)
		{
			m_ProgressStatus.clear();
			m_ProgressStatus.append(nDots, '.');
			DrawStatus();
			DrawCmd(true);
			refresh();
		}
	}

	Unlock();
}

// get at least n spaces
static tuk GetSpaces(i32 n)
{
	static tuk spaceBuffer = 0;
	static i32 spaceBufferSz = 0;

	if (n > spaceBufferSz)
	{
		spaceBufferSz = std::max(spaceBufferSz * 2, n);
		delete[] spaceBuffer;
		spaceBuffer = new char[spaceBufferSz];
		memset(spaceBuffer, ' ', spaceBufferSz);
	}

	return spaceBuffer;
}

void CUNIXConsole::DrawHeader()
{
	IS_SHOW_CONSOLE

	tukk const headerString = m_HeaderString.c_str();
	i32 headerLength = m_HeaderString.size();
	i32 padLeft = 0, padRight = 0;
	tukk term = termname();

	assert(IsLocked());

	if (m_HeaderHeight == 0)
		return;

	if (headerLength >= (i32)m_Width)
	{
		padLeft = 0;
		padRight = 0;
		headerLength = m_Width;
	}
	else
	{
		padLeft = (m_Width - headerLength) / 2;
		padRight = m_Width - headerLength - padLeft;
	}
	move(m_HeaderHeight - 1, 0);
		#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID
	// For the Linux console ncurses reports the A_UNDERLINE is supported, even
	// thought it is not.  We'll do an explicit test for the terminal type
	// "linux" (i.e.  Linux console).
	if ((termattrs() & A_UNDERLINE) && strcasecmp(term, "linux"))
		attrset(A_UNDERLINE);
	else
		#endif
	{
		if (m_EnableColor)
			attrset(A_BOLD | COLOR_PAIR(m_ColorPair[2] /* blue */));
		else
			attrset(A_REVERSE);
	}
	scrollok(stdscr, FALSE);
	addnstr(GetSpaces(padLeft), padLeft);
	addnstr(headerString, headerLength);
	addnstr(GetSpaces(padRight), padRight);
	scrollok(stdscr, TRUE);
	attrset(A_NORMAL);
}

// Output a single log line.
// If the noOutput flag is set to 'true', then no output is written to the
// screen and no cursor movements are performed.
// The method returns the current output column (i.e. the output column for
// the next charater to be written to the log window).
// Note: Even it the noOutput flag is set, the method will update the m_Color
// field of the UNIX console.
unsigned CUNIXConsole::DrawLogLine(const string& line, bool noOutput)
{
	IS_SHOW_CONSOLE_RET(0)

	const unsigned row = m_Height - m_CmdHeight - m_StatusHeight - 1;
	unsigned column = 0;

	if (!noOutput)
	{
		assert(IsLocked());
		move(row, column);
		attrset(A_NORMAL);
	}
	for (string::const_iterator it = line.begin(), itEnd = line.end(); it != itEnd; )
	{
		char c = *it++;
		if (column == m_Width - 1)
		{
			if (!noOutput)
			{
				attr_t colorAttr = 0;
				if (m_EnableColor)
					colorAttr = COLOR_PAIR(UNIXConsole_WRAP_COLOR);
				attrset(A_NORMAL | colorAttr);
				addch(UNIXConsole_WRAP_CHAR);
				ScrollLog();
				move(row, 0);
				SetColorRaw(m_Color);
			}
			column = 0;
		}
		if (c == 1)
		{
			assert(it != itEnd);
			i32 color = *it - '0';
			m_Color = color;
			++it;
			if (!noOutput)
				SetColorRaw(color);
			continue;
		}
		if (!noOutput)
			addch(c);
		++column;
	}
	return column;
}

// The scrollUp parameter indicates how many log lines to scroll up from the
// bottom.
void CUNIXConsole::DrawLog()
{
	IS_SHOW_CONSOLE

	unsigned scrollUp = m_ScrollUp;

	assert(IsLocked());

	if (IsTooSmall())
		return;
	if (m_LineBuffer.empty())
		return;

	// DrawLog is called only on refresh and on window resize, so performance is
	// not an issue.  We'll simply repaint by re-sending the log lines from the
	// scroll buffer.
	i32 nLines = m_LineBuffer.size();
	i32 lastLine = nLines - 1 - (i32)scrollUp;
	i32 firstLine = lastLine - GetLogHeight();

	if (firstLine < 0)
		firstLine = 0;
	for (i32 i = firstLine; i <= lastLine; ++i)
	{
		const string& line = m_LineBuffer[i];
		if (i > firstLine)
			ScrollLog();
		DrawLogLine(line);
	}
}

void CUNIXConsole::DrawStatus(i32 maxLines)
{
	IS_SHOW_CONSOLE

	unsigned row = m_Height - m_CmdHeight - m_StatusHeight;
	tukk statusLeft = nullptr;
	tukk statusRight = nullptr;
	char bufferLeft[256];
	char bufferRight[256];

	assert(IsLocked());

	if (IsTooSmall() || maxLines == 0 || m_StatusHeight == 0)
	{
		return;
	}

	if (maxLines == -1)
	{
		maxLines = m_StatusHeight;
	}

	// If we're scrolled, then the right size shows a scroll indicator.
	if (m_ScrollUp > 0)
	{
		i32k logHeight = GetLogHeight();
		i32 logBottomLine = (i32)m_LineBuffer.size() - m_ScrollUp;
		assert(logBottomLine >= 0);
		float percent = 100.f * (float)logBottomLine / m_LineBuffer.size();
		if (m_ScrollUp == m_LineBuffer.size() - logHeight)
		{
			drx_strcpy(bufferRight, "| SCROLL:TOP ");
		}
		else
		{
			drx_sprintf(bufferRight, "| SCROLL:%.1f%% ", percent);
		}
		statusRight = bufferRight;
	}

	if (!m_Prompt.empty())
	{
		// No status display when a user prompt is active.
	}
	else if (!m_ProgressStatus.empty())
	{
		drx_sprintf(bufferLeft, " %s", m_ProgressStatus.c_str());
		statusLeft = bufferLeft;
	}
	else if (m_OnUpdateCalled)
	{
		// Standard status display.
		// Map name and game rules on the left.
		// Current update rate and player count on the right.
		tukk mapName = m_svMap->GetString();
		tukk gameRules = m_svGameRules->GetString();
		drx_sprintf(bufferLeft, " map:%s rules:%s", mapName, gameRules);
		statusLeft = bufferLeft;
		float updateRate = 0.0f;
		static float displayUpdateRate = 0.0f;
		if (m_pTimer)
		{
			updateRate = m_pTimer->GetFrameRate();
		#if 0
			// Avoid jumping numbers in the update rate display.  Per update the
			// displayed update rate changes by at most maxDeltaRate.
			const static float maxDeltaRate = 10.0f;
			if (updateRate - displayUpdateRate > maxDeltaRate)
				displayUpdateRate += maxDeltaRate;
			else if (updateRate - displayUpdateRate < -maxDeltaRate)
				displayUpdateRate -= maxDeltaRate;
			else
				displayUpdateRate = updateRate;
		#else
			// Display the update rate as reported by the timer.
			displayUpdateRate = updateRate;
		#endif
		}
		else
		{
			displayUpdateRate = 0.0f;
		}
		if (!statusRight)
		{
			drx_strcpy(bufferRight, "| ");

			size_t len = strlen(bufferRight);

			i32 numPlayers = 0;
		#if defined(UC_ENABLE_PLAYER_COUNT)
			numPlayers = GetNumPlayers();
			if (numPlayers > 0)
			{
				drx_sprintf(
				  &bufferRight[len], sizeof(bufferRight) - len,
				  "players:%i ",
				  numPlayers);
				len = strlen(bufferRight);
			}
		#endif
			if (m_pConsole)
			{
				drx_sprintf(
				  &bufferRight[len], sizeof(bufferRight) - len,
				  "upd:%.1fms(%.2f..%.2f) rate:%.1f/s up:%.1fk/s dn: %.1fk/s",
				  m_updStats.avgUpdateTime, m_updStats.minUpdateTime, m_updStats.maxUpdateTime,
				  displayUpdateRate, m_nubStats.bandwidthUp / 1000.0f, m_nubStats.bandwidthDown / 1000.0f);

				m_syslogStats.Update(displayUpdateRate, numPlayers, m_nubStats);
			}
			else
			{
				drx_strcpy(&bufferRight[len], sizeof(bufferRight) - len, "BUSY ");
			}

			statusRight = bufferRight;
		}
	}
	else
	{
		// No status display (blank).  This branch is taken on the very first draw
		// operation of the UNIX console.
	}

	if (!statusLeft)
	{
		statusLeft = "";
	}
	if (!statusRight)
	{
		statusRight = "";
	}

	i32 leftWidth = strlen(statusLeft);
	i32 rightWidth = strlen(statusRight);
	i32 pad = 0;

	if (leftWidth + rightWidth > (i32)m_Width)
	{
		if (rightWidth > (i32)m_Width)
		{
			leftWidth = 0;
			rightWidth = m_Width;
		}
		else
		{
			leftWidth = m_Width - rightWidth;
		}
	}
	else
	{
		pad = m_Width - leftWidth - rightWidth;
	}

	move(row, 0);
	attrset(A_REVERSE | A_BOLD);
	scrollok(stdscr, FALSE);
	for (i32 i = 0; i < leftWidth; ++i)
		addch(statusLeft[i]);
	for (i32 i = 0; i < pad; ++i)
		addch(' ');
	for (i32 i = 0; i < rightWidth; ++i)
		addch(statusRight[i]);
	scrollok(stdscr, TRUE);
	attrset(A_NORMAL);
}

void CUNIXConsole::DrawFullscreen()
{
	IS_SHOW_CONSOLE

	  scrollok(stdscr, FALSE);

	i32 maxy = 1;
	for (DynArray<SConDrawCmd>::iterator iter = m_drawCmds.begin(); iter != m_drawCmds.end(); ++iter)
		maxy = max(maxy, iter->y);

	i32 scrolly = min(maxy - 1, m_ScrollUp);

	for (DynArray<SConDrawCmd>::iterator iter = m_drawCmds.begin(); iter != m_drawCmds.end(); ++iter)
	{
		switch (iter->op)
		{
		case eCDO_PutText:
			{
				i32 y = iter->y - scrolly;
				if (y < 0 || y > (i32)m_Height - 4)
					break;
				if (iter->x < 0 || iter->x > (i32)m_Width)
					break;
				i32 len = strlen(iter->text);
				if (iter->x + len > (i32)m_Width)
					len = m_Width - iter->x;
				move(y + 1, iter->x);
				for (i32 i = 0; i < len; i++)
					addch(iter->text[i]);
			}
		}
	}
	scrollok(stdscr, TRUE);
}

void CUNIXConsole::DrawCmd(bool cursorOnly)
{
	IS_SHOW_CONSOLE

	unsigned row = m_Height - m_CmdHeight;
	unsigned column = 0;
	attr_t colorAttr = 0;
	const unsigned promptWidth = strlen(UNIXConsole_PROMPT);
	const unsigned moreLeftWidth = strlen(UNIXConsole_MORE_LEFT);
	const unsigned moreRightWidth = strlen(UNIXConsole_MORE_RIGHT);

	assert(IsLocked());

	// If the window is too small, then don't draw anything.
	if (IsTooSmall()
	    || m_CmdHeight == 0
	    || m_Width < promptWidth + moreLeftWidth + moreRightWidth)
		return;

	if (!m_Prompt.empty())
	{
		DrawCmdPrompt();
		return;
	}

	if (!cursorOnly)
	{
		scrollok(stdscr, FALSE);

		// Draw the command prompt.
		if (m_EnableColor)
			colorAttr = COLOR_PAIR(UNIXConsole_PROMPT_COLOR);
		attrset(A_BOLD | colorAttr);
		move(row, 0);
		for (tukk p = UNIXConsole_PROMPT; *p; ++p)
		{
			addch(*p);
			++column;
		}

		// Draw the left scroll indicator (if scrolled).
		if (m_ScrollPosition > 0)
		{
			if (m_EnableColor)
				colorAttr = COLOR_PAIR(UNIXConsole_MORE_COLOR);
			attrset(A_NORMAL | colorAttr);
			for (tukk p = UNIXConsole_MORE_LEFT; *p; ++p)
			{
				addch(*p);
				++column;
			}
		}

		// Draw the input line.  We'll draw to the end of the command window
		// (leaving the last cell blank) and the overdraw the more indicator (if
		// required).
		string::const_iterator it = m_InputLine.begin();
		string::const_iterator itEnd = m_InputLine.end();
		if (m_ScrollPosition > 0)
		{
			for (i32 i = m_ScrollPosition + strlen(UNIXConsole_MORE_LEFT);
			     i > 0;
			     --i, ++it)
				;
		}
		attrset(A_NORMAL);
		bool lineTruncated = false;
		for (; it != itEnd; ++it)
		{
			char c = *it;
			if (row == m_Height - 1 && column == m_Width - 1)
			{
				lineTruncated = true;
				break;
			}
			if (column == m_Width)
			{
				row += 1;
				column = 0;
				assert(row < m_Height);
				move(row, column);
			}
			addch(c);
			++column;
		}

		// Draw the right scroll indicator (if required).
		if (lineTruncated)
		{
			move(m_Height - 1, m_Width - moreRightWidth);
			if (m_EnableColor)
				colorAttr = COLOR_PAIR(UNIXConsole_MORE_COLOR);
			attrset(A_NORMAL | colorAttr);
			for (tukk p = UNIXConsole_MORE_RIGHT; *p; ++p)
			{
				addch(*p);
				++column;
			}
		}
		else
		{
			attrset(A_NORMAL);
			clrtobot();
			move(m_Height - 1, m_Width - 1);
			addch(' ');
		}

		scrollok(stdscr, TRUE);
	}

	// Update the cursor position.
	column = m_CursorPosition - m_ScrollPosition + promptWidth;
	row = m_Height - m_CmdHeight;
	if (column >= m_Width)
	{
		row += column / m_Width;
		column %= m_Width;
	}
	move(row, column);
	refresh();
}

void CUNIXConsole::DrawCmdPrompt()
{
	IS_SHOW_CONSOLE

	unsigned row = m_Height - m_CmdHeight;
	unsigned column = 0;

	string::const_iterator it = m_Prompt.begin();
	string::const_iterator itEnd = m_Prompt.end();
	attrset(A_BOLD);
	clrtobot();
	scrollok(stdscr, FALSE);
	move(row, column);
	for (; it != itEnd; ++it)
	{
		char c = *it;
		if (row == m_Height - 1 && column == m_Width - 1)
			break;
		if (column == m_Width)
		{
			row += 1;
			column = 0;
			move(row, column);
		}
		addch(c);
		++column;
	}
	scrollok(stdscr, TRUE);
	move(row, column);
	refresh();
}

char CUNIXConsole::Prompt(tukk promptString, tukk responseChars)
{
	char response = 0;

	Lock();

	while (m_PromptResponse != 0)
		m_PromptCond.Wait(m_Lock);
		#ifndef _NDEBUG
	// This method is called from __assert_fail, so we better don't put any
	// asserts in here...
	if (!m_Prompt.empty() || !*promptString || !*responseChars)
		abort();
	if (strlen(responseChars) + 1 > sizeof m_PromptResponseChars)
		abort();
		#endif
	m_Prompt = promptString;
	drx_strcpy(m_PromptResponseChars, responseChars);
	DrawCmd();
	while (m_PromptResponse == 0)
		m_PromptCond.Wait(m_Lock);
	response = m_PromptResponse;
	m_PromptResponse = 0;
	m_Prompt.clear();
	m_PromptResponseChars[0] = 0;
	m_PromptCond.Notify();
	DrawCmd();

	Unlock();

	return response;
}

bool CUNIXConsole::IsInputThread()
{
	return gEnv->pThreadUpr->GetThreadId("UNIXConsoleInput") == DrxGetCurrentThreadId();
}

void CUNIXConsole::PrintF(tukk format, ...)
{
	char lineBuffer[1024];
	va_list args;
	va_start(args, format);
	drx_vsprintf(lineBuffer, format, args);
	Print(lineBuffer);
	va_end(args);
}

void CUNIXConsole::Print(tukk line)
{
	IS_SHOW_CONSOLE

	static string lastLine;
	static bool firstCall = true;
	const size_t lineLength = strlen(line);
	size_t lineOffset = 0;

	Lock();

	// Check if the last line is a true prefix of the specified text argument.
	// It it is a prefix, then the output is added to the last line sent to the
	// sink.
	if (!firstCall
	    && lineLength > lastLine.size()
	    && !strncmp(line, lastLine.c_str(), lastLine.size()))
	{
		// Line continued.
		lineOffset = lastLine.size();
		ContinueLine(); // Will set the correct color.
	}
	else
	{
		NewLine();
		SetColor();
	}
	lastLine = line;
	firstCall = false;

	for (size_t i = lineOffset; i < lineLength; ++i)
	{
		char c = line[i];
		switch (c)
		{
		case '\\':
			if (i < lineLength - 1 && line[i + 1] == 'n')
			{
				// Sequence "\\n", treat as "\n".
				NewLine();
				++i;
				continue;
			}
			break;
		case '\n':
			NewLine();
			continue;
		case '\r':
			ClearLine();
			continue;
		case '\t':
			// We'll do it like the graphical console, just add 4 spaces and don't
			// care about TAB stops.
			for (unsigned j = 0; j < 4; ++j)
				Put(' ');
			continue;
		case '$':
			if (i < lineLength - 1)
			{
				++i;
				char colorChar = line[i];
				if (isdigit(colorChar))
				{
					SetColor(colorChar - '0');
					continue;
				}
				if (colorChar == 'o' || colorChar == 'O')
				{
					// Ignore.
					continue;
				}
			}
			break;
		default:
			break;
		}
		if (c < 0x20)
		{
			// Unrecognized control character.  Ignore.
			continue;
		}
		Put((u8)c);
	}
	DrawCmd(true);
	//	Flush();

	Unlock();
}

void CUNIXConsole::OnInitProgress(tukk sProgressMsg)
{
	if (!m_Initialized)
		return;

	Lock();
	m_ProgressStatus = sProgressMsg;
	DrawStatus();
	DrawCmd(true);
	Flush();
	Unlock();
}

void CUNIXConsole::OnInit(ISystem* pSystem)
{
	if (m_RequireDedicatedServer && !gEnv->IsDedicated())
		return;

	Lock();

	if (!m_Initialized)
		Init();

	assert(m_pSystem == NULL);
	m_pSystem = pSystem;
	assert(m_pConsole == NULL);
	m_pConsole = pSystem->GetIConsole();

	// Add the output print sink to the system console.
	if (m_pConsole != 0)
	{
		m_pConsole->AddOutputPrintSink(this);
	}

	// Start the input thread.
	m_pInputThread = new CUNIXConsoleInputThread(*this);

	if (!gEnv->pThreadUpr->SpawnThread(m_pInputThread, "UNIXConsoleInput"))
	{
		DrxFatalError("Error spawning \"UNIXConsoleInput\" thread.");
	}

	Unlock();

		#if defined(NCURSES)
	// Setup the signal handler.
	struct sigaction action;
	memset(&action, 0, sizeof action);
	action.sa_handler = CUNIXConsoleSignalHandler::Handler;
	sigfillset(&action.sa_mask);
	CUNIXConsoleSignalHandler::m_pUNIXConsole = this;
	sigaction(SIGWINCH, &action, NULL);
	sigset_t mask;
	memset(&mask, 0, sizeof mask);
	sigemptyset(&mask);
	sigaddset(&mask, SIGWINCH);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
		#endif
}

void CUNIXConsole::OnShutdown()
{
	if (!m_Initialized)
		return;

	Lock();
	assert(!m_OnShutdownCalled);
	m_pConsole->RemoveOutputPrintSink(this);
	m_OnShutdownCalled = true;
	Unlock();

	Cleanup();
}

void CUNIXConsole::OnUpdate()
{
	DRX_PROFILE_FUNCTION(PROFILE_SYSTEM);

	IS_SHOW_CONSOLE

	if (!m_Initialized)
		return;

	bool updateStatus = false;
	static CTimeValue lastStatusUpdate = 0.f;

	if (m_OnShutdownCalled)
		return;

	Lock();

	if (!m_OnUpdateCalled)
	{
		m_OnUpdateCalled = true;
		assert(m_svMap == NULL);
		assert(m_svGameRules == NULL);
		m_svMap = m_pConsole->GetCVar("sv_map");
		m_svGameRules = m_pConsole->GetCVar("sv_gamerules");
		assert(m_pTimer == NULL);
		m_pTimer = m_pSystem->GetITimer();
	}

	if (!m_ProgressStatus.empty())
	{
		m_ProgressStatus.clear();
		updateStatus = true;
	}
	CTimeValue now = m_pTimer->GetAsyncTime();
	if ((now - lastStatusUpdate).GetSeconds() > 0.1f)
		updateStatus = true;
	m_LastUpdateTime = now;

	if (updateStatus)
	{
		DrawStatus();
		DrawCmd(true);
		Flush();
		lastStatusUpdate = now;
	}

	while (!m_CommandQueue.empty())
	{
		const string& command = m_CommandQueue[0];
		Unlock();
		m_pConsole->ExecuteString(command.c_str());
		m_pConsole->AddCommandToHistory(command.c_str());
		Lock();
		m_CommandQueue.pop_front();
	}

	if (IGameFramework* pGameFramework = gEnv->pGameFramework)
		if (INetNub* pNub = pGameFramework->GetServerNetNub())
			m_nubStats = pNub->GetStatistics();
	m_pSystem->GetUpdateStats(m_updStats);

	bool fsMode = !m_drawCmds.empty();
	if (fsMode || fsMode != m_fsMode)
	{
		m_fsMode = fsMode;
		Repaint();
	}

	Unlock();
}

void CUNIXConsole::GetMemoryUsage(IDrxSizer* pSizer)
{
	size_t size = sizeof *this;

	Lock();

	// We're using string (for various reasons), so our best guess of the
	// size is the .size().
	size += m_HeaderString.size();
	size += m_LineBuffer.size() * sizeof(string);
	for (TLineBuffer::const_iterator it = m_LineBuffer.begin(),
	     itEnd = m_LineBuffer.end();
	     it != itEnd;
	     ++it)
		size += it->size();
	size += m_CommandQueue.size() * sizeof(string);
	for (TCommandQueue::const_iterator it = m_CommandQueue.begin(),
	     itEnd = m_CommandQueue.end();
	     it != itEnd;
	     ++it)
		size += it->size();
	size += m_CommandHistory.size() * sizeof(string);
	for (TCommandHistory::const_iterator it = m_CommandHistory.begin(),
	     itEnd = m_CommandHistory.end();
	     it != itEnd;
	     ++it)
		size += it->size();
	if (m_pInputThread != NULL)
		size += sizeof *m_pInputThread;
	size += m_InputLine.size();
	size += m_SavedInputLine.size();
	size += m_ProgressStatus.size();

	Unlock();

	pSizer->AddObject(this, size);
}

Vec2_tpl<i32> CUNIXConsole::BeginDraw()
{
	m_newCmds.resize(0);
	return Vec2_tpl<i32>(80, 25 - 3);
}

void CUNIXConsole::PutText(i32 x, i32 y, tukk msg)
{
	IS_SHOW_CONSOLE

	SConDrawCmd cmd;
	cmd.op = eCDO_PutText;
	cmd.x = x;
	cmd.y = y;
	drx_strcpy(cmd.text, msg);
	m_newCmds.push_back(cmd);
}

void CUNIXConsole::EndDraw()
{
	IS_SHOW_CONSOLE

	  Lock();
	m_drawCmds.swap(m_newCmds);
	Unlock();
}

void CUNIXConsoleInputThread::ThreadEntry()
{
		#if !DRX_PLATFORM_WINDOWS
	fd_set rdfds;
		#endif
	bool interrupted = false;

	// The input thread selects stdin (0) and the interrupt pipe.  The select()
	// call has a timeout (typically 0.5 sec) and will call the InputIdle()
	// method whenever the timer expires.
	while (true)
	{
		interrupted = false;
		#if !DRX_PLATFORM_WINDOWS
		FD_ZERO(&rdfds);
		FD_SET(m_IntrPipe[0], &rdfds);
		FD_SET(0, &rdfds);
		timeval tv;
		memset(&tv, 0, sizeof tv);
		tv.tv_sec = 0;
		tv.tv_usec = 100000; // 0.1 sec.
		if (select(m_IntrPipe[0] + 1, &rdfds, NULL, NULL, &tv) != -1)
		{
			if (FD_ISSET(m_IntrPipe[0], &rdfds))
			{
				char buf;
				read(m_IntrPipe[0], &buf, 1);
				interrupted = true;
			}
			else if (!FD_ISSET(0, &rdfds))
			{
				// Both m_IntrPipe[0] and 0 are not ready, so this must be a timeout.
				m_UNIXConsole.InputIdle();
				continue;
			}
		}
		else
		{
			// Got interrupted by a signal.
			assert(errno == EINTR);
			interrupted = true;
		}
		#else
		HANDLE handles[2];
		handles[0] = m_IntrEvent;
		handles[1] = GetStdHandle(STD_INPUT_HANDLE);
		DWORD result = WaitForMultipleObjects(2, handles, false, 10 /* 0.1 sec */);
		switch (result)
		{
		case WAIT_OBJECT_0:
			interrupted = true;
			ResetEvent(m_IntrEvent);
			break;
		case WAIT_OBJECT_0 + 1:
			break;
		case WAIT_TIMEOUT:
			m_UNIXConsole.InputIdle();
			continue;
		case WAIT_FAILED:
			assert(!"WaitForMultipleObjects() failed");
		}
		#endif
		if (interrupted)
		{
			if (m_Cancelled)
				break;
			m_UNIXConsole.Lock();
			m_UNIXConsole.CheckResize();
			m_UNIXConsole.Unlock();
			continue;
		}
		i32 c = getch();
		m_UNIXConsole.Lock();

		// Handle prompt responses.
		if (!m_UNIXConsole.m_Prompt.empty())
		{
			bool acceptAll = false;
			char response = 0;
			if (strchr(m_UNIXConsole.m_PromptResponseChars, '@'))
				acceptAll = true;
			if (c == KEY_ENTER || c == '\r')
				c = '\n';
			if (c == KEY_BACKSPACE || c == 0x7f)
				c = '\010';
			if (c <= 0xff && strchr(m_UNIXConsole.m_PromptResponseChars, (char)c))
				response = (char)c;
			else if (acceptAll && (isprint(c) || c == '\n'))
				response = (char)c;
			else
			{
				beep();
				m_UNIXConsole.Unlock();
				continue;
			}
			m_UNIXConsole.m_PromptResponse = response;
			m_UNIXConsole.m_PromptCond.Notify();
			m_UNIXConsole.Unlock();
			continue;
		}

		// if console is hided then pass only F10 key
		if (!m_UNIXConsole.m_bShowConsole)
		{
			if (KEY_F(10) == c) m_UNIXConsole.KeyF(10);
			m_UNIXConsole.Unlock();
			continue;
		}

		switch (c)
		{
		case ERR:
			break;
		case KEY_RESIZE:
			// Window size changed.  This key is received only if the ncurses
			// library is configured to handle SIGWINCH and if no other SIGWINCH
			// handler has been installed.
			m_UNIXConsole.SetSize(COLS, LINES);
			m_UNIXConsole.Repaint();
			break;
		case KEY_ENTER:
		case PADENTER:
		case '\r':
		case '\n':
			m_UNIXConsole.KeyEnter();
			break;
		case KEY_UP:
		case '\020': // CTRL-P
			m_UNIXConsole.KeyUp();
			break;
		case KEY_DOWN:
		case '\016': // CTRL-N
			m_UNIXConsole.KeyDown();
			break;
		case KEY_LEFT:
			m_UNIXConsole.KeyLeft();
			break;
		case KEY_RIGHT:
			m_UNIXConsole.KeyRight();
			break;
		case KEY_HOME:
		case '\001': // CTRL-A
			m_UNIXConsole.KeyHome(false);
			break;
		case CTL_HOME:
			m_UNIXConsole.KeyHome(true);
			break;
		case KEY_END:
		case '\005': // CTRL-E
			m_UNIXConsole.KeyEnd(false);
			break;
		case CTL_END:
			m_UNIXConsole.KeyEnd(true);
			break;
		case KEY_BACKSPACE:
		#if DRX_PLATFORM_MAC
		// Mac OS X returns delete key instead of backspace
		case 0x7f:
		#else
		case '\010': // CTRL-H
		#endif
			m_UNIXConsole.KeyBackspace();
			break;
		case KEY_DC:
		case KEY_SDC:
		case '\004': // CTRL-D
			m_UNIXConsole.KeyDelete();
			break;
		case '\027': // CTRL-W
			m_UNIXConsole.KeyDeleteWord();
			break;
		case '\013': // CTRL-K
			m_UNIXConsole.KeyKill();
			break;
		case '\014': // CTRL-L
			m_UNIXConsole.KeyRepaint();
			break;
		case '\t': // TAB
			m_UNIXConsole.KeyTab();
			break;
		case KEY_NPAGE:
		case '\006': // CTRL-F
			m_UNIXConsole.KeyPgDown(false);
			break;
		case CTL_PGDN:
			m_UNIXConsole.KeyPgDown(true);
			break;
		case KEY_PPAGE:
		case '\002': // CTRL-B
			m_UNIXConsole.KeyPgUp(false);
			break;
		case CTL_PGUP:
			m_UNIXConsole.KeyPgUp(true);
			break;
		case KEY_F(10):
			m_UNIXConsole.KeyF(10);
			break;
		case KEY_F(11):
			m_UNIXConsole.KeyF(11);
			break;
		default:
			if (c >= 0x20 && c <= 0xff)
				m_UNIXConsole.Key(c);
			break;
		}
		m_UNIXConsole.Unlock();
	}
}

void CUNIXConsole::KeyF(i32 id)
{
		#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID
	if (11 == id)
	{
		def_prog_mode();
		endwin();
		m_bShowConsole = false;
		system("/bin/bash");
		reset_prog_mode();
		refresh();
		m_bShowConsole = true;
	}
	else if (10 == id)
	{
		if (m_bShowConsole)
		{
			def_prog_mode();
			endwin();
			m_bShowConsole = false;
		}
		else
		{
			reset_prog_mode();
			refresh();
			m_bShowConsole = true;
		}
	}
		#endif
}

CUNIXConsole* CUNIXConsoleSignalHandler::m_pUNIXConsole = NULL;

void CUNIXConsoleSignalHandler::Handler(i32 signum)
{
		#if defined(NCURSES)
	switch (signum)
	{
	case SIGWINCH:
		m_pUNIXConsole->m_WindowResized = true;
		m_pUNIXConsole->m_pInputThread->Interrupt();
		break;
	default:
		break;
	}
		#endif
}

	#endif // USE_UNIXCONSOLE

///////////////////////////////////////////////////////////////////////////////////////
//
// simple light-weight console implementation
//
///////////////////////////////////////////////////////////////////////////////////////
CNULLConsole::CNULLConsole(bool isDaemonMode) : m_isDaemon(isDaemonMode)
{
}

void CNULLConsole::Print(tukk inszText)
{
	if (m_isDaemon)
		return;

	#if DRX_PLATFORM_WINDOWS
	DWORD written;
	char buf[1024];
	drx_sprintf(buf, "%s\n", inszText);
	WriteConsole(m_hOut, buf, strlen(buf), &written, NULL);
	#elif DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_MAC
	printf("%s\n", inszText);
	#endif
}

void CNULLConsole::OnInit(ISystem* pSystem)
{
	m_syslogStats.Init();

	if (m_isDaemon)
		return;

	IConsole* pConsole = pSystem->GetIConsole();
	pConsole->AddOutputPrintSink(this);

	#if DRX_PLATFORM_WINDOWS
	AllocConsole();
	m_hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	#endif
}

void CNULLConsole::OnUpdate()
{
	i32 numPlayers = 0;
	float srvRate = 0;
	INetNub::SStatistics netNub;

	#if defined(UC_ENABLE_PLAYER_COUNT)
	if (!gEnv->pSystem->IsQuitting())
	{
		IGameFramework* pGameFramework = gEnv->pGameFramework;
		if (pGameFramework)
		{
			CGameServerNub* pGameServerNub = reinterpret_cast<CDrxAction*>(pGameFramework)->GetGameServerNub();
			if (pGameServerNub)
				numPlayers = pGameServerNub->GetPlayerCount();

			if (INetNub* pNub = pGameFramework->GetServerNetNub())
				netNub = pNub->GetStatistics();
		}
	}
	#endif
	if (ITimer* pTimer = gEnv->pSystem->GetITimer())
		srvRate = pTimer->GetFrameRate();

	m_syslogStats.Update(srvRate, numPlayers, netNub);
}

void CNULLConsole::PutText(i32 x, i32 y, tukk msg)
{
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Logging server internal statistics into syslog service
//
///////////////////////////////////////////////////////////////////////////////////////
CSyslogStats::CSyslogStats() :
	m_syslog_stats(0),
	m_syslog_period(SYSLOG_DEFAULT_PERIOD)
{
}

CSyslogStats::~CSyslogStats()
{
	#if DRX_PLATFORM_LINUX || DRX_PLATFORM_MAC
	closelog();
	if (gEnv->pConsole)
	{
		gEnv->pConsole->UnregisterVariable("syslog_stats");
		gEnv->pConsole->UnregisterVariable("syslog_period");
	}
	#endif
}

void CSyslogStats::Init()
{
	#if DRX_PLATFORM_LINUX || DRX_PLATFORM_MAC
		#if DRX_PLATFORM_LINUX
	openlog("LinuxdEngStart", LOG_PID, LOG_USER);
		#else
	openlog("Mac3dEngStart", LOG_PID, LOG_USER);
		#endif

	if (gEnv->pConsole)
	{
		REGISTER_CVAR2("syslog_stats", &m_syslog_stats, 0, 0, "Start/Stop logging server info into syslog");
		REGISTER_CVAR2("syslog_period", &m_syslog_period, SYSLOG_DEFAULT_PERIOD, 0, "Syslog logging timeout period");
	}
	#endif
}

void CSyslogStats::Update(float srvRate, i32 numPlayers, INetNub::SStatistics& netNubStats)
{
	#if DRX_PLATFORM_LINUX || DRX_PLATFORM_MAC
	if (m_syslog_stats)
	{
		m_syslogCurrTime = gEnv->pTimer->GetAsyncTime();

		if ((m_syslogCurrTime - m_syslogStartTime).GetMilliSeconds() > m_syslog_period)
		{
			syslog(LOG_NOTICE, "rate:%.1f/s, players:%i, up:%.1fk/s, dn:%.1fk/s",
			       srvRate,
			       numPlayers,
			       netNubStats.bandwidthUp / 1000.0f,
			       netNubStats.bandwidthDown / 1000.0f);

			m_syslogStartTime = m_syslogCurrTime;
		}
	}
	#endif
}

#endif // defined(USE_DEDICATED_SERVER_CONSOLE)

// vim:ts=2
