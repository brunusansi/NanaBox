/*
 * PROJECT:    NanaBox Anti-Detection Edition
 * FILE:       drivers/nanabox_hvfilter/driver.h
 * PURPOSE:    Main driver header file
 *
 * LICENSE:    The MIT License
 */

#ifndef NBX_DRIVER_H
#define NBX_DRIVER_H

#define _KERNEL_MODE
#include <ntddk.h>
#include <wdf.h>

#include "NbxHvFilterShared.h"

//
// Debug print macros
//
#define NBX_TAG 'xbNF'  // 'FNbx' in little-endian

#if DBG
#define NBX_PRINT(_level_, _format_, ...) \
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, _level_, "[NanaBoxHvFilter] " _format_, ##__VA_ARGS__)
#else
#define NBX_PRINT(_level_, _format_, ...) \
    KdPrintEx((DPFLTR_IHVDRIVER_ID, _level_, "[NanaBoxHvFilter] " _format_, ##__VA_ARGS__))
#endif

#define NBX_INFO(_format_, ...) \
    NBX_PRINT(DPFLTR_INFO_LEVEL, _format_, ##__VA_ARGS__)

#define NBX_ERROR(_format_, ...) \
    NBX_PRINT(DPFLTR_ERROR_LEVEL, _format_, ##__VA_ARGS__)

#define NBX_WARNING(_format_, ...) \
    NBX_PRINT(DPFLTR_WARNING_LEVEL, _format_, ##__VA_ARGS__)

//
// Driver context structure
//
typedef struct _NBX_DRIVER_CONTEXT {
    PDEVICE_OBJECT DeviceObject;
    UNICODE_STRING DeviceName;
    UNICODE_STRING SymbolicLinkName;
    BOOLEAN IsActive;
    CHAR ActiveProfileName[NBX_MAX_PROFILE_NAME_LENGTH];
    ULONG ActiveFlags;
} NBX_DRIVER_CONTEXT, *PNBX_DRIVER_CONTEXT;

//
// Global driver context
//
extern NBX_DRIVER_CONTEXT g_DriverContext;

//
// Function prototypes
//

// From driver.c
DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD DriverUnload;

// From device.c
NTSTATUS
NbxCreateDevice(
    _In_ PDRIVER_OBJECT DriverObject
);

VOID
NbxDestroyDevice(
    _In_ PDEVICE_OBJECT DeviceObject
);

// From dispatch.c
DRIVER_DISPATCH NbxDispatchCreate;
DRIVER_DISPATCH NbxDispatchClose;
DRIVER_DISPATCH NbxDispatchDeviceControl;

// From ioctl.c
NTSTATUS
NbxHandleIoctl(
    _In_ ULONG IoControlCode,
    _In_reads_bytes_opt_(InputBufferLength) PVOID InputBuffer,
    _In_ ULONG InputBufferLength,
    _Out_writes_bytes_opt_(OutputBufferLength) PVOID OutputBuffer,
    _In_ ULONG OutputBufferLength,
    _Out_ PULONG_PTR BytesReturned
);

#endif // !NBX_DRIVER_H
