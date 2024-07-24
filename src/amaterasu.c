
#include "amaterasu.h"

struct Amaterasu Amaterasu;

DECLARE_CONST_UNICODE_STRING(REG_CALLBACK_ALTITUDE, L"388990");

static const FLT_OPERATION_REGISTRATION Callbacks[] = {
    {
        IRP_MJ_CREATE,
        0,
        AmaterasuDefaultPreCallback,
        NULL
    },
    {
        IRP_MJ_READ,
        0,
        AmaterasuDefaultPreCallback,
        NULL
    },
    {
        IRP_MJ_WRITE,
        0,
        AmaterasuDefaultPreCallback,
        NULL
    },
    {IRP_MJ_OPERATION_END}
};

/*
 *  'FLT_REGISTRATION' structure provides a framework for defining the behavior
 *  of a file system filter driver within the Windows Filter Manager.
 */
FLT_REGISTRATION FilterRegistration = {

    sizeof(FLT_REGISTRATION),
    FLT_REGISTRATION_VERSION,

    /*
     *  'FLTFL_REGISTRATION_SUPPORT_NPFS_MSFS' specifies support for Named
     *  Pipes File System (NPFS) and Mailslot File System (MSFS), and
     *  'FLTFL_REGISTRATION_SUPPORT_DAX_VOLUME' specifies support for Direct
     *  Access (DAX) volumes.
     */
    FLTFL_REGISTRATION_SUPPORT_NPFS_MSFS | FLTFL_REGISTRATION_SUPPORT_DAX_VOLUME,
    NULL,
    Callbacks,
    AmaterasuUnload,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static NTSTATUS AmaterasuInitLoadImageFilter(void) {

    NTSTATUS Status;

    Status = PsSetLoadImageNotifyRoutine(&AmaterasuLoadImageCallback);
    if(!NT_SUCCESS(Status)) {
        return Status;
    }

    return Status;
}

static NTSTATUS AmaterasuInitRegFilter(_In_ PDRIVER_OBJECT DriverObject) {

    NTSTATUS Status;

    Status = CmRegisterCallbackEx(AmaterasuRegCallback, &REG_CALLBACK_ALTITUDE, DriverObject, NULL, &Amaterasu.Cookie, NULL);
    if(!NT_SUCCESS(Status)) {
        return Status;
    }

    return Status;
}

/*
 *  AmaterasuInitProcFilter() -
 *
 */
static NTSTATUS AmaterasuInitProcFilter(void) {

    NTSTATUS Status;

    Status = PsSetCreateProcessNotifyRoutine(&AmaterasuProcCallback, FALSE);
    if (NT_SUCCESS(Status)) {
        Status = PsSetCreateThreadNotifyRoutine(&AmaterasuThreadCallback);
        if(!NT_SUCCESS(Status)) {
            PsSetCreateProcessNotifyRoutine(&AmaterasuProcCallback, TRUE);
        }
    }

    return Status;
}

/*
 *  AmaterasuInitFsFilter() -
 *
 *  @DriverObject:
 *
 *  Return:
 *    -
 *    -
 */
static NTSTATUS AmaterasuInitFsFilter(_In_ PDRIVER_OBJECT DriverObject) {

    NTSTATUS Status;

    Status = FltRegisterFilter(DriverObject, &FilterRegistration, &Amaterasu.FilterHandle);
    if (NT_SUCCESS(Status)) {

        /*
         *  'FltStartFiltering()' notifies the Filter Manager that 'Amaterasu' is
         *  ready to begin attaching to volumes and filtering I/O requests.
         */
        Status = FltStartFiltering(Amaterasu.FilterHandle);
    }

    return Status;
}

/*
 *  AmaterasuInitFilters() -
 *
 *  @DriverObject:
 *
 *  Return:
 *    -
 *    -HISTÓRIA
 */
static NTSTATUS AmaterasuInitFilters(_In_ PDRIVER_OBJECT DriverObject) {

    NTSTATUS Status = STATUS_SUCCESS;

    if(Amaterasu.DriverSettings->EnabledCallbacks[FS_CALLBACK]) {
        Status = AmaterasuInitFsFilter(DriverObject);
        if(!NT_SUCCESS(Status)) {
            return Status;
        }
        Amaterasu.DriverSettings->EnabledCallbacks[FS_CALLBACK] = 0;
    }

    if(Amaterasu.DriverSettings->EnabledCallbacks[PROC_CALLBACK]) {
        Status = AmaterasuInitProcFilter();
        if(!NT_SUCCESS(Status)) {
            return Status; 
        }
        Amaterasu.DriverSettings->EnabledCallbacks[PROC_CALLBACK] = 0;
    }

    if(Amaterasu.DriverSettings->EnabledCallbacks[LOAD_IMAGE_CALLBACK]) {
        Status = AmaterasuInitLoadImageFilter();
        if(!NT_SUCCESS(Status)) {
            return Status; 
        }
        Amaterasu.DriverSettings->EnabledCallbacks[LOAD_IMAGE_CALLBACK] = 0;
    }

    if(Amaterasu.DriverSettings->EnabledCallbacks[REG_CALLBACK]) {
        Status = AmaterasuInitRegFilter(DriverObject);
        if(!NT_SUCCESS(Status)) {
            return Status;
        }
        Amaterasu.DriverSettings->EnabledCallbacks[REG_CALLBACK] = 0;
    }

    return Status;
}

/*
 *  AmaterasuInit() -
 *
 *  @DriverObject:
 *  @RegistryPath:
 *
 *  Return:
 *    -
 *    -
 */
static NTSTATUS AmaterasuInit(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {

    NTSTATUS Status;

    Status = STATUS_SUCCESS;

    KeInitializeSpinLock(&Amaterasu.HandleArrLock);

    AmaterasuInitFilters(DriverObject);
    Amaterasu.InfoList = InfoListGet(NonPagedPool, MAX_RECORDS);
    if (!Amaterasu.InfoList) {
        return STATUS_UNSUCCESSFUL;
    }

    Amaterasu.DriverObject = DriverObject;
    Amaterasu.RegistryPath = RegistryPath;

    return Status;
}

/*
 *  Create() -
 *
 *  @DeviceObject:
 *  @Irp:
 *
 *  Return:
 *    -
 *    -
 */
NTSTATUS Create(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp) {
    UNREFERENCED_PARAMETER(Irp);
    UNREFERENCED_PARAMETER(DeviceObject);
    return STATUS_SUCCESS;
}

/*
 *  Close() -
 *
 *  @DeviceObject:
 *  @Irp:
 *
 *  Return:
 *    -
 *    -
 */
NTSTATUS Close(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp) {
    UNREFERENCED_PARAMETER(Irp);
    UNREFERENCED_PARAMETER(DeviceObject);
    return STATUS_SUCCESS;
}

/*
 *  AmaterasuSetup() -
 *
 *  @Irp:
 *  @IrpIoStack:
 *  @InfoSize:
 *
 *  Return:
 *    -
 *    -
 */
NTSTATUS AmaterasuSetup(PIRP Irp, PIO_STACK_LOCATION IrpIoStack, PULONG InfoSize) {
    
	NTSTATUS Status;
    PDRIVER_SETTINGS DriverSettings; 
    ULONG BufferLen;

    Status = STATUS_SUCCESS;
    DriverSettings = SystemBuffer(Irp);
	BufferLen = OutputBufferLength(IrpIoStack);

	AmaterasuInitFilters(Amaterasu.DriverObject);
    Amaterasu.InfoList = InfoListGet(NonPagedPool, DriverSettings->ListMaxRecords);
    if (!Amaterasu.InfoList) {
        return STATUS_UNSUCCESSFUL;
    }

    Amaterasu.DriverSettings = DriverSettings;

	return Status;
}


/*
 *  DeviceSetup() -
 *
 *  @DriverObject:
 *
 *  Return:
 *    -
 *    -
 */
NTSTATUS DeviceSetup(_In_ PDRIVER_OBJECT DriverObject) {

    NTSTATUS Status;
    PDEVICE_OBJECT Device;
    UNICODE_STRING DriverName;
    UNICODE_STRING DeviceSymLink;

    RtlInitUnicodeString(&DriverName, DRIVER_NAME);
    RtlInitUnicodeString(&DeviceSymLink, DEVICE_SYM_LINK);

    Status = IoCreateDevice(
        DriverObject,
        0,
        &DriverName,
        FILE_DEVICE_UNKNOWN,
        FILE_DEVICE_SECURE_OPEN,
        FALSE,
        &Device
    );

    if (NT_SUCCESS(Status)) {
        Status = IoCreateSymbolicLink(&DeviceSymLink, &DriverName);
        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

    DriverObject->DeviceObject = Device;

    return Status;
}

/*
 *  DispatchRoutinesSetup() -
 *
 *  @DriverObject:
 *
 *  Return:
 *    -
 *    -
 */
NTSTATUS DispatchRoutinesSetup(_In_ PDRIVER_OBJECT DriverObject){

    //Dispatch routines needed for driver handle creation
    DriverObject->MajorFunction[IRP_MJ_CREATE] = Create;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = Close;

    //Dispatch routines to use ioctl communication
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoControl;

    return STATUS_SUCCESS;
}

/*
 *  AmaterasuCleanup() -
 *
 */
NTSTATUS AmaterasuCleanup(void) {

    InfoListFree(&Amaterasu.InfoList);
}

/*
 *  AmaterasuUnload() -
 *
 *  @Flags:
 *
 *  Return:
 *    -
 *    -
 */
NTSTATUS AmaterasuUnload(_In_ FLT_FILTER_UNLOAD_FLAGS Flags) {

    UNREFERENCED_PARAMETER(Flags);

    AmaterasuCleanup();

    if(!Amaterasu.DriverSettings->EnabledCallbacks[FS_CALLBACK]) {
        FltUnregisterFilter(Amaterasu.FilterHandle);
    }

    if(!Amaterasu.DriverSettings->EnabledCallbacks[PROC_CALLBACK]) {
        PsSetCreateProcessNotifyRoutine(&AmaterasuProcCallback, TRUE);
        PsRemoveCreateThreadNotifyRoutine(&AmaterasuThreadCallback);
    }
    
    if(!Amaterasu.DriverSettings->EnabledCallbacks[LOAD_IMAGE_CALLBACK]) {
        PsRemoveLoadImageNotifyRoutine(&AmaterasuLoadImageCallback);
    }

    if(!Amaterasu.DriverSettings->EnabledCallbacks[REG_CALLBACK]) {
        CmUnRegisterCallback(Amaterasu.Cookie);
    }
}

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
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {

    NTSTATUS Status;

    /* Makes non-paged pools (kernel pools) allocations non-executable. */
    ExInitializeDriverRuntime(DrvRtPoolNxOptIn);

    Status = AmaterasuInit(DriverObject, RegistryPath);
    if (NT_SUCCESS(Status)) {
        Status = DeviceSetup(DriverObject);
        if(NT_SUCCESS(Status)) {
            Status = DispatchRoutinesSetup(DriverObject);
        }
    }

    if(!NT_SUCCESS(Status)) {
        AmaterasuCleanup();
    }

    return Status;
}
