# NanaBox Configuration Examples

This directory contains example configuration files demonstrating different use cases and anti-detection profiles.

## Files Overview

### General Examples

#### basic-vm.json
A minimal virtual machine configuration with no anti-detection features enabled. Suitable for:
- Development and testing
- Non-sensitive workloads
- Learning the configuration format
- Maximum compatibility and performance

**Profile**: vanilla

#### bare-metal-profile.json
Advanced configuration with full anti-detection features enabled. Suitable for:
- Gaming with anti-cheat systems
- Security research
- Malware analysis labs
- Maximum bare-metal mimicry

**Profile**: bare-metal

Features enabled:
- Custom SMBIOS from real Dell hardware
- MAC address from real Intel NIC
- CPUID hypervisor hiding
- MSR interception
- ACPI device hiding
- TPM 2.0 enabled

### Gaming Profiles

#### profile-valorant.json
Optimized for **Valorant** with Riot Vanguard anti-cheat compatibility.
- **Hardware Profile**: ASUS ROG STRIX Z690 with Intel Core i7-12700K
- **Anti-Detection**: bare-metal profile with strict timing
- **Features**: Full CPUID/MSR/ACPI spoofing, TPM 2.0 enabled
- **Use Case**: Running Valorant in VM for testing or isolated gaming

#### profile-roblox.json
Optimized for **Roblox** with Byfron anti-cheat compatibility.
- **Hardware Profile**: Gigabyte B550 AORUS with AMD Ryzen 5 5600X
- **Anti-Detection**: balanced profile with relaxed timing
- **Features**: CPUID/MSR/ACPI spoofing, no TPM requirement
- **Use Case**: Running Roblox in VM for account separation

### Integration Examples

#### hvfilter-integration-example.cpp
Example C++ code demonstrating how to integrate the HvFilter driver with NanaBox VM configuration.

**Purpose**: Shows developers how to:
- Check if the HvFilter driver is available
- Connect to the driver from user-mode
- Load anti-detection profiles from VM configuration
- Query driver status
- Clear profiles on VM shutdown

**Usage**: This is example code to be integrated into NanaBox source files, not a standalone program.

**See also**: `docs/driver-hvfilter.md` for complete driver documentation

#### profile-pubg.json
Optimized for **PUBG** with BattlEye anti-cheat compatibility.
- **Hardware Profile**: MSI MPG Z790 with Intel Core i9-13900K
- **Anti-Detection**: bare-metal profile with strict timing
- **Features**: Full CPUID/MSR/ACPI spoofing, TPM 2.0 enabled
- **Use Case**: Running PUBG in VM for competitive gaming

#### profile-ea-ac.json
Optimized for **EA Games** with EA Anti-Cheat compatibility.
- **Hardware Profile**: ASRock Z690 Steel Legend with Intel Core i7-13700K
- **Anti-Detection**: bare-metal profile with strict timing
- **Features**: Full CPUID/MSR/ACPI spoofing, TPM 2.0 enabled
- **Use Case**: Running EA games (Apex Legends, etc.) in VM

### Additional Gaming Profiles (Coming Soon)

Additional gaming profiles are planned for Phase 5:
- **FaceIT Anti-Cheat**: For CS:GO and other FaceIT platform games
- **Expert Tencent**: For Tencent games (PUBG Mobile, etc.)
- **EasyAntiCheat (EAC)**: Generic EAC profile for various games
- **BattlEye**: Generic BattlEye profile for various games

## Configuration Fields

### Core Fields
- **Type**: Always "VirtualMachine"
- **Version**: Configuration schema version (currently 1)
- **GuestType**: "Windows" or "Linux"
- **Name**: Display name for the VM
- **ProcessorCount**: Number of virtual CPUs
- **MemorySize**: RAM in megabytes

### Anti-Detection Fields

#### AntiDetectionProfile
High-level preset: `vanilla`, `balanced`, or `bare-metal`

#### Smbios
System Management BIOS configuration. Use values from real hardware:
- **Manufacturer**: System manufacturer (e.g., "Dell Inc.")
- **ProductName**: System model (e.g., "OptiPlex 7090")
- **Version**: BIOS version
- **SerialNumber**: System serial number
- **UUID**: System UUID (GUID format)
- **SKUNumber**: Product SKU
- **Family**: Product family name
- **BaseBoardSerialNumber**: Motherboard serial
- **ChassisSerialNumber**: Chassis serial
- **ChassisAssetTag**: Asset tag

#### CpuId (Phase 3+)
CPU identification spoofing:
- **Enabled**: Enable CPUID spoofing
- **HideHypervisor**: Clear hypervisor present bit
- **VendorString**: CPU vendor ("GenuineIntel" or "AuthenticAMD")
- **MaskVirtualizationFeatures**: Hide VMX/SVM features

#### MsrIntercept (Phase 3+)
Model-Specific Register control:
- **Enabled**: Enable MSR interception
- **BlockHyperVMsrs**: Block access to Hyper-V MSRs
- **NormalizeTSC**: Normalize Time Stamp Counter behavior

#### AcpiOverride (Phase 4+)
ACPI table customization:
- **Enabled**: Enable ACPI overrides
- **RemoveHyperVDevices**: Remove Hyper-V ACPI devices
- **CustomDSDT**: Path to custom DSDT table file

## Creating Your Own Configuration

1. Start with `basic-vm.json` as a template
2. Customize hardware settings (CPU, RAM, disk)
3. Choose an anti-detection profile
4. If using `bare-metal` profile, customize SMBIOS with real hardware values
5. Test incrementally, starting with basic profile first

## Finding Real Hardware Values

To obtain realistic SMBIOS values from real hardware:

### Windows
```powershell
# System information
Get-WmiObject Win32_ComputerSystem | Select-Object Manufacturer, Model
Get-WmiObject Win32_BIOS | Select-Object SerialNumber, Version
Get-WmiObject Win32_BaseBoard | Select-Object SerialNumber, Product

# UUID
Get-WmiObject Win32_ComputerSystemProduct | Select-Object UUID
```

### Linux
```bash
# System information
sudo dmidecode -t system
sudo dmidecode -t baseboard
sudo dmidecode -t bios
```

## Testing Anti-Detection

After configuring your VM, test detection evasion with:
- Pafish
- Al-Khaser
- CPU-Z / HWiNFO
- Specific anti-cheat systems you're targeting

## Support

For questions or issues:
- See full documentation in `/docs/`
- Check the configuration reference: `Documents/ConfigurationReference.md`
- Open an issue on GitHub

## License

These example files are part of NanaBox Anti-Detection Edition and are provided under the MIT License.
