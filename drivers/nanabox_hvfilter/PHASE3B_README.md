# Phase 3B: CPUID and MSR Configuration Framework

## Summary

Phase 3B implements the configuration framework for CPUID and MSR interception in the nanabox_hvfilter driver. This phase establishes the infrastructure for anti-detection policies while deferring actual CPU-level interception to Phase 3C.

## What's New in Phase 3B

### Driver Extensions

1. **CPUID Policy Support**
   - New `NBX_CPUID_POLICY` structure in shared header
   - Policy validation (vendor strings, feature masks)
   - Configuration storage in driver context
   - Detailed logging of CPUID settings
   - Safe activation/deactivation framework

2. **MSR Policy Support**
   - New `NBX_MSR_POLICY` structure in shared header
   - Support for three MSR modes: PASSTHROUGH, ZERO, BLOCK
   - Policy validation (MSR mode ranges)
   - Configuration storage in driver context
   - Detailed logging of MSR settings
   - Safe activation/deactivation framework

3. **Extended IOCTL Structures**
   - `NBX_SET_PROFILE_INPUT` now includes CPUID and MSR policies
   - `NBX_GET_STATUS_OUTPUT` now includes active policies
   - Backward compatible with Phase 3A structures

4. **New Source Files**
   - `cpuid_msr.c`: CPUID and MSR configuration implementation
   - Validation functions for policies
   - Activation/deactivation logic
   - Comprehensive logging

### User-Mode Helper Updates

1. **Predefined Profiles**
   - `roblox`: Roblox (Byfron) profile with AuthenticAMD vendor
   - `valorant`: Valorant (Vanguard) profile with GenuineIntel vendor
   - `expert-tencent`: Tencent ACE profile with GenuineIntel vendor

2. **Enhanced Commands**
   - Simple profile loading: `NbxHvFilterClient.exe set roblox`
   - Custom profile with flags: `NbxHvFilterClient.exe set Custom 0x3`
   - Detailed status output showing policies

3. **Improved Output**
   - Shows CPUID policy details (vendor, flags)
   - Shows MSR policy details (mode)
   - Phase 3B status indicators

### Documentation Updates

1. **development-notes.md**
   - Phase 3B marked as complete
   - Phase 3C outlined for future work
   - Clear distinction between phases

2. **driver-hvfilter.md**
   - Extended IOCTL documentation
   - Phase 3B implementation details
   - Testing procedures
   - Known limitations

3. **New Testing Guide**
   - `PHASE3B_TESTING.md`: Step-by-step testing guide
   - Examples for all profiles
   - Expected outputs
   - Troubleshooting tips

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    User-Mode Application                     │
│              (NanaBox or NbxHvFilterClient)                  │
└────────────────────────┬────────────────────────────────────┘
                         │ DeviceIoControl
                         │ (IOCTL_NBX_HVFILTER_SET_PROFILE)
                         ▼
┌─────────────────────────────────────────────────────────────┐
│                  nanabox_hvfilter.sys                        │
│  ┌──────────────────────────────────────────────────────┐  │
│  │              dispatch.c / ioctl.c                    │  │
│  │         (IOCTL dispatch and handling)                │  │
│  └────────────────────┬─────────────────────────────────┘  │
│                       │                                      │
│                       ▼                                      │
│  ┌──────────────────────────────────────────────────────┐  │
│  │                 cpuid_msr.c                          │  │
│  │  ┌────────────────────────────────────────────────┐ │  │
│  │  │  NbxActivateCpuIdInterception()                │ │  │
│  │  │  - Validate CPUID policy                       │ │  │
│  │  │  - Store configuration                         │ │  │
│  │  │  - Log settings                                │ │  │
│  │  │  - [Phase 3C] Apply interception               │ │  │
│  │  └────────────────────────────────────────────────┘ │  │
│  │  ┌────────────────────────────────────────────────┐ │  │
│  │  │  NbxActivateMsrInterception()                  │ │  │
│  │  │  - Validate MSR policy                         │ │  │
│  │  │  - Store configuration                         │ │  │
│  │  │  - Log settings                                │ │  │
│  │  │  - [Phase 3C] Apply filtering                  │ │  │
│  │  └────────────────────────────────────────────────┘ │  │
│  └──────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

