/*
 * PROJECT:    NanaBox Anti-Detection Edition
 * FILE:       tools/NbxHvFilterClient/NbxHvFilterClient.cpp
 * PURPOSE:    User-mode helper for nanabox_hvfilter driver
 *
 * LICENSE:    The MIT License
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "../../drivers/nanabox_hvfilter/NbxHvFilterShared.h"

/**
 * @brief Open the driver device
 * 
 * @return HANDLE Device handle or INVALID_HANDLE_VALUE on failure
 */
static HANDLE OpenDriver()
{
    HANDLE hDevice = CreateFileA(
        NANABOX_HVFILTER_USER_DEVICE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hDevice == INVALID_HANDLE_VALUE) {
        printf("[ERROR] Failed to open driver device: %s (Error: %lu)\n",
               NANABOX_HVFILTER_USER_DEVICE_NAME,
               GetLastError());
        printf("[INFO] Make sure the driver is installed and loaded.\n");
    }

    return hDevice;
}

/**
 * @brief Send SET_PROFILE IOCTL to driver (Phase 3B - with CPUID/MSR policies)
 * 
 * @param hDevice Device handle
 * @param profileName Profile name
 * @param flags Profile flags
 * @param cpuidPolicy Optional CPUID policy (can be NULL)
 * @param msrPolicy Optional MSR policy (can be NULL)
 * @return BOOL TRUE on success, FALSE on failure
 */
static BOOL SetProfileWithPolicies(
    HANDLE hDevice,
    const char* profileName,
    DWORD flags,
    const NBX_CPUID_POLICY* cpuidPolicy,
    const NBX_MSR_POLICY* msrPolicy)
{
    NBX_SET_PROFILE_INPUT input = { 0 };
    DWORD bytesReturned = 0;

    // Copy profile name
    strncpy_s(input.ProfileName, sizeof(input.ProfileName), profileName, _TRUNCATE);
    input.Flags = flags;

    // Copy CPUID policy if provided
    if (cpuidPolicy != NULL) {
        memcpy(&input.CpuIdPolicy, cpuidPolicy, sizeof(NBX_CPUID_POLICY));
    } else {
        // Set default CPUID policy based on flags
        input.CpuIdPolicy.Enabled = (flags & NBX_PROFILE_FLAG_CPUID) ? TRUE : FALSE;
        input.CpuIdPolicy.HideHypervisor = input.CpuIdPolicy.Enabled;
        input.CpuIdPolicy.MaskVirtualizationFeatures = input.CpuIdPolicy.Enabled;
        input.CpuIdPolicy.VendorString[0] = '\0';  // Empty means use host vendor
    }

    // Copy MSR policy if provided
    if (msrPolicy != NULL) {
        memcpy(&input.MsrPolicy, msrPolicy, sizeof(NBX_MSR_POLICY));
    } else {
        // Set default MSR policy based on flags
        input.MsrPolicy.Enabled = (flags & NBX_PROFILE_FLAG_MSR_INTERCEPT) ? TRUE : FALSE;
        input.MsrPolicy.HyperVMsrMode = NBX_MSR_MODE_ZERO;  // Default to zero mode
    }

    printf("[INFO] Sending SET_PROFILE request (Phase 3B)...\n");
    printf("       Profile: %s\n", profileName);
    printf("       Flags: 0x%08X\n", flags);
    printf("         - CPUID: %s\n", (flags & NBX_PROFILE_FLAG_CPUID) ? "Yes" : "No");
    printf("         - MSR Intercept: %s\n", (flags & NBX_PROFILE_FLAG_MSR_INTERCEPT) ? "Yes" : "No");
    printf("         - Timing: %s\n", (flags & NBX_PROFILE_FLAG_TIMING) ? "Yes" : "No");
    printf("         - PCI: %s\n", (flags & NBX_PROFILE_FLAG_PCI) ? "Yes" : "No");

    // Show CPUID policy details
    if (input.CpuIdPolicy.Enabled) {
        printf("       CPUID Policy:\n");
        printf("         - Hide Hypervisor: %s\n", input.CpuIdPolicy.HideHypervisor ? "Yes" : "No");
        printf("         - Mask Virtualization: %s\n", input.CpuIdPolicy.MaskVirtualizationFeatures ? "Yes" : "No");
        if (input.CpuIdPolicy.VendorString[0] != '\0') {
            printf("         - Vendor String: '%s'\n", input.CpuIdPolicy.VendorString);
        } else {
            printf("         - Vendor String: (use host)\n");
        }
    }

    // Show MSR policy details
    if (input.MsrPolicy.Enabled) {
        printf("       MSR Policy:\n");
        printf("         - Hyper-V MSR Mode: ");
        switch (input.MsrPolicy.HyperVMsrMode) {
            case NBX_MSR_MODE_PASSTHROUGH:
                printf("PASSTHROUGH\n");
                break;
            case NBX_MSR_MODE_ZERO:
                printf("ZERO\n");
                break;
            case NBX_MSR_MODE_BLOCK:
                printf("BLOCK\n");
                break;
            default:
                printf("UNKNOWN\n");
                break;
        }
    }

    BOOL result = DeviceIoControl(
        hDevice,
        IOCTL_NBX_HVFILTER_SET_PROFILE,
        &input,
        sizeof(input),
        NULL,
        0,
        &bytesReturned,
        NULL
    );

    if (result) {
        printf("[SUCCESS] Profile set successfully\n");
        printf("[INFO] Check driver logs (DebugView) for detailed configuration\n");
    } else {
        printf("[ERROR] Failed to set profile (Error: %lu)\n", GetLastError());
    }

    return result;
}

