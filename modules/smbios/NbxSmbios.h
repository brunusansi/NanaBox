/*
 * PROJECT:    NanaBox Anti-Detection Edition
 * FILE:       modules/smbios/NbxSmbios.h
 * PURPOSE:    SMBIOS injection and manipulation API (Phase 2)
 *
 * LICENSE:    The MIT License
 *
 * DESCRIPTION:
 *   This module provides host-side SMBIOS (System Management BIOS) injection
 *   capabilities for NanaBox VMs. It enables spoofing of hardware identification
 *   to achieve bare-metal-like detection resistance.
 *
 * IMPLEMENTATION APPROACH:
 *   - Primary: EFI shim/loader that rewrites SMBIOS tables before guest OS boots
 *   - Secondary: Hyper-V/HCS API where available (limited control)
 *   - Fallback: Registry and WMI manipulation (least reliable)
 *
 * ANTI-CHEAT TARGETS:
 *   - Easy Anti-Cheat (EAC, including Epic's EAC)
 *   - BattlEye
 *   - Riot Vanguard (Valorant)
 *   - FACEIT AC
 *   - Expert Anti-Cheat (Tencent Expert)
 *   - EA Javelin
 *   - Ricochet (CoD)
 */

#ifndef NANABOX_SMBIOS_H
#define NANABOX_SMBIOS_H

#include <Windows.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * SMBIOS structure types (DMTF SMBIOS Specification v3.5.0)
 */
#define NBX_SMBIOS_TYPE_BIOS_INFO           0
#define NBX_SMBIOS_TYPE_SYSTEM_INFO         1
#define NBX_SMBIOS_TYPE_BASEBOARD_INFO      2
#define NBX_SMBIOS_TYPE_CHASSIS_INFO        3
#define NBX_SMBIOS_TYPE_PROCESSOR_INFO      4
#define NBX_SMBIOS_TYPE_OEM_STRINGS         11

/*
 * SMBIOS Baseboard Configuration
 */
typedef struct _NBX_SMBIOS_BASEBOARD {
    LPCWSTR Manufacturer;
    LPCWSTR Product;
    LPCWSTR Version;
    LPCWSTR SerialNumber;
} NBX_SMBIOS_BASEBOARD, *PNBX_SMBIOS_BASEBOARD;

/*
 * SMBIOS Chassis Configuration
 */
typedef struct _NBX_SMBIOS_CHASSIS {
    LPCWSTR Type;           // e.g., "Desktop", "Laptop", "Tower"
    LPCWSTR SerialNumber;
} NBX_SMBIOS_CHASSIS, *PNBX_SMBIOS_CHASSIS;

/*
 * SMBIOS Profile Configuration
 * Comprehensive structure for SMBIOS spoofing
 */
typedef struct _NBX_SMBIOS_PROFILE {
    DWORD Version;                          // Structure version (currently 1)
    BOOL Enabled;                           // Enable SMBIOS spoofing
    
    // System Information (Type 1)
    LPCWSTR Vendor;                         // System manufacturer
    LPCWSTR Product;                        // System product name
    LPCWSTR Version;                        // System version
    LPCWSTR SerialNumber;                   // System serial number
    LPCWSTR SkuNumber;                      // SKU number
    LPCWSTR Family;                         // Product family
    GUID Uuid;                              // System UUID
    
    // Baseboard Information (Type 2)
    NBX_SMBIOS_BASEBOARD Baseboard;
    
    // Chassis Information (Type 3)
    NBX_SMBIOS_CHASSIS Chassis;
    
    // OEM Strings (Type 11)
    LPCWSTR* OemStrings;                    // Array of OEM strings
    DWORD OemStringCount;
    
    // Template name (e.g., "desktop-intel-2019")
    LPCWSTR Template;
    
    // Advanced options
    BOOL ReplaceAll;                        // Replace all SMBIOS tables (risky)
    BOOL PreserveSerial;                    // Keep original serial if non-virtual
} NBX_SMBIOS_PROFILE, *PNBX_SMBIOS_PROFILE;

/*
 * SMBIOS Injection Result
 */
typedef struct _NBX_SMBIOS_RESULT {
    HRESULT Status;                         // S_OK on success
    DWORD Method;                           // Injection method used (see flags)
    WCHAR ErrorMessage[256];                // Detailed error message
} NBX_SMBIOS_RESULT, *PNBX_SMBIOS_RESULT;

/*
 * SMBIOS Injection Methods
 */
#define NBX_SMBIOS_METHOD_NONE          0x00
#define NBX_SMBIOS_METHOD_EFI_SHIM      0x01    // EFI loader (best)
#define NBX_SMBIOS_METHOD_HCS_API       0x02    // Hyper-V Compute Service API
#define NBX_SMBIOS_METHOD_REGISTRY      0x04    // Registry manipulation (limited)
#define NBX_SMBIOS_METHOD_WMI           0x08    // WMI manipulation (guest-side)

/*
 * Core API Functions
 */

