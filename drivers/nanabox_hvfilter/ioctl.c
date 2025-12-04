/*
 * PROJECT:    NanaBox Anti-Detection Edition
 * FILE:       drivers/nanabox_hvfilter/ioctl.c
 * PURPOSE:    IOCTL handler implementation
 *
 * LICENSE:    The MIT License
 */

#include "driver.h"

/**
 * @brief Handle SET_PROFILE IOCTL
 * 
 * @param InputBuffer Pointer to input buffer
 * @param InputBufferLength Size of input buffer
 * @return NTSTATUS Status code
 */
static
NTSTATUS
NbxHandleSetProfile(
    _In_reads_bytes_(InputBufferLength) PVOID InputBuffer,
    _In_ ULONG InputBufferLength
)
{
    PNBX_SET_PROFILE_INPUT profileInput;

    //
    // Validate input buffer size
    //
    if (InputBufferLength < sizeof(NBX_SET_PROFILE_INPUT)) {
        NBX_ERROR("NbxHandleSetProfile: Buffer too small, expected=%u, got=%u\n",
                  sizeof(NBX_SET_PROFILE_INPUT), InputBufferLength);
        return STATUS_BUFFER_TOO_SMALL;
    }

    if (InputBuffer == NULL) {
        NBX_ERROR("NbxHandleSetProfile: Input buffer is NULL\n");
        return STATUS_INVALID_PARAMETER;
    }

    profileInput = (PNBX_SET_PROFILE_INPUT)InputBuffer;

    //
    // Ensure profile name is null-terminated safely
    // This guarantees a null terminator exists even if the input was full
    //
    profileInput->ProfileName[NBX_MAX_PROFILE_NAME_LENGTH - 1] = '\0';
    
    //
    // Ensure vendor string is null-terminated safely
    //
    profileInput->CpuIdPolicy.VendorString[NBX_MAX_VENDOR_STRING_LENGTH - 1] = '\0';

    NBX_INFO("NbxHandleSetProfile: ProfileName='%s', Flags=0x%08X\n",
             profileInput->ProfileName, profileInput->Flags);

    //
    // Deactivate any existing interception first
    //
    if (g_DriverContext.CpuIdActive) {
        NBX_INFO("NbxHandleSetProfile: Deactivating existing CPUID interception\n");
        NbxDeactivateCpuIdInterception();
        g_DriverContext.CpuIdActive = FALSE;
    }
    if (g_DriverContext.MsrActive) {
        NBX_INFO("NbxHandleSetProfile: Deactivating existing MSR interception\n");
        NbxDeactivateMsrInterception();
        g_DriverContext.MsrActive = FALSE;
    }

    //
    // Store profile information in global context
    //
    RtlZeroMemory(g_DriverContext.ActiveProfileName, sizeof(g_DriverContext.ActiveProfileName));
    RtlCopyMemory(
        g_DriverContext.ActiveProfileName,
        profileInput->ProfileName,
        NBX_MAX_PROFILE_NAME_LENGTH - 1
    );
    g_DriverContext.ActiveProfileName[NBX_MAX_PROFILE_NAME_LENGTH - 1] = '\0';
    
    g_DriverContext.ActiveFlags = profileInput->Flags;
    g_DriverContext.IsActive = TRUE;

    //
    // Store CPUID and MSR policies
    //
    RtlCopyMemory(&g_DriverContext.CpuIdPolicy, &profileInput->CpuIdPolicy, sizeof(NBX_CPUID_POLICY));
    RtlCopyMemory(&g_DriverContext.MsrPolicy, &profileInput->MsrPolicy, sizeof(NBX_MSR_POLICY));

    //
    // Activate CPUID interception if enabled
    //
    if (profileInput->CpuIdPolicy.Enabled) {
        NTSTATUS status;
        NBX_INFO("NbxHandleSetProfile: Activating CPUID interception\n");
        NBX_INFO("  - HideHypervisor: %s\n", profileInput->CpuIdPolicy.HideHypervisor ? "Yes" : "No");
        NBX_INFO("  - MaskVirtualization: %s\n", profileInput->CpuIdPolicy.MaskVirtualizationFeatures ? "Yes" : "No");
        NBX_INFO("  - VendorString: '%s'\n", profileInput->CpuIdPolicy.VendorString);

        status = NbxActivateCpuIdInterception(&profileInput->CpuIdPolicy);
        if (!NT_SUCCESS(status)) {
            NBX_WARNING("NbxHandleSetProfile: Failed to activate CPUID interception, status=0x%08X (continuing)\n", status);
        } else {
            g_DriverContext.CpuIdActive = TRUE;
            NBX_INFO("NbxHandleSetProfile: CPUID interception activated successfully\n");
        }
    }

    //
    // Activate MSR interception if enabled
    //
    if (profileInput->MsrPolicy.Enabled) {
        NTSTATUS status;
        NBX_INFO("NbxHandleSetProfile: Activating MSR interception\n");
        NBX_INFO("  - HyperVMsrMode: %u\n", profileInput->MsrPolicy.HyperVMsrMode);

        status = NbxActivateMsrInterception(&profileInput->MsrPolicy);
        if (!NT_SUCCESS(status)) {
            NBX_WARNING("NbxHandleSetProfile: Failed to activate MSR interception, status=0x%08X (continuing)\n", status);
        } else {
            g_DriverContext.MsrActive = TRUE;
            NBX_INFO("NbxHandleSetProfile: MSR interception activated successfully\n");
        }
    }

    NBX_INFO("NbxHandleSetProfile: Profile loaded successfully\n");

    return STATUS_SUCCESS;
}

