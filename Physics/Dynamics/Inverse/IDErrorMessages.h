///@file error message utility functions
#ifndef IDUTILS_H_
#define IDUTILS_H_
#include <cstring>
/// name of file being compiled, without leading path components
#define __INVDYN_FILE_WO_DIR__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#if !defined(DRX3D_ID_WO_BULLET) && !defined(DRX3D_USE_INVERSE_DYNAMICS_WITH_BULLET2)
#include <drx3D/Common/b3Logging.h>
#define drx3d_id_error_message(...) drx3DError(__VA_ARGS__)
#define drx3d_id_warning_message(...) drx3DWarning(__VA_ARGS__)
#define id_printf(...) drx3DPrintf(__VA_ARGS__)
#else  // DRX3D_ID_WO_BULLET
#include <cstdio>
/// print error message with file/line information
#define drx3d_id_error_message(...)                                             \
	do                                                                       \
	{                                                                        \
		fprintf(stderr, "[Error:%s:%d] ", __INVDYN_FILE_WO_DIR__, __LINE__); \
		fprintf(stderr, __VA_ARGS__);                                        \
	} while (0)
/// print warning message with file/line information
#define drx3d_id_warning_message(...)                                             \
	do                                                                         \
	{                                                                          \
		fprintf(stderr, "[Warning:%s:%d] ", __INVDYN_FILE_WO_DIR__, __LINE__); \
		fprintf(stderr, __VA_ARGS__);                                          \
	} while (0)
#define id_printf(...) printf(__VA_ARGS__)
#endif  // DRX3D_ID_WO_BULLET
#endif
