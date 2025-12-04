# Phase 3B Testing Guide

## Overview

This guide explains how to test the Phase 3B implementation of CPUID and MSR policy configuration in the nanabox_hvfilter driver.

**Important**: Phase 3B provides the configuration framework only. Actual CPU/MSR interception requires Phase 3C+ implementation.

## Prerequisites

- Windows 10/11 with WDK installed
- Test-signing enabled (Secure Boot disabled)
- Administrator privileges
- DebugView or similar kernel log viewer

## Building the Driver

### Option 1: Visual Studio

1. Open `nanabox_hvfilter.vcxproj` in Visual Studio 2022
2. Select `Release` or `Debug` configuration
3. Select `x64` platform
4. Build the solution (Ctrl+Shift+B)

### Option 2: Command Line

```cmd
cd drivers\nanabox_hvfilter
msbuild nanabox_hvfilter.vcxproj /p:Configuration=Release /p:Platform=x64
```

### Build the User-Mode Helper

```cmd
cd tools\NbxHvFilterClient
msbuild NbxHvFilterClient.vcxproj /p:Configuration=Release /p:Platform=x64
```

## Installing and Loading the Driver

### 1. Enable Test Signing (One-Time Setup)

```cmd
bcdedit /set testsigning on
bcdedit /set nointegritychecks on
shutdown /r /t 0
```

After reboot, you should see "Test Mode" watermark.

### 2. Sign the Driver

```cmd
cd drivers\nanabox_hvfilter\x64\Release

# Create test certificate (first time only)
makecert -r -pe -ss PrivateCertStore -n "CN=NanaBox Test" nanabox_test.cer
certmgr.exe /add nanabox_test.cer /s /r localMachine root
certmgr.exe /add nanabox_test.cer /s /r localMachine trustedpublisher

# Sign the driver
signtool sign /v /s PrivateCertStore /n "NanaBox Test" nanabox_hvfilter.sys
```

### 3. Install the Driver

```cmd
# Copy driver to system directory
copy nanabox_hvfilter.sys C:\Windows\System32\drivers\

# Create the service
sc create NanaBoxHvFilter type= kernel binPath= C:\Windows\System32\drivers\nanabox_hvfilter.sys

# Start the service
sc start NanaBoxHvFilter
```

### 4. Verify Driver is Running

```cmd
sc query NanaBoxHvFilter
```

Should show `STATE: 4 RUNNING`.

## Testing Phase 3B Features

### Test 1: Basic Status Query

```cmd
NbxHvFilterClient.exe status
```

**Expected Output:**
```
===============================================================
NanaBox HvFilter Client v1.0.0
===============================================================

[INFO] Sending GET_STATUS request...
[SUCCESS] Status retrieved successfully (Phase 3B)
       Driver Version: 1.0.0
       Active Profile: None
       Active Flags: 0x00000000
       Is Active: No
       CPUID Policy: Disabled
       MSR Policy: Disabled
```

### Test 2: Load Roblox Profile

```cmd
NbxHvFilterClient.exe set roblox
```

**Expected Output:**
```
[INFO] Using Roblox (Byfron) profile - balanced anti-detection
[INFO] Sending SET_PROFILE request (Phase 3B)...
       Profile: roblox
       Flags: 0x00000003
         - CPUID: Yes
         - MSR Intercept: Yes
         - Timing: No
         - PCI: No
       CPUID Policy:
         - Hide Hypervisor: Yes
         - Mask Virtualization: Yes
         - Vendor String: 'AuthenticAMD'
       MSR Policy:
         - Hyper-V MSR Mode: ZERO
[SUCCESS] Profile set successfully
[INFO] Check driver logs (DebugView) for detailed configuration
```

**Check DebugView Logs:**
Look for messages like:
```
[NanaBoxHvFilter] NbxHandleSetProfile: ProfileName='roblox', Flags=0x00000003
[NanaBoxHvFilter] NbxHandleSetProfile: Activating CPUID interception
[NanaBoxHvFilter]   - HideHypervisor: Yes
[NanaBoxHvFilter]   - MaskVirtualization: Yes
[NanaBoxHvFilter]   - VendorString: 'AuthenticAMD'
[NanaBoxHvFilter] NbxActivateCpuIdInterception: Configuring CPUID interception (Phase 3B - Documentation only)
[NanaBoxHvFilter]   [PHASE 3B] CPUID configuration stored (not yet intercepting):
[NanaBoxHvFilter]     - Hide Hypervisor Bit (CPUID.1.ECX[31]): YES
[NanaBoxHvFilter]     - Mask Virtualization Features (VMX/SVM): YES
[NanaBoxHvFilter]     - CPU Vendor Override: 'AuthenticAMD'
[NanaBoxHvFilter]   [PHASE 3B] CPUID interception NOT yet implemented - configuration stored only
[NanaBoxHvFilter] NbxHandleSetProfile: Activating MSR interception
[NanaBoxHvFilter]   - HyperVMsrMode: 1
[NanaBoxHvFilter] NbxActivateMsrInterception: Configuring MSR interception (Phase 3B - Documentation only)
[NanaBoxHvFilter]   [PHASE 3B] MSR configuration stored (not yet intercepting):
[NanaBoxHvFilter]     - Hyper-V MSR Range (0x40000000-0x400000FF) Mode: ZERO (return 0 for reads, ignore writes)
[NanaBoxHvFilter]   [PHASE 3B] MSR interception NOT yet implemented - configuration stored only
```

### Test 3: Query Status with Active Profile

```cmd
NbxHvFilterClient.exe status
```

**Expected Output:**
```
[SUCCESS] Status retrieved successfully (Phase 3B)
       Driver Version: 1.0.0
       Active Profile: roblox
       Active Flags: 0x00000003
         - CPUID: Yes
         - MSR Intercept: Yes
         - Timing: No
         - PCI: No
       Is Active: Yes
       CPUID Policy:
         - Enabled: Yes
         - Hide Hypervisor: Yes
         - Mask Virtualization: Yes
         - Vendor String: 'AuthenticAMD'
       MSR Policy:
         - Enabled: Yes
         - Hyper-V MSR Mode: ZERO (return 0)

[INFO] Phase 3B: Configuration framework ready
[INFO] Actual CPU/MSR interception requires Phase 3C implementation
[INFO] Check driver logs (DebugView) for detailed policy application
```

### Test 4: Load Valorant Profile

```cmd
NbxHvFilterClient.exe set valorant
```

**Expected Output:**
```
[INFO] Using Valorant (Riot Vanguard) profile - bare-metal anti-detection
[INFO] Sending SET_PROFILE request (Phase 3B)...
       Profile: valorant
       Flags: 0x00000003
         - CPUID: Yes
         - MSR Intercept: Yes
       CPUID Policy:
         - Hide Hypervisor: Yes
         - Mask Virtualization: Yes
         - Vendor String: 'GenuineIntel'
       MSR Policy:
         - Hyper-V MSR Mode: ZERO
[SUCCESS] Profile set successfully
```

**Note**: Valorant uses `GenuineIntel` vendor string, while Roblox uses `AuthenticAMD`.

### Test 5: Load Tencent ACE Profile

```cmd
NbxHvFilterClient.exe set expert-tencent
```

**Expected Output:**
```
[INFO] Using Tencent ACE profile - bare-metal anti-detection
[INFO] Sending SET_PROFILE request (Phase 3B)...
       Profile: expert-tencent
       Flags: 0x00000003
         - CPUID: Yes
         - MSR Intercept: Yes
       CPUID Policy:
         - Hide Hypervisor: Yes
         - Mask Virtualization: Yes
         - Vendor String: 'GenuineIntel'
       MSR Policy:
         - Hyper-V MSR Mode: ZERO
[SUCCESS] Profile set successfully
```

### Test 6: Clear Profile

```cmd
NbxHvFilterClient.exe clear
```

**Expected Output:**
```
[INFO] Sending CLEAR_PROFILE request...
[SUCCESS] Profile cleared successfully
```

