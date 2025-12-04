/*
 * PROJECT:    NanaBox Anti-Detection Edition
 * FILE:       drivers/nanabox_hvfilter/NbxHvFilterShared.h
 * PURPOSE:    Shared definitions for nanabox_hvfilter driver (User-mode and Kernel-mode)
 *
 * LICENSE:    The MIT License
 */

#ifndef NBX_HVFILTER_SHARED_H
#define NBX_HVFILTER_SHARED_H

#ifdef _KERNEL_MODE
#include <ntddk.h>
#else
#include <windows.h>
#endif

//
// Version information
//
#define NANABOX_HVFILTER_VERSION_MAJOR  1
#define NANABOX_HVFILTER_VERSION_MINOR  0
#define NANABOX_HVFILTER_VERSION_BUILD  0

//
// Device names
//
#define NANABOX_HVFILTER_DEVICE_NAME_W     L"\\Device\\NanaBoxHvFilter"
#define NANABOX_HVFILTER_SYMBOLIC_NAME_W   L"\\DosDevices\\NanaBoxHvFilter"
#define NANABOX_HVFILTER_USER_DEVICE_NAME  "\\\\.\\NanaBoxHvFilter"

//
// IOCTL codes
//
#define IOCTL_NBX_BASE                      0x8000

#define IOCTL_NBX_HVFILTER_SET_PROFILE \
    CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_NBX_BASE + 0, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_NBX_HVFILTER_GET_STATUS \
    CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_NBX_BASE + 1, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_NBX_HVFILTER_CLEAR_PROFILE \
    CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_NBX_BASE + 2, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// Maximum sizes
//
#define NBX_MAX_PROFILE_NAME_LENGTH     64
#define NBX_MAX_STRING_LENGTH           256
#define NBX_MAX_VENDOR_STRING_LENGTH    13  // "GenuineIntel" + null terminator
#define NBX_CPU_VENDOR_STRING_LENGTH    12  // Length without null terminator

//
// Known CPU vendor strings
//
#define NBX_VENDOR_INTEL                "GenuineIntel"
#define NBX_VENDOR_AMD                  "AuthenticAMD"

//
// Profile flags
//
#define NBX_PROFILE_FLAG_CPUID          0x00000001
#define NBX_PROFILE_FLAG_MSR_INTERCEPT  0x00000002
#define NBX_PROFILE_FLAG_TIMING         0x00000004
#define NBX_PROFILE_FLAG_PCI            0x00000008

//
// Status codes
//
#define NBX_STATUS_SUCCESS              0x00000000
#define NBX_STATUS_ERROR                0x00000001
#define NBX_STATUS_INVALID_PARAMETER    0x00000002
#define NBX_STATUS_BUFFER_TOO_SMALL     0x00000003
#define NBX_STATUS_NOT_SUPPORTED        0x00000004

//
// MSR interception modes
//
#define NBX_MSR_MODE_PASSTHROUGH        0
#define NBX_MSR_MODE_ZERO               1
#define NBX_MSR_MODE_BLOCK              2

#ifndef _KERNEL_MODE
#pragma pack(push, 1)
#endif

//
// CPUID policy structure (Phase 3B)
//
typedef struct _NBX_CPUID_POLICY {
#ifdef _KERNEL_MODE
    BOOLEAN Enabled;
    BOOLEAN HideHypervisor;
    BOOLEAN MaskVirtualizationFeatures;
    CHAR VendorString[NBX_MAX_VENDOR_STRING_LENGTH];
#else
    BOOL Enabled;
    BOOL HideHypervisor;
    BOOL MaskVirtualizationFeatures;
    CHAR VendorString[NBX_MAX_VENDOR_STRING_LENGTH];
#endif
} NBX_CPUID_POLICY, *PNBX_CPUID_POLICY;

//
// MSR policy structure (Phase 3B)
//
typedef struct _NBX_MSR_POLICY {
#ifdef _KERNEL_MODE
    BOOLEAN Enabled;
    ULONG HyperVMsrMode;  // NBX_MSR_MODE_*
#else
    BOOL Enabled;
    DWORD HyperVMsrMode;  // NBX_MSR_MODE_*
#endif
} NBX_MSR_POLICY, *PNBX_MSR_POLICY;

//
// SET_PROFILE input structure (extended for Phase 3B)
//
typedef struct _NBX_SET_PROFILE_INPUT {
#ifdef _KERNEL_MODE
    CHAR ProfileName[NBX_MAX_PROFILE_NAME_LENGTH];
    ULONG Flags;
    NBX_CPUID_POLICY CpuIdPolicy;
    NBX_MSR_POLICY MsrPolicy;
#else
    CHAR ProfileName[NBX_MAX_PROFILE_NAME_LENGTH];
    DWORD Flags;
    NBX_CPUID_POLICY CpuIdPolicy;
    NBX_MSR_POLICY MsrPolicy;
#endif
} NBX_SET_PROFILE_INPUT, *PNBX_SET_PROFILE_INPUT;

//
// GET_STATUS output structure (extended for Phase 3B)
//
typedef struct _NBX_GET_STATUS_OUTPUT {
#ifdef _KERNEL_MODE
    CHAR ActiveProfileName[NBX_MAX_PROFILE_NAME_LENGTH];
    ULONG ActiveFlags;
    ULONG DriverVersion;
    BOOLEAN IsActive;
    NBX_CPUID_POLICY CpuIdPolicy;
    NBX_MSR_POLICY MsrPolicy;
#else
    CHAR ActiveProfileName[NBX_MAX_PROFILE_NAME_LENGTH];
    DWORD ActiveFlags;
    DWORD DriverVersion;
    BOOL IsActive;
    NBX_CPUID_POLICY CpuIdPolicy;
    NBX_MSR_POLICY MsrPolicy;
#endif
} NBX_GET_STATUS_OUTPUT, *PNBX_GET_STATUS_OUTPUT;

#ifndef _KERNEL_MODE
#pragma pack(pop)
#endif

#endif // !NBX_HVFILTER_SHARED_H
