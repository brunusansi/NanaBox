# NanaBox Anti-Detection Edition - Kernel Drivers (Phase 3)

This directory contains kernel-mode drivers for guest-side anti-detection capabilities.

## Overview

The NanaBox anti-detection drivers run **inside the guest VM** to intercept and modify CPUID instructions and MSR (Model-Specific Register) access, hiding virtualization signatures from anti-cheat systems.

## Current Status: Phase 3A (Skeleton) - ✅ COMPLETE

**What's Implemented**:
- ✅ Driver device creation and symbolic link (`\Device\NanaBoxHvFilter`)
- ✅ IRP dispatch routines (CREATE, CLOSE, DEVICE_CONTROL)
- ✅ IOCTL interface (SET_PROFILE, GET_STATUS, CLEAR_PROFILE)
- ✅ User-mode helper utility (NbxHvFilterClient.exe)
- ✅ Safe input validation and error handling
- ✅ Comprehensive logging and diagnostics

**What's NOT Implemented (Phase 3B)**:
- ❌ CPUID instruction interception
- ❌ MSR read/write filtering
- ❌ Timing normalization
- ❌ PCI topology control

**Safety**: This skeleton driver does NOT modify any CPU behavior or system structures. It only provides the IOCTL interface for configuration management.

## Drivers

### nanabox_hvfilter.sys
**Unified Hypervisor Filter Driver (Skeleton)**

- **Purpose**: IOCTL-based configuration interface for anti-detection profiles
- **Type**: Kernel-mode driver (WDM)
- **OS Support**: Windows 10 1809+, Windows 11
- **Architecture**: x64 only
- **Current Phase**: 3A (Skeleton Only)

**Implemented Features**:
- ✅ Device creation and IOCTL handling
- ✅ Profile management (set/get/clear)
- ✅ Logging with KdPrintEx
- ✅ Safe error handling

**Planned Features (Phase 3B)**:
- ⏳ CPUID instruction interception and spoofing
- ⏳ MSR read/write (RDMSR/WRMSR) interception
- ⏳ Hyper-V hypervisor bit hiding (CPUID.1.ECX[31])
- ⏳ Hyper-V MSR range blocking (0x40000000-0x400000FF)
- ⏳ Profile-based configuration loading
- ⏳ Per-process filtering (optional)

## Build Requirements

### Software
- **Windows 10/11** (build environment)
- **Visual Studio 2022** with Desktop Development with C++
- **Windows Driver Kit (WDK) 10.0.22621.0** or later
- **Windows SDK 10.0.22621.0** or later

### Installation
1. Install Visual Studio 2022
2. Install WDK from: https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk
3. Install Windows SDK (included with WDK or VS)

## Building

### Method 1: Visual Studio (Recommended)
```cmd
cd drivers\nanabox_hvfilter
msbuild nanabox_hvfilter.vcxproj /p:Configuration=Release /p:Platform=x64
```

### Method 2: Command Line (WDK Build Environment)
```cmd
cd drivers\nanabox_hvfilter
set EWDK_DIR=C:\EWDK
call %EWDK_DIR%\BuildEnv\SetupBuildEnv.cmd
msbuild /t:build /p:Configuration=Release /p:Platform=x64
```

### Build Output
```
drivers/
└── nanabox_hvfilter/
    ├── x64/
    │   └── Release/
    │       ├── nanabox_hvfilter.sys       # Driver binary
    │       ├── nanabox_hvfilter.inf       # Installation INF
    │       ├── nanabox_hvfilter.pdb       # Debug symbols
    │       └── nanabox_hvfilter.cat       # Catalog file
    └── ...
```

## Test Signing

During development, drivers must be test-signed since they're not signed by a trusted CA.

### Enable Test Signing (Administrator Command Prompt)
```cmd
bcdedit /set testsigning on
bcdedit /set nointegritychecks on
shutdown /r /t 0
```

### Disable Secure Boot
- Restart and enter BIOS/UEFI
- Disable Secure Boot
- Save and reboot

### Sign Driver (Self-Signed Certificate)
```cmd
makecert -r -pe -ss PrivateCertStore -n "CN=NanaBox Test Certificate" nanabox_test.cer
certmgr.exe /add nanabox_test.cer /s /r localMachine root
certmgr.exe /add nanabox_test.cer /s /r localMachine trustedpublisher
inf2cat /driver:. /os:10_X64
signtool sign /v /s PrivateCertStore /n "NanaBox Test Certificate" /t http://timestamp.digicert.com nanabox_hvfilter.sys
```

