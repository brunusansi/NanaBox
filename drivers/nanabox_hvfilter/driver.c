/*
 * PROJECT:    NanaBox Anti-Detection Edition
 * FILE:       drivers/nanabox_hvfilter/driver.c
 * PURPOSE:    Main driver entry point and initialization
 *
 * LICENSE:    The MIT License
 */

#include "driver.h"

//
// Global driver context
//
NBX_DRIVER_CONTEXT g_DriverContext = { 0 };

/**
 * @brief Driver entry point
 * 
 * @param DriverObject Pointer to the driver object
 * @param RegistryPath Pointer to the registry path
 * @return NTSTATUS Status code
 */
NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    NTSTATUS status;

    UNREFERENCED_PARAMETER(RegistryPath);

    NBX_INFO("DriverEntry: NanaBox HvFilter Driver v%d.%d.%d\n",
             NANABOX_HVFILTER_VERSION_MAJOR,
             NANABOX_HVFILTER_VERSION_MINOR,
             NANABOX_HVFILTER_VERSION_BUILD);

    //
    // Initialize driver context
    //
    RtlZeroMemory(&g_DriverContext, sizeof(g_DriverContext));
    g_DriverContext.IsActive = FALSE;
    RtlCopyMemory(g_DriverContext.ActiveProfileName, "None", sizeof("None"));

    //
    // Set up driver dispatch routines
    //
    DriverObject->MajorFunction[IRP_MJ_CREATE] = NbxDispatchCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = NbxDispatchClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = NbxDispatchDeviceControl;
    DriverObject->DriverUnload = DriverUnload;

    //
    // Create device
    //
    status = NbxCreateDevice(DriverObject);
    if (!NT_SUCCESS(status)) {
        NBX_ERROR("DriverEntry: Failed to create device, status=0x%08X\n", status);
        return status;
    }

    NBX_INFO("DriverEntry: Driver loaded successfully\n");

    return STATUS_SUCCESS;
}

/**
 * @brief Driver unload routine
 * 
 * @param DriverObject Pointer to the driver object
 */
VOID
DriverUnload(
    _In_ PDRIVER_OBJECT DriverObject
)
{
    NBX_INFO("DriverUnload: Unloading driver\n");

    //
    // Destroy device
    //
    if (g_DriverContext.DeviceObject != NULL) {
        NbxDestroyDevice(g_DriverContext.DeviceObject);
    }

    NBX_INFO("DriverUnload: Driver unloaded successfully\n");
}
