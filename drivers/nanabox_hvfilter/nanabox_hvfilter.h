/*
 * PROJECT:    NanaBox Anti-Detection Edition
 * FILE:       drivers/nanabox_hvfilter/nanabox_hvfilter.h
 * PURPOSE:    Unified Hypervisor Filter Driver for CPUID and MSR Control (Phase 3)
 *
 * LICENSE:    The MIT License
 */

#ifndef NANABOX_HVFILTER_H
#define NANABOX_HVFILTER_H

#include <ntddk.h>
#include <wdf.h>

#define NANABOX_HVFILTER_VERSION_MAJOR  1
#define NANABOX_HVFILTER_VERSION_MINOR  0
#define NANABOX_HVFILTER_VERSION_BUILD  0

#define NANABOX_HVFILTER_DEVICE_NAME     L"\\Device\\NanaBoxHvFilter"
#define NANABOX_HVFILTER_DOS_NAME        L"\\DosDevices\\NanaBoxHvFilter"

#define IOCTL_NBX_BASE                   0x8000
#define IOCTL_NBX_GET_VERSION            CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_NBX_BASE + 0, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_NBX_SET_CPUID_PROFILE      CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_NBX_BASE + 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_NBX_SET_MSR_PROFILE        CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_NBX_BASE + 2, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_NBX_GET_STATUS             CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_NBX_BASE + 4, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _NBX_CPUID_PROFILE {
    ULONG Version;
    BOOLEAN Enabled;
    BOOLEAN HideHypervisorBit;
    CHAR VendorString[13];
    CHAR BrandString[49];
    ULONG FeatureMaskEcx;
    ULONG FeatureMaskEdx;
} NBX_CPUID_PROFILE, *PNBX_CPUID_PROFILE;

typedef enum _NBX_MSR_MODE {
    NBX_MSR_MODE_PASS = 0,
    NBX_MSR_MODE_ZERO = 1,
    NBX_MSR_MODE_MIRROR = 2,
    NBX_MSR_MODE_FAKE = 3,
    NBX_MSR_MODE_BLOCK = 4,
} NBX_MSR_MODE;

typedef struct _NBX_MSR_RULE {
    ULONG Msr;
    NBX_MSR_MODE Mode;
    ULONG64 FakeValue;
} NBX_MSR_RULE, *PNBX_MSR_RULE;

typedef struct _NBX_MSR_PROFILE {
    ULONG Version;
    BOOLEAN Enabled;
    ULONG RuleCount;
    NBX_MSR_RULE Rules[64];
} NBX_MSR_PROFILE, *PNBX_MSR_PROFILE;

#endif // !NANABOX_HVFILTER_H
