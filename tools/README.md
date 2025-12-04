# NanaBox Anti-Detection Edition - Tools

This directory contains user-mode tools and utilities for NanaBox Anti-Detection Edition.

## Tools

### NbxHvFilterClient

**User-mode helper for nanabox_hvfilter driver**

A command-line utility that communicates with the `nanabox_hvfilter.sys` kernel driver to manage anti-detection profiles.

**Location**: `tools/NbxHvFilterClient/`

**Purpose**:
- Set anti-detection profiles on the driver
- Query current driver status
- Clear active profiles

**Usage**:
```cmd
# Set a profile
NbxHvFilterClient.exe set <profile_name> <flags>

# Get current status
NbxHvFilterClient.exe status

# Clear active profile
NbxHvFilterClient.exe clear
```

**Examples**:
```cmd
# Set Valorant profile with CPUID and MSR interception
NbxHvFilterClient.exe set Valorant 0x00000003

# Set BareMetal profile with all features
NbxHvFilterClient.exe set BareMetal 0x0000000F

# Get current status
NbxHvFilterClient.exe status

# Clear profile
NbxHvFilterClient.exe clear
```

**Profile Flags**:
- `0x00000001` - CPUID spoofing
- `0x00000002` - MSR interception
- `0x00000004` - Timing normalization
- `0x00000008` - PCI topology control

**Building**:
```cmd
cd tools\NbxHvFilterClient
msbuild NbxHvFilterClient.vcxproj /p:Configuration=Release /p:Platform=x64
```

**Requirements**:
- Windows 10/11 x64
- nanabox_hvfilter.sys driver must be installed and running
- Administrator privileges (to access kernel driver)

**Documentation**: See `docs/driver-hvfilter.md` for detailed information.

## Future Tools

Additional tools may be added in future phases:
- Profile configuration manager
- Hardware information extractor
- Anti-detection test suite
- VM configuration validator
