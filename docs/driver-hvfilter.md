# NanaBox HvFilter Driver - Technical Documentation

## Overview

The **nanabox_hvfilter** driver is a Windows kernel-mode driver that provides the foundational infrastructure for anti-detection capabilities in NanaBox Anti-Detection Edition. This is Phase 3 of the project roadmap.

**Current Status (Phase 3B)**: 
- ✅ IOCTLs and device creation functional
- ✅ CPUID policy configuration and validation
- ✅ MSR policy configuration and validation
- ✅ Profile activation/deactivation framework
- ⚠️ Actual CPUID/MSR interception NOT yet implemented (requires hypervisor-level access)

## Purpose

This driver serves as the kernel-mode component that:
- **Currently (Phase 3B)**: Receives, validates, and stores CPUID/MSR policies from VM configuration
- **Future (Phase 3C+)**: Will intercept CPUID instructions to hide virtualization signatures
- **Future (Phase 3C+)**: Will filter MSR (Model-Specific Register) access to block Hyper-V detection
- **Future (Phase 4)**: Will provide timing normalization for anti-cheat compatibility
- **Future (Phase 4)**: Will control PCI device topology visibility

**Important**: Phase 3B focuses on configuration management and provides a safe framework for future interception. Actual CPU-level interception requires hypervisor cooperation and will be implemented in Phase 3C.

## Architecture

### Components

```
drivers/nanabox_hvfilter/
├── driver.c              # DriverEntry, DriverUnload, main initialization
├── driver.h              # Main driver header with prototypes
├── device.c              # Device creation and destruction
├── dispatch.c            # IRP dispatch handlers (CREATE, CLOSE, DEVICE_CONTROL)
├── ioctl.c               # IOCTL handler implementation
├── cpuid_msr.c           # CPUID and MSR interception (Phase 3B+)
├── NbxHvFilterShared.h   # Shared definitions (user-mode and kernel-mode)
├── nanabox_hvfilter.inf  # Driver installation script
└── nanabox_hvfilter.vcxproj  # Visual Studio project file

tools/NbxHvFilterClient/
├── NbxHvFilterClient.cpp      # User-mode helper utility
└── NbxHvFilterClient.vcxproj  # Visual Studio project file
```

### Device Information

- **Device Name**: `\Device\NanaBoxHvFilter`
- **Symbolic Link**: `\DosDevices\NanaBoxHvFilter`
- **User-mode Access**: `\\.\NanaBoxHvFilter`

## IOCTL Interface

### IOCTL Codes

The driver exposes three IOCTLs for configuration management:

#### 1. `IOCTL_NBX_HVFILTER_SET_PROFILE` (0x80000000)

Sets the active anti-detection profile with CPUID and MSR policies.

**Input**: `NBX_SET_PROFILE_INPUT` (Extended in Phase 3B)
```c
typedef struct _NBX_CPUID_POLICY {
    BOOL Enabled;                        // Enable CPUID interception
    BOOL HideHypervisor;                 // Hide hypervisor bit (CPUID.1.ECX[31])
    BOOL MaskVirtualizationFeatures;     // Mask VMX/SVM features
    CHAR VendorString[13];               // CPU vendor override ("GenuineIntel" or "AuthenticAMD")
} NBX_CPUID_POLICY;

typedef struct _NBX_MSR_POLICY {
    BOOL Enabled;                        // Enable MSR interception
    DWORD HyperVMsrMode;                 // Hyper-V MSR handling mode
} NBX_MSR_POLICY;

typedef struct _NBX_SET_PROFILE_INPUT {
    CHAR ProfileName[64];                // Profile name (e.g., "roblox", "valorant")
    DWORD Flags;                         // Profile flags (bitmask)
    NBX_CPUID_POLICY CpuIdPolicy;        // CPUID policy (Phase 3B)
    NBX_MSR_POLICY MsrPolicy;            // MSR policy (Phase 3B)
} NBX_SET_PROFILE_INPUT;
```

**Flags**:
- `NBX_PROFILE_FLAG_CPUID` (0x00000001) - Enable CPUID spoofing
- `NBX_PROFILE_FLAG_MSR_INTERCEPT` (0x00000002) - Enable MSR interception
- `NBX_PROFILE_FLAG_TIMING` (0x00000004) - Enable timing normalization
- `NBX_PROFILE_FLAG_PCI` (0x00000008) - Enable PCI topology control

**MSR Modes**:
- `NBX_MSR_MODE_PASSTHROUGH` (0) - Pass through to host
- `NBX_MSR_MODE_ZERO` (1) - Return zero for reads
- `NBX_MSR_MODE_BLOCK` (2) - Block access (return error)

**Output**: None

**Returns**: `STATUS_SUCCESS` or error code

**Phase 3B Behavior**: Validates and stores the policy configuration. Logs what interception would be applied. Actual CPU/MSR interception requires Phase 3C+ implementation.

#### 2. `IOCTL_NBX_HVFILTER_GET_STATUS` (0x80000001)

Retrieves the current driver status, active profile, and policy configuration.

**Input**: None

**Output**: `NBX_GET_STATUS_OUTPUT` (Extended in Phase 3B)
```c
typedef struct _NBX_GET_STATUS_OUTPUT {
    CHAR ActiveProfileName[64];          // Currently active profile name
    DWORD ActiveFlags;                   // Currently active flags
    DWORD DriverVersion;                 // Driver version (encoded)
    BOOL IsActive;                       // TRUE if a profile is active
    NBX_CPUID_POLICY CpuIdPolicy;        // Active CPUID policy (Phase 3B)
    NBX_MSR_POLICY MsrPolicy;            // Active MSR policy (Phase 3B)
} NBX_GET_STATUS_OUTPUT;
```

**Returns**: `STATUS_SUCCESS` or error code

#### 3. `IOCTL_NBX_HVFILTER_CLEAR_PROFILE` (0x80000002)

Clears the active profile and resets the driver to default state.

**Input**: None

**Output**: None

**Returns**: `STATUS_SUCCESS` or error code

### Error Codes

- `STATUS_SUCCESS` (0x00000000) - Operation succeeded
- `STATUS_BUFFER_TOO_SMALL` - Input/output buffer is too small
- `STATUS_INVALID_PARAMETER` - Invalid parameter provided
- `STATUS_INVALID_DEVICE_REQUEST` - Unknown IOCTL code

## Building the Driver

### Prerequisites

- **Windows 10/11** (build machine)
- **Visual Studio 2022** with "Desktop development with C++"
- **Windows Driver Kit (WDK) 10.0.22621.0** or later
- **Windows SDK 10.0.22621.0** or later

### Installation Steps

1. Install Visual Studio 2022
2. Download and install WDK from Microsoft:
   https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk
3. Install Windows SDK (included with WDK or Visual Studio)

### Build Commands

#### Using Visual Studio

Open `nanabox_hvfilter.vcxproj` in Visual Studio and build the solution.

#### Using Command Line

```cmd
cd drivers\nanabox_hvfilter
msbuild nanabox_hvfilter.vcxproj /p:Configuration=Release /p:Platform=x64
```

### Build Output

- `nanabox_hvfilter.sys` - Driver binary
- `nanabox_hvfilter.pdb` - Debug symbols
- `nanabox_hvfilter.inf` - Installation INF file

## Installing the Driver

### Prerequisites

**WARNING**: Test-signed drivers require specific system configuration:

1. **Disable Secure Boot** in BIOS/UEFI
2. **Enable Test Signing mode** (requires reboot)

### Enable Test Signing

Open an Administrator Command Prompt and run:

```cmd
bcdedit /set testsigning on
bcdedit /set nointegritychecks on
shutdown /r /t 0
```

After reboot, you should see "Test Mode" watermark on the desktop.

### Create Test Certificate (First Time Only)

```cmd
makecert -r -pe -ss PrivateCertStore -n "CN=NanaBox Test Certificate" nanabox_test.cer
certmgr.exe /add nanabox_test.cer /s /r localMachine root
certmgr.exe /add nanabox_test.cer /s /r localMachine trustedpublisher
```

### Sign the Driver

```cmd
cd drivers\nanabox_hvfilter\x64\Release
signtool sign /v /s PrivateCertStore /n "NanaBox Test Certificate" nanabox_hvfilter.sys
```

### Install the Driver

#### Method 1: Manual Installation (Recommended for Development)

```cmd
# Copy driver to system directory
copy nanabox_hvfilter.sys C:\Windows\System32\drivers\

# Create the service
sc create NanaBoxHvFilter type= kernel binPath= C:\Windows\System32\drivers\nanabox_hvfilter.sys

# Start the service
sc start NanaBoxHvFilter
```

#### Method 2: Using INF File

```cmd
# Install using Device Manager
pnputil /add-driver nanabox_hvfilter.inf /install
```

### Verify Installation

```cmd
# Check if driver is loaded
sc query NanaBoxHvFilter

# Check driver logs
# Use DbgView or similar tool to view kernel debug output
```

## Using the User-Mode Helper

### Build the Helper

```cmd
cd tools\NbxHvFilterClient
msbuild NbxHvFilterClient.vcxproj /p:Configuration=Release /p:Platform=x64
```

### Usage Examples

#### Set a Profile

```cmd
NbxHvFilterClient.exe set Valorant 0x00000003
```

This sets the "Valorant" profile with CPUID (0x1) and MSR Intercept (0x2) flags enabled.

