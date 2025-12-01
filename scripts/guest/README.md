# Guest-Side Configuration Scripts

This directory contains PowerShell scripts that run inside the guest VM to perform anti-detection hardening that cannot be accomplished from the host side.

## Purpose

Guest-side scripts handle operations that require running inside the VM:
- Registry modification to hide virtualization artifacts
- Disabling or removing Hyper-V integration services
- Hiding or renaming VM-specific drivers
- Guest-side CPUID/MSR spoofing driver installation
- Post-boot hardening and cleanup

## Planned Scripts (Phase 3+)

### Initial Setup
- `Initialize-AntiDetection.ps1`: Master script that orchestrates guest-side hardening
- `Install-Drivers.ps1`: Install CPUID/MSR/ACPI spoofing drivers
- `Set-RegistryHardening.ps1`: Modify registry to hide VM indicators

### Integration Services
- `Disable-HyperVServices.ps1`: Disable Hyper-V integration services
- `Remove-VMTools.ps1`: Remove or hide VM tools and drivers
- `Hide-VMDrivers.ps1`: Rename VM-specific drivers to avoid detection

### Driver Installation
- `Install-CPUIDDriver.ps1`: Install CPUID spoofing driver
- `Install-MSRDriver.ps1`: Install MSR interception driver
- `Install-ACPIDriver.ps1`: Install ACPI override driver
- `Install-TSCDriver.ps1`: Install TSC normalization driver

### Cleanup
- `Clear-VMRegistry.ps1`: Remove VM-related registry keys
- `Clear-VMFiles.ps1`: Remove VM-related files and artifacts
- `Reset-NetworkAdapters.ps1`: Reset network adapter configuration

## Usage

### Automatic Execution
Scripts are deployed to the VM and executed automatically:
1. NanaBox creates a temporary share or ISO with scripts
2. VM boots and runs `Initialize-AntiDetection.ps1`
3. Master script executes individual hardening scripts
4. VM reboots as needed
5. Temporary files are cleaned up

### Manual Execution
For testing or customization:
```powershell
# Example: Run full hardening
.\Initialize-AntiDetection.ps1 -Profile "bare-metal"

# Example: Install specific driver
.\Install-CPUIDDriver.ps1 -DriverPath "C:\Drivers\cpuid-spoof.sys"

# Example: Disable integration services
.\Disable-HyperVServices.ps1 -DisableAll
```

## Guest Drivers

Guest-side drivers are located in the `/drivers/` directory (to be created in Phase 3):
- `cpuid-spoof.sys`: CPUID instruction interception
- `msr-spoof.sys`: MSR read/write filtering
- `acpi-spoof.sys`: ACPI table modification
- `tsc-spoof.sys`: TSC normalization

See `/drivers/README.md` (Phase 3) for driver documentation.

## Requirements

- Windows 10/11 guest OS
- Administrator privileges inside guest
- PowerShell 5.1 or later
- Some scripts require test signing enabled for drivers

## Safety Considerations

⚠️ **Warning**: Guest-side scripts modify system files and drivers. Use with caution:
- Always test in a non-production VM first
- Create VM snapshots before running scripts
- Some changes may affect system stability
- Disabling integration services reduces VM manageability

## Development Status

**Phase 0-1**: Directory structure created, scripts to be implemented in Phase 3+

## Contributing

When adding new scripts:
1. Test thoroughly in isolated VMs
2. Document all system changes made
3. Provide rollback/undo functionality
4. Check for Windows version compatibility
5. Handle driver signing requirements

## License

MIT License (same as NanaBox project)
