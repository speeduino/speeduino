#ifndef _DIGITALIO_MATCHED_BOARD
#error "This header's Arduino configuration heuristics couldn't match this board configuration. No fast I/O is available. The header may be out of date."
#endif
#undef _DIGITALIO_MATCHED_BOARD

#ifndef DIGITALIO_MANUAL
#define digitalWrite digitalWriteSafe
#define digitalRead digitalReadSafe
#define pinMode pinModeSafe
#endif

#endif
#endif
