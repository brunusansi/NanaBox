/*
 * PROJECT:    NanaBox Anti-Detection Edition
 * FILE:       modules/acpi/NbxAcpi.h
 * PURPOSE:    ACPI table injection and manipulation API (Phase 2)
 *
 * LICENSE:    The MIT License
 *
 * DESCRIPTION:
 *   This module provides host-side ACPI (Advanced Configuration and Power
 *   Interface) table manipulation for NanaBox VMs. It enables hiding or
 *   modifying ACPI devices and signatures to avoid virtualization detection.
 *
 * IMPLEMENTATION APPROACH:
 *   - Primary: EFI helper disk that injects custom ACPI tables at boot
 *   - Secondary: ACPI table override via Windows Boot Configuration Data (BCD)
 *   - Note: Runtime ACPI modification is extremely limited
 *
 * DETECTION VECTORS MITIGATED:
 *   - Hyper-V ACPI device signatures (VMBus, VMGEN)
 *   - Virtual battery indicators
 *   - Virtual docking station indicators
 *   - Synthetic ACPI methods and devices
 *
 * ANTI-CHEAT TARGETS:
 *   - Easy Anti-Cheat (EAC) - checks ACPI namespace
 *   - BattlEye - scans for virtual ACPI devices
 *   - Riot Vanguard (Valorant) - comprehensive ACPI checks
 *   - FACEIT AC
 *   - Expert Anti-Cheat (Tencent Expert)
 */

#ifndef NANABOX_ACPI_H
#define NANABOX_ACPI_H

#include <Windows.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ACPI Table Signatures (4-byte magic values)
 */
#define NBX_ACPI_SIG_DSDT  0x54445344    // "DSDT" - Differentiated System Description Table
#define NBX_ACPI_SIG_SSDT  0x54445353    // "SSDT" - Secondary System Description Table
#define NBX_ACPI_SIG_MADT  0x43495041    // "APIC" - Multiple APIC Description Table
#define NBX_ACPI_SIG_FADT  0x50434146    // "FACP" - Fixed ACPI Description Table

/*
 * Hyper-V ACPI Device IDs to Remove/Spoof
 */
#define NBX_HYPERV_VMBUS_DEVICE_ID      L"VMBUS"
#define NBX_HYPERV_VMGEN_DEVICE_ID      L"VMGENCOUNTER"
#define NBX_HYPERV_VMPCI_DEVICE_ID      L"VMPCI"
#define NBX_HYPERV_SYNTHVID_DEVICE_ID   L"SynthVid"

/*
 * ACPI Configuration
 */
typedef struct _NBX_ACPI_PROFILE {
    DWORD Version;                          // Structure version (currently 1)
    BOOL Enabled;                           // Enable ACPI manipulation
    
    // OEM identification
    CHAR OemId[7];                          // 6 chars + null terminator (e.g., "INTEL ")
    CHAR OemTableId[9];                     // 8 chars + null terminator
    
    // Table overrides
    LPCWSTR* OverrideTables;                // Paths to custom DSDT/SSDT blobs (.aml files)
    DWORD OverrideTableCount;
    
    // Anti-virtualization options
    BOOL FixHyperVSignatures;               // Remove/modify Hyper-V ACPI signatures
    BOOL SpoofBattery;                      // Add fake battery device (laptops only)
    BOOL SpoofDock;                         // Add fake docking station
    BOOL RemoveVmBus;                       // Remove VMBus device from ACPI
    BOOL RemoveVmGen;                       // Remove VM Generation Counter
    BOOL RemoveSynthetic;                   // Remove all synthetic Hyper-V devices
    
    // Advanced options
    BOOL PreserveTpm;                       // Keep TPM device (required for Windows 11)
    BOOL CustomDsdt;                        // Use custom DSDT instead of modifications
} NBX_ACPI_PROFILE, *PNBX_ACPI_PROFILE;

/*
 * ACPI Injection Result
 */
typedef struct _NBX_ACPI_RESULT {
    HRESULT Status;
    DWORD Method;                           // Injection method used
    DWORD TablesModified;                   // Number of tables modified
    DWORD DevicesRemoved;                   // Number of Hyper-V devices removed
    WCHAR ErrorMessage[256];
} NBX_ACPI_RESULT, *PNBX_ACPI_RESULT;

/*
 * ACPI Injection Methods
 */
#define NBX_ACPI_METHOD_NONE            0x00
#define NBX_ACPI_METHOD_EFI_HELPER      0x01    // EFI helper disk (best)
#define NBX_ACPI_METHOD_BCD_OVERRIDE    0x02    // Boot Configuration Data
#define NBX_ACPI_METHOD_RUNTIME         0x04    // Runtime modification (limited)

/*
 * Core API Functions
 */

/**
 * Apply ACPI profile to a VM
 * 
 * @param VmId - VM GUID or name
 * @param Profile - ACPI profile configuration
 * @param Result - Result structure (optional)
 * @return HRESULT - S_OK on success
 * 
 * NOTES:
 *   - VM must be stopped for table injection
 *   - Creates EFI helper VHD if using EFI method
 *   - Changes take effect on next boot
 *   - May require disabling Secure Boot
 */
