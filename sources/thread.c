#include "thread.h"
s32 suspendThread(lwp_t intThreadId) {
s32 varout=0;
    if (intThreadId!=LWP_THREAD_NULL) {
        if (!LWP_ThreadIsSuspended(intThreadId)) {
            varout=LWP_SuspendThread(intThreadId);
        }
    }
    return varout;
}
s32 resumeThread(lwp_t intThreadId) {
s32 varout=0;
    if (intThreadId!=LWP_THREAD_NULL) {
        if (LWP_ThreadIsSuspended(intThreadId)) {
            varout=LWP_ResumeThread(intThreadId);
        }
    }
    return varout;
}
