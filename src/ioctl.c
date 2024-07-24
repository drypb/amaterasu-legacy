
#include "amaterasu.h"

/*
 *  InfoCloneSetup() -
 *
 *  @Irp:
 *  @IrpIoStack:
 *  @Info:
 *  @InfoBuffer:
 *  @InfoBufferLen:
 *
 *  Return:
 *    -
 *    -
 */
static inline NTSTATUS InfoCloneSetup(
	_In_ PIRP Irp,
	_In_ PIO_STACK_LOCATION IrpIoStack,
	_Out_ PINFO* Info,
	_Out_ PINFO_STATIC* InfoBuffer,
	_Out_ PULONG InfoBufferLen
) {

	if (IsListEmpty(&Amaterasu.InfoList->Head)) {
		return STATUS_UNSUCCESSFUL;
    }

	*Info = InfoListRemoveTail(Amaterasu.InfoList);
	*InfoBuffer = SystemBuffer(Irp);
	*InfoBufferLen = OutputBufferLength(IrpIoStack);

	return STATUS_SUCCESS;
}

/*
 *  InfoClone() -
 *
 *  @Irp:
 *  @IrpIoStack:
 *  @InfoSize:
 *
 *  Return:
 *    -
 *    -
 */
NTSTATUS InfoClone(_In_ PIRP Irp, _In_ PIO_STACK_LOCATION IrpIoStack, _Out_ PULONG InfoSize) {

	NTSTATUS Status;
	PINFO Info;
	PINFO_STATIC InfoBuffer;
	ULONG InfoBufferLen;

	*InfoSize = 0;

    Status = STATUS_SUCCESS;
	Status = InfoCloneSetup(Irp,IrpIoStack, &Info, &InfoBuffer, &InfoBufferLen);
	if (!NT_SUCCESS(Status) || !InfoBuffer) {
		return Status;
	}

	if (InfoBuffer && (sizeof *Info <= InfoBufferLen)) {
		InfoCopy(InfoBuffer, Info);
		*InfoSize = InfoBufferLen;
		InfoFree(&Info);
	} 

	return Status;
}

/*
 *  IoControl() -
 *
 *  @Device:
 *  @Irp:
 *
 *  Return:
 *    -
 *    -
 */
NTSTATUS IoControl(_In_ PDEVICE_OBJECT Device, _In_ PIRP Irp) {

	PIO_STACK_LOCATION IrpIoStack;
	NTSTATUS Status;
    ULONG IoCtl;
    ULONG ClonedInfoSize;

	UNREFERENCED_PARAMETER(Device);

    Status = STATUS_SUCCESS;
    IrpIoStack = IoGetCurrentIrpStackLocation(Irp);
    ClonedInfoSize = 0;

	if (IrpIoStack) {
		IoCtl = IrpIoStack->Parameters.DeviceIoControl.IoControlCode;
		switch (IoCtl) {
		    case IOCTL_GET_INFO:
			    Status = InfoClone(Irp, IrpIoStack, &ClonedInfoSize);
			    break;
		    case IOCTL_AMATERASU_SETUP:
			    Status = AmaterasuSetup(Irp, IrpIoStack, &ClonedInfoSize);
			    break;
		    default:
			    KdPrint(("default io control operation!!!\n"));
			    break;
		}
	}

	Irp->IoStatus.Status = Status;
	Irp->IoStatus.Information = ClonedInfoSize;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return Status;
}
