/*************************************************************************
 Dinrus Source File.
 Разработка (C), Dinrus Studios, 2001-2013.
 -------------------------------------------------------------------------
 $Id$
 $DateTime$
 Описание:
 Console implementation for iOS, reports back to the main interface.
 -------------------------------------------------------------------------
 История:
 - Jul 19,2013:	Created by Leander Beernaert
 
 *************************************************************************/

#include <drx3D/Sys/StdAfx.h>
#if defined(IOS)
#include "IOSConsole.h"



CIOSConsole::CIOSConsole():
m_isInitialized(false)
{
    
}

CIOSConsole::~CIOSConsole()
{
    
}

// Interface IOutputPrintSink /////////////////////////////////////////////
void CIOSConsole::Print(tukk line)
{
    printf("MSG: %s\n", line);
}
// Interface ISystemUserCallback //////////////////////////////////////////
bool CIOSConsole::OnError(tukk errorString)
{
    printf("ERR: %s\n", errorString);
    return true;
}

void CIOSConsole::OnInitProgress(tukk sProgressMsg)
{
    (void) sProgressMsg;
    // Do Nothing
}
void CIOSConsole::OnInit(ISystem *pSystem)
{
    if (!m_isInitialized)
    {
        IConsole* pConsole = pSystem->GetIConsole();
        if (pConsole != 0)
        {
            pConsole->AddOutputPrintSink(this);
        }
        m_isInitialized = true;
    }
}
void CIOSConsole::OnShutdown()
{
    if (m_isInitialized)
    {
        // remove outputprintsink
        m_isInitialized = false;
    }
}
void CIOSConsole::OnUpdate()
{
    // Do Nothing
}
void CIOSConsole::GetMemoryUsage(IDrxSizer *pSizer)
{
    size_t size = sizeof(*this);
    
    
    
    pSizer->AddObject(this, size);
}

// Interface ITextModeConsole /////////////////////////////////////////////
Vec2_tpl<i32> CIOSConsole::BeginDraw()
{
    return Vec2_tpl<i32>(0,0);
}
void CIOSConsole::PutText( i32 x, i32 y, tukk  msg )
{
    printf("PUT: %s\n", msg);
}
void CIOSConsole::EndDraw() {
    // Do Nothing
}
#endif // IOS