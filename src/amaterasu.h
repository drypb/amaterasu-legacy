#ifndef AMATERASU_H
#define AMATERASU_H

#include "info.h"
#include "infolist.h"
#include "callbacks.h"
#include "ioctl.h"
#include "utils.h"

#define DRIVER_NAME L"\\Device\\MiniFilter"
#define DEVICE_SYM_LINK L"\\DosDevices\\MiniFilter"
#define CALLBACK_NUMBER 4


struct _DriverSettings {
    BOOLEAN EnabledCallbacks[CALLBACK_NUMBER];
    ULONG ListMaxRecords;
    WCHAR TargetName[MAX_PATH];
};

typedef struct _DriverSettings DRIVER_SETTINGS, * PDRIVER_SETTINGS;

enum CallbacksTypes {
    FS_CALLBACK,
    PROC_CALLBACK,
    LOAD_IMAGE_CALLBACK,
    REG_CALLBACK
};

typedef enum CallbacksTypes CALLBACK_TYPES, * PCALLBACK_TYPES;


struct Amaterasu {

    PDRIVER_OBJECT  DriverObject;
    PUNICODE_STRING RegistryPath;
    PFLT_FILTER     FilterHandle;

    KSPIN_LOCK      HandleArrLock;
    SIZE_T          HandleArrSize;
    HANDLE          HandleArr[1024];
    
    PINFO_LIST      InfoList;

    PDRIVER_SETTINGS DriverSettings;

    LARGE_INTEGER Cookie;

};

extern struct Amaterasu Amaterasu;

extern NTSTATUS
Create(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
);

extern NTSTATUS 
Close(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
);

extern NTSTATUS
AmaterasuSetup(
    PIRP Irp, 
    PIO_STACK_LOCATION IrpIoStack, 
    PULONG InfoSize
);

/*
 *	DriverEntry() - Initializes the driver upon loading into memory.
 *
 *	@DriverObject: Pointer to the driver object created by the I/O manager.
 *	@RegistryPath: Path to the driver's registry key in the registry.
 *
 *  Return:
 *    - 'STATUS_SUCCESS' on success.
 *    - The appropriate 'NTSTATUS' is returned to indicate failure.
 */
extern NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
);

NTSTATUS AmaterasuUnload(_In_ FLT_FILTER_UNLOAD_FLAGS Flags);

#endif  /* AMATERASU_H */