#### Get Current Status

```cmd
NbxHvFilterClient.exe status
```

Output:
```
===============================================================
NanaBox HvFilter Client v1.0.0
===============================================================

[INFO] Sending GET_STATUS request...
[SUCCESS] Status retrieved successfully
       Driver Version: 1.0.0
       Active Profile: Valorant
       Active Flags: 0x00000003
         - CPUID: Yes
         - MSR Intercept: Yes
         - Timing: No
         - PCI: No
       Is Active: Yes
```

#### Clear Profile

```cmd
NbxHvFilterClient.exe clear
```

### Integration with VM Configuration

The user-mode helper can be integrated with NanaBox's configuration system:

```cpp
// Example: Load profile from VM configuration
NanaBox::VirtualMachineConfiguration vmConfig;
// ... load config from JSON ...

// Build flags from configuration
DWORD flags = 0;
if (vmConfig.CpuId.Enabled) {
    flags |= NBX_PROFILE_FLAG_CPUID;
}
if (vmConfig.MsrIntercept.Enabled) {
    flags |= NBX_PROFILE_FLAG_MSR_INTERCEPT;
}
if (vmConfig.Timing.Enabled) {
    flags |= NBX_PROFILE_FLAG_TIMING;
}
if (vmConfig.Pci.Enabled) {
    flags |= NBX_PROFILE_FLAG_PCI;
}

// Send to driver
SetProfile(hDevice, profileName, flags);
```

## Logging and Diagnostics

### Viewing Driver Logs

The driver uses `KdPrintEx` for logging with the prefix `[NanaBoxHvFilter]`.

#### Using DebugView (Sysinternals)

1. Download DebugView from Microsoft
2. Run as Administrator
3. Enable "Capture Kernel" in the menu
4. View logs in real-time

#### Using WinDbg

```cmd
# Enable kernel debugging (requires reboot)
bcdedit /debug on
bcdedit /dbgsettings serial debugport:1 baudrate:115200

# Connect WinDbg and view logs
```

### Log Messages

- **DriverEntry**: Driver initialization
- **DriverUnload**: Driver cleanup
- **NbxDispatchCreate**: Device opened by user-mode application
- **NbxDispatchClose**: Device closed
- **NbxDispatchDeviceControl**: IOCTL received
- **NbxHandleSetProfile**: Profile set with details
- **NbxHandleGetStatus**: Status retrieved
- **NbxHandleClearProfile**: Profile cleared
- **NbxActivateCpuIdInterception**: CPUID policy configured (Phase 3B)
- **NbxActivateMsrInterception**: MSR policy configured (Phase 3B)
- **NbxDeactivateCpuIdInterception**: CPUID policy cleared
- **NbxDeactivateMsrInterception**: MSR policy cleared

## Phase 3B: CPUID and MSR Configuration

### Current Implementation (Phase 3B)

Phase 3B provides the configuration framework for CPUID and MSR interception:

**What Phase 3B Does**:
- ✅ Receives CPUID and MSR policies from VM configuration
- ✅ Validates policy configuration (vendor strings, MSR modes, etc.)
- ✅ Stores policies in driver context
- ✅ Logs detailed configuration information
- ✅ Provides activation/deactivation framework
- ✅ Safe cleanup on profile changes or driver unload

**What Phase 3B Does NOT Do**:
- ⚠️ Does NOT actually intercept CPUID instructions
- ⚠️ Does NOT actually filter MSR access
- ⚠️ Does NOT modify CPU behavior

### Why Actual Interception is Deferred to Phase 3C

Running as a guest driver inside a Hyper-V VM presents significant technical challenges:

1. **CPUID Interception**: Guest OS drivers cannot directly intercept CPUID instructions without hypervisor cooperation. This requires either:
   - Hyper-V enlightenment integration
   - Host-side implementation using Windows Hypervisor Platform (WHP) API
   - Custom hypervisor module

2. **MSR Filtering**: Guest-level MSR access is already controlled by the hypervisor. Effective MSR filtering requires:
   - Hyper-V MSR intercept registration
   - Host-side WHP MSR filtering
   - Special Hyper-V enlightenments

3. **Safety First**: Phase 3B establishes the safe configuration framework before attempting complex CPU-level operations.

### Testing Profile Configuration (Phase 3B)

You can test that profile configuration is working correctly:

1. **Load a profile** with the user-mode helper:
   ```cmd
   NbxHvFilterClient.exe set roblox 0x00000003
   ```

2. **Check driver logs** (using DebugView):
   - Look for `[PHASE 3B] CPUID configuration stored`
   - Look for `[PHASE 3B] MSR configuration stored`
   - Verify Hide Hypervisor, Vendor String, MSR Mode settings

