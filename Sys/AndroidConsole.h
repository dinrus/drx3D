// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:
   Реализация консоли для Android, репортирует обратно в главный интерфейс.
   -------------------------------------------------------------------------
   История:
   - Aug 26,2013:	Created by Leander Beernaert

*************************************************************************/

#ifndef _ANDROIDCONSOLE_H_
#define _ANDROIDCONSOLE_H_

#include <drx3D/Sys/IConsole.h>
#include <drx3D/Sys/ITextModeConsole.h>
#include <drx3D/Network/INetwork.h>

class CAndroidConsole : public ISystemUserCallback,
	                      public IOutputPrintSink,
	                      public ITextModeConsole
{

	CAndroidConsole(const CAndroidConsole&);
	CAndroidConsole& operator=(const CAndroidConsole&);

	bool m_isInitialized;
public:
	static DrxCriticalSectionNonRecursive s_lock;
public:
	CAndroidConsole();
	~CAndroidConsole();

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

#endif // _ANDROIDCONSOLE_H_
