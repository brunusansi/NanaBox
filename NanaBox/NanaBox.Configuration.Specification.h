/*
 * PROJECT:    NanaBox
 * FILE:       NanaBox.Configuration.Specification.h
 * PURPOSE:    Definition for the Virtual Machine Configuration Specification
 *
 * LICENSE:    The MIT License
 *
 * MAINTAINER: MouriNaruto (Kenji.Mouri@outlook.com)
 */

#ifndef NANABOX_CONFIGURATION_SPECIFICATION
#define NANABOX_CONFIGURATION_SPECIFICATION

#if (defined(__cplusplus) && __cplusplus >= 201703L)
#elif (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
#else
#error "[NanaBox] You should use a C++ compiler with the C++17 standard."
#endif

#include <Windows.h>

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace NanaBox
{
    enum class GuestType : std::int32_t
    {
        Unknown = 0,
        Windows = 1,
        Linux = 2,
    };

    enum class UefiConsoleMode : std::int32_t
    {
        Disabled = 0,
        Default = 1,
        ComPort1 = 2,
        ComPort2 = 3,
    };

    enum class GpuAssignmentMode : std::int32_t
    {
        Disabled = 0,
        Default = 1,
        List = 2,
        Mirror = 3,
    };

    enum class ScsiDeviceType : std::int32_t
    {
        Unknown = 0, // Private and only used for configuration parsing.
        VirtualDisk = 1,
        VirtualImage = 2,
        PhysicalDevice = 3,
    };

    /// <summary>
    /// Anti-detection profile presets
    /// Phase 1: Configuration schema only
    /// Phase 2+: Actual implementation
    /// </summary>
    enum class AntiDetectionProfile : std::int32_t
    {
        Vanilla = 0,    // No anti-detection (default)
        Balanced = 1,   // Moderate anti-detection with good performance
        BareMetal = 2,  // Maximum anti-detection effort
    };

    struct ComPortsConfiguration
    {
        UefiConsoleMode UefiConsole = UefiConsoleMode::Disabled;
        std::string ComPort1;
        std::string ComPort2;
    };

    struct GpuConfiguration
    {
        GpuAssignmentMode AssignmentMode = GpuAssignmentMode::Disabled;
        bool EnableHostDriverStore = false;
        std::map<std::string, std::uint16_t> SelectedDevices;
    };

    struct NetworkAdapterConfiguration
    {
        bool Connected = false;
        std::string MacAddress;
        std::string EndpointId;
    };

    struct ScsiDeviceConfiguration
    {
        ScsiDeviceType Type;
        std::string Path;
    };

    struct KeyboardConfiguration
    {
        bool RedirectKeyCombinations = true;
        std::int32_t FullScreenHotkey = VK_CANCEL; // CTRL + ALT + ?
        std::int32_t CtrlEscHotkey = VK_HOME; // ALT + ?
        std::int32_t AltEscHotkey = VK_INSERT; // ALT + ?
        std::int32_t AltTabHotkey = VK_PRIOR; // ALT + ?
        std::int32_t AltShiftTabHotkey = VK_NEXT; // ALT + ?
        std::int32_t AltSpaceHotkey = VK_DELETE; // ALT + ?
        std::int32_t CtrlAltDelHotkey = VK_END; // CTRL + ALT + ?
        std::int32_t FocusReleaseLeftHotkey = VK_LEFT; // CTRL + ALT + ?
        std::int32_t FocusReleaseRightHotkey = VK_RIGHT; // CTRL + ALT + ?
    };

    struct EnhancedSessionConfiguration
    {
        bool RedirectAudio = true;
        bool RedirectAudioCapture = false;
        bool RedirectDrives = false;
        bool RedirectPrinters = false;
        bool RedirectPorts = false;
        bool RedirectSmartCards = false;
        bool RedirectClipboard = true;
        bool RedirectDevices = false;
        bool RedirectPOSDevices = false;
        bool RedirectDynamicDrives = false;
        bool RedirectDynamicDevices = false;
        std::vector<std::string> Drives;
        std::vector<std::string> Devices;
    };

    struct ChipsetInformationConfiguration
    {
        std::string BaseBoardSerialNumber;
        std::string ChassisSerialNumber;
        std::string ChassisAssetTag;
        std::string Manufacturer; // At least 20348.
        std::string ProductName; // At least 20348.
        std::string Version; // At least 20348.
        std::string SerialNumber; // At least 20348.
        std::string UUID; // At least 20348.
        std::string SKUNumber; // At least 20348.
        std::string Family; // At least 20348.
    };

    struct VideoMonitorConfiguration
    {
        std::uint16_t HorizontalResolution = 1024;
        std::uint16_t VerticalResolution = 768;
        bool DisableBasicSessionDpiScaling = false;
        bool EnableDpiScalingValueOverride = false; // Not Implemented
        bool EnableContentResizing = true; // Not Implemented
        bool ShowFullScreenModeConnectionBar = true; // Not Implemented
        std::uint32_t OverriddenDpiScalingValue = 100; // Not Implemented
    };

    struct Plan9ShareConfiguration
    {
        bool ReadOnly = false;
        std::uint32_t Port;
        std::string Path;
        std::string Name;
    };

    /// <summary>
    /// Phase 2: SMBIOS baseboard configuration
    /// </summary>
    struct SmbiosBaseboardConfiguration
    {
        std::string Manufacturer;
        std::string Product;
        std::string Version;
        std::string SerialNumber;
    };

    /// <summary>
    /// Phase 2: SMBIOS chassis configuration
    /// </summary>
    struct SmbiosChassisConfiguration
    {
        std::string Type;           // e.g., "Desktop", "Laptop", "Tower"
        std::string SerialNumber;
    };

    /// <summary>
    /// Phase 2: Extended SMBIOS configuration
    /// </summary>
    struct SmbiosConfiguration
    {
        bool Enabled = false;
        std::string Vendor;
        std::string Product;
        std::string Version;
        std::string SerialNumber;
        std::string SkuNumber;
        std::string Family;
        SmbiosBaseboardConfiguration Baseboard;
        SmbiosChassisConfiguration Chassis;
        std::string Uuid;
        std::vector<std::string> OemStrings;
        std::string Template;       // e.g., "desktop-intel-2019", "desktop-amd-2021"
    };

    /// <summary>
    /// Phase 2: ACPI configuration
    /// </summary>
    struct AcpiConfiguration
    {
        bool Enabled = false;
        std::string OemId;
        std::string OemTableId;
        std::vector<std::string> OverrideTables;    // Paths to custom DSDT/SSDT blobs
        bool FixHyperVSignatures = false;
        bool SpoofBattery = false;
        bool SpoofDock = false;
    };

    /// <summary>
    /// Phase 3: CPUID leaf override configuration
    /// </summary>
    struct CpuIdLeafOverride
    {
        std::uint32_t Leaf;
        std::uint32_t Subleaf;
        std::string Eax;            // Can be hex value or "auto" or "mask:0x..."
        std::string Ebx;
        std::string Ecx;
        std::string Edx;
    };

    /// <summary>
    /// Phase 3: Extended CPUID spoofing configuration
    /// </summary>
    struct CpuIdConfiguration
    {
        bool Enabled = false;
        bool HideHypervisorBit = false;
        std::string VendorId;                       // "GenuineIntel", "AuthenticAMD"
        std::string BrandString;
        std::map<std::string, std::string> FeatureMasks;    // e.g., {"ecx": "0xFFFFFFFF", "edx": "0xFFFFFFFF"}
        std::vector<CpuIdLeafOverride> LeafOverrides;
        std::vector<std::string> Templates;         // e.g., "intel-8c-2020", "amd-8c-2022"
    };

    /// <summary>
    /// Phase 3: MSR rule configuration
    /// </summary>
    struct MsrRule
    {
        std::string Msr;                            // MSR address (hex)
        std::string Mode;                           // "mirror", "fake", "zero"
        std::string FakeValue;                      // Optional fake value for "fake" mode
    };

    /// <summary>
    /// Phase 3: Extended MSR (Model-Specific Register) interception configuration
    /// </summary>
    struct MsrInterceptConfiguration
    {
        bool Enabled = false;
        std::vector<MsrRule> Rules;
        std::string Template;                       // e.g., "default-gaming-safe", "valorant-safe", "eac-safe"
    };

    /// <summary>
    /// Phase 4: ACPI table override configuration (legacy - use AcpiConfiguration instead)
    /// Kept for backward compatibility with Phase 1 configs
    /// </summary>
    struct AcpiOverrideConfiguration
    {
        bool Enabled = false;
        bool RemoveHyperVDevices = false;
        std::string CustomDSDT;
    };

    /// <summary>
    /// Phase 4: Timing normalization mode
    /// </summary>
    enum class TimingMode : std::int32_t
    {
        Off = 0,        // No timing normalization
        Relaxed = 1,    // Basic timing adjustments
        Balanced = 2,   // Balanced timing normalization
        Strict = 3,     // Maximum timing accuracy, may impact performance
    };

    /// <summary>
    /// Phase 4: TSC configuration
    /// </summary>
    struct TscConfiguration
    {
        bool NormalizeFrequency = false;
        std::uint32_t TargetFrequencyMHz = 0;
        std::string JitterModel;                    // "none", "low-pc-like", "medium"
    };

    /// <summary>
    /// Phase 4: QPC configuration
    /// </summary>
    struct QpcConfiguration
    {
        std::string Backend;                        // "default", "tsc", "hpet"
        bool StabilityHints = false;
    };

    /// <summary>
    /// Phase 4: Extended timing normalization configuration
    /// </summary>
    struct TimingConfiguration
    {
        bool Enabled = false;
        TimingMode Mode = TimingMode::Off;
        TscConfiguration Tsc;
        QpcConfiguration Qpc;
    };

    /// <summary>
    /// Phase 4: Legacy timing strategy (backward compatibility)
    /// </summary>
    enum class TimingStrategy : std::int32_t
    {
        Off = 0,
        Relaxed = 1,
        Strict = 2,
    };

    /// <summary>
    /// Phase 4: PCI device configuration entry
    /// </summary>
    struct PciDeviceConfiguration
    {
        std::string Class;                          // Device class
        std::string VendorId;                       // PCI vendor ID (hex)
        std::string DeviceId;                       // PCI device ID (hex)
        std::string SubsystemVendorId;              // Subsystem vendor ID (hex)
        std::string SubsystemId;                    // Subsystem ID (hex)
        std::string Description;                    // Human-readable description
    };

    /// <summary>
    /// Phase 4: Extended PCI topology configuration
    /// </summary>
    struct PciConfiguration
    {
        bool Enabled = false;
        std::string LayoutTemplate;                 // e.g., "desktop-single-gpu-2021"
        std::vector<PciDeviceConfiguration> Devices;
    };

    /// <summary>
    /// VM metadata for tracking and isolation
    /// Phase 1: Configuration schema extension
    /// </summary>
    struct VirtualMachineMetadata
    {
        std::string Description;                     // User-friendly VM description
        std::string Notes;                           // Additional notes
        std::string AccountId;                       // Account/profile identifier for isolation
        std::string ProfileId;                       // Profile identifier (gaming, cloud, etc.)
        std::string CreationTimestamp;               // ISO 8601 timestamp
        std::string LastUpdatedTimestamp;            // ISO 8601 timestamp
        std::uint32_t SchemaVersion = 1;             // Configuration schema version
    };

    struct VirtualMachineConfiguration
    {
        std::uint32_t Version = 1;
        GuestType GuestType = GuestType::Unknown;
        std::string Name;
        std::uint32_t ProcessorCount = 0;
        std::uint64_t MemorySize = 0;
        ComPortsConfiguration ComPorts;
        GpuConfiguration Gpu;
        std::vector<NetworkAdapterConfiguration> NetworkAdapters;
        std::vector<ScsiDeviceConfiguration> ScsiDevices;
        bool SecureBoot = false;
        bool Tpm = false;
        std::string GuestStateFile;
        std::string RuntimeStateFile;
        std::string SaveStateFile;
        bool ExposeVirtualizationExtensions = false;
        KeyboardConfiguration Keyboard;
        EnhancedSessionConfiguration EnhancedSession;
        ChipsetInformationConfiguration ChipsetInformation;
        VideoMonitorConfiguration VideoMonitor;
        std::vector<std::string> Policies;
        std::vector<Plan9ShareConfiguration> Plan9Shares;
        
        // Anti-Detection Edition fields (Phase 1+)
        // Note: ChipsetInformation already serves as basic SMBIOS configuration
        VirtualMachineMetadata Metadata;
        AntiDetectionProfile AntiDetectionProfile = AntiDetectionProfile::Vanilla;
        
        // Phase 2+ Extended Anti-Detection fields
        SmbiosConfiguration Smbios;                 // Phase 2: Extended SMBIOS configuration
        AcpiConfiguration Acpi;                     // Phase 2: ACPI configuration
        CpuIdConfiguration CpuId;                   // Phase 3: Extended CPUID configuration
        MsrInterceptConfiguration MsrIntercept;     // Phase 3: Extended MSR configuration
        AcpiOverrideConfiguration AcpiOverride;     // Phase 1 legacy (kept for compatibility)
        TimingConfiguration Timing;                 // Phase 4: Timing normalization
        PciConfiguration Pci;                       // Phase 4: PCI topology
    };
}


#endif // !NANABOX_CONFIGURATION_SPECIFICATION
