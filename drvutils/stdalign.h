#ifndef __FREESTND_C_HDRS_STDALIGN_H
#define __FREESTND_C_HDRS_STDALIGN_H 1

#ifndef __cplusplus

#if defined(__STDC_VERSION__) && __STDC_VERSION__ > 201710L
    /* These do not need to be defined for C23+ */
#else
#  undef alignas
#  define alignas _Alignas
#  undef alignof
#  define alignof _Alignof

#  undef __alignof_is_defined
#  define __alignof_is_defined 1
#  undef __alignas_is_defined
#  define __alignas_is_defined 1
#endif

#endif

#endif