HRESULT WINAPI NbxApplyAcpiProfile(
    _In_ LPCWSTR VmId,
    _In_ PNBX_ACPI_PROFILE Profile,
    _Out_opt_ PNBX_ACPI_RESULT Result
);

/**
 * Extract ACPI tables from VM or host
 * 
 * @param VmId - VM GUID or NULL for host system
 * @param OutputDirectory - Directory to save extracted tables
 * @param TableCount - Output: number of tables extracted
 * @return HRESULT - S_OK on success
 * 
 * NOTES:
 *   - Extracts DSDT, SSDT, MADT, FADT tables
 *   - Saves as .aml files (ACPI Machine Language)
 *   - Useful for creating custom table modifications
 */
HRESULT WINAPI NbxExtractAcpiTables(
    _In_opt_ LPCWSTR VmId,
    _In_ LPCWSTR OutputDirectory,
    _Out_ PDWORD TableCount
);

/**
 * Scan ACPI tables for Hyper-V signatures
 * 
 * @param VmId - VM GUID or NULL for host
 * @param Devices - Output array of detected Hyper-V device IDs
 * @param DeviceCount - Output: number of devices found
 * @return HRESULT - S_OK on success
 * 
 * NOTES:
 *   - Scans DSDT and SSDT tables
 *   - Detects VMBUS, VMGEN, synthetic devices
 *   - Useful for verification after profile application
 */
HRESULT WINAPI NbxScanAcpiForHyperV(
    _In_opt_ LPCWSTR VmId,
    _Outptr_result_buffer_(*DeviceCount) LPWSTR** Devices,
    _Out_ PDWORD DeviceCount
);

/**
 * Create EFI helper disk for ACPI table injection
 * 
 * @param OutputPath - Path for helper VHD file
 * @param Profile - ACPI profile to embed
 * @return HRESULT - S_OK on success
 * 
 * NOTES:
 *   - Creates bootable EFI VHD
 *   - Contains custom ACPI tables
 *   - Must be attached as first boot device
 *   - Chains to actual OS disk after injection
 */
HRESULT WINAPI NbxCreateAcpiHelperDisk(
    _In_ LPCWSTR OutputPath,
    _In_ PNBX_ACPI_PROFILE Profile
);

/**
 * Remove Hyper-V devices from ACPI table
 * 
 * @param TablePath - Path to ACPI table file (.aml)
 * @param OutputPath - Path for modified table
 * @return HRESULT - S_OK on success
 * 
 * NOTES:
 *   - Modifies DSDT/SSDT to remove Hyper-V devices
 *   - Uses ACPICA tools (iasl compiler) if available
 *   - Falls back to binary patching
 */
HRESULT WINAPI NbxRemoveHyperVFromAcpiTable(
    _In_ LPCWSTR TablePath,
    _In_ LPCWSTR OutputPath
);

/**
 * Validate ACPI profile
 * 
 * @param Profile - Profile to validate
 * @param ErrorMessage - Output buffer for errors
 * @param ErrorMessageSize - Buffer size
 * @return BOOL - TRUE if valid
 * 
 * CHECKS:
 *   - OEM ID/Table ID format
 *   - Custom table file existence
 *   - Compatibility with Windows 11 TPM requirements
 */
BOOL WINAPI NbxValidateAcpiProfile(
    _In_ PNBX_ACPI_PROFILE Profile,
    _Out_writes_(ErrorMessageSize) LPWSTR ErrorMessage,
    _In_ DWORD ErrorMessageSize
);

/**
 * Get available ACPI manipulation capabilities
 * 
 * @return DWORD - Bitmask of available methods
 */
DWORD WINAPI NbxGetAcpiCapabilities(VOID);

/*
 * TODO (Phase 2 Implementation):
 * - Implement ACPI table parser (DSDT/SSDT)
 * - Create EFI helper disk generator
 * - Integrate ACPICA tools (iasl compiler) for table decompilation/recompilation
 * - Build binary patcher for ACPI tables (when iasl not available)
 * - Implement device removal logic (VMBUS, VMGEN, etc.)
 * - Add signature fixing for Hyper-V OEM strings
 * - Create battery/dock spoofing device descriptors
 * - Test with Windows 11 TPM/Secure Boot enabled
 * 
 * LIMITATIONS:
 * - ACPI table modification is complex and error-prone
 * - Incorrect tables can prevent VM boot
 * - Windows 11 Secure Boot complicates unsigned table loading
 * - Runtime ACPI modification is nearly impossible
 * - TPM requirements must be preserved for Windows 11
 * 
 * SECURITY CONSIDERATIONS:
 * - ACPI injection modifies boot-time firmware data
 * - May trigger Windows security warnings
 * - Incompatible with Secure Boot without signed helper
 * - Mark as HIGHLY EXPERIMENTAL
 * - Document risks clearly to users
 * 
 * REFERENCES:
 * - ACPI Specification 6.4: https://uefi.org/specifications
 * - ACPICA Tools: https://acpica.org/
 * - Intel ASL Compiler: https://acpica.org/downloads
 */

#ifdef __cplusplus
}
#endif

#endif // !NANABOX_ACPI_H
