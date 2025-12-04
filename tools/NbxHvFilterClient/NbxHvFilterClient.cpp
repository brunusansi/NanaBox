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
 * @brief Send SET_PROFILE IOCTL to driver
 * 
 * @param hDevice Device handle
 * @param profileName Profile name
 * @param flags Profile flags
 * @return BOOL TRUE on success, FALSE on failure
 */
static BOOL SetProfile(HANDLE hDevice, const char* profileName, DWORD flags)
{
    NBX_SET_PROFILE_INPUT input = { 0 };
    DWORD bytesReturned = 0;

    // Copy profile name
    strncpy_s(input.ProfileName, sizeof(input.ProfileName), profileName, _TRUNCATE);
    input.Flags = flags;

    printf("[INFO] Sending SET_PROFILE request...\n");
    printf("       Profile: %s\n", profileName);
    printf("       Flags: 0x%08X\n", flags);
    printf("         - CPUID: %s\n", (flags & NBX_PROFILE_FLAG_CPUID) ? "Yes" : "No");
    printf("         - MSR Intercept: %s\n", (flags & NBX_PROFILE_FLAG_MSR_INTERCEPT) ? "Yes" : "No");
    printf("         - Timing: %s\n", (flags & NBX_PROFILE_FLAG_TIMING) ? "Yes" : "No");
    printf("         - PCI: %s\n", (flags & NBX_PROFILE_FLAG_PCI) ? "Yes" : "No");

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
    } else {
        printf("[ERROR] Failed to set profile (Error: %lu)\n", GetLastError());
    }

    return result;
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
        printf("[SUCCESS] Status retrieved successfully\n");
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
 * @brief Print usage information
 */
static void PrintUsage()
{
    printf("\n");
    printf("NanaBox HvFilter Client - User-mode helper for nanabox_hvfilter driver\n");
    printf("Copyright (c) NanaBox Anti-Detection Edition Contributors\n");
    printf("\n");
    printf("Usage:\n");
    printf("  NbxHvFilterClient.exe set <profile_name> <flags>\n");
    printf("  NbxHvFilterClient.exe status\n");
    printf("  NbxHvFilterClient.exe clear\n");
    printf("\n");
    printf("Commands:\n");
    printf("  set <profile> <flags>   Set active profile with flags\n");
    printf("  status                  Get current driver status\n");
    printf("  clear                   Clear active profile\n");
    printf("\n");
    printf("Flags (hexadecimal bitmask):\n");
    printf("  0x%08X  CPUID\n", NBX_PROFILE_FLAG_CPUID);
    printf("  0x%08X  MSR Intercept\n", NBX_PROFILE_FLAG_MSR_INTERCEPT);
    printf("  0x%08X  Timing\n", NBX_PROFILE_FLAG_TIMING);
    printf("  0x%08X  PCI\n", NBX_PROFILE_FLAG_PCI);
    printf("\n");
    printf("Examples:\n");
    printf("  NbxHvFilterClient.exe set Valorant 0x00000003\n");
    printf("  NbxHvFilterClient.exe set BareMetal 0x0000000F\n");
    printf("  NbxHvFilterClient.exe status\n");
    printf("  NbxHvFilterClient.exe clear\n");
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
        if (argc < 4) {
            printf("[ERROR] Missing arguments for 'set' command\n");
            PrintUsage();
            exitCode = 1;
        } else {
            const char* profileName = argv[2];
            DWORD flags = (DWORD)strtoul(argv[3], NULL, 0);
            
            if (!SetProfile(hDevice, profileName, flags)) {
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