## Installation

### Method 1: Manual Installation
```cmd
# Copy driver files to system directory
copy nanabox_hvfilter.sys C:\Windows\System32\drivers\
copy nanabox_hvfilter.inf C:\Windows\INF\

# Install driver
pnputil /add-driver nanabox_hvfilter.inf /install

# Start driver service
sc create NanaBoxHvFilter type= kernel binPath= C:\Windows\System32\drivers\nanabox_hvfilter.sys
sc start NanaBoxHvFilter
```

### Method 2: INF Installation
```cmd
# Right-click nanabox_hvfilter.inf and select "Install"
# Or use command line:
pnputil /add-driver nanabox_hvfilter.inf /install
rundll32.exe setupapi.dll,InstallHinfSection DefaultInstall 132 nanabox_hvfilter.inf
```

### Method 3: DevCon (Driver Developer Tool)
```cmd
devcon install nanabox_hvfilter.inf Root\NanaBoxHvFilter
```

## Usage

### Load Driver
```cmd
sc start NanaBoxHvFilter
```

### Configure CPUID Profile
```powershell
$device = [System.IO.File]::Open("\\\\.\\NanaBoxHvFilter", [System.IO.FileMode]::Open, [System.IO.FileAccess]::ReadWrite)
# ... send IOCTL_NBX_SET_CPUID_PROFILE with profile data
$device.Close()
```

### Configure MSR Profile
```powershell
$device = [System.IO.File]::Open("\\\\.\\NanaBoxHvFilter", [System.IO.FileMode]::Open, [System.IO.FileAccess]::ReadWrite)
# ... send IOCTL_NBX_SET_MSR_PROFILE with profile data
$device.Close()
```

### Query Driver Status
```cmd
# Using custom tool (to be implemented)
NanaBoxCtl.exe status
```

### Unload Driver
```cmd
sc stop NanaBoxHvFilter
sc delete NanaBoxHvFilter
```

## IOCTLs

| IOCTL | Code | Description |
|-------|------|-------------|
| `IOCTL_NBX_GET_VERSION` | 0x8000 | Get driver version |
| `IOCTL_NBX_SET_CPUID_PROFILE` | 0x8001 | Load CPUID interception profile |
| `IOCTL_NBX_SET_MSR_PROFILE` | 0x8002 | Load MSR interception profile |
| `IOCTL_NBX_RESET_PROFILE` | 0x8003 | Reset to default (no interception) |
| `IOCTL_NBX_GET_STATUS` | 0x8004 | Get driver status and statistics |
| `IOCTL_NBX_BIND_TO_PROCESS` | 0x8005 | Bind interception to specific process |
| `IOCTL_NBX_UNBIND_PROCESS` | 0x8006 | Unbind process filtering |

## Profiles

Pre-configured profiles are available in `profiles/cpuid/` and `profiles/msr/`:

| Profile | Target | CPUID | MSR | Criticality |
|---------|--------|-------|-----|-------------|
| `default-gaming-safe` | Generic | Yes | Basic | Low |
| `valorant-safe` | Riot Vanguard | Yes | Strict | **CRITICAL** |
| `eac-safe` | Easy Anti-Cheat | Yes | Medium | Medium-High |
| `battlEye-safe` | BattlEye | Yes | Medium | High |
| `faceit-safe` | FACEIT AC | Yes | Medium | Medium-High |

### Loading a Profile
1. Read profile JSON from `profiles/cpuid/valorant-safe.json` and `profiles/msr/valorant-safe.json`
2. Convert to `NBX_CPUID_PROFILE` and `NBX_MSR_PROFILE` structures
3. Send via `IOCTL_NBX_SET_CPUID_PROFILE` and `IOCTL_NBX_SET_MSR_PROFILE`
4. Verify with `IOCTL_NBX_GET_STATUS`

## Testing

### Driver Verifier
```cmd
# Enable for NanaBoxHvFilter
verifier /standard /driver nanabox_hvfilter.sys

# Disable
verifier /reset
```

### Manual Testing
```cmd
# Test CPUID interception
cpuid /0x1        # Should hide hypervisor bit if profile loaded

# Test MSR blocking
rdmsr 0x40000000  # Should fail or return zero if MSR profile loaded
```

### Anti-Cheat Testing
Test with actual anti-cheat systems in a controlled environment:
1. Install game with anti-cheat
2. Load appropriate driver profile
3. Launch game
4. Monitor for detection/bans
5. Collect logs and telemetry

## Debugging

### WinDbg Kernel Debugging
```cmd
bcdedit /debug on
bcdedit /dbgsettings serial debugport:1 baudrate:115200
```

### Debug Output
```cmd
# View driver debug messages
dbgview.exe
```

### Crash Dumps
```cmd
# Configure for kernel memory dumps
wmic recoveros set DebugInfoType = 1
```

## Production Signing

For production deployment, drivers must be properly signed with an Extended Validation (EV) Code Signing certificate.

### Requirements
- EV Code Signing Certificate from trusted CA (DigiCert, Sectigo, etc.)
- Hardware Security Module (HSM) or USB token for certificate storage
- Windows Hardware Developer Center Dashboard account

### Process
1. Obtain EV Code Signing Certificate
2. Register with Windows Hardware Developer Center
3. Submit driver for attestation signing
4. Download signed driver package
5. Distribute via Windows Update or manual installation

### Cost
- EV Code Signing Certificate: ~$300-500/year
- Windows Dev Center: One-time fee ~$99

## Troubleshooting

### Driver Won't Load
- **Cause**: Not test-signed or Secure Boot enabled
- **Fix**: Enable test signing, disable Secure Boot

### System Crashes (BSOD)
- **Cause**: Driver bug or invalid profile
- **Fix**: Boot to Safe Mode, disable driver, check Event Viewer

### IOCTL Fails
- **Cause**: Insufficient permissions or driver not loaded
- **Fix**: Run as Administrator, check driver status with `sc query NanaBoxHvFilter`

### Anti-Cheat Still Detects
- **Cause**: Profile incomplete or driver signature detected
- **Fix**: Review profile settings, consider driver presence detection

## Limitations

### Technical
- Requires test signing or EV certificate
- Cannot intercept kernel-mode CPUID on all Windows versions
- MSR interception has performance overhead (~5-15%)
- May conflict with other virtualization drivers

### Security
- Driver runs at kernel level (RING 0)
- Reduces system security posture
- May be detected by advanced anti-cheat systems
- Unsigned drivers won't load with Secure Boot enabled

### Compatibility
- x64 only (no x86 support)
- Windows 10 1809+ required
- May not work with all processor types
- VT-x/AMD-V recommended but not required

## Future Work (Phase 3+)

- [ ] Implement CPUID instruction hooking
- [ ] Implement MSR interception
- [ ] Add profile loader/manager
- [ ] Create user-mode control utility
- [ ] Add kernel debugger extension
- [ ] Implement per-process filtering
- [ ] Add telemetry and diagnostics
- [ ] Create automated test suite
- [ ] Document signing process
- [ ] Add installer/uninstaller

## References

- [Windows Driver Kit Documentation](https://learn.microsoft.com/en-us/windows-hardware/drivers/)
- [Kernel-Mode Driver Framework (KMDF)](https://learn.microsoft.com/en-us/windows-hardware/drivers/wdf/)
- [Driver Signing Requirements](https://learn.microsoft.com/en-us/windows-hardware/drivers/install/driver-signing)
- [CPUID Instruction Reference](https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-2a-manual.html)
- [MSR Reference (Intel)](https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3b-manual.html)
- [Hyper-V MSR Reference](https://learn.microsoft.com/en-us/virtualization/hyper-v-on-windows/tlfs/tlfs)

## License

MIT License (maintained from upstream NanaBox project)

## Security & Legal Notice

⚠️ **IMPORTANT**:
- These drivers operate at kernel level and can affect system stability
- Use only in controlled environments (VMs, test systems)
- Do NOT use for cheating or violating terms of service
- Intended for anti-cheat research, security testing, and legitimate use cases only
- Users are responsible for compliance with applicable laws and terms of service
- Improper use may result in account bans, legal action, or system instability

## Support

For issues, questions, or contributions:
- GitHub Issues: https://github.com/brunusansi/NanaBox/issues
- Documentation: [docs/](../docs/)
- Community: GitHub Discussions
