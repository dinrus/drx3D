
#ifndef D3_CHROME_TRACE_UTIL_H
#define D3_CHROME_TRACE_UTIL_H

#include <drxtypes.h>

void b3ChromeUtilsStartTimings();
void b3ChromeUtilsStopTimingsAndWriteJsonFile(tukk fileNamePrefix);
void b3ChromeUtilsEnableProfiling();

#endif  //D3_CHROME_TRACE_UTIL_H