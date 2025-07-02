// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Input/StdAfx.h>

#ifdef USE_SYNERGY_INPUT

	#include <drx3D/CoreX/Renderer/IRenderer.h>
	#include <drx3D/Input/Synergy/SynergyContext.h>
	#include <drx3D/Network/DrxSocks.h>

enum ArgType
{
	ARG_END = 0,
	ARG_UINT8,
	ARG_UINT16,
	ARG_UINT32
};

	#define MAX_ARGS 16

struct Stream
{
	explicit Stream(i32 size)
	{
		buffer = (u8*)malloc(size);
		end = data = buffer;
		bufferSize = size;
		packet = NULL;
	}

	~Stream()
	{
		free(buffer);
	}

	u8* data;
	u8* end;
	u8* buffer;
	u8* packet;
	i32    bufferSize;

	void  Rewind()                      { data = buffer; }
	i32   GetBufferSize()               { return bufferSize; }

	tuk GetBuffer()                   { return (tuk)buffer; }
	tuk GetData()                     { return (tuk)data; }

	void  SetLength(i32 len)            { end = data + len; }
	i32   GetLength()                   { return (i32)(end - data); }

	i32   ReadU32()                     { i32 ret = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]; data += 4; return ret; }
	i32   ReadU16()                     { i32 ret = (data[0] << 8) | data[1]; data += 2; return ret; }
	i32   ReadU8()                      { i32 ret = data[0]; data += 1; return ret; }
	void  Eat(i32 len)                  { data += len; }

	void  InsertString(tukk str) { i32 len = strlen(str); memcpy(end, str, len); end += len; }
	void  InsertU32(i32 a)              { end[0] = a >> 24; end[1] = a >> 16; end[2] = a >> 8; end[3] = a; end += 4; }
	void  InsertU16(i32 a)              { end[0] = a >> 8; end[1] = a; end += 2; }
	void  InsertU8(i32 a)               { end[0] = a; end += 1; }
	void  OpenPacket()                  { packet = end; end += 4; }
	void  ClosePacket()                 { i32 len = GetLength() - sizeof(u32); packet[0] = len >> 24; packet[1] = len >> 16; packet[2] = len >> 8; packet[3] = len; packet = NULL; }
};

typedef bool (* packetCallback)(CSynergyContext* pContext, i32* pArgs, Stream* pStream, i32 streamLeft);

struct Packet
{
	tukk    pattern;
	ArgType        args[MAX_ARGS + 1];
	packetCallback callback;
};

static bool synergyConnectFunc(CSynergyContext* pContext)
{
	if (pContext->m_socket >= 0)
	{
		DrxSock::closesocket(pContext->m_socket);
	}
	pContext->m_socket = DrxSock::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (pContext->m_socket >= 0)
	{
		DRXSOCKADDR_IN sa;
		sa.sin_family = AF_INET;
		sa.sin_port = htons(24800);
		sa.sin_addr.s_addr = DrxSock::DNSLookup(pContext->m_host.c_str());
		if (sa.sin_addr.s_addr)
		{
			i32k result = DrxSock::connect(pContext->m_socket, (const DRXSOCKADDR*)&sa, sizeof(sa));
			if (DrxSock::TranslateSocketError(result) == DrxSock::eCSE_NO_ERROR)
			{
				return true;
			}
		}
		DrxSock::closesocket(pContext->m_socket);
		pContext->m_socket = -1;
	}
	return false;
}

static bool synergySendFunc(CSynergyContext* pContext, tukk buffer, i32 length)
{
	i32 ret = DrxSock::send(pContext->m_socket, buffer, length, 0);
	return (ret == length) ? true : false;
}

static bool synergyReceiveFunc(CSynergyContext* pContext, tuk buffer, i32 maxLength, i32* outLength)
{
	i32 ret = DrxSock::recv(pContext->m_socket, buffer, maxLength, 0);
	*outLength = ret;
	return (ret > 0) ? true : false;
}

static bool synergyPacket(CSynergyContext* pContext, i32* pArgs, Stream* pStream, i32 streamLeft)
{
	Stream s(256);
	s.OpenPacket();
	s.InsertString("Synergy");
	s.InsertU16(1);
	s.InsertU16(4);
	s.InsertU32(pContext->m_name.length());
	s.InsertString(pContext->m_name.c_str());
	s.ClosePacket();
	return synergySendFunc(pContext, s.GetBuffer(), s.GetLength());
}

