# Building nanabox_hvfilter Driver

This document provides detailed instructions for building the NanaBox HvFilter driver on Windows.

## Prerequisites

### Required Software

1. **Windows 10/11** (x64) - Build machine
2. **Visual Studio 2022** with:
   - Desktop development with C++
   - Windows Driver Kit (WDK) integration
3. **Windows Driver Kit (WDK) 10.0.22621.0** or later
4. **Windows SDK 10.0.22621.0** or later

### Installation Steps

#### 1. Install Visual Studio 2022

Download from: https://visualstudio.microsoft.com/downloads/

Required workloads:
- Desktop development with C++
- Include these components:
  - MSVC v143 - VS 2022 C++ x64/x86 build tools
  - Windows 10/11 SDK
  - C++ ATL for latest build tools

#### 2. Install Windows Driver Kit (WDK)

Download from: https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk

**Important**: 
- Install WDK **after** Visual Studio
- WDK version must match your Windows SDK version
- Choose "Full WDK" for complete driver development

#### 3. Verify Installation

Open Developer Command Prompt and verify:

```cmd
# Check Visual Studio
where msbuild
# Output: C:\Program Files\Microsoft Visual Studio\2022\...\MSBuild.exe

# Check WDK
where cl
# Should find compiler in WDK path

# Check SDK
dir "C:\Program Files (x86)\Windows Kits\10\Include"
# Should show SDK versions
```

## Building the Driver

### Method 1: Visual Studio GUI (Recommended)

1. Open `NanaBox.slnx` in Visual Studio 2022
2. Set configuration to `Release` and platform to `x64`
3. Right-click on `nanabox_hvfilter` project
4. Select "Build"
5. Check for build errors in Output window

**Build Output Location**:
```
x64/Release/drivers/
├── nanabox_hvfilter.sys    # Driver binary
├── nanabox_hvfilter.pdb    # Debug symbols
└── nanabox_hvfilter.inf    # Installation INF
```

### Method 2: Command Line (MSBuild)

Open Visual Studio Developer Command Prompt:

```cmd
cd drivers\nanabox_hvfilter
msbuild nanabox_hvfilter.vcxproj /p:Configuration=Release /p:Platform=x64
```

### Method 3: WDK Build Environment

For enterprise or automated builds:

```cmd
# Set up WDK environment
call "C:\Program Files (x86)\Windows Kits\10\BuildEnv\SetupBuildEnv.cmd"

# Build
cd drivers\nanabox_hvfilter
msbuild /t:rebuild /p:Configuration=Release /p:Platform=x64
```

## Build Configurations

### Debug Build

For development and debugging:

```cmd
msbuild nanabox_hvfilter.vcxproj /p:Configuration=Debug /p:Platform=x64
```

**Features**:
- Debug symbols included
- Assertions enabled
- DBG=1 preprocessor flag
- Optimizations disabled
- Verbose logging

### Release Build

For testing and deployment:

```cmd
msbuild nanabox_hvfilter.vcxproj /p:Configuration=Release /p:Platform=x64
```

**Features**:
- Optimized code
- Minimal logging
- No debug assertions
- Smaller binary size

## Troubleshooting Build Issues

### Issue: "WDK not found" or "Cannot find kernel headers"

**Solution**:
1. Verify WDK installation: `dir "C:\Program Files (x86)\Windows Kits\10\Include"`
2. Reinstall WDK if missing
3. Ensure WDK was installed **after** Visual Studio

### Issue: "Platform Toolset 'WindowsKernelModeDriver10.0' not found"

**Solution**:
1. Install WDK Extension for Visual Studio
2. Repair Visual Studio installation
3. Check Visual Studio Extensions: Extensions → Manage Extensions → Search "WDK"

### Issue: "error C1083: Cannot open include file: 'ntddk.h'"

**Solution**:
1. Verify WDK installation path in project properties
2. Ensure $(KMDF_INC_PATH) and $(WDK_INC_PATH) are set correctly
3. Reinstall WDK if headers are missing

### Issue: Build succeeds but driver won't load

**Solution**:
1. Verify test-signing is enabled (see INSTALL.md)
2. Check driver signature with: `signtool verify /v /pa nanabox_hvfilter.sys`
3. Sign the driver if needed (see INSTALL.md)
4. Check Event Viewer for load errors

## Post-Build Steps

After successful build, follow these steps:

### 1. Verify Build Output

```cmd
cd x64\Release\drivers
dir nanabox_hvfilter.*
```

Expected files:
- `nanabox_hvfilter.sys` (driver binary, ~50-100 KB)
- `nanabox_hvfilter.pdb` (debug symbols)
- `nanabox_hvfilter.inf` (installation INF)

### 2. Check Driver Metadata

```cmd
dumpbin /headers nanabox_hvfilter.sys
```

Verify:
- Machine type: x64 (0x8664)
- Subsystem: Native (1)
- File type: Driver (.SYS)

### 3. Sign the Driver

For test-signing (development only):

```cmd
# Create test certificate (first time only)
makecert -r -pe -ss PrivateCertStore -n "CN=NanaBox Test Certificate" nanabox_test.cer
certmgr.exe /add nanabox_test.cer /s /r localMachine root
certmgr.exe /add nanabox_test.cer /s /r localMachine trustedpublisher

# Sign the driver
signtool sign /v /s PrivateCertStore /n "NanaBox Test Certificate" nanabox_hvfilter.sys
```

### 4. Install the Driver

See `INSTALL.md` for detailed installation instructions.

## Building the User-Mode Client

The user-mode client (NbxHvFilterClient.exe) is built separately:

```cmd
cd tools\NbxHvFilterClient
msbuild NbxHvFilterClient.vcxproj /p:Configuration=Release /p:Platform=x64
```

**Output**: `x64/Release/tools/NbxHvFilterClient.exe`

## Continuous Integration / Build Automation

For CI/CD pipelines:

```cmd
# Full clean build with both driver and client
msbuild NanaBox.slnx /t:rebuild /p:Configuration=Release /p:Platform=x64

# Build output
# - x64/Release/drivers/nanabox_hvfilter.sys
# - x64/Release/tools/NbxHvFilterClient.exe
```

## Next Steps

After building successfully:

1. **Sign the driver** (see above)
2. **Install the driver** (see `INSTALL.md`)
3. **Test with user-mode client** (see `docs/driver-hvfilter.md`)
4. **Verify in Device Manager** (see `INSTALL.md`)

## Resources

- [WDK Download](https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk)
- [Driver Development Documentation](https://learn.microsoft.com/en-us/windows-hardware/drivers/)
- [MSBuild Reference](https://learn.microsoft.com/en-us/visualstudio/msbuild/)
- [SignTool Documentation](https://learn.microsoft.com/en-us/windows/win32/seccrypto/signtool)
