# NanaBox Development Environment Setup Scripts

This directory contains setup and configuration scripts for preparing a development environment for NanaBox Anti-Detection Edition.

## Scripts

### Setup-NbxHvFilterPhase3B.ps1 (⭐ Recommended for Phase 3B)

**Purpose**: Automated Phase 3B setup for building, installing, starting, and testing the nanabox_hvfilter driver.

**Requirements**:
- Windows 10/11 (build 19041+)
- PowerShell 5.1 or later
- Administrator privileges
- Test-signing enabled (script will check)
- Visual Studio 2022 with C++ workload
- Windows Driver Kit (WDK) 10.0.22621.0+

**What it does**:
1. **Environment Validation**: Checks for MSBuild, WDK, test-signing status
2. **Build**: Compiles nanabox_hvfilter.sys and NbxHvFilterClient.exe
3. **Install/Update**: Installs or updates the driver, handles existing installations
4. **Start**: Starts the driver service
5. **Test**: Runs IOCTL sanity checks with NbxHvFilterClient

**Usage**:

```powershell
# Complete Phase 3B setup (recommended)
.\Setup-NbxHvFilterPhase3B.ps1

# Build in Debug configuration
.\Setup-NbxHvFilterPhase3B.ps1 -Configuration Debug

# Dry run - see what would be done without installing
.\Setup-NbxHvFilterPhase3B.ps1 -DryRun

# Use existing built binaries (skip build)
.\Setup-NbxHvFilterPhase3B.ps1 -SkipBuild

# Build only, don't install
.\Setup-NbxHvFilterPhase3B.ps1 -SkipInstall -SkipStart

# Get help and see all options
Get-Help .\Setup-NbxHvFilterPhase3B.ps1 -Full
```

**See Also**: [PHASE3B_TESTING.md](../../drivers/nanabox_hvfilter/PHASE3B_TESTING.md) for detailed testing instructions.

---

## Quick Start for Phase 3B Testing

**New to Phase 3B? Start here:**

1. **Enable test-signing** (one-time setup):
   ```cmd
   # Run as Administrator
   bcdedit /set testsigning on
   bcdedit /set nointegritychecks on
   shutdown /r /t 0
   ```
   Also disable Secure Boot in BIOS/UEFI.

2. **Run the Phase 3B setup script**:
   ```powershell
   # Open PowerShell as Administrator
   cd scripts\setup
   .\Setup-NbxHvFilterPhase3B.ps1
   ```

3. **Verify everything works**:
   The script will automatically build, install, start, and test the driver. Look for:
   ```
   Build:    OK
   Install:  OK
   Start:    OK (Running)
   IOCTL Test: OK
   ```

4. **Test CPUID/MSR configuration**:
   ```cmd
   cd tools\NbxHvFilterClient\x64\Release
   NbxHvFilterClient.exe set TestProfile 0x00000003
   NbxHvFilterClient.exe status
   ```

See [PHASE3B_TESTING.md](../../drivers/nanabox_hvfilter/PHASE3B_TESTING.md) for comprehensive testing scenarios.

---

### Setup-NanaBoxDevEnv.ps1

**Purpose**: General-purpose automated setup wizard for NanaBox kernel driver development environment.

**Requirements**:
- Windows 10/11 (build 19041+)
- PowerShell 5.1 or later
- Administrator privileges

**What it does**:
1. **Environment Check**: Detects Visual Studio, MSBuild, WDK, and Windows SDK
2. **Test Signing**: Enables test signing mode for loading unsigned/test-signed drivers
3. **Certificate Creation**: Creates and installs a self-signed test certificate for driver signing
4. **Driver Build**: Builds the NanaBox kernel drivers using MSBuild
5. **Driver Installation**: Optionally installs and starts the kernel driver

**Usage**:

```powershell
# Basic usage - run the complete setup wizard interactively
.\Setup-NanaBoxDevEnv.ps1

# Phase 3B mode - delegates to Setup-NbxHvFilterPhase3B.ps1
.\Setup-NanaBoxDevEnv.ps1 -Phase3B

# Skip environment checks (if you know you have everything installed)
.\Setup-NanaBoxDevEnv.ps1 -SkipEnvironmentCheck

# Only setup environment (no build/install)
.\Setup-NanaBoxDevEnv.ps1 -SkipBuild -SkipInstall

# Skip test signing setup (if already enabled)
.\Setup-NanaBoxDevEnv.ps1 -SkipTestSigning

# Skip certificate creation (if already exists)
.\Setup-NanaBoxDevEnv.ps1 -SkipCertificate

# Get help and see all options
Get-Help .\Setup-NanaBoxDevEnv.ps1 -Full
```

**Interactive Mode**:

The script runs in interactive mode by default, prompting for confirmation before each major action:

```
Do you want to enable test signing? [Y/n]: Y
Do you want to create test certificate? [Y/n]: Y
Do you want to build the driver? [Y/n]: Y
Do you want to install the driver? [y/N]: n
```

**Output Examples**:

