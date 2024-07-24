#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "info.h"
#include "infolist.h"
#include "callbacks.h"
#include "ioctl.h"
#include "utils.h"

extern FLT_PREOP_CALLBACK_STATUS
AmaterasuDefaultPreCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
);

extern void AmaterasuLoadImageCallback(
	PUNICODE_STRING FullImageName,
	HANDLE ProcessId,
	PIMAGE_INFO ImageInfo
);

extern NTSTATUS AmaterasuRegCallback(
	PVOID CallbackContext,
	PVOID RegNotifyClass,
	PVOID RegStruct
);

extern void AmaterasuProcCallback(
	_In_ HANDLE PPID,
	_In_ HANDLE PID,
	_In_ BOOLEAN Active
);

extern void AmaterasuThreadCallback(
	_In_ HANDLE PPID,
	_In_ HANDLE TID,
	_In_ BOOLEAN Active
);

#endif  /* CALLBACKS_H */
