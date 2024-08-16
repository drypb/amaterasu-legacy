#include "amaterasu.h"
#include "registration.h"


DECLARE_CONST_UNICODE_STRING(REG_CALLBACK_ALTITUDE, L"388990");

NTSTATUS AmaterasuCleanup(FLT_FILTER_UNLOAD_FLAGS Flags);

BOOLEAN EnabledCallbacks[] = {1,0,0,0};

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
const FLT_REGISTRATION FilterRegistration = {

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
    AmaterasuCleanup,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};



static NTSTATUS AmaterasuInitFsFilter(_In_ PDRIVER_OBJECT DriverObject) {

    NTSTATUS Status;

    Status = FltRegisterFilter(DriverObject, &FilterRegistration, &Amaterasu.FilterHandle);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    /*
     *  'FltStartFiltering()' notifies the Filter Manager that 'Amaterasu' is
     *  ready to begin attaching to volumes and filtering I/O requests.
     */
    Status = FltStartFiltering(Amaterasu.FilterHandle);
    if (!NT_SUCCESS(Status)) {
        FltUnregisterFilter(Amaterasu.FilterHandle);
    }

    return Status;
}




static NTSTATUS AmaterasuInitProcFilter(void) {

    NTSTATUS Status;

    Status = PsSetCreateProcessNotifyRoutine(&AmaterasuProcCallback, FALSE);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = PsSetCreateThreadNotifyRoutine(&AmaterasuThreadCallback);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }


    return Status;
}


static NTSTATUS AmaterasuInitFilters(_In_ PDRIVER_OBJECT DriverObject) {

    NTSTATUS Status = STATUS_SUCCESS;
    if (IsFsCallbackOn(Amaterasu)) {
        Status = AmaterasuInitFsFilter(DriverObject);
        if (!NT_SUCCESS(Status)) {
            FltUnregisterFilter(Amaterasu.FilterHandle);
            return Status;
        }
        KdPrint(("FsCallback registered\n"));
    }

    if (IsProcCallbackOn(Amaterasu)) {
        Status = AmaterasuInitProcFilter();
        if (!NT_SUCCESS(Status))
            return Status;
        KdPrint(("ProcCallback registered\n"));
    }

    if (IsLoadImageCallbackOn(Amaterasu)) {
        Status = PsSetLoadImageNotifyRoutine(&AmaterasuLoadImageCallback);
        if (!NT_SUCCESS(Status))
            return Status;
        KdPrint(("LoadImageCallback registered\n"));
    }

    if (IsRegCallbackOn(Amaterasu)) {
        Status = CmRegisterCallbackEx(AmaterasuRegCallback, &REG_CALLBACK_ALTITUDE, DriverObject, NULL, &Amaterasu.Cookie, NULL);
        if (!NT_SUCCESS(Status))
            return Status;
        KdPrint(("RegCallback registered\n"));
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

    KdPrint(("AmaterasuInit\n"));


    NTSTATUS Status = STATUS_SUCCESS;

    RtlCopyMemory(EnabledCallbacks(Amaterasu), EnabledCallbacks, CALLBACK_NUMBER * sizeof * EnabledCallbacks);

    AmaterasuInitFilters(DriverObject);

    Amaterasu.InfoList = InfoListGet(NonPagedPool, MAX_RECORDS);
    if (!Amaterasu.InfoList) {
        return STATUS_UNSUCCESSFUL;
    }

    Amaterasu.DriverObject = DriverObject;
    Amaterasu.RegistryPath = RegistryPath;

    return Status;
}



//NTSTATUS Create(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
//    return STATUS_SUCCESS;
//}
//
//NTSTATUS Close(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
//    return STATUS_SUCCESS;
//}

//NTSTATUS AmaterasuSetup(PIRP Irp, PIO_STACK_LOCATION IrpIoStack, PULONG InfoSize) {
//
//	NTSTATUS Status = STATUS_SUCCESS;
//    PDRIVER_SETTINGS DriverSettings; 
//    ULONG BufferLen;
//    
//    DriverSettings = SystemBuffer(Irp);
//	BufferLen = OutputBufferLength(IrpIoStack);
//
//    RtlCopyMemory(Amaterasu.DriverSettings, DriverSettings, sizeof * DriverSettings);
//
//	AmaterasuInitFilters(Amaterasu.DriverObject);
//
//    Amaterasu.InfoList = InfoListGet(NonPagedPool, MaxRecords(Amaterasu));
//    if (!Amaterasu.InfoList) {
//        return STATUS_UNSUCCESSFUL;
//    }
//
//	return Status;
//}


NTSTATUS DeviceSetup(PDRIVER_OBJECT DriverObject) {
    NTSTATUS Status;
    PDEVICE_OBJECT device;
    UNICODE_STRING driverName, deviceSymLink;

    RtlInitUnicodeString(&driverName, DRIVER_NAME);
    RtlInitUnicodeString(&deviceSymLink, DEVICE_SYM_LINK);

    Status = IoCreateDevice(DriverObject, 0, &driverName,
        FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN,
        FALSE, &device);

    if (!NT_SUCCESS(Status))
        return Status;

    Status = IoCreateSymbolicLink(&deviceSymLink, &driverName);

    if (!NT_SUCCESS(Status))
        return Status;

    DriverObject->DeviceObject = device;

    return Status;
}

NTSTATUS DispatchRoutinesSetup(PDRIVER_OBJECT DriverObject){
    //Dispatch routines needed for driver handle creation
    DriverObject->MajorFunction[IRP_MJ_CREATE] = Create;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = Close;

    //Dispatch routines to use ioctl communication
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoControl;

    return STATUS_SUCCESS;
}

//NTSTATUS AmaterasuCleanup(FLT_FILTER_UNLOAD_FLAGS Flags) {
//
//    NTSTATUS Status;
//
//    InfoListFree(&Amaterasu.InfoList);
//
//    FltUnregisterFilter(Amaterasu.FilterHandle);
//
//
//    Status = PsSetCreateProcessNotifyRoutine(&AmaterasuProcCallback, TRUE);
//    if (!NT_SUCCESS(Status)) {
//        return Status;
//    }
//
//    Status = PsRemoveCreateThreadNotifyRoutine(&AmaterasuThreadCallback);
//    if (!NT_SUCCESS(Status)) {
//        return Status;
//    }
//
//    Status = PsRemoveLoadImageNotifyRoutine(&AmaterasuLoadImageCallback);
//    if (!NT_SUCCESS(Status)) {
//        return Status;
//    }
//
//    Status = CmUnRegisterCallback(Amaterasu.Cookie);
//
//    return Status;
//
//}

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

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Entry alo\n");

    /* Makes non-paged pools (kernel pools) allocations non-executable. */
    ExInitializeDriverRuntime(DrvRtPoolNxOptIn);

    Status = AmaterasuInit(DriverObject, RegistryPath);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = DeviceSetup(DriverObject);
    if (!NT_SUCCESS(Status)) {
        FltUnregisterFilter(Amaterasu.FilterHandle);
        return Status;
    }

    Status = DispatchRoutinesSetup(DriverObject);
    if (!NT_SUCCESS(Status)) {
        FltUnregisterFilter(Amaterasu.FilterHandle);
        return Status;
    }


    return Status;
}