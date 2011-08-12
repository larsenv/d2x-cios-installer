#ifndef _THREAD_H_
#define _THREAD_H_
#include <ogc/lwp.h>
#include <gctypes.h>
s32 suspendThread(lwp_t intThreadId);
s32 resumeThread(lwp_t intThreadId);
#endif