/**
 * @brief Handle GET_STATUS IOCTL
 * 
 * @param OutputBuffer Pointer to output buffer
 * @param OutputBufferLength Size of output buffer
 * @param BytesReturned Pointer to receive bytes written
 * @return NTSTATUS Status code
 */
static
NTSTATUS
NbxHandleGetStatus(
    _Out_writes_bytes_(OutputBufferLength) PVOID OutputBuffer,
    _In_ ULONG OutputBufferLength,
    _Out_ PULONG_PTR BytesReturned
)
{
    PNBX_GET_STATUS_OUTPUT statusOutput;

    //
    // Validate output buffer size
    //
    if (OutputBufferLength < sizeof(NBX_GET_STATUS_OUTPUT)) {
        NBX_ERROR("NbxHandleGetStatus: Buffer too small, expected=%u, got=%u\n",
                  sizeof(NBX_GET_STATUS_OUTPUT), OutputBufferLength);
        return STATUS_BUFFER_TOO_SMALL;
    }

    if (OutputBuffer == NULL) {
        NBX_ERROR("NbxHandleGetStatus: Output buffer is NULL\n");
        return STATUS_INVALID_PARAMETER;
    }

    statusOutput = (PNBX_GET_STATUS_OUTPUT)OutputBuffer;

    //
    // Zero the output buffer first
    //
    RtlZeroMemory(statusOutput, sizeof(NBX_GET_STATUS_OUTPUT));

    //
    // Fill in status information
    //
    RtlCopyMemory(
        statusOutput->ActiveProfileName,
        g_DriverContext.ActiveProfileName,
        NBX_MAX_PROFILE_NAME_LENGTH
    );
    statusOutput->ActiveFlags = g_DriverContext.ActiveFlags;
    statusOutput->DriverVersion = (NANABOX_HVFILTER_VERSION_MAJOR << 16) |
                                  (NANABOX_HVFILTER_VERSION_MINOR << 8) |
                                  NANABOX_HVFILTER_VERSION_BUILD;
    statusOutput->IsActive = g_DriverContext.IsActive;

    //
    // Include CPUID and MSR policy information
    //
    RtlCopyMemory(&statusOutput->CpuIdPolicy, &g_DriverContext.CpuIdPolicy, sizeof(NBX_CPUID_POLICY));
    RtlCopyMemory(&statusOutput->MsrPolicy, &g_DriverContext.MsrPolicy, sizeof(NBX_MSR_POLICY));

    *BytesReturned = sizeof(NBX_GET_STATUS_OUTPUT);

    NBX_INFO("NbxHandleGetStatus: ProfileName='%s', Flags=0x%08X, Version=0x%08X, Active=%d\n",
             statusOutput->ActiveProfileName,
             statusOutput->ActiveFlags,
             statusOutput->DriverVersion,
             statusOutput->IsActive);
    NBX_INFO("  - CPUID Active: %s, MSR Active: %s\n",
             g_DriverContext.CpuIdActive ? "Yes" : "No",
             g_DriverContext.MsrActive ? "Yes" : "No");

    return STATUS_SUCCESS;
}