## Policy Structures

### CPUID Policy

```c
typedef struct _NBX_CPUID_POLICY {
    BOOL Enabled;                        // Enable CPUID interception
    BOOL HideHypervisor;                 // Hide hypervisor bit (CPUID.1.ECX[31])
    BOOL MaskVirtualizationFeatures;     // Mask VMX/SVM features
    CHAR VendorString[13];               // CPU vendor ("GenuineIntel" or "AuthenticAMD")
} NBX_CPUID_POLICY;
```

**Fields**:
- `Enabled`: Controls whether CPUID interception is active
- `HideHypervisor`: When TRUE, hides CPUID.1.ECX[31] (hypervisor present bit)
- `MaskVirtualizationFeatures`: When TRUE, masks VMX/SVM feature bits
- `VendorString`: Overrides CPU vendor string (12 chars + null terminator)

### MSR Policy

```c
typedef struct _NBX_MSR_POLICY {
    BOOL Enabled;                        // Enable MSR interception
    DWORD HyperVMsrMode;                 // Hyper-V MSR handling mode
} NBX_MSR_POLICY;
```

**Fields**:
- `Enabled`: Controls whether MSR interception is active
- `HyperVMsrMode`: How to handle Hyper-V MSRs (0x40000000-0x400000FF)
  - `0` (PASSTHROUGH): Allow normal access
  - `1` (ZERO): Return 0 for reads, ignore writes
  - `2` (BLOCK): Block access (return error)

## Profile Configurations

### Roblox (Byfron)

```c
Flags: NBX_PROFILE_FLAG_CPUID | NBX_PROFILE_FLAG_MSR_INTERCEPT

CPUID Policy:
  - Enabled: TRUE
  - HideHypervisor: TRUE
  - MaskVirtualizationFeatures: TRUE
  - VendorString: "AuthenticAMD"

MSR Policy:
  - Enabled: TRUE
  - HyperVMsrMode: NBX_MSR_MODE_ZERO
```

**Rationale**: Roblox Byfron uses balanced detection. AMD CPU is common in gaming systems. Zero mode for MSRs prevents detection via Hyper-V MSR reads.

### Valorant (Riot Vanguard)

```c
Flags: NBX_PROFILE_FLAG_CPUID | NBX_PROFILE_FLAG_MSR_INTERCEPT

CPUID Policy:
  - Enabled: TRUE
  - HideHypervisor: TRUE
  - MaskVirtualizationFeatures: TRUE
  - VendorString: "GenuineIntel"

MSR Policy:
  - Enabled: TRUE
  - HyperVMsrMode: NBX_MSR_MODE_ZERO
```

**Rationale**: Valorant's Vanguard is aggressive. Intel CPU is prevalent in high-end gaming. Zero mode for MSRs essential.

### Tencent ACE (Expert)

```c
Flags: NBX_PROFILE_FLAG_CPUID | NBX_PROFILE_FLAG_MSR_INTERCEPT

CPUID Policy:
  - Enabled: TRUE
  - HideHypervisor: TRUE
  - MaskVirtualizationFeatures: TRUE
  - VendorString: "GenuineIntel"

MSR Policy:
  - Enabled: TRUE
  - HyperVMsrMode: NBX_MSR_MODE_ZERO
```

**Rationale**: Tencent ACE is kernel-level anti-cheat. Intel CPU common in Chinese gaming market. Zero mode prevents MSR-based detection.

## What Phase 3B Does

✅ **Configuration Management**
- Receives CPUID and MSR policies from user-mode
- Validates policy parameters
- Stores policies in driver context
- Provides query interface for active policies

✅ **Logging and Diagnostics**
- Detailed logging of policy activation
- Policy parameter logging (vendor, modes, flags)
- Deactivation logging
- Phase 3B status indicators

✅ **Safety**
- Input validation (vendor strings, MSR modes)
- Safe cleanup on profile changes
- Safe cleanup on driver unload
- Error handling for invalid policies

✅ **Profile Support**
- Framework for game-specific profiles
- Predefined configurations for Roblox, Valorant, Tencent
- Custom profile support

## What Phase 3B Does NOT Do

⚠️ **No Actual CPU Interception**
- CPUID instructions are NOT intercepted
- MSR reads/writes are NOT filtered
- CPU behavior is UNCHANGED

⚠️ **Guest-Level Limitations**
- Running as guest driver limits capabilities
- True interception requires hypervisor cooperation
- Host-side implementation needed for effectiveness

⚠️ **Phase 3C Required**
- Actual CPUID hooking deferred to Phase 3C
- Actual MSR filtering deferred to Phase 3C
- Full anti-detection requires Phase 3C+ implementation

## Why Defer Interception to Phase 3C?

### Technical Challenges

1. **CPUID Interception**
   - Guest OS cannot directly intercept CPUID without hypervisor
   - Requires Hyper-V enlightenment integration
   - May need host-side WHP API implementation

2. **MSR Filtering**
   - MSR access controlled by hypervisor at lower level
   - Guest driver cannot override hypervisor MSR handling
   - Requires special Hyper-V MSR intercept registration

3. **Safety First**
   - Phase 3B establishes safe configuration framework
   - Validates approach before complex CPU operations
   - Allows testing of configuration flow

### Phase 3B Benefits

- ✅ Configuration framework tested and validated
- ✅ Policy structures finalized
- ✅ User interface established
- ✅ Logging infrastructure in place
- ✅ Safe foundation for Phase 3C work

## Testing

See `PHASE3B_TESTING.md` for comprehensive testing guide.

**Quick Test**:
```cmd
# Load driver
sc start NanaBoxHvFilter

# Test Roblox profile
NbxHvFilterClient.exe set roblox

# Check status
NbxHvFilterClient.exe status

# View logs in DebugView
```

## Future Work (Phase 3C)

### Research Areas

1. **Hyper-V Enlightenments**
   - Hyper-V MSR intercept APIs
   - CPUID enlightenment integration
   - Synthetic MSR support

2. **Windows Hypervisor Platform (WHP)**
   - Host-side interception using WHP API
   - WHvSetVirtualProcessorRegisters
   - WHvSetPartitionProperty

3. **Custom Hypervisor Module**
   - Thin hypervisor layer for interception
   - CPUID VM-exit handling
   - MSR bitmap configuration

### Implementation Plan

1. Research Hyper-V APIs for guest-level interception
2. Prototype WHP-based host-side implementation
3. Compare guest vs. host approaches
4. Implement chosen solution
5. Test with detection tools (Pafish, Al-Khaser)
6. Validate with target anti-cheats

## Files Changed in Phase 3B

### Driver Files
- `drivers/nanabox_hvfilter/NbxHvFilterShared.h` - Extended with CPUID/MSR structures
- `drivers/nanabox_hvfilter/driver.h` - Extended driver context
- `drivers/nanabox_hvfilter/driver.c` - Initialize new context fields
- `drivers/nanabox_hvfilter/ioctl.c` - Extended IOCTL handling
- `drivers/nanabox_hvfilter/cpuid_msr.c` - New: CPUID/MSR implementation
- `drivers/nanabox_hvfilter/nanabox_hvfilter.vcxproj` - Added cpuid_msr.c

### User-Mode Files
- `tools/NbxHvFilterClient/NbxHvFilterClient.cpp` - Extended with policy support

### Documentation
- `docs/development-notes.md` - Phase 3B status updated
- `docs/driver-hvfilter.md` - Extended with Phase 3B details
- `drivers/nanabox_hvfilter/PHASE3B_TESTING.md` - New: Testing guide
- `drivers/nanabox_hvfilter/PHASE3B_README.md` - New: This file

## Dependencies

- Windows 10/11 (Host OS)
- Windows Driver Kit (WDK) 10.0.22621.0+
- Visual Studio 2022
- Test-signing enabled

## License

MIT License - See LICENSE.md

## Contributors

Phase 3B implementation by NanaBox Anti-Detection Edition Contributors.

## Support

- Documentation: `docs/driver-hvfilter.md`
- Testing Guide: `PHASE3B_TESTING.md`
- GitHub Issues: https://github.com/brunusansi/NanaBox/issues
