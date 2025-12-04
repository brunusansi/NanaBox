# NanaBox Anti-Detection Edition - Configuration Guide

## Overview

This document provides a comprehensive guide to the NanaBox Anti-Detection Edition configuration format. The configuration is stored in JSON format with a `.7b` file extension and UTF-8 with BOM encoding.

## Schema Versioning

The configuration schema supports versioning to allow for evolution and backward compatibility:

- **Version 1**: Initial release with Phase 1-4 configuration fields
- **SchemaVersion** field in Metadata allows tracking configuration format evolution
- Future versions will include migration tools to upgrade old configurations

## Complete JSON Schema

### Root Structure

```json
{
    "NanaBox": {
        "Type": "VirtualMachine",
        "Version": 1,
        "Metadata": { ... },
        "GuestType": "Windows",
        "Name": "MyVM",
        "ProcessorCount": 4,
        "MemorySize": 8192,
        "ComPorts": { ... },
        "Gpu": { ... },
        "NetworkAdapters": [ ... ],
        "ScsiDevices": [ ... ],
        "SecureBoot": true,
        "Tpm": false,
        "GuestStateFile": "MyVM.vmgs",
        "RuntimeStateFile": "MyVM.vmrs",
        "SaveStateFile": "MyVM.SaveState.vmrs",
        "ExposeVirtualizationExtensions": false,
        "Keyboard": { ... },
        "EnhancedSession": { ... },
        "ChipsetInformation": { ... },
        "VideoMonitor": { ... },
        "Policies": [ ... ],
        "Plan9Shares": [ ... ],
        "AntiDetectionProfile": "vanilla",
        "CpuId": { ... },
        "MsrIntercept": { ... },
        "AcpiOverride": { ... },
        "Timing": { ... },
        "Pci": { ... }
    }
}
```

## Core Configuration Fields

### Type (Required)
- **Type**: String
- **Value**: "VirtualMachine"
- **Description**: Identifies this as a VM configuration

### Version (Required)
- **Type**: Integer
- **Default**: 1
- **Description**: Configuration format version

### Metadata (Optional, Phase 1+)
Configuration metadata for tracking and multi-account isolation.

```json
"Metadata": {
    "Description": "User-friendly VM description",
    "Notes": "Additional notes or documentation",
    "AccountId": "account-identifier",
    "ProfileId": "profile-type",
    "CreationTimestamp": "2025-12-04T10:30:00Z",
    "LastUpdatedTimestamp": "2025-12-04T15:45:00Z",
    "SchemaVersion": 1
}
```

**Fields**:
- **Description**: Human-readable description of the VM
- **Notes**: Additional notes, instructions, or documentation
- **AccountId**: Unique identifier for account/profile isolation
- **ProfileId**: Profile type identifier (e.g., valorant, eac-generic, battleye, etc.)
- **CreationTimestamp**: ISO 8601 timestamp of VM creation
- **LastUpdatedTimestamp**: ISO 8601 timestamp of last configuration update
- **SchemaVersion**: Configuration schema version number

### GuestType (Required)
- **Type**: String
- **Values**: "Windows", "Linux", "Unknown"
- **Description**: Guest operating system type

### Name (Required)
- **Type**: String
- **Description**: Display name for the virtual machine

### ProcessorCount (Required)
- **Type**: Integer
- **Description**: Number of virtual CPU cores
- **Recommendations**:
  - Gaming: 6-8 cores
  - Development: 2-4 cores

### MemorySize (Required)
- **Type**: Integer
- **Unit**: Megabytes (MB)
- **Description**: RAM allocation
- **Recommendations**:
  - Gaming (high-end): 16384 (16 GB)
  - Gaming (mid-range): 8192 (8 GB)
  - Development: 4096 (4 GB)

### ComPorts (Optional)
COM port configuration for UEFI console and serial ports.

```json
"ComPorts": {
    "UefiConsole": "Disabled",
    "ComPort1": "\\\\.\\pipe\\vmpipe1",
    "ComPort2": "\\\\.\\pipe\\vmpipe2"
}
```

**Fields**:
- **UefiConsole**: "Disabled", "Default", "ComPort1", "ComPort2"
- **ComPort1**: Named pipe path for COM1
- **ComPort2**: Named pipe path for COM2

### Gpu (Optional)
GPU assignment configuration.

```json
"Gpu": {
    "AssignmentMode": "Default",
    "EnableHostDriverStore": false,
    "SelectedDevices": []
}
```

**Fields**:
- **AssignmentMode**: "Disabled", "Default", "List", "Mirror"
  - Disabled: No GPU assignment
  - Default: Use default GPU-PV
  - List: Assign specific GPUs
  - Mirror: Mirror host GPU configuration
- **EnableHostDriverStore**: Boolean, enables host driver store access
- **SelectedDevices**: Array of GPU device paths (for "List" mode)