3. **Query status**:
   ```cmd
   NbxHvFilterClient.exe status
   ```
   Should show CPUID and MSR policies

4. **Clear profile**:
   ```cmd
   NbxHvFilterClient.exe clear
   ```
   Should see deactivation messages in logs

### Profile Examples for Testing

#### Roblox (Byfron) Profile
```c
CpuIdPolicy.Enabled = TRUE
CpuIdPolicy.HideHypervisor = TRUE
CpuIdPolicy.MaskVirtualizationFeatures = TRUE
CpuIdPolicy.VendorString = "AuthenticAMD"

MsrPolicy.Enabled = TRUE
MsrPolicy.HyperVMsrMode = NBX_MSR_MODE_ZERO
```

#### Valorant (Riot Vanguard) Profile
```c
CpuIdPolicy.Enabled = TRUE
CpuIdPolicy.HideHypervisor = TRUE
CpuIdPolicy.MaskVirtualizationFeatures = TRUE
CpuIdPolicy.VendorString = "GenuineIntel"

MsrPolicy.Enabled = TRUE
MsrPolicy.HyperVMsrMode = NBX_MSR_MODE_ZERO
```

## Safety and Security

### What This Driver Does (Phase 3B)

- ✅ Creates a device object and symbolic link
- ✅ Handles IOCTLs for profile management
- ✅ Validates and stores CPUID/MSR policies
- ✅ Logs all operations with detailed policy information
- ✅ Validates input buffers and policy parameters
- ✅ Cleans up resources on unload
- ✅ Safe activation/deactivation of configuration

### What This Driver Does NOT Do (Phase 3B)

- ❌ Does NOT hook CPUID instructions (Phase 3C+)
- ❌ Does NOT intercept MSR access (Phase 3C+)
- ❌ Does NOT modify timing behavior (Phase 4)
- ❌ Does NOT alter PCI topology (Phase 4)
- ❌ Does NOT patch any system structures (IDT, SSDT, etc.)
- ❌ Does NOT modify CPU registers (CR0, CR4, etc.)

### Safety Measures

1. **Input Validation**: All IOCTL inputs validated for size, NULL checks, and valid values
2. **Policy Validation**: Vendor strings, MSR modes, and flags validated before storing
3. **Safe String Handling**: Profile names and vendor strings null-terminated and truncated
4. **Error Handling**: All error paths handled cleanly with appropriate logging
5. **No Memory Leaks**: Device objects, symbolic links, and contexts properly cleaned up
6. **Buffered I/O**: Uses `METHOD_BUFFERED` for safe data transfer
7. **Conservative Approach**: No CPU modification in Phase 3B, only configuration management

## Troubleshooting

### Driver Fails to Load

**Error**: "The hash for the file is not present in the specified catalog file."

**Solution**: 
- Ensure test signing is enabled
- Sign the driver with a test certificate
- Verify Secure Boot is disabled

### Cannot Open Device

**Error**: "Failed to open driver device" (Error 2 - File Not Found)

**Solution**:
- Verify driver is loaded: `sc query NanaBoxHvFilter`
- Check device creation in DebugView logs
- Ensure driver loaded successfully without errors

### IOCTL Fails

**Error**: "Failed to set profile" (Error 50 - Not Supported)

**Solution**:
- This might indicate an unknown IOCTL code
- Verify you're using the correct IOCTL codes
- Check driver logs for details

## Next Steps (Future Phases)

The configuration framework established in Phase 3B will be extended in future phases:

1. **CPUID Interception** (Phase 3C)
   - Implement actual CPUID instruction interception
   - Consider host-side implementation using WHP API
   - Spoof CPU vendor strings
   - Hide hypervisor bit (CPUID.1.ECX[31])
   - Mask virtualization features

2. **MSR Interception** (Phase 3)
   - Hook RDMSR/WRMSR instructions
   - Block Hyper-V MSR range (0x40000000-0x400000FF)
   - Provide fake MSR values

3. **Timing Normalization** (Phase 4)
   - Normalize TSC (Time Stamp Counter)
   - Adjust APIC timer behavior
   - Stabilize QPC (QueryPerformanceCounter)

4. **PCI Topology Control** (Phase 4)
   - Hide Hyper-V PCI devices
   - Spoof PCI device IDs
   - Present realistic device topology

## References

- [Windows Driver Kit Documentation](https://learn.microsoft.com/en-us/windows-hardware/drivers/)
- [Kernel-Mode Driver Architecture](https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/)
- [Driver Signing Requirements](https://learn.microsoft.com/en-us/windows-hardware/drivers/install/driver-signing)
- [IOCTL Programming](https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/introduction-to-i-o-control-codes)

## License

MIT License - Copyright (c) NanaBox Anti-Detection Edition Contributors
