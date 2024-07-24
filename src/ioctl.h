#ifndef IOCTL_H
#define IOCTL_H

#include "common.h"

#define SystemBuffer(Irp) Irp->AssociatedIrp.SystemBuffer
#define OutputBufferLength(IrpIoStack) IrpIoStack->Parameters.DeviceIoControl.OutputBufferLength

#define IOCTL_GET_INFO CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
#define IOCTL_AMATERASU_SETUP CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)

extern NTSTATUS
InfoClone(
	_In_ PIRP Irp,
	_In_ PIO_STACK_LOCATION IrpIoStack,
	_Out_ PULONG InfoSize
);

extern NTSTATUS
IoControl(
        _In_ PDEVICE_OBJECT Device, 
        _In_ PIRP Irp
    );

#endif