### NetworkAdapters (Required)
Array of network adapter configurations.

```json
"NetworkAdapters": [
    {
        "Connected": true,
        "MacAddress": "00-15-5D-AB-CD-EF",
        "EndpointId": ""
    }
]
```

**Fields**:
- **Connected**: Boolean, adapter connection state
- **MacAddress**: MAC address in format "XX-XX-XX-XX-XX-XX" (empty for auto-generate)
- **EndpointId**: Network endpoint/switch identifier (empty for default)

**Anti-Detection Notes**:
- Use realistic MAC addresses from real hardware vendors
- Gaming profiles: Use consumer NIC MAC ranges from brands like:
  - Intel: 00-15-5D, D8-9E-F3, AC-DE-48
  - Realtek: 00-E0-4C, 52-54-00, 00-D8-61
  - Broadcom: 00-10-18, B8-27-EB, DC-A6-32

### ScsiDevices (Required)
Array of SCSI storage devices.

```json
"ScsiDevices": [
    {
        "Type": "VirtualDisk",
        "Path": "MyVM.vhdx"
    },
    {
        "Type": "VirtualImage",
        "Path": ""
    }
]
```

**Fields**:
- **Type**: "VirtualDisk", "VirtualImage", "PhysicalDevice"
- **Path**: Path to VHDX/ISO or physical device

### SecureBoot (Optional)
- **Type**: Boolean
- **Default**: false
- **Description**: Enable UEFI Secure Boot

### Tpm (Optional)
- **Type**: Boolean
- **Default**: false
- **Description**: Enable TPM 2.0
- **Requirements**: Windows 11 Version 24H2+ Host OS
- **Anti-Detection**: Required by some anti-cheat systems

### GuestStateFile (Optional)
- **Type**: String
- **Description**: Path to guest state file (.vmgs)

### RuntimeStateFile (Optional)
- **Type**: String
- **Description**: Path to runtime state file (.vmrs)

### SaveStateFile (Optional)
- **Type**: String
- **Description**: Path to save state file (.SaveState.vmrs)

### ExposeVirtualizationExtensions (Optional)
- **Type**: Boolean
- **Default**: false
- **Description**: Enable nested virtualization
- **Anti-Detection**: Keep disabled for bare-metal profiles

## Anti-Detection Configuration

### AntiDetectionProfile (Phase 1+)
High-level anti-detection preset.

```json
"AntiDetectionProfile": "bare-metal"
```

**Values**:
- **vanilla** (default): No anti-detection, standard Hyper-V behavior
- **balanced**: Moderate anti-detection with good performance
- **bare-metal**: Maximum anti-detection effort

**Usage Guidelines**:
- vanilla: Development, testing, non-sensitive workloads
- balanced: Light gaming, general use
- bare-metal: Anti-cheat gaming, security research, maximum stealth

### ChipsetInformation (Phase 1+)
SMBIOS (System Management BIOS) configuration.

```json
"ChipsetInformation": {
    "Manufacturer": "Dell Inc.",
    "ProductName": "OptiPlex 7090",
    "Version": "1.0",
    "SerialNumber": "4ZMK3X3",
    "UUID": "4c4c4544-005a-4d10-804b-b3c04f335833",
    "SKUNumber": "0A10",
    "Family": "OptiPlex",
    "BaseBoardSerialNumber": "/4ZMK3X3/CNFCP0013O0116/",
    "ChassisSerialNumber": "4ZMK3X3",
    "ChassisAssetTag": ""
}
```

**Fields** (all strings):
- **Manufacturer**: System manufacturer (e.g., "Dell Inc.", "ASUS", "MSI")
- **ProductName**: System/motherboard model
- **Version**: BIOS/firmware version
- **SerialNumber**: System serial number
- **UUID**: System UUID in GUID format
- **SKUNumber**: Product SKU
- **Family**: Product family name
- **BaseBoardSerialNumber**: Motherboard serial number
- **ChassisSerialNumber**: Chassis serial number
- **ChassisAssetTag**: Asset tag

**Best Practices**:
- Use values from real hardware for bare-metal profiles
- Ensure consistency (e.g., Dell manufacturer with Dell product name)
- Randomize serials for multi-account isolation
- Keep serials consistent per account/VM for fingerprinting

### CpuId (Phase 3+)
CPU identification spoofing configuration.

```json
"CpuId": {
    "Enabled": true,
    "HideHypervisor": true,
    "VendorString": "GenuineIntel",
    "MaskVirtualizationFeatures": true
}
```

**Fields**:
- **Enabled**: Boolean, enable CPUID spoofing
- **HideHypervisor**: Boolean, hide hypervisor present bit (CPUID.1.ECX[31])
- **VendorString**: String, CPU vendor ("GenuineIntel" or "AuthenticAMD")
- **MaskVirtualizationFeatures**: Boolean, hide VMX/SVM features