```
============================================================================
 NanaBox Anti-Detection Edition - Development Environment Setup Wizard v1.0.0
============================================================================

>>> Checking for Visual Studio installation...
[OK] Found Visual Studio at: C:\Program Files\Microsoft Visual Studio\2022\Community

>>> Checking test signing status...
[WARN] Test signing is DISABLED

>>> Enabling test signing mode...
[OK] Test signing enabled successfully
[WARN] A system reboot is required for changes to take effect!

>>> Creating self-signed test certificate...
[OK] Certificate created successfully
[OK] Installed to Trusted Root Certification Authorities

>>> Building NanaBox kernel driver...
[OK] Driver built successfully!
[OK] Build output location: drivers\nanabox_hvfilter\x64\Release

============================================================================
 Setup Complete!
============================================================================
```

## Prerequisites

### Required Software

1. **Visual Studio 2022** (Community, Professional, or Enterprise)
   - Install from: https://visualstudio.microsoft.com/downloads/
   - Required workloads:
     - Desktop development with C++
   
2. **Windows Driver Kit (WDK) 10.0.22621.0+**
   - Install from: https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk
   - Includes driver development headers, libraries, and tools
   
3. **Windows SDK 10.0.22621.0+**
   - Usually included with Visual Studio or WDK
   - Install from: https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/

### System Configuration

Before running drivers:

1. **Disable Secure Boot** (required for test-signed drivers):
   - Restart computer and enter BIOS/UEFI settings
   - Find Secure Boot option (usually under Security or Boot settings)
   - Set to "Disabled"
   - Save and exit

2. **Enable Test Signing** (done automatically by the script):
   ```cmd
   bcdedit /set testsigning on
   ```
   - Requires Administrator privileges
   - Requires system reboot to take effect

## Post-Setup

After running the setup wizard:

### Driver Management

**Check driver status**:
```cmd
sc query nanabox_hvfilter
```

**Start driver**:
```cmd
sc start nanabox_hvfilter
```

**Stop driver**:
```cmd
sc stop nanabox_hvfilter
```

**Remove driver**:
```cmd
sc stop nanabox_hvfilter
sc delete nanabox_hvfilter
pnputil /delete-driver nanabox_hvfilter.inf /uninstall
```

### Rebuilding Drivers

After making changes to driver source code:

```powershell
cd drivers\nanabox_hvfilter
msbuild /p:Configuration=Release /p:Platform=x64
```

Or use the Visual Studio IDE to build.

### Debugging

**Enable kernel debugging**:
```cmd
bcdedit /debug on
bcdedit /dbgsettings serial debugport:1 baudrate:115200
```

**View debug output**:
- Use [DebugView](https://learn.microsoft.com/en-us/sysinternals/downloads/debugview) from Sysinternals
- Or use WinDbg for kernel debugging

## Troubleshooting

### "This script requires Administrator privileges"

**Solution**: Right-click PowerShell and select "Run as Administrator"

### "Visual Studio not found"

**Solution**: 
1. Install Visual Studio 2022 with C++ workload
2. Or use `-SkipEnvironmentCheck` if using alternative build tools

### "Test signing not enabled after reboot"

**Possible causes**:
- Secure Boot is still enabled (disable in BIOS)
- BitLocker may prevent test signing (temporarily disable)
- Group Policy may override settings

**Verification**:
```cmd
bcdedit /enum "{current}" | findstr testsigning
```
Should show: `testsigning             Yes`

### "Driver failed to load"

**Possible causes**:
1. Test signing not enabled
2. Secure Boot still enabled
3. Driver not properly signed
4. Driver code has bugs (check Event Viewer)

**Check Event Viewer**:
```
Event Viewer → Windows Logs → System
Filter for Source: "Microsoft-Windows-Kernel-PnP" or "Service Control Manager"
```

### "Build failed"

**Common issues**:
- WDK not properly installed
- Wrong WDK version (need 10.0.22621.0+)
- Project file missing or corrupted
- Missing include paths

**Check build log**:
```
drivers\nanabox_hvfilter\build.log
```

## Security Considerations

⚠️ **IMPORTANT WARNINGS**:

1. **Test Signing Mode**: Disabling driver signature enforcement reduces system security. Only use on development/test machines.

2. **Kernel Drivers**: Kernel drivers run with highest privileges. Bugs can cause system crashes (BSOD) or security vulnerabilities.

3. **Secure Boot**: Disabling Secure Boot reduces protection against rootkits and bootkits. Re-enable after development.

4. **Production Use**: Test-signed drivers are for development only. Production deployment requires:
   - Extended Validation (EV) Code Signing Certificate (~$300-500/year)
   - Windows Hardware Developer Center attestation signing
   - Proper driver testing and validation

## Additional Resources

- **NanaBox Documentation**: [/docs](/docs)
- **Driver Development**: [/drivers/README.md](/drivers/README.md)
- **WDK Documentation**: https://learn.microsoft.com/en-us/windows-hardware/drivers/
- **Driver Signing**: https://learn.microsoft.com/en-us/windows-hardware/drivers/install/driver-signing
- **Test Signing**: https://learn.microsoft.com/en-us/windows-hardware/drivers/install/the-testsigning-boot-configuration-option

## Support

For issues or questions:
- GitHub Issues: https://github.com/brunusansi/NanaBox/issues
- Documentation: https://github.com/brunusansi/NanaBox/docs

## License

MIT License (same as NanaBox project)
