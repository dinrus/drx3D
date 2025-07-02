// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:
   Console implementation for iOS, reports back to the main interface.
   -------------------------------------------------------------------------
   История:
   - Jul 19,2013:	Created by Leander Beernaert

*************************************************************************/

#ifndef _IOSCONSOLE_H_
#define _IOSCONSOLE_H_

#include <drx3D/Sys/IConsole.h>
#include <drx3D/Sys/ITextModeConsole.h>
#include <drx3D/Network/INetwork.h>

class CIOSConsole : public ISystemUserCallback,
	                  public IOutputPrintSink,
	                  public ITextModeConsole
{

	CIOSConsole(const CIOSConsole&);
	CIOSConsole& operator=(const CIOSConsole&);

	bool m_isInitialized;
public:
	static DrxCriticalSectionNonRecursive s_lock;
public:
	CIOSConsole();
	~CIOSConsole();

	// Interface IOutputPrintSink /////////////////////////////////////////////
	DLL_EXPORT virtual void Print(tukk line);

	// Interface ISystemUserCallback //////////////////////////////////////////
	virtual bool          OnError(tukk errorString);
	virtual bool          OnSaveDocument()  { return false; }
	virtual void          OnProcessSwitch() {}
	virtual void          OnInitProgress(tukk sProgressMsg);
	virtual void          OnInit(ISystem*);
	virtual void          OnShutdown();
	virtual void          OnUpdate();
	virtual void          GetMemoryUsage(IDrxSizer* pSizer);
	void                  SetRequireDedicatedServer(bool) {}
	virtual void          SetHeader(tukk)          {}
	// Interface ITextModeConsole /////////////////////////////////////////////
	virtual Vec2_tpl<i32> BeginDraw();
	virtual void          PutText(i32 x, i32 y, tukk msg);
	virtual void          EndDraw();

};

#endif // _IOSCONSOLE_H_