static bool synergyQueryInfo(CSynergyContext* pContext, i32* pArgs, Stream* pStream, i32 streamLeft)
{
	Stream s(256);
	s.OpenPacket();
	s.InsertString("DINF");
	s.InsertU16(0);
	s.InsertU16(0);
	if (gEnv->pRenderer)
	{
		s.InsertU16(gEnv->pRenderer->GetWidth());
		s.InsertU16(gEnv->pRenderer->GetHeight());
	}
	else
	{
		s.InsertU16(1920);
		s.InsertU16(1080);
	}
	s.InsertU16(0);
	s.InsertU16(0);
	s.InsertU16(0);
	s.ClosePacket();
	return synergySendFunc(pContext, s.GetBuffer(), s.GetLength());
}

static bool synergyKeepAlive(CSynergyContext* pContext, i32* pArgs, Stream* pStream, i32 streamLeft)
{
	Stream s(256);
	s.OpenPacket();
	s.InsertString("CALV");
	s.ClosePacket();
	return synergySendFunc(pContext, s.GetBuffer(), s.GetLength());
}

static bool synergyEnterScreen(CSynergyContext* pContext, i32* pArgs, Stream* pStream, i32 streamLeft)
{
	#ifdef SUPPORT_HW_MOUSE_CURSOR
	if (gEnv->pRenderer && gEnv->pRenderer->GetIHWMouseCursor())
	{
		gEnv->pRenderer->GetIHWMouseCursor()->SetPosition(pArgs[0], pArgs[1]);
		gEnv->pRenderer->GetIHWMouseCursor()->Show();
	}
	#endif
	AUTO_LOCK(pContext->m_mouseLock);
	pContext->m_mouseState.x = pArgs[0];
	pContext->m_mouseState.y = pArgs[1];
	pContext->m_mouseQueue.push_back(pContext->m_mouseState);
	return true;
}

static bool synergyExitScreen(CSynergyContext* pContext, i32* pArgs, Stream* pStream, i32 streamLeft)
{
	#ifdef SUPPORT_HW_MOUSE_CURSOR
	if (gEnv->pRenderer && gEnv->pRenderer->GetIHWMouseCursor())
	{
		gEnv->pRenderer->GetIHWMouseCursor()->Hide();
	}
	#endif
	return true;
}

static bool synergyMouseMove(CSynergyContext* pContext, i32* pArgs, Stream* pStream, i32 streamLeft)
{
	#ifdef SUPPORT_HW_MOUSE_CURSOR
	if (gEnv->pRenderer && gEnv->pRenderer->GetIHWMouseCursor())
	{
		gEnv->pRenderer->GetIHWMouseCursor()->SetPosition(pArgs[0], pArgs[1]);
	}
	#endif
	AUTO_LOCK(pContext->m_mouseLock);
	pContext->m_mouseState.x = pArgs[0];
	pContext->m_mouseState.y = pArgs[1];
	pContext->m_mouseQueue.push_back(pContext->m_mouseState);
	return true;
}

static bool synergyMouseButtonDown(CSynergyContext* pContext, i32* pArgs, Stream* pStream, i32 streamLeft)
{
	i32 buttonNum = pArgs[0] - 1;
	if (buttonNum < 3)
	{
		AUTO_LOCK(pContext->m_mouseLock);
		pContext->m_mouseState.button[buttonNum] = true;
		pContext->m_mouseQueue.push_back(pContext->m_mouseState);
	}
	return true;
}

static bool synergyMouseButtonUp(CSynergyContext* pContext, i32* pArgs, Stream* pStream, i32 streamLeft)
{
	i32 buttonNum = pArgs[0] - 1;
	if (buttonNum < 3)
	{
		AUTO_LOCK(pContext->m_mouseLock);
		pContext->m_mouseState.button[buttonNum] = false;
		pContext->m_mouseQueue.push_back(pContext->m_mouseState);
	}
	return true;
}