/**
 * Apply SMBIOS profile to a VM
 * 
 * @param VmId - VM GUID or name
 * @param Profile - SMBIOS profile configuration
 * @param Result - Result structure (optional, can be NULL)
 * @return HRESULT - S_OK on success, error code otherwise
 * 
 * NOTES:
 *   - VM must be stopped for EFI shim method
 *   - Some fields may be ignored if method doesn't support them
 *   - Check Result->Method to see which injection method was used
 */
HRESULT WINAPI NbxApplySmbiosProfile(
    _In_ LPCWSTR VmId,
    _In_ PNBX_SMBIOS_PROFILE Profile,
    _Out_opt_ PNBX_SMBIOS_RESULT Result
);

/**
 * Load SMBIOS profile from JSON template file
 * 
 * @param TemplatePath - Path to JSON template file
 * @param Profile - Output profile structure (caller must free with NbxFreeSmbiosProfile)
 * @return HRESULT - S_OK on success
 * 
 * EXAMPLE:
 *   NBX_SMBIOS_PROFILE* profile;
 *   NbxLoadSmbiosTemplate(L"profiles/smbios/desktop-intel-2022.json", &profile);
 *   NbxApplySmbiosProfile(L"MyVM", profile, NULL);
 *   NbxFreeSmbiosProfile(profile);
 */
HRESULT WINAPI NbxLoadSmbiosTemplate(
    _In_ LPCWSTR TemplatePath,
    _Outptr_ PNBX_SMBIOS_PROFILE* Profile
);

/**
 * Free SMBIOS profile allocated by NbxLoadSmbiosTemplate
 */
VOID WINAPI NbxFreeSmbiosProfile(
    _In_ PNBX_SMBIOS_PROFILE Profile
);

/**
 * Generate unique identifiers for SMBIOS profile
 * 
 * @param Profile - Profile to update with unique values
 * @param PreserveVendor - Keep vendor/product fields unchanged
 * @return HRESULT - S_OK on success
 * 
 * NOTES:
 *   - Generates unique UUID, serial numbers following vendor patterns
 *   - Ensures multi-account isolation by randomizing identifiers
 *   - Preserves manufacturer-specific serial number formats
 */
HRESULT WINAPI NbxGenerateUniqueSmbiosIds(
    _Inout_ PNBX_SMBIOS_PROFILE Profile,
    _In_ BOOL PreserveVendor
);

/**
 * Extract SMBIOS information from host system
 * 
 * @param Profile - Output profile with host's SMBIOS data
 * @return HRESULT - S_OK on success
 * 
 * NOTES:
 *   - Useful for creating realistic profiles based on physical hardware
 *   - Caller should randomize serial numbers before using in VM
 */
HRESULT WINAPI NbxExtractHostSmbios(
    _Outptr_ PNBX_SMBIOS_PROFILE* Profile
);

/**
 * Validate SMBIOS profile for consistency and realism
 * 
 * @param Profile - Profile to validate
 * @param ErrorMessage - Output buffer for validation errors
 * @param ErrorMessageSize - Size of error message buffer
 * @return BOOL - TRUE if valid, FALSE otherwise
 * 
 * CHECKS:
 *   - Manufacturer/product consistency
 *   - Serial number format validity
 *   - UUID format
 *   - Realistic value ranges
 */
BOOL WINAPI NbxValidateSmbiosProfile(
    _In_ PNBX_SMBIOS_PROFILE Profile,
    _Out_writes_(ErrorMessageSize) LPWSTR ErrorMessage,
    _In_ DWORD ErrorMessageSize
);

/**
 * Get available SMBIOS injection methods for current system
 * 
 * @return DWORD - Bitmask of available methods (NBX_SMBIOS_METHOD_*)
 * 
 * NOTES:
 *   - Check capabilities before applying profile
 *   - EFI shim requires Secure Boot disabled or signed loader
 */
DWORD WINAPI NbxGetSmbiosCapabilities(VOID);

/*
 * TODO (Phase 2 Implementation):
 * - Implement EFI shim loader for SMBIOS table injection
 * - Research Hyper-V HCS API for SMBIOS control (if any)
 * - Create SMBIOS binary builder for raw table generation
 * - Add per-VM persistence (store profile in VM config)
 * - Implement template validation and manufacturer detection
 * - Add support for custom SMBIOS structures (OEM-specific)
 * 
 * LIMITATIONS:
 * - Hyper-V has limited native SMBIOS control
 * - EFI approach requires VM restart to apply changes
 * - Some SMBIOS fields may be read-only at runtime
 * - Windows 11 TPM/Secure Boot requirements may complicate injection
 * 
 * SECURITY CONSIDERATIONS:
 * - SMBIOS injection modifies firmware-visible data
 * - May trigger Windows security warnings if Secure Boot enabled
 * - Should be clearly marked as experimental/advanced feature
 * - Users must understand implications (reduced security posture)
 */

#ifdef __cplusplus
}
#endif

#endif // !NANABOX_SMBIOS_H
