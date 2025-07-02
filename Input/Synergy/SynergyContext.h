// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SYNERGYCONTEXT_H__
#define __SYNERGYCONTEXT_H__

#ifdef USE_SYNERGY_INPUT

	#include <drx3D/Network/DrxSocks.h>
	#include <drx3D/CoreX/Thread/IThreadUpr.h>

	#define MAX_CLIPBOARD_SIZE         1024

	#define SYNERGY_MODIFIER_SHIFT     0x1
	#define SYNERGY_MODIFIER_CTRL      0x2
	#define SYNERGY_MODIFIER_LALT      0x4
	#define SYNERGY_MODIFIER_WINDOWS   0x10
	#define SYNERGY_MODIFIER_RALT      0x20
	#define SYNERGY_MODIFIER_CAPSLOCK  0x1000
	#define SYNERGY_MODIFIER_NUMLOCK   0x2000
	#define SYNERGY_MODIFIER_SCROLLOCK 0x4000

class CSynergyContext : public IThread, public CMultiThreadRefCount
{
public:
	CSynergyContext(tukk pIdentifier, tukk pHost);
	~CSynergyContext();

	// Start accepting work on thread
	virtual void ThreadEntry();

	bool         GetKey(u32& key, bool& bPressed, bool& bRepeat, u32& modifier);
	bool         GetMouse(u16& x, u16& y, u16& wheelX, u16& wheelY, bool& buttonL, bool& buttonM, bool& buttonR);
	tukk  GetClipboard();

public:
	struct KeyPress
	{
		KeyPress(u32 _key, bool _bPressed, bool _bRepeat, u32 _modifier) { key = _key; bPressed = _bPressed; bRepeat = _bRepeat; modifier = _modifier; }
		u32 key;
		bool   bPressed;
		bool   bRepeat;
		u32 modifier;
	};
	struct MouseEvent
	{
		u16 x, y, wheelX, wheelY;
		bool   button[3];
	};

	DrxCriticalSection     m_keyboardLock;
	DrxCriticalSection     m_mouseLock;
	DrxCriticalSection     m_clipboardLock;
	std::deque<KeyPress>   m_keyboardQueue;
	std::deque<MouseEvent> m_mouseQueue;
	char                   m_clipboard[MAX_CLIPBOARD_SIZE];
	char                   m_clipboardThread[MAX_CLIPBOARD_SIZE];
	string                 m_host;
	string                 m_name;
	MouseEvent             m_mouseState;
	DRXSOCKET              m_socket;
	i32                    m_packetOverrun;

private:

	bool m_bQuit;
};

#endif // USE_SYNERGY_INPUT

#endif // __SYNERGY_CONTEXT_H__
