// This is a hack to make __assert_fail and friends non-noreturn.
// Whenever <assert.h> is included, we'll include this file (enforced via -I).

#if !defined(_LinuxLauncher_assert_h_)
#define _LinuxLauncher_assert_h_

#define __assert_fail __assert_fail__SKIP
#define __assert_perror_fail __assert_perror_fail__SKIP
#include_next <assert.h>
#undef __assert_fail
#undef __assert_perror_fail

#if defined(__cplusplus)
extern "C" {
#endif
void __assert_fail(
		tukk assertion,
		tukk file,
		u32 line,
		tukk function)
  __attribute__ ((visibility("default")))
	;

void __assert_perror_fail(
		i32 errnum,
		tukk file,
		u32 line,
		tukk function)
	__attribute__ ((visibility("default")))
	;
#if defined(__cplusplus)
} // extern "C"
#endif

#endif // _LinuxLauncher_assert_h_

// vim:ts=2

