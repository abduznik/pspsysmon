#ifndef PSP_COMPAT_H
#define PSP_COMPAT_H

/* GCC 14+ compatibility stub for pspdev/pspdev Docker builds.
 * pspmodulemgr_kernel.h references SceLoadCoreExecFileInfo but the
 * SDK does not define it. This stub must be included BEFORE any
 * PSPSDK kernel headers in every .c file. */
typedef struct { int _dummy; } SceLoadCoreExecFileInfo;

#endif /* PSP_COMPAT_H */