**Implementation Status**: Schema ready, implementation in Phase 3

### MsrIntercept (Phase 3+)
Model-Specific Register interception configuration.

```json
"MsrIntercept": {
    "Enabled": true,
    "BlockHyperVMsrs": true,
    "NormalizeTSC": true
}
```

**Fields**:
- **Enabled**: Boolean, enable MSR interception
- **BlockHyperVMsrs**: Boolean, block Hyper-V MSR range (0x40000000-0x400000FF)
- **NormalizeTSC**: Boolean, normalize Time Stamp Counter behavior

**Implementation Status**: Schema ready, implementation in Phase 3

### AcpiOverride (Phase 4+)
ACPI table override configuration.

```json
"AcpiOverride": {
    "Enabled": true,
    "RemoveHyperVDevices": true,
    "CustomDSDT": ""
}
```

**Fields**:
- **Enabled**: Boolean, enable ACPI overrides
- **RemoveHyperVDevices**: Boolean, remove Hyper-V ACPI devices (VMBus, etc.)
- **CustomDSDT**: String, path to custom DSDT table file (.aml)

**Implementation Status**: Schema ready, implementation in Phase 4

### Timing (Phase 4+)
Timing normalization configuration.

```json
"Timing": {
    "Strategy": "strict",
    "NormalizeTSC": true,
    "NormalizeAPIC": true,
    "NormalizeHPET": true
}
```

**Fields**:
- **Strategy**: String, timing strategy
  - "off": No timing normalization
  - "relaxed": Basic timing adjustments
  - "strict": Maximum timing accuracy (may impact performance)
- **NormalizeTSC**: Boolean, normalize Time Stamp Counter
- **NormalizeAPIC**: Boolean, normalize APIC timer
- **NormalizeHPET**: Boolean, normalize High Precision Event Timer

**Implementation Status**: Schema ready, implementation in Phase 4

### Pci (Phase 4+)
PCI topology configuration for bare-metal-like device layout.

```json
"Pci": {
    "Enabled": false,
    "Devices": [
        {
            "DeviceType": "GPU",
            "VendorId": "10de",
            "DeviceId": "2684",
            "SubsystemVendorId": "1043",
            "SubsystemId": "88d2"
        }
    ]
}
```

**Fields**:
- **Enabled**: Boolean, enable PCI topology customization
- **Devices**: Array of PCI device configurations
  - **DeviceType**: String, device type ("GPU", "NIC", "Storage", etc.)
  - **VendorId**: String, PCI vendor ID (hex)
  - **DeviceId**: String, PCI device ID (hex)
  - **SubsystemVendorId**: String, subsystem vendor ID (hex)
  - **SubsystemId**: String, subsystem ID (hex)

**Implementation Status**: Schema ready, implementation in Phase 4

## Configuration Validation

### Required Fields
- NanaBox.Type
- NanaBox.Version
- NanaBox.Name
- NanaBox.ProcessorCount
- NanaBox.MemorySize
- NanaBox.NetworkAdapters (at least one)
- NanaBox.ScsiDevices (at least one)

### Optional But Recommended
- Metadata (for tracking and isolation)
- AntiDetectionProfile (specify intent)
- ChipsetInformation (for anti-detection)

## Configuration Examples by Use Case

### Development/Testing
- AntiDetectionProfile: "vanilla"
- ProcessorCount: 2-4
- MemorySize: 4096
- SecureBoot: false
- Tpm: false
- Anti-detection: disabled

### Gaming (Anti-Cheat)
- AntiDetectionProfile: "bare-metal"
- ProcessorCount: 6-8
- MemorySize: 16384
- SecureBoot: true
- Tpm: true
- ChipsetInformation: Real hardware values
- CpuId: Enabled with hypervisor hiding
- MsrIntercept: Enabled
- AcpiOverride: Enabled
- Timing: "strict"

### Competitive Gaming (Anti-Cheat Systems)
- AntiDetectionProfile: "bare-metal"
- ProcessorCount: 8
- MemorySize: 16384
- SecureBoot: true
- Tpm: true (required by most kernel-level anti-cheats)
- ChipsetInformation: Real consumer gaming hardware values
- CpuId: Enabled with hypervisor hiding
- MsrIntercept: Enabled with MSR blocking
- AcpiOverride: Enabled with Hyper-V device removal
- Timing: "strict" for maximum accuracy

## See Also

- [Configuration Reference](../Documents/ConfigurationReference.md) - Full field documentation
- [Anti-Detection Overview](anti-detection-overview.md) - Detection vectors and mitigations
- [Profiles](profiles.md) - Built-in profile documentation
- [Examples](../examples/README.md) - Example configurations
