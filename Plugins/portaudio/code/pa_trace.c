/*
 * $Id$
 * Portable Audio I/O Library Trace Facility
 * Store trace information in real-time for later printing.
 *
 * Based on the Open Source API proposed by Ross Bencina
 * Copyright (c) 1999-2000 Phil Burk
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * The text above constitutes the entire PortAudio license; however,
 * the PortAudio community also makes the following non-binding requests:
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version. It is also
 * requested that these non-binding requests be included along with the
 * license above.
 */

/** @file
 @ingroup common_src

 @brief Real-time safe event trace logging facility for debugging.
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "pa_trace.h"
#include "pa_util.h"
#include "pa_debugprint.h"

#if PA_TRACE_REALTIME_EVENTS

static char const *traceTextArray[PA_MAX_TRACE_RECORDS];
static i32 traceIntArray[PA_MAX_TRACE_RECORDS];
static i32 traceIndex = 0;
static i32 traceBlock = 0;

/*********************************************************************/
void PaUtil_ResetTraceMessages()
{
    traceIndex = 0;
}

/*********************************************************************/
void PaUtil_DumpTraceMessages()
{
    i32 i;
    i32 messageCount = (traceIndex < PA_MAX_TRACE_RECORDS) ? traceIndex : PA_MAX_TRACE_RECORDS;

    printf("DumpTraceMessages: traceIndex = %d\n", traceIndex );
    for( i=0; i<messageCount; i++ )
    {
        printf("%3d: %s = 0x%08X\n",
               i, traceTextArray[i], traceIntArray[i] );
    }
    PaUtil_ResetTraceMessages();
    fflush(stdout);
}

/*********************************************************************/
void PaUtil_AddTraceMessage( tukk msg, i32 data )
{
    if( (traceIndex == PA_MAX_TRACE_RECORDS) && (traceBlock == 0) )
    {
        traceBlock = 1;
        /*  PaUtil_DumpTraceMessages(); */
    }
    else if( traceIndex < PA_MAX_TRACE_RECORDS )
    {
        traceTextArray[traceIndex] = msg;
        traceIntArray[traceIndex] = data;
        traceIndex++;
    }
}

/************************************************************************/
/* High performance log alternative                                     */
/************************************************************************/

typedef zu64  PaUint64;

typedef struct __PaHighPerformanceLog
{
    unsigned    magik;
    i32         writePtr;
    i32         readPtr;
    i32         size;
    double      refTime;
    tuk       data;
} PaHighPerformanceLog;

static const unsigned kMagik = 0xcafebabe;

#define USEC_PER_SEC    (1000000ULL)

i32 PaUtil_InitializeHighSpeedLog( LogHandle* phLog, unsigned maxSizeInBytes )
{
    PaHighPerformanceLog* pLog = (PaHighPerformanceLog*)PaUtil_AllocateMemory(sizeof(PaHighPerformanceLog));
    if (pLog == 0)
    {
        return paInsufficientMemory;
    }
    assert(phLog != 0);
    *phLog = pLog;

    pLog->data = (tuk)PaUtil_AllocateMemory(maxSizeInBytes);
    if (pLog->data == 0)
    {
        PaUtil_FreeMemory(pLog);
        return paInsufficientMemory;
    }
    pLog->magik = kMagik;
    pLog->size = maxSizeInBytes;
    pLog->refTime = PaUtil_GetTime();
    return paNoError;
}

void PaUtil_ResetHighSpeedLogTimeRef( LogHandle hLog )
{
    PaHighPerformanceLog* pLog = (PaHighPerformanceLog*)hLog;
    assert(pLog->magik == kMagik);
    pLog->refTime = PaUtil_GetTime();
}

typedef struct __PaLogEntryHeader
{
    i32    size;
    double timeStamp;
} PaLogEntryHeader;

#ifdef __APPLE__
#define _vsnprintf vsnprintf
#define min(a,b) ((a)<(b)?(a):(b))
#endif


i32 PaUtil_AddHighSpeedLogMessage( LogHandle hLog, tukk fmt, ... )
{
    va_list l;
    i32 n = 0;
    PaHighPerformanceLog* pLog = (PaHighPerformanceLog*)hLog;
    if (pLog != 0)
    {
        PaLogEntryHeader* pHeader;
        tuk p;
        i32 maxN;
        assert(pLog->magik == kMagik);
        pHeader = (PaLogEntryHeader*)( pLog->data + pLog->writePtr );
        p = (tuk)( pHeader + 1 );
        maxN = pLog->size - pLog->writePtr - 2 * sizeof(PaLogEntryHeader);

        pHeader->timeStamp = PaUtil_GetTime() - pLog->refTime;
        if (maxN > 0)
        {
            if (maxN > 32)
            {
                va_start(l, fmt);
                n = _vsnprintf(p, min(1024, maxN), fmt, l);
                va_end(l);
            }
            else {
                n = sprintf(p, "End of log...");
            }
            n = ((n + sizeof(unsigned)) & ~(sizeof(unsigned)-1)) + sizeof(PaLogEntryHeader);
            pHeader->size = n;
#if 0
            PaUtil_DebugPrint("%05u.%03u: %s\n", pHeader->timeStamp/1000, pHeader->timeStamp%1000, p);
#endif
            pLog->writePtr += n;
        }
    }
    return n;
}

void PaUtil_DumpHighSpeedLog( LogHandle hLog, tukk fileName )
{
    FILE* f = (fileName != NULL) ? fopen(fileName, "w") : stdout;
    unsigned localWritePtr;
    PaHighPerformanceLog* pLog = (PaHighPerformanceLog*)hLog;
    assert(pLog->magik == kMagik);
    localWritePtr = pLog->writePtr;
    while (pLog->readPtr != localWritePtr)
    {
        const PaLogEntryHeader* pHeader = (const PaLogEntryHeader*)( pLog->data + pLog->readPtr );
        tukk p = (tukk)( pHeader + 1 );
        const PaUint64 ts = (const PaUint64)( pHeader->timeStamp * USEC_PER_SEC );
        assert(pHeader->size < (1024+sizeof(unsigned)+sizeof(PaLogEntryHeader)));
        fprintf(f, "%05u.%03u: %s\n", (unsigned)(ts/1000), (unsigned)(ts%1000), p);
        pLog->readPtr += pHeader->size;
    }
    if (f != stdout)
    {
        fclose(f);
    }
}

void PaUtil_DiscardHighSpeedLog( LogHandle hLog )
{
    PaHighPerformanceLog* pLog = (PaHighPerformanceLog*)hLog;
    assert(pLog->magik == kMagik);
    PaUtil_FreeMemory(pLog->data);
    PaUtil_FreeMemory(pLog);
}

#else
/* This stub was added so that this file will generate a symbol.
 * Otherwise linker/archiver programs will complain.
 */
i32 PaUtil_TraceStubToSatisfyLinker(void)
{
    return 0;
}
#endif /* TRACE_REALTIME_EVENTS */