static bool synergyKeyboardDown(CSynergyContext* pContext, i32* pArgs, Stream* pStream, i32 streamLeft)
{
	AUTO_LOCK(pContext->m_keyboardLock);
	pContext->m_keyboardQueue.push_back(CSynergyContext::KeyPress(pArgs[2], true, false, pArgs[1]));
	return true;
}

static bool synergyKeyboardUp(CSynergyContext* pContext, i32* pArgs, Stream* pStream, i32 streamLeft)
{
	AUTO_LOCK(pContext->m_keyboardLock);
	pContext->m_keyboardQueue.push_back(CSynergyContext::KeyPress(pArgs[2], false, false, pArgs[1]));
	return true;
}

static bool synergyKeyboardRepeat(CSynergyContext* pContext, i32* pArgs, Stream* pStream, i32 streamLeft)
{
	AUTO_LOCK(pContext->m_keyboardLock);
	pContext->m_keyboardQueue.push_back(CSynergyContext::KeyPress(pArgs[3], true, true, pArgs[1]));
	return true;
}

static bool synergyClipboard(CSynergyContext* pContext, i32* pArgs, Stream* pStream, i32 streamLeft)
{
	for (i32 i = 0; i < pArgs[3]; i++)
	{
		i32 format = pStream->ReadU32();
		i32 size = pStream->ReadU32();
		if (format == 0) // Is text
		{
			AUTO_LOCK(pContext->m_clipboardLock);
			size = std::min(size, (i32)MAX_CLIPBOARD_SIZE - 1);
			memcpy(pContext->m_clipboardThread, pStream->GetData(), size);
			pContext->m_clipboardThread[size] = '\0';
		}
		pStream->Eat(size);
	}
	return true;
}

static bool synergyBye(CSynergyContext* pContext, i32* pArgs, Stream* pStream, i32 streamLeft)
{
	DrxLog("SYNERGY: Server said bye. Disconnecting\n");
	return false;
}

static Packet s_packets[] = {
	{ "Synergy", { ARG_UINT16, ARG_UINT16             }, synergyPacket         },
	{ "QINF",    {},           synergyQueryInfo       },
	{ "CALV",    {},           synergyKeepAlive       },
	{ "CINN",    { ARG_UINT16, ARG_UINT16, ARG_UINT32, ARG_UINT16}, synergyEnterScreen    },
	{ "COUT",    {},           synergyExitScreen      },
	{ "CBYE",    {},           synergyBye             },
	{ "DMMV",    { ARG_UINT16, ARG_UINT16             }, synergyMouseMove      },
	{ "DMDN",    { ARG_UINT8 },synergyMouseButtonDown },
	{ "DMUP",    { ARG_UINT8 },synergyMouseButtonUp   },
	{ "DKDN",    { ARG_UINT16, ARG_UINT16, ARG_UINT16 }, synergyKeyboardDown   },
	{ "DKUP",    { ARG_UINT16, ARG_UINT16, ARG_UINT16 }, synergyKeyboardUp     },
	{ "DKRP",    { ARG_UINT16, ARG_UINT16, ARG_UINT16, ARG_UINT16}, synergyKeyboardRepeat },
	{ "DCLP",    { ARG_UINT8,  ARG_UINT32, ARG_UINT32, ARG_UINT32}, synergyClipboard      }
};

static bool ProcessPackets(CSynergyContext* pContext, Stream* s)
{
	while (s->data < s->end)
	{
		i32 packetLen = s->ReadU32();
		tukk packetStart = s->GetData();
		i32 i;
		if (packetLen > s->GetLength())
		{
			DrxLogAlways("SYNERGY: Packet overruns buffer. Probably lots of data on clipboard? (Size:%d LeftInBuffer:%d)\n", packetLen, s->GetLength());
			pContext->m_packetOverrun = packetLen - s->GetLength();
			return true;
		}
		for (i = 0; i < DRX_ARRAY_COUNT(s_packets); i++)
		{
			i32 len = strlen(s_packets[i].pattern);
			if (packetLen >= len && memcmp(s->GetData(), s_packets[i].pattern, len) == 0)
			{
				bool bDone = false;
				i32 numArgs = 0;
				i32 args[MAX_ARGS];
				s->Eat(len);
				while (!bDone)
				{
					switch (s_packets[i].args[numArgs])
					{
					case ARG_UINT8:
						args[numArgs++] = s->ReadU8();
						break;
					case ARG_UINT16:
						args[numArgs++] = s->ReadU16();
						break;
					case ARG_UINT32:
						args[numArgs++] = s->ReadU32();
						break;
					case ARG_END:
						bDone = true;
						break;
					}
				}
				if (s_packets[i].callback)
				{
					if (!s_packets[i].callback(pContext, args, s, packetLen - (i32)(s->GetData() - packetStart)))
					{
						return false;
					}
				}
				s->Eat(packetLen - (i32)(s->GetData() - packetStart));
				break;
			}
		}
		if (i == DRX_ARRAY_COUNT(s_packets))
		{
			tuk data = s->GetData() + packetLen;
			//DrxLog("SYNERGY: Can't understand packet:%s Length:%d\n", s->GetData(), packetLen);
			s->Eat(packetLen);
		}
	}
	return true;
}