**Check DebugView Logs:**
```
[NanaBoxHvFilter] NbxHandleClearProfile: Clearing active profile
[NanaBoxHvFilter] NbxHandleClearProfile: Deactivating CPUID interception
[NanaBoxHvFilter] NbxDeactivateCpuIdInterception: Deactivating CPUID interception
[NanaBoxHvFilter]   [PHASE 3B] CPUID policy cleared
[NanaBoxHvFilter] NbxHandleClearProfile: Deactivating MSR interception
[NanaBoxHvFilter] NbxDeactivateMsrInterception: Deactivating MSR interception
[NanaBoxHvFilter]   [PHASE 3B] MSR policy cleared
```

### Test 7: Custom Profile with Manual Flags

```cmd
NbxHvFilterClient.exe set CustomTest 0x00000003
```

**Expected Output:**
```
[INFO] Sending SET_PROFILE request (Phase 3B)...
       Profile: CustomTest
       Flags: 0x00000003
         - CPUID: Yes
         - MSR Intercept: Yes
       CPUID Policy:
         - Hide Hypervisor: Yes
         - Mask Virtualization: Yes
         - Vendor String: (use host)
       MSR Policy:
         - Hyper-V MSR Mode: ZERO
[SUCCESS] Profile set successfully
```

**Note**: Custom profiles use default policies with no vendor string override.

### Test 8: Switch Between Profiles

```cmd
# Load Roblox profile
NbxHvFilterClient.exe set roblox

# Check status
NbxHvFilterClient.exe status

# Switch to Valorant profile
NbxHvFilterClient.exe set valorant

# Check status again
NbxHvFilterClient.exe status
```

**Expected Behavior:**
- Driver should cleanly deactivate Roblox policies before activating Valorant
- DebugView should show deactivation messages followed by activation messages
- Status should reflect the new profile and policies

## Uninstalling the Driver

When done testing:

```cmd
# Stop the driver
sc stop NanaBoxHvFilter

# Delete the service
sc delete NanaBoxHvFilter

# Remove the driver file
del C:\Windows\System32\drivers\nanabox_hvfilter.sys
```

## Troubleshooting

### Driver Fails to Load

**Symptom**: `sc start NanaBoxHvFilter` fails with error.

**Solutions**:
- Ensure test signing is enabled
- Verify driver is signed
- Check Secure Boot is disabled
- Review Event Viewer (System logs)

### Cannot Open Device

**Symptom**: `NbxHvFilterClient.exe` shows "Failed to open driver device".

**Solutions**:
- Verify driver is running: `sc query NanaBoxHvFilter`
- Check DebugView for driver errors
- Ensure running as Administrator

### No Logs in DebugView

**Symptom**: DebugView shows no logs from driver.

**Solutions**:
- Enable "Capture Kernel" in DebugView menu
- Run DebugView as Administrator
- Verify driver is loaded

## Known Limitations (Phase 3B)

1. **No Actual CPUID Interception**: Phase 3B only stores the configuration. CPUID instructions are not intercepted.
2. **No Actual MSR Filtering**: Phase 3B only stores the configuration. MSR reads/writes are not filtered.
3. **Guest-Level Constraints**: Running inside Hyper-V guest makes true interception extremely difficult without hypervisor cooperation.
4. **Phase 3C Required**: Actual CPU-level interception requires Phase 3C implementation with:
   - Hyper-V enlightenment integration OR
   - Host-side WHP API implementation OR
   - Custom hypervisor module

## Success Criteria for Phase 3B

- ✅ Driver loads without errors
- ✅ IOCTL communication works
- ✅ Profiles can be loaded and queried
- ✅ Policies are validated and stored
- ✅ Detailed logging shows configuration
- ✅ Profiles can be switched cleanly
- ✅ Driver unloads without crashes
- ⚠️ Actual CPU behavior unchanged (expected for Phase 3B)

## Next Steps

Phase 3C will implement actual CPUID and MSR interception:
- Research Hyper-V enlightenment APIs
- Investigate WHP (Windows Hypervisor Platform) integration
- Consider host-side implementation for better control
- Implement actual CPU instruction hooks
- Test with detection tools (Pafish, Al-Khaser)

## Support

For issues or questions:
- Check driver logs in DebugView
- Review documentation in `docs/driver-hvfilter.md`
- Open GitHub issue with logs and reproduction steps
