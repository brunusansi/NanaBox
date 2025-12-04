# NanaBox HvFilter Driver - Technical Documentation

## Overview

The **nanabox_hvfilter** driver is a Windows kernel-mode driver skeleton that provides the foundational infrastructure for anti-detection capabilities in NanaBox Anti-Detection Edition. This is Phase 3 of the project roadmap.

**Current Status**: Skeleton implementation only - IOCTLs and device creation are functional, but CPUID/MSR/timing/PCI interception is NOT yet implemented.

## Purpose

This driver serves as the kernel-mode component that will eventually:
- Intercept CPUID instructions to hide virtualization signatures
- Filter MSR (Model-Specific Register) access to block Hyper-V detection
- Provide timing normalization for anti-cheat compatibility
- Control PCI device topology visibility

**Important**: In this phase, the driver only handles configuration via IOCTLs and logs requests. It does NOT modify any CPU behavior or intercept anything.

## Architecture

### Components

```
drivers/nanabox_hvfilter/
├── driver.c              # DriverEntry, DriverUnload, main initialization
├── driver.h              # Main driver header with prototypes
├── device.c              # Device creation and destruction
├── dispatch.c            # IRP dispatch handlers (CREATE, CLOSE, DEVICE_CONTROL)
├── ioctl.c               # IOCTL handler implementation
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

Sets the active anti-detection profile.

**Input**: `NBX_SET_PROFILE_INPUT`
```c
typedef struct _NBX_SET_PROFILE_INPUT {
    CHAR ProfileName[64];    // Profile name (e.g., "Valorant", "BareMetal")
    DWORD Flags;             // Profile flags (bitmask)
} NBX_SET_PROFILE_INPUT;
```

**Flags**:
- `NBX_PROFILE_FLAG_CPUID` (0x00000001) - Enable CPUID spoofing
- `NBX_PROFILE_FLAG_MSR_INTERCEPT` (0x00000002) - Enable MSR interception
- `NBX_PROFILE_FLAG_TIMING` (0x00000004) - Enable timing normalization
- `NBX_PROFILE_FLAG_PCI` (0x00000008) - Enable PCI topology control

**Output**: None

**Returns**: `STATUS_SUCCESS` or error code

#### 2. `IOCTL_NBX_HVFILTER_GET_STATUS` (0x80000001)

Retrieves the current driver status and active profile.

**Input**: None

**Output**: `NBX_GET_STATUS_OUTPUT`
```c
typedef struct _NBX_GET_STATUS_OUTPUT {
    CHAR ActiveProfileName[64];  // Currently active profile name
    DWORD ActiveFlags;           // Currently active flags
    DWORD DriverVersion;         // Driver version (encoded)
    BOOL IsActive;               // TRUE if a profile is active
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

### Automated Setup (Recommended)

For Phase 3B development and testing, use the automated setup script:

```powershell
# Run as Administrator
cd scripts\setup

# Complete Phase 3B setup: build, install, start, and test
.\Setup-NbxHvFilterPhase3B.ps1

# Or use the main setup script with Phase3B flag
.\Setup-NanaBoxDevEnv.ps1 -Phase3B
```

The setup script will:
1. ✅ Validate build environment (Visual Studio, MSBuild, WDK)
2. ✅ Build nanabox_hvfilter.sys and NbxHvFilterClient.exe
3. ✅ Install or update the driver
4. ✅ Start the driver service
5. ✅ Run basic IOCTL sanity checks

See [PHASE3B_TESTING.md](../drivers/nanabox_hvfilter/PHASE3B_TESTING.md) for detailed usage and testing instructions.

### Manual Build

If you prefer to build manually:

#### Prerequisites

- **Windows 10/11** (build machine)
- **Visual Studio 2022** with "Desktop development with C++"
- **Windows Driver Kit (WDK) 10.0.22621.0** or later
- **Windows SDK 10.0.22621.0** or later

#### Installation Steps

1. Install Visual Studio 2022
2. Download and install WDK from Microsoft:
   https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk
3. Install Windows SDK (included with WDK or Visual Studio)

#### Build Commands

**Using Visual Studio:**

Open `nanabox_hvfilter.vcxproj` in Visual Studio and build the solution.

**Using Command Line:**

```cmd
cd drivers\nanabox_hvfilter
msbuild nanabox_hvfilter.vcxproj /p:Configuration=Release /p:Platform=x64
```

#### Build Output

- `nanabox_hvfilter.sys` - Driver binary
- `nanabox_hvfilter.pdb` - Debug symbols
- `nanabox_hvfilter.inf` - Installation INF file

## Installing the Driver

### Automated Installation (Recommended)

The easiest way to install and test the driver is using the Phase 3B setup script:

```powershell
# Run as Administrator
cd scripts\setup

# Automated build, install, start, and test
.\Setup-NbxHvFilterPhase3B.ps1
```

This handles all the prerequisites, installation, and testing automatically. See [PHASE3B_TESTING.md](../drivers/nanabox_hvfilter/PHASE3B_TESTING.md) for details.

### Manual Installation

If you need to install manually or troubleshoot issues:

#### Prerequisites

**WARNING**: Test-signed drivers require specific system configuration:

1. **Disable Secure Boot** in BIOS/UEFI
2. **Enable Test Signing mode** (requires reboot)

#### Enable Test Signing

Open an Administrator Command Prompt and run:

```cmd
bcdedit /set testsigning on
bcdedit /set nointegritychecks on
shutdown /r /t 0
```

After reboot, you should see "Test Mode" watermark on the desktop.

#### Create Test Certificate (First Time Only)

```cmd
makecert -r -pe -ss PrivateCertStore -n "CN=NanaBox Test Certificate" nanabox_test.cer
certmgr.exe /add nanabox_test.cer /s /r localMachine root
certmgr.exe /add nanabox_test.cer /s /r localMachine trustedpublisher
```

#### Sign the Driver

```cmd
cd drivers\nanabox_hvfilter\x64\Release
signtool sign /v /s PrivateCertStore /n "NanaBox Test Certificate" nanabox_hvfilter.sys
```

#### Install the Driver

**Method 1: Manual Installation (Recommended for Development)**

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

## Safety and Security

### What This Driver Does NOT Do (Yet)

This skeleton driver is intentionally limited in scope:

- ❌ Does NOT hook CPUID instructions
- ❌ Does NOT intercept MSR access
- ❌ Does NOT modify timing behavior
- ❌ Does NOT alter PCI topology
- ❌ Does NOT patch any system structures (IDT, SSDT, etc.)
- ❌ Does NOT modify CPU registers (CR0, CR4, etc.)

### What It Does Do

- ✅ Creates a device object and symbolic link
- ✅ Handles IOCTLs for profile management
- ✅ Logs all operations
- ✅ Validates input buffers
- ✅ Cleans up resources on unload

### Safety Measures

1. **Input Validation**: All IOCTL inputs are validated for size and NULL checks
2. **Safe String Handling**: Profile names are null-terminated and truncated if needed
3. **Error Handling**: All error paths are handled cleanly
4. **No Memory Leaks**: Device objects and symbolic links are properly cleaned up
5. **Buffered I/O**: Uses `METHOD_BUFFERED` for safe data transfer

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

This driver skeleton will be extended in future phases to implement:

1. **CPUID Interception** (Phase 3)
   - Hook CPUID instruction execution
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
