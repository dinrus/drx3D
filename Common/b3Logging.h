#ifndef D3_LOGGING_H
#define D3_LOGGING_H

#include <drxtypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

///We add the do/while so that the statement "if (condition) drx3DPrintf("test"); else {...}" would fail
///You can also customize the message by uncommenting out a different line below
#define drx3DPrintf(...) b3OutputPrintfVarArgsInternal(__VA_ARGS__)
	//#define drx3DPrintf(...) do {b3OutputPrintfVarArgsInternal("drx3DPrintf[%s,%d]:",__FILE__,__LINE__);b3OutputPrintfVarArgsInternal(__VA_ARGS__); } while(0)
	//#define drx3DPrintf b3OutputPrintfVarArgsInternal
	//#define drx3DPrintf(...) printf(__VA_ARGS__)
	//#define drx3DPrintf(...)
#define drx3DWarning(...) do{	b3OutputWarningMessageVarArgsInternal("Предупреждение drx3D[%s,%d]:\n", __FILE__, __LINE__);b3OutputWarningMessageVarArgsInternal(__VA_ARGS__);} while (0)
#define drx3DError(...)do	{b3OutputErrorMessageVarArgsInternal("Ошибка drx3D[%s,%d]:\n", __FILE__, __LINE__);b3OutputErrorMessageVarArgsInternal(__VA_ARGS__);} while (0)
#ifndef D3_NO_PROFILE

	void b3EnterProfileZone(tukk name);
	void b3LeaveProfileZone();
#ifdef __cplusplus

	class b3ProfileZone
	{
	public:
		b3ProfileZone(tukk name)
		{
			b3EnterProfileZone(name);
		}

		~b3ProfileZone()
		{
			b3LeaveProfileZone();
		}
	};

#define D3_PROFILE(name) b3ProfileZone __profile(name)
#endif

#else  //D3_NO_PROFILE

#define D3_PROFILE(name)
#define b3StartProfile(a)
#define b3StopProfile

#endif  //#ifndef D3_NO_PROFILE

	typedef void(drx3DPrintfFunc)(tukk msg);
	typedef void(drx3DWarningMessageFunc)(tukk msg);
	typedef void(drx3DErrorMessageFunc)(tukk msg);
	typedef void(b3EnterProfileZoneFunc)(tukk msg);
	typedef void(b3LeaveProfileZoneFunc)();

	///The developer can route drx3DPrintf output using their own implementation
	void b3SetCustomPrintfFunc(drx3DPrintfFunc* printfFunc);
	void b3SetCustomWarningMessageFunc(drx3DWarningMessageFunc* warningMsgFunc);
	void b3SetCustomErrorMessageFunc(drx3DErrorMessageFunc* errorMsgFunc);

	///Set custom profile zone functions (zones can be nested)
	void b3SetCustomEnterProfileZoneFunc(b3EnterProfileZoneFunc* enterFunc);
	void b3SetCustomLeaveProfileZoneFunc(b3LeaveProfileZoneFunc* leaveFunc);

	///Don't use those internal functions directly, use the drx3DPrintf or b3SetCustomPrintfFunc instead (or warning/error version)
	void b3OutputPrintfVarArgsInternal(tukk str, ...);
	void b3OutputWarningMessageVarArgsInternal(tukk str, ...);
	void b3OutputErrorMessageVarArgsInternal(tukk str, ...);

#ifdef __cplusplus
}
#endif

#endif  //D3_LOGGING_H