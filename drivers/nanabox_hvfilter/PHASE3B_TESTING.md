# Phase 3B Testing Guide - CPUID/MSR Configuration Framework

## Overview

This document describes how to test the Phase 3B implementation of the nanabox_hvfilter driver, which includes the CPUID/MSR configuration framework. Phase 3B focuses on configuration management via IOCTLs and does not yet implement actual CPU instruction interception.

## What Phase 3B Includes

✅ **Implemented in Phase 3B:**
- Driver device and symbolic link creation
- IOCTL interface for profile management (SET_PROFILE, GET_STATUS, CLEAR_PROFILE)
- Profile configuration storage in driver memory
- User-mode client utility (NbxHvFilterClient.exe)
- Complete build and installation infrastructure

⏳ **Not Yet Implemented (Future Phases):**
- Actual CPUID instruction interception
- MSR read/write filtering
- CPU behavior modification
- Real-time CPUID/MSR policy enforcement

## Quick Start with Setup Script

The easiest way to build, install, and test the Phase 3B driver is using the automated setup script.

### Prerequisites

1. **System Requirements:**
   - Windows 10/11 x64
   - Administrator privileges
   - Test-signing enabled (see below)
   - Secure Boot disabled in BIOS/UEFI

2. **Development Tools:**
   - Visual Studio 2022 with C++ workload
   - Windows Driver Kit (WDK) 10.0.22621.0+
   - Windows SDK 10.0.22621.0+

3. **Enable Test-Signing (One-Time Setup):**
   ```cmd
   # Run as Administrator
   bcdedit /set testsigning on
   bcdedit /set nointegritychecks on
   shutdown /r /t 0
   ```
   
   After reboot, you should see "Test Mode" watermark on desktop.

### Using the Setup Script

Open PowerShell as Administrator and navigate to the repository:

```powershell
cd scripts\setup
```

#### Complete Setup (Recommended)

Build, install, start, and test the driver with one command:

```powershell
.\Setup-NbxHvFilterPhase3B.ps1
```

This will:
1. ✅ Validate environment (MSBuild, WDK)
2. ✅ Build nanabox_hvfilter.sys and NbxHvFilterClient.exe
3. ✅ Install/update the driver
4. ✅ Start the driver service
5. ✅ Run basic IOCTL communication tests

#### Debug Build

To build in Debug configuration:

```powershell
.\Setup-NbxHvFilterPhase3B.ps1 -Configuration Debug
```

#### Dry Run

To see what the script would do without actually installing:

```powershell
.\Setup-NbxHvFilterPhase3B.ps1 -DryRun
```

#### Skip Steps

You can skip individual steps:

```powershell
# Use existing built binaries
.\Setup-NbxHvFilterPhase3B.ps1 -SkipBuild

# Build but don't install
.\Setup-NbxHvFilterPhase3B.ps1 -SkipInstall -SkipStart

# Build and install but don't start
.\Setup-NbxHvFilterPhase3B.ps1 -SkipStart

# Build, install, and start but don't test
.\Setup-NbxHvFilterPhase3B.ps1 -SkipTest
```

#### Get Help

```powershell
Get-Help .\Setup-NbxHvFilterPhase3B.ps1 -Full
```

### Expected Output

Successful run should look like:

```
============================================================================
 NanaBox HvFilter Phase 3B Setup v1.0.0
============================================================================

Configuration: Release
Platform: x64

[OK] Running with Administrator privileges
[INFO] Repository root: C:\...\NanaBox
[INFO] Driver path: C:\...\NanaBox\drivers\nanabox_hvfilter
[INFO] Client path: C:\...\NanaBox\tools\NbxHvFilterClient

============================================================================
 Step 1: Environment Validation
============================================================================

>>> Checking test signing status...
[OK] Test signing is ENABLED

>>> Checking for MSBuild...
[OK] Found MSBuild at: C:\Program Files\...\MSBuild.exe

>>> Checking for Windows Driver Kit (WDK)...
[OK] Found WDK at: C:\Program Files (x86)\Windows Kits\10\Include

============================================================================
 Step 2: Build Driver and Client
============================================================================

>>> Building nanabox_hvfilter driver...
[INFO] Driver source: C:\...\drivers\nanabox_hvfilter
[INFO] Build configuration: Release
[INFO] Build platform: x64
[INFO] Found project file: nanabox_hvfilter.vcxproj
[INFO] Building with MSBuild...
[OK] Driver built successfully!
[OK] Build output location: C:\...\x64\Release
[OK] Driver binary: C:\...\x64\Release\nanabox_hvfilter.sys
[OK] Driver INF: C:\...\x64\Release\nanabox_hvfilter.inf

>>> Building NbxHvFilterClient...
[OK] Client built successfully!

============================================================================
 Step 3: Install/Update Driver
============================================================================

>>> Installing/Updating nanabox_hvfilter driver...
[INFO] Performing first-time driver installation...
[INFO] Copying driver to: C:\Windows\System32\drivers\nanabox_hvfilter.sys
[OK] Driver file copied successfully
[INFO] Creating driver service...
[OK] Driver service created successfully

============================================================================
 Step 4: Start Driver Service
============================================================================

>>> Starting driver service...
[INFO] Starting driver service...
[OK] Driver service started successfully

============================================================================
 Step 5: Run Sanity Checks
============================================================================

>>> Running sanity checks with NbxHvFilterClient...
[INFO] Client executable: C:\...\x64\Release\NbxHvFilterClient.exe
[INFO] Executing: NbxHvFilterClient.exe status

===============================================================
NanaBox HvFilter Client v1.0.0
===============================================================

[INFO] Sending GET_STATUS request...
[SUCCESS] Status retrieved successfully
       Driver Version: 1.0.0
       Active Profile: None
       Active Flags: 0x00000000
       Is Active: No

[OK] IOCTL test: PASSED
[OK] Driver is responding correctly to status queries

============================================================================
 Phase 3B Setup Summary
============================================================================

Build:    OK
Install:  OK
Start:    OK (Running)
IOCTL Test: OK

[OK] All Phase 3B setup steps completed successfully!

Next steps:
  1. Test CPUID/MSR configuration with NbxHvFilterClient
  2. Review driver logs in DebugView
  3. See PHASE3B_TESTING.md for detailed test scenarios
```

## Manual Testing (Without Setup Script)

If you prefer to test manually or need to debug individual steps, follow these procedures:

### 1. Manual Build

```cmd
cd drivers\nanabox_hvfilter
msbuild nanabox_hvfilter.vcxproj /p:Configuration=Release /p:Platform=x64

cd ..\..\tools\NbxHvFilterClient
msbuild NbxHvFilterClient.vcxproj /p:Configuration=Release /p:Platform=x64
```

### 2. Manual Installation

```cmd
# Copy driver to system directory
copy x64\Release\nanabox_hvfilter.sys C:\Windows\System32\drivers\

# Create service
sc create NanaBoxHvFilter type= kernel start= demand binPath= C:\Windows\System32\drivers\nanabox_hvfilter.sys DisplayName= "NanaBox Hypervisor Filter Driver"

# Start service
sc start NanaBoxHvFilter
```

### 3. Manual Testing

```cmd
# Check service status
sc query NanaBoxHvFilter

# Run client
cd tools\NbxHvFilterClient\x64\Release
NbxHvFilterClient.exe status
```

## Testing Scenarios

### Scenario 1: Basic IOCTL Communication

Verify that the driver responds to status queries:

```cmd
NbxHvFilterClient.exe status
```

**Expected Result:**
- Driver version displayed
- No active profile (none set yet)
- All flags should be 0x00000000

### Scenario 2: Set Profile Configuration

Set a test profile with CPUID and MSR flags:

```cmd
NbxHvFilterClient.exe set TestProfile 0x00000003
```

This sets flags:
- `0x00000001` - CPUID spoofing
- `0x00000002` - MSR interception

**Expected Result:**
```
[INFO] Sending SET_PROFILE request...
       Profile: TestProfile
       Flags: 0x00000003
         - CPUID: Yes
         - MSR Intercept: Yes
         - Timing: No
         - PCI: No
[SUCCESS] Profile set successfully
```

### Scenario 3: Verify Profile Is Active

Check that the profile was stored correctly:

```cmd
NbxHvFilterClient.exe status
```

**Expected Result:**
```
Active Profile: TestProfile
Active Flags: 0x00000003
  - CPUID: Yes
  - MSR Intercept: Yes
  - Timing: No
  - PCI: No
Is Active: Yes
```

### Scenario 4: Clear Profile

Remove the active profile:

```cmd
NbxHvFilterClient.exe clear
```

**Expected Result:**
```
[INFO] Sending CLEAR_PROFILE request...
[SUCCESS] Profile cleared successfully
```

Verify it's cleared:
```cmd
NbxHvFilterClient.exe status
```

Should show:
```
Active Profile: None
Active Flags: 0x00000000
Is Active: No
```

### Scenario 5: Multiple Profile Configurations

Test different profile combinations:

```cmd
# CPUID only
NbxHvFilterClient.exe set CPUIDOnly 0x00000001
NbxHvFilterClient.exe status

# MSR only
NbxHvFilterClient.exe set MSROnly 0x00000002
NbxHvFilterClient.exe status

# All features
NbxHvFilterClient.exe set FullProfile 0x0000000F
NbxHvFilterClient.exe status

# Clear
NbxHvFilterClient.exe clear
```

## Viewing Driver Logs

### Using DebugView

1. Download [DebugView](https://learn.microsoft.com/en-us/sysinternals/downloads/debugview) from Sysinternals
2. Run as Administrator
3. Enable "Capture Kernel" in the Capture menu
4. Look for messages prefixed with `[NanaBoxHvFilter]`

**Expected Log Messages:**
```
[NanaBoxHvFilter] DriverEntry: Entering
[NanaBoxHvFilter] Device created: \Device\NanaBoxHvFilter
[NanaBoxHvFilter] Symbolic link created: \DosDevices\NanaBoxHvFilter
[NanaBoxHvFilter] DriverEntry: Initialization complete
[NanaBoxHvFilter] NbxDispatchCreate: Device opened
[NanaBoxHvFilter] NbxHandleSetProfile: Profile='TestProfile' Flags=0x00000003
[NanaBoxHvFilter] NbxHandleGetStatus: Returning status
[NanaBoxHvFilter] NbxHandleClearProfile: Profile cleared
[NanaBoxHvFilter] NbxDispatchClose: Device closed
```

### Using Event Viewer

```cmd
eventvwr.msc
```

Navigate to: **Windows Logs → System**

Filter by Source: "Service Control Manager"

Look for entries related to "NanaBoxHvFilter"

## Troubleshooting

### Issue: "Test signing is DISABLED"

**Solution:**
```cmd
bcdedit /set testsigning on
bcdedit /set nointegritychecks on
shutdown /r /t 0
```

Also verify Secure Boot is disabled in BIOS/UEFI.

### Issue: "MSBuild not found" or "WDK not found"

**Solution:**
- Install Visual Studio 2022 with "Desktop development with C++" workload
- Install Windows Driver Kit from: https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk
- Or use `-SkipBuild` with pre-built binaries

### Issue: "Build failed"

**Solution:**
- Check build log: `drivers\nanabox_hvfilter\build_phase3b.log`
- Verify WDK is properly installed
- Ensure Windows SDK version matches WDK version
- Try building in Visual Studio IDE for better error messages

### Issue: "Driver service failed to start"

**Possible Causes:**
1. Test-signing not enabled
2. Secure Boot still enabled
3. Driver not properly signed
4. Driver initialization error

**Solutions:**
- Verify test-signing: `bcdedit /enum | findstr testsigning`
- Check Event Viewer for error details
- Review driver logs in DebugView
- See full troubleshooting in `INSTALL.md`

### Issue: "Failed to open driver device" (NbxHvFilterClient)

**Solution:**
```cmd
# Check if driver is running
sc query NanaBoxHvFilter

# If not running, start it
sc start NanaBoxHvFilter

# If start fails, check Event Viewer and DebugView
```

### Issue: "IOCTL test: FAILED"

**Possible Causes:**
- Driver not responding
- IOCTL interface broken
- Wrong IOCTL codes

**Solutions:**
- Verify driver is running: `sc query NanaBoxHvFilter`
- Check DebugView for IOCTL handling messages
- Try stopping and restarting: `sc stop NanaBoxHvFilter && sc start NanaBoxHvFilter`

## Updating the Driver

When making code changes and rebuilding:

```powershell
# Quick update with the script
.\Setup-NbxHvFilterPhase3B.ps1

# Or manually:
# 1. Stop the driver
sc stop NanaBoxHvFilter

# 2. Rebuild
cd drivers\nanabox_hvfilter
msbuild /t:rebuild /p:Configuration=Release /p:Platform=x64

# 3. Run the setup script (it will handle update)
..\..\scripts\setup\Setup-NbxHvFilterPhase3B.ps1 -SkipBuild
```

## Uninstalling

To completely remove the driver:

```cmd
# Stop the service
sc stop NanaBoxHvFilter

# Delete the service
sc delete NanaBoxHvFilter

# Remove driver file
del C:\Windows\System32\drivers\nanabox_hvfilter.sys
```

## Phase 3B Limitations

**Important:** Phase 3B is a configuration framework only. It does NOT:
- ❌ Intercept CPUID instructions
- ❌ Filter MSR access
- ❌ Modify CPU behavior
- ❌ Hide hypervisor signatures

Phase 3B only:
- ✅ Accepts profile configurations via IOCTL
- ✅ Stores configuration in driver memory
- ✅ Returns configuration status to user-mode

**Real CPUID/MSR interception will be implemented in future phases.**

## Next Steps After Phase 3B Testing

Once Phase 3B testing is complete and the configuration framework is validated:

1. **Phase 3C** (Future): Implement actual CPUID instruction hooking
2. **Phase 3D** (Future): Implement MSR read/write filtering
3. **Phase 4**: Add timing normalization and PCI topology control
4. **Phase 5**: Profile system and automation

## Testing Checklist

Use this checklist to verify Phase 3B functionality:

- [ ] Setup script runs without errors
- [ ] Driver builds successfully (both Debug and Release)
- [ ] Client builds successfully
- [ ] Driver installs/updates correctly
- [ ] Driver service starts successfully
- [ ] Driver appears in Device Manager
- [ ] Client can communicate with driver (status command works)
- [ ] SET_PROFILE IOCTL works with various flag combinations
- [ ] GET_STATUS IOCTL returns correct profile data
- [ ] CLEAR_PROFILE IOCTL resets the driver state
- [ ] Driver logs appear in DebugView
- [ ] No errors in Event Viewer
- [ ] Driver can be stopped cleanly
- [ ] Driver can be restarted without issues
- [ ] Driver uninstalls cleanly

## Additional Resources

- **Full Installation Guide:** [INSTALL.md](INSTALL.md)
- **Build Documentation:** [BUILD.md](BUILD.md)
- **Driver Technical Docs:** [../../docs/driver-hvfilter.md](../../docs/driver-hvfilter.md)
- **Quick Start:** [QUICKSTART.md](QUICKSTART.md)
- **Development Notes:** [../../docs/development-notes.md](../../docs/development-notes.md)

## Support

For issues or questions:
- Check the troubleshooting sections in this document
- Review driver logs in DebugView
- Check Event Viewer for system errors
- See [../../docs/driver-hvfilter.md](../../docs/driver-hvfilter.md) for detailed documentation
- File issues on GitHub: https://github.com/brunusansi/NanaBox/issues

## License

MIT License - Same as NanaBox project