/**
 * @brief Handle CLEAR_PROFILE IOCTL
 * 
 * @return NTSTATUS Status code
 */
static
NTSTATUS
NbxHandleClearProfile(
    VOID
)
{
    NBX_INFO("NbxHandleClearProfile: Clearing active profile\n");

    //
    // Deactivate CPUID interception if active
    //
    if (g_DriverContext.CpuIdActive) {
        NBX_INFO("NbxHandleClearProfile: Deactivating CPUID interception\n");
        NbxDeactivateCpuIdInterception();
        g_DriverContext.CpuIdActive = FALSE;
    }

    //
    // Deactivate MSR interception if active
    //
    if (g_DriverContext.MsrActive) {
        NBX_INFO("NbxHandleClearProfile: Deactivating MSR interception\n");
        NbxDeactivateMsrInterception();
        g_DriverContext.MsrActive = FALSE;
    }

    //
    // Clear profile information
    //
    RtlZeroMemory(g_DriverContext.ActiveProfileName, sizeof(g_DriverContext.ActiveProfileName));
    RtlCopyMemory(g_DriverContext.ActiveProfileName, "None", sizeof("None"));
    g_DriverContext.ActiveFlags = 0;
    g_DriverContext.IsActive = FALSE;

    //
    // Clear policy structures
    //
    RtlZeroMemory(&g_DriverContext.CpuIdPolicy, sizeof(NBX_CPUID_POLICY));
    RtlZeroMemory(&g_DriverContext.MsrPolicy, sizeof(NBX_MSR_POLICY));

    NBX_INFO("NbxHandleClearProfile: Profile cleared successfully\n");

    return STATUS_SUCCESS;
}

/**
 * @brief Main IOCTL handler
 * 
 * @param IoControlCode IOCTL code
 * @param InputBuffer Pointer to input buffer
 * @param InputBufferLength Size of input buffer
 * @param OutputBuffer Pointer to output buffer
 * @param OutputBufferLength Size of output buffer
 * @param BytesReturned Pointer to receive bytes written
 * @return NTSTATUS Status code
 */
NTSTATUS
NbxHandleIoctl(
    _In_ ULONG IoControlCode,
    _In_reads_bytes_opt_(InputBufferLength) PVOID InputBuffer,
    _In_ ULONG InputBufferLength,
    _Out_writes_bytes_opt_(OutputBufferLength) PVOID OutputBuffer,
    _In_ ULONG OutputBufferLength,
    _Out_ PULONG_PTR BytesReturned
)
{
    NTSTATUS status;

    *BytesReturned = 0;

    switch (IoControlCode) {
        case IOCTL_NBX_HVFILTER_SET_PROFILE:
            status = NbxHandleSetProfile(InputBuffer, InputBufferLength);
            break;

        case IOCTL_NBX_HVFILTER_GET_STATUS:
            status = NbxHandleGetStatus(OutputBuffer, OutputBufferLength, BytesReturned);
            break;

        case IOCTL_NBX_HVFILTER_CLEAR_PROFILE:
            status = NbxHandleClearProfile();
            break;

        default:
            NBX_WARNING("NbxHandleIoctl: Unknown IOCTL code 0x%08X\n", IoControlCode);
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }

    return status;
}
