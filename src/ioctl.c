#include "ioctl.h"

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

	line();
	if (!Amaterasu.InfoList->RecordsAllocated) {
		line();
		return STATUS_UNSUCCESSFUL;
    }
	line();

	__try {
		*Info = InfoListRemoveTail(Amaterasu.InfoList);
		line();
		*InfoBuffer = SystemBuffer(Irp);
		line();
		*InfoBufferLen = OutputBufferLength(IrpIoStack);
		line();
	} except(EXCEPTION_CONTINUE_EXECUTION) {
		return STATUS_UNSUCCESSFUL;
	}

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

	line();
	Status = STATUS_UNSUCCESSFUL;
	//if (InfoSize) {
		line();
		*InfoSize = 0;

		Status = STATUS_SUCCESS;
		line();
		line();
		Status = InfoCloneSetup(Irp, IrpIoStack, &Info, &InfoBuffer, &InfoBufferLen);
		if (!NT_SUCCESS(Status) || !InfoBuffer) {
			return Status;
		}
		line();

		DbgPrint("InfoBufferLen: %ul\n", InfoBufferLen);
		if (InfoBuffer && (sizeof(INFO_STATIC) <= InfoBufferLen)) {
			line();
			InfoCopy(InfoBuffer, Info);
			line();
			*InfoSize = InfoBufferLen;
			line();
			InfoFree(&Info);
			line();
		}
		line();

		DbgPrint("InfoSize: %ul\n", *InfoSize);
//	}

	line();
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
	__try {
		if (Irp) {
			Status = STATUS_SUCCESS;
			IrpIoStack = IoGetCurrentIrpStackLocation(Irp);
			ClonedInfoSize = 0;

			if (IrpIoStack) {
				IoCtl = IrpIoStack->Parameters.DeviceIoControl.IoControlCode;
				switch (IoCtl) {
				case IOCTL_GET_INFO:
					line();
					DbgPrint("InfoClone %d\n", Amaterasu.InfoList->RecordsAllocated);

					Status = InfoClone(Irp, IrpIoStack, &ClonedInfoSize);
					DbgPrint("Size: %d", ClonedInfoSize);
					break;
				case IOCTL_AMATERASU_SETUP:
					Status = AmaterasuSetup(Irp, IrpIoStack);
					ClonedInfoSize = sizeof(int);
					break;
				default:
					KdPrint(("default io control operation!!!\n"));
					break;
				}
			}
			line();
			Irp->IoStatus.Status = Status;
			line();
			Irp->IoStatus.Information = ClonedInfoSize;
			line();

			IoCompleteRequest(Irp, IO_NO_INCREMENT);
			line();
		}
		else {
			DbgPrint("IRP IS NULLL\n");
		}
	} except(EXCEPTION_CONTINUE_EXECUTION) {
		line();
	}

	return Status;
}
