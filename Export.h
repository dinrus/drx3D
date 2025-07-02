
#ifndef DRX3D_EXPORT_H
#define DRX3D_EXPORT_H

#include <drx/Core/Core.h>

#ifdef DRX3D_STATIC_DEFINE
#  define DRX3D_EXPORT
#  define DRX3D_NO_EXPORT
#else
#  ifndef DRX3D_EXPORT
#    ifdef drx3D_EXPORTS
        /* We are building this library */
#      define DRX3D_EXPORT
#    else
        /* We are using this library */
#      define DRX3D_EXPORT
#    endif
#  endif

#  ifndef DRX3D_NO_EXPORT
#    define DRX3D_NO_EXPORT
#  endif
#endif

#ifndef DRX3D_DEPRECATED
#  define DRX3D_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef DRX3D_DEPRECATED_EXPORT
#  define DRX3D_DEPRECATED_EXPORT DRX3D_EXPORT DRX3D_DEPRECATED
#endif

#ifndef DRX3D_DEPRECATED_NO_EXPORT
#  define DRX3D_DEPRECATED_NO_EXPORT DRX3D_NO_EXPORT DRX3D_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef DRX3D_NO_DEPRECATED
#    define DRX3D_NO_DEPRECATED
#  endif
#endif

#endif /* DRX3D_EXPORT_H */
