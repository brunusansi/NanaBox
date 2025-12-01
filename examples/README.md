# NanaBox Configuration Examples

This directory contains example configuration files demonstrating different use cases and anti-detection profiles.

## Files

### basic-vm.json
A minimal virtual machine configuration with no anti-detection features enabled. Suitable for:
- Development and testing
- Non-sensitive workloads
- Learning the configuration format
- Maximum compatibility and performance

**Profile**: vanilla

### bare-metal-profile.json
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
- Check the configuration reference: `/Documents/ConfigurationReference.md`
- Open an issue on GitHub

## License

These example files are part of NanaBox Anti-Detection Edition and are provided under the MIT License.
