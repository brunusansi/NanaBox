# Host-Side Automation Scripts

This directory contains PowerShell scripts and helper utilities that run on the Hyper-V host to configure virtual machines with anti-detection features.

## Purpose

Host-side scripts handle operations that must be performed outside the guest VM:
- Hardware identity discovery and extraction
- SMBIOS configuration generation
- ACPI table extraction and modification
- VHD helper disk creation for boot-time injection
- MAC address configuration
- GPU passthrough setup

## Planned Scripts (Phase 2+)

### Hardware Discovery
- `Get-HostHardwareInfo.ps1`: Extract host SMBIOS, ACPI, and hardware IDs
- `Export-SMBIOS.ps1`: Export host SMBIOS to binary format
- `Export-ACPI.ps1`: Export host ACPI tables

### Configuration Generation
- `New-SpoofConfig.ps1`: Generate anti-detection configuration from host hardware
- `New-CleanSMBIOS.ps1`: Create sanitized SMBIOS binary for VM injection
- `New-CleanACPI.ps1`: Create sanitized ACPI tables

### VM Configuration
- `Set-VMAntiDetection.ps1`: Apply anti-detection configuration to a VM
- `New-EFIHelperDisk.ps1`: Create VHD with EFI bootloader for SMBIOS/ACPI injection
- `Set-VMNetworkAdapter.ps1`: Configure network adapter with custom MAC

### GPU Passthrough
- `Enable-GPUPV.ps1`: Configure GPU paravirtualization
- `Set-GPUSpoofing.ps1`: Spoof GPU vendor/device IDs

## Usage

Scripts are invoked automatically by NanaBox when:
- Creating a new VM with anti-detection enabled
- Applying an anti-detection profile
- Modifying VM hardware configuration

They can also be run manually:
```powershell
# Example: Extract host hardware info
.\Get-HostHardwareInfo.ps1 -OutputPath ".\host-hardware.json"

# Example: Create clean SMBIOS for VM
.\New-CleanSMBIOS.ps1 -InputFile ".\host-hardware.json" -OutputFile ".\smbios_clean.bin"
```

## Requirements

- Windows 10/11 or Windows Server 2022+
- Hyper-V feature enabled
- PowerShell 5.1 or later
- Administrator privileges
- Some scripts may require additional modules (will be documented in each script)

## Development Status

**Phase 0-1**: Directory structure created, scripts to be implemented in Phase 2+

## Contributing

When adding new scripts:
1. Follow PowerShell best practices (approved verbs, proper parameters)
2. Include comment-based help
3. Handle errors gracefully
4. Log operations for troubleshooting
5. Test on both Windows 10/11 and Server

## License

MIT License (same as NanaBox project)