/**
 * @brief Send SET_PROFILE IOCTL to driver (simple wrapper for backward compatibility)
 * 
 * @param hDevice Device handle
 * @param profileName Profile name
 * @param flags Profile flags
 * @return BOOL TRUE on success, FALSE on failure
 */
static BOOL SetProfile(HANDLE hDevice, const char* profileName, DWORD flags)
{
    return SetProfileWithPolicies(hDevice, profileName, flags, NULL, NULL);
}

/**
 * @brief Send GET_STATUS IOCTL to driver
 * 
 * @param hDevice Device handle
 * @return BOOL TRUE on success, FALSE on failure
 */
static BOOL GetStatus(HANDLE hDevice)
{
    NBX_GET_STATUS_OUTPUT output = { 0 };
    DWORD bytesReturned = 0;

    printf("[INFO] Sending GET_STATUS request...\n");

    BOOL result = DeviceIoControl(
        hDevice,
        IOCTL_NBX_HVFILTER_GET_STATUS,
        NULL,
        0,
        &output,
        sizeof(output),
        &bytesReturned,
        NULL
    );

    if (result) {
        printf("[SUCCESS] Status retrieved successfully (Phase 3B)\n");
        printf("       Driver Version: %u.%u.%u\n",
               (output.DriverVersion >> 16) & 0xFF,
               (output.DriverVersion >> 8) & 0xFF,
               output.DriverVersion & 0xFF);
        printf("       Active Profile: %s\n", output.ActiveProfileName);
        printf("       Active Flags: 0x%08X\n", output.ActiveFlags);
        printf("         - CPUID: %s\n", (output.ActiveFlags & NBX_PROFILE_FLAG_CPUID) ? "Yes" : "No");
        printf("         - MSR Intercept: %s\n", (output.ActiveFlags & NBX_PROFILE_FLAG_MSR_INTERCEPT) ? "Yes" : "No");
        printf("         - Timing: %s\n", (output.ActiveFlags & NBX_PROFILE_FLAG_TIMING) ? "Yes" : "No");
        printf("         - PCI: %s\n", (output.ActiveFlags & NBX_PROFILE_FLAG_PCI) ? "Yes" : "No");
        printf("       Is Active: %s\n", output.IsActive ? "Yes" : "No");

        // Display CPUID policy (Phase 3B)
        if (output.CpuIdPolicy.Enabled) {
            printf("       CPUID Policy:\n");
            printf("         - Enabled: Yes\n");
            printf("         - Hide Hypervisor: %s\n", output.CpuIdPolicy.HideHypervisor ? "Yes" : "No");
            printf("         - Mask Virtualization: %s\n", output.CpuIdPolicy.MaskVirtualizationFeatures ? "Yes" : "No");
            if (output.CpuIdPolicy.VendorString[0] != '\0') {
                printf("         - Vendor String: '%s'\n", output.CpuIdPolicy.VendorString);
            } else {
                printf("         - Vendor String: (use host)\n");
            }
        } else {
            printf("       CPUID Policy: Disabled\n");
        }

        // Display MSR policy (Phase 3B)
        if (output.MsrPolicy.Enabled) {
            printf("       MSR Policy:\n");
            printf("         - Enabled: Yes\n");
            printf("         - Hyper-V MSR Mode: ");
            switch (output.MsrPolicy.HyperVMsrMode) {
                case NBX_MSR_MODE_PASSTHROUGH:
                    printf("PASSTHROUGH (allow normal access)\n");
                    break;
                case NBX_MSR_MODE_ZERO:
                    printf("ZERO (return 0)\n");
                    break;
                case NBX_MSR_MODE_BLOCK:
                    printf("BLOCK (return error)\n");
                    break;
                default:
                    printf("UNKNOWN (%u)\n", output.MsrPolicy.HyperVMsrMode);
                    break;
            }
        } else {
            printf("       MSR Policy: Disabled\n");
        }

        printf("\n");
        printf("[INFO] Phase 3B: Configuration framework ready\n");
        printf("[INFO] Actual CPU/MSR interception requires Phase 3C implementation\n");
        printf("[INFO] Check driver logs (DebugView) for detailed policy application\n");
    } else {
        printf("[ERROR] Failed to get status (Error: %lu)\n", GetLastError());
    }

    return result;
}

/**
 * @brief Send CLEAR_PROFILE IOCTL to driver
 * 
 * @param hDevice Device handle
 * @return BOOL TRUE on success, FALSE on failure
 */
static BOOL ClearProfile(HANDLE hDevice)
{
    DWORD bytesReturned = 0;

    printf("[INFO] Sending CLEAR_PROFILE request...\n");

    BOOL result = DeviceIoControl(
        hDevice,
        IOCTL_NBX_HVFILTER_CLEAR_PROFILE,
        NULL,
        0,
        NULL,
        0,
        &bytesReturned,
        NULL
    );

    if (result) {
        printf("[SUCCESS] Profile cleared successfully\n");
    } else {
        printf("[ERROR] Failed to clear profile (Error: %lu)\n", GetLastError());
    }

    return result;
}

/**
 * @brief Apply predefined profile (roblox, valorant, expert-tencent)
 * 
 * @param hDevice Device handle
 * @param profileName Profile name
 * @return BOOL TRUE on success, FALSE on failure
 */
static BOOL SetPredefinedProfile(HANDLE hDevice, const char* profileName)
{
    NBX_CPUID_POLICY cpuidPolicy = { 0 };
    NBX_MSR_POLICY msrPolicy = { 0 };
    DWORD flags = 0;

    if (_stricmp(profileName, "roblox") == 0) {
        // Roblox (Byfron) profile - balanced anti-detection
        flags = NBX_PROFILE_FLAG_CPUID | NBX_PROFILE_FLAG_MSR_INTERCEPT;
        
        cpuidPolicy.Enabled = TRUE;
        cpuidPolicy.HideHypervisor = TRUE;
        cpuidPolicy.MaskVirtualizationFeatures = TRUE;
        strncpy_s(cpuidPolicy.VendorString, sizeof(cpuidPolicy.VendorString), "AuthenticAMD", _TRUNCATE);

        msrPolicy.Enabled = TRUE;
        msrPolicy.HyperVMsrMode = NBX_MSR_MODE_ZERO;

        printf("[INFO] Using Roblox (Byfron) profile - balanced anti-detection\n");
    }
    else if (_stricmp(profileName, "valorant") == 0) {
        // Valorant (Riot Vanguard) profile - bare-metal anti-detection
        flags = NBX_PROFILE_FLAG_CPUID | NBX_PROFILE_FLAG_MSR_INTERCEPT;
        
        cpuidPolicy.Enabled = TRUE;
        cpuidPolicy.HideHypervisor = TRUE;
        cpuidPolicy.MaskVirtualizationFeatures = TRUE;
        strncpy_s(cpuidPolicy.VendorString, sizeof(cpuidPolicy.VendorString), "GenuineIntel", _TRUNCATE);

        msrPolicy.Enabled = TRUE;
        msrPolicy.HyperVMsrMode = NBX_MSR_MODE_ZERO;

        printf("[INFO] Using Valorant (Riot Vanguard) profile - bare-metal anti-detection\n");
    }
    else if (_stricmp(profileName, "expert-tencent") == 0 || _stricmp(profileName, "tencent-ace") == 0) {
        // Tencent ACE profile - bare-metal anti-detection
        flags = NBX_PROFILE_FLAG_CPUID | NBX_PROFILE_FLAG_MSR_INTERCEPT;
        
        cpuidPolicy.Enabled = TRUE;
        cpuidPolicy.HideHypervisor = TRUE;
        cpuidPolicy.MaskVirtualizationFeatures = TRUE;
        strncpy_s(cpuidPolicy.VendorString, sizeof(cpuidPolicy.VendorString), "GenuineIntel", _TRUNCATE);

        msrPolicy.Enabled = TRUE;
        msrPolicy.HyperVMsrMode = NBX_MSR_MODE_ZERO;

        printf("[INFO] Using Tencent ACE profile - bare-metal anti-detection\n");
    }
    else {
        printf("[ERROR] Unknown predefined profile: %s\n", profileName);
        printf("[INFO] Available profiles: roblox, valorant, expert-tencent\n");
        return FALSE;
    }

    return SetProfileWithPolicies(hDevice, profileName, flags, &cpuidPolicy, &msrPolicy);
}

/**
 * @brief Print usage information
 */
static void PrintUsage()
{
    printf("\n");
    printf("NanaBox HvFilter Client - User-mode helper for nanabox_hvfilter driver\n");
    printf("Copyright (c) NanaBox Anti-Detection Edition Contributors\n");
    printf("Phase 3B: CPUID and MSR configuration framework\n");
    printf("\n");
    printf("Usage:\n");
    printf("  NbxHvFilterClient.exe set <profile_name> [<flags>]\n");
    printf("  NbxHvFilterClient.exe status\n");
    printf("  NbxHvFilterClient.exe clear\n");
    printf("\n");
    printf("Commands:\n");
    printf("  set <profile> [flags]   Set active profile (with optional flags)\n");
    printf("  status                  Get current driver status and policies\n");
    printf("  clear                   Clear active profile\n");
    printf("\n");
    printf("Predefined Profiles (Phase 3B):\n");
    printf("  roblox                  Roblox (Byfron) - AMD CPU, balanced\n");
    printf("  valorant                Valorant (Vanguard) - Intel CPU, bare-metal\n");
    printf("  expert-tencent          Tencent ACE - Intel CPU, bare-metal\n");
    printf("\n");
    printf("Custom Flags (hexadecimal bitmask):\n");
    printf("  0x%08X  CPUID\n", NBX_PROFILE_FLAG_CPUID);
    printf("  0x%08X  MSR Intercept\n", NBX_PROFILE_FLAG_MSR_INTERCEPT);
    printf("  0x%08X  Timing\n", NBX_PROFILE_FLAG_TIMING);
    printf("  0x%08X  PCI\n", NBX_PROFILE_FLAG_PCI);
    printf("\n");
    printf("Examples:\n");
    printf("  NbxHvFilterClient.exe set roblox\n");
    printf("  NbxHvFilterClient.exe set valorant\n");
    printf("  NbxHvFilterClient.exe set expert-tencent\n");
    printf("  NbxHvFilterClient.exe set CustomProfile 0x00000003\n");
    printf("  NbxHvFilterClient.exe status\n");
    printf("  NbxHvFilterClient.exe clear\n");
    printf("\n");
    printf("Note: Phase 3B provides configuration framework only.\n");
    printf("      Actual CPU/MSR interception requires Phase 3C implementation.\n");
    printf("\n");
}

/**
 * @brief Main entry point
 */
int main(int argc, char* argv[])
{
    HANDLE hDevice;
    int exitCode = 0;

    printf("===============================================================\n");
    printf("NanaBox HvFilter Client v%d.%d.%d\n",
           NANABOX_HVFILTER_VERSION_MAJOR,
           NANABOX_HVFILTER_VERSION_MINOR,
           NANABOX_HVFILTER_VERSION_BUILD);
    printf("===============================================================\n\n");

    if (argc < 2) {
        PrintUsage();
        return 1;
    }

    // Open driver device
    hDevice = OpenDriver();
    if (hDevice == INVALID_HANDLE_VALUE) {
        return 1;
    }

    // Process command
    if (_stricmp(argv[1], "set") == 0) {
        if (argc < 3) {
            printf("[ERROR] Missing profile name for 'set' command\n");
            PrintUsage();
            exitCode = 1;
        } else {
            const char* profileName = argv[2];
            
            // Check if this is a predefined profile
            if (_stricmp(profileName, "roblox") == 0 ||
                _stricmp(profileName, "valorant") == 0 ||
                _stricmp(profileName, "expert-tencent") == 0 ||
                _stricmp(profileName, "tencent-ace") == 0) {
                // Use predefined profile with CPUID/MSR policies
                if (!SetPredefinedProfile(hDevice, profileName)) {
                    exitCode = 1;
                }
            } else if (argc >= 4) {
                // Custom profile with flags
                DWORD flags = (DWORD)strtoul(argv[3], NULL, 0);
                if (!SetProfile(hDevice, profileName, flags)) {
                    exitCode = 1;
                }
            } else {
                printf("[ERROR] Custom profiles require flags argument\n");
                printf("[INFO] For predefined profiles, use: roblox, valorant, expert-tencent\n");
                PrintUsage();
                exitCode = 1;
            }
        }
    }
    else if (_stricmp(argv[1], "status") == 0) {
        if (!GetStatus(hDevice)) {
            exitCode = 1;
        }
    }
    else if (_stricmp(argv[1], "clear") == 0) {
        if (!ClearProfile(hDevice)) {
            exitCode = 1;
        }
    }
    else {
        printf("[ERROR] Unknown command: %s\n", argv[1]);
        PrintUsage();
        exitCode = 1;
    }

    // Close driver device
    CloseHandle(hDevice);

    printf("\n");
    return exitCode;
}