CSynergyContext::CSynergyContext(tukk pIdentifier, tukk pHost)
	: m_host(pHost)
	, m_name(pIdentifier)
	, m_socket(-1)
	, m_packetOverrun(0)
	, m_bQuit(false)
{
	m_clipboard[0] = '\0';
	m_clipboardThread[0] = '\0';
	ZeroStruct(m_mouseState);
}

void CSynergyContext::ThreadEntry()
{
	Stream s(4 * 1024);
	bool bReconnect = true;
	while (!m_bQuit)
	{
		if (bReconnect)
		{
			if (synergyConnectFunc(this))
			{
				bReconnect = false;
			}
			else
			{
	#ifdef SUPPORT_HW_MOUSE_CURSOR
				if (gEnv->pRenderer && gEnv->pRenderer->GetIHWMouseCursor())
				{
					gEnv->pRenderer->GetIHWMouseCursor()->Hide();
				}
	#endif
				Sleep(500);
			}
		}
		else
		{
			i32 outLen = 0;
			if (synergyReceiveFunc(this, s.GetBuffer(), s.GetBufferSize(), &outLen))
			{
				s.Rewind();
				s.SetLength(outLen);
				if (!ProcessPackets(this, &s))
				{
					DrxLog("SYNERGY: Packet processing failed. Reconnecting.\n");
					bReconnect = true;
				}
				if (m_packetOverrun > 0)
				{
					while (m_packetOverrun > 0)
					{
						if (!synergyReceiveFunc(this, s.GetBuffer(), std::min(m_packetOverrun, s.GetBufferSize()), &outLen))
						{
							bReconnect = true;
							break;
						}
						m_packetOverrun -= outLen;
					}
				}
			}
			else
			{
				DrxLog("SYNERGY: Receive failed. Reconnecting.\n");
				bReconnect = true;
			}
		}
	}
}

bool CSynergyContext::GetKey(u32& key, bool& bPressed, bool& bRepeat, u32& modifier)
{
	AUTO_LOCK(m_keyboardLock);
	if (m_keyboardQueue.empty())
		return false;
	KeyPress& kp = m_keyboardQueue.front();
	key = kp.key;
	bPressed = kp.bPressed;
	bRepeat = kp.bRepeat;
	modifier = kp.modifier;
	m_keyboardQueue.pop_front();
	return true;
}

bool CSynergyContext::GetMouse(u16& x, u16& y, u16& wheelX, u16& wheelY, bool& buttonL, bool& buttonM, bool& buttonR)
{
	AUTO_LOCK(m_mouseLock);
	if (m_mouseQueue.empty())
		return false;
	MouseEvent& me = m_mouseQueue.front();
	x = me.x;
	y = me.y;
	wheelX = me.wheelX;
	wheelY = me.wheelY;
	buttonL = me.button[0];
	buttonM = me.button[1];
	buttonR = me.button[2];
	m_mouseQueue.pop_front();
	return true;
}

tukk CSynergyContext::GetClipboard()
{
	AUTO_LOCK(m_clipboardLock);
	drx_strcpy(m_clipboard, m_clipboardThread);
	return m_clipboard;
}

CSynergyContext::~CSynergyContext()
{
	m_bQuit = true;
	if (m_socket >= 0)
		DrxSock::closesocket(m_socket);

	gEnv->pThreadUpr->JoinThread(this, eJM_Join);
}

#endif // USE_SYNERGY_INPUT
