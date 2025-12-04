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
    /// CPUID spoofing configuration
    /// Phase 1: Configuration schema only (reserved for future use)
    /// Phase 3: Implementation with guest-side driver
    /// </summary>
    struct CpuIdConfiguration
    {
        bool Enabled = false;                        // Enable CPUID spoofing
        bool HideHypervisor = false;                 // TODO(Phase3): Hide hypervisor present bit
        std::string VendorString;                    // TODO(Phase3): CPU vendor string ("GenuineIntel", "AuthenticAMD")
        bool MaskVirtualizationFeatures = false;     // TODO(Phase3): Hide VMX/SVM features
    };

    /// <summary>
    /// MSR (Model-Specific Register) interception configuration
    /// Phase 1: Configuration schema only (reserved for future use)
    /// Phase 3: Implementation with HCS API and/or guest-side driver
    /// </summary>
    struct MsrInterceptConfiguration
    {
        bool Enabled = false;                        // Enable MSR interception
        bool BlockHyperVMsrs = false;                // TODO(Phase3): Block access to Hyper-V MSR range
        bool NormalizeTSC = false;                   // TODO(Phase4): Normalize Time Stamp Counter behavior
    };

    /// <summary>
    /// ACPI table override configuration
    /// Phase 1: Configuration schema only (reserved for future use)
    /// Phase 4: Implementation with EFI helper and table injection
    /// </summary>
    struct AcpiOverrideConfiguration
    {
        bool Enabled = false;                        // Enable ACPI overrides
        bool RemoveHyperVDevices = false;            // TODO(Phase4): Remove Hyper-V ACPI devices
        std::string CustomDSDT;                      // TODO(Phase4): Path to custom DSDT table file
    };

    /// <summary>
    /// Timing normalization strategy
    /// Phase 1: Configuration schema only (reserved for future use)
    /// Phase 4: Implementation with timing adjustments
    /// </summary>
    enum class TimingStrategy : std::int32_t
    {
        Off = 0,        // No timing normalization
        Relaxed = 1,    // Basic timing adjustments
        Strict = 2,     // Maximum timing accuracy, may impact performance
    };

    /// <summary>
    /// Timing normalization configuration
    /// Phase 1: Configuration schema only (reserved for future use)
    /// Phase 4: Implementation for TSC, APIC, HPET normalization
    /// </summary>
    struct TimingConfiguration
    {
        TimingStrategy Strategy = TimingStrategy::Off;  // Timing normalization strategy
        bool NormalizeTSC = false;                      // TODO(Phase4): Normalize Time Stamp Counter
        bool NormalizeAPIC = false;                     // TODO(Phase4): Normalize APIC timer
        bool NormalizeHPET = false;                     // TODO(Phase4): Normalize High Precision Event Timer
    };

    /// <summary>
    /// PCI device configuration entry
    /// Phase 1: Configuration schema only (reserved for future use)
    /// Phase 4: Implementation for bare-metal-like PCI topology
    /// </summary>
    struct PciDeviceConfiguration
    {
        std::string DeviceType;                      // TODO(Phase4): "GPU", "NIC", "Storage", etc.
        std::string VendorId;                        // TODO(Phase4): PCI vendor ID (hex)
        std::string DeviceId;                        // TODO(Phase4): PCI device ID (hex)
        std::string SubsystemVendorId;               // TODO(Phase4): Subsystem vendor ID (hex)
        std::string SubsystemId;                     // TODO(Phase4): Subsystem ID (hex)
    };

    /// <summary>
    /// PCI topology configuration
    /// Phase 1: Configuration schema only (reserved for future use)
    /// Phase 4: Implementation for realistic PCI device layout
    /// </summary>
    struct PciConfiguration
    {
        bool Enabled = false;                        // Enable PCI topology customization
        std::vector<PciDeviceConfiguration> Devices; // TODO(Phase4): Custom PCI devices
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
        // Note: ChipsetInformation already serves as SMBIOS configuration
        VirtualMachineMetadata Metadata;
        AntiDetectionProfile AntiDetectionProfile = AntiDetectionProfile::Vanilla;
        CpuIdConfiguration CpuId;
        MsrInterceptConfiguration MsrIntercept;
        AcpiOverrideConfiguration AcpiOverride;
        TimingConfiguration Timing;
        PciConfiguration Pci;
    };
}

#endif // !NANABOX_CONFIGURATION_SPECIFICATION
