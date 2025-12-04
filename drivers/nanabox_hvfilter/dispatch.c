/*
 * PROJECT:    NanaBox Anti-Detection Edition
 * FILE:       drivers/nanabox_hvfilter/dispatch.c
 * PURPOSE:    IRP dispatch routines
 *
 * LICENSE:    The MIT License
 */

#include "driver.h"

/**
 * @brief Handle IRP_MJ_CREATE requests
 * 
 * @param DeviceObject Pointer to the device object
 * @param Irp Pointer to the IRP
 * @return NTSTATUS Status code
 */
NTSTATUS
NbxDispatchCreate(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    NBX_INFO("NbxDispatchCreate: Device opened\n");

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

/**
 * @brief Handle IRP_MJ_CLOSE requests
 * 
 * @param DeviceObject Pointer to the device object
 * @param Irp Pointer to the IRP
 * @return NTSTATUS Status code
 */
NTSTATUS
NbxDispatchClose(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    NBX_INFO("NbxDispatchClose: Device closed\n");

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

/**
 * @brief Handle IRP_MJ_DEVICE_CONTROL requests
 * 
 * @param DeviceObject Pointer to the device object
 * @param Irp Pointer to the IRP
 * @return NTSTATUS Status code
 */
NTSTATUS
NbxDispatchDeviceControl(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
)
{
    NTSTATUS status;
    PIO_STACK_LOCATION irpStack;
    ULONG ioControlCode;
    PVOID inputBuffer;
    PVOID outputBuffer;
    ULONG inputBufferLength;
    ULONG outputBufferLength;
    ULONG_PTR bytesReturned = 0;

    UNREFERENCED_PARAMETER(DeviceObject);

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;
    inputBufferLength = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

    //
    // For METHOD_BUFFERED, both input and output use the same buffer
    //
    inputBuffer = Irp->AssociatedIrp.SystemBuffer;
    outputBuffer = Irp->AssociatedIrp.SystemBuffer;

    NBX_INFO("NbxDispatchDeviceControl: IOCTL=0x%08X, InputLen=%u, OutputLen=%u\n",
             ioControlCode, inputBufferLength, outputBufferLength);

    //
    // Handle IOCTL
    //
    status = NbxHandleIoctl(
        ioControlCode,
        inputBuffer,
        inputBufferLength,
        outputBuffer,
        outputBufferLength,
        &bytesReturned
    );

    //
    // Complete the IRP
    //
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = bytesReturned;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}
