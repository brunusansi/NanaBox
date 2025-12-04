/*
 * PROJECT:    NanaBox Anti-Detection Edition
 * FILE:       drivers/nanabox_hvfilter/device.c
 * PURPOSE:    Device creation and management
 *
 * LICENSE:    The MIT License
 */

#include "driver.h"

/**
 * @brief Create device and symbolic link
 * 
 * @param DriverObject Pointer to the driver object
 * @return NTSTATUS Status code
 */
NTSTATUS
NbxCreateDevice(
    _In_ PDRIVER_OBJECT DriverObject
)
{
    NTSTATUS status;
    PDEVICE_OBJECT deviceObject = NULL;
    UNICODE_STRING deviceName;
    UNICODE_STRING symbolicName;

    NBX_INFO("NbxCreateDevice: Creating device\n");

    //
    // Initialize device name
    //
    RtlInitUnicodeString(&deviceName, NANABOX_HVFILTER_DEVICE_NAME_W);

    //
    // Create device object
    //
    status = IoCreateDevice(
        DriverObject,
        0,                              // No device extension
        &deviceName,
        FILE_DEVICE_UNKNOWN,
        FILE_DEVICE_SECURE_OPEN,
        FALSE,                          // Not exclusive
        &deviceObject
    );

    if (!NT_SUCCESS(status)) {
        NBX_ERROR("NbxCreateDevice: IoCreateDevice failed, status=0x%08X\n", status);
        return status;
    }

    //
    // Initialize symbolic link name
    //
    RtlInitUnicodeString(&symbolicName, NANABOX_HVFILTER_SYMBOLIC_NAME_W);

    //
    // Create symbolic link
    //
    status = IoCreateSymbolicLink(&symbolicName, &deviceName);
    if (!NT_SUCCESS(status)) {
        NBX_ERROR("NbxCreateDevice: IoCreateSymbolicLink failed, status=0x%08X\n", status);
        IoDeleteDevice(deviceObject);
        return status;
    }

    //
    // Set device flags
    //
    deviceObject->Flags |= DO_BUFFERED_IO;
    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    //
    // Save device information in global context
    //
    g_DriverContext.DeviceObject = deviceObject;
    g_DriverContext.DeviceName = deviceName;
    g_DriverContext.SymbolicLinkName = symbolicName;

    NBX_INFO("NbxCreateDevice: Device created successfully\n");

    return STATUS_SUCCESS;
}

/**
 * @brief Destroy device and symbolic link
 * 
 * @param DeviceObject Pointer to the device object
 */
VOID
NbxDestroyDevice(
    _In_ PDEVICE_OBJECT DeviceObject
)
{
    NBX_INFO("NbxDestroyDevice: Destroying device\n");

    //
    // Delete symbolic link
    //
    if (g_DriverContext.SymbolicLinkName.Buffer != NULL) {
        IoDeleteSymbolicLink(&g_DriverContext.SymbolicLinkName);
    }

    //
    // Delete device object
    //
    if (DeviceObject != NULL) {
        IoDeleteDevice(DeviceObject);
    }

    NBX_INFO("NbxDestroyDevice: Device destroyed successfully\n");
}
