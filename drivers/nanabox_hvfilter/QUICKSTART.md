# NanaBox HvFilter Driver - Quick Start Guide

Fast-track guide for developers who want to get the driver running quickly.

## 🚀 Prerequisites

- Windows 10/11 x64
- Visual Studio 2022
- Windows Driver Kit (WDK) 10.0.22621.0+
- Administrator privileges

## ⚡ Quick Build

```cmd
# Open Developer Command Prompt
cd drivers\nanabox_hvfilter
msbuild nanabox_hvfilter.vcxproj /p:Configuration=Release /p:Platform=x64
```

Output: `x64\Release\drivers\nanabox_hvfilter.sys`

## ⚙️ Quick Install

### 1. Enable Test-Signing (One-Time Setup)

```cmd
# Run as Administrator
bcdedit /set testsigning on
bcdedit /set nointegritychecks on
shutdown /r /t 0
```

**Important**: Also disable Secure Boot in BIOS/UEFI

### 2. Create & Install Certificate (One-Time Setup)

```cmd
# Run as Administrator
makecert -r -pe -ss PrivateCertStore -n "CN=NanaBox Test Certificate" nanabox_test.cer
certmgr.exe /add nanabox_test.cer /s /r localMachine root
certmgr.exe /add nanabox_test.cer /s /r localMachine trustedpublisher
```

### 3. Sign the Driver

```cmd
cd x64\Release\drivers
signtool sign /v /s PrivateCertStore /n "NanaBox Test Certificate" nanabox_hvfilter.sys
```

### 4. Install & Start

```cmd
# Copy driver
copy nanabox_hvfilter.sys C:\Windows\System32\drivers\

# Create service
sc create NanaBoxHvFilter type= kernel start= demand binPath= C:\Windows\System32\drivers\nanabox_hvfilter.sys

# Start driver
sc start NanaBoxHvFilter
```

## ✅ Verify Installation

```cmd
# Check service status
sc query NanaBoxHvFilter

# Test with client
cd tools\NbxHvFilterClient
NbxHvFilterClient.exe status
```

Expected output:
```
Driver Version: 1.0.0
Active Profile: None
Is Active: No
```

## 🎮 Quick Usage

### Set Profile
```cmd
NbxHvFilterClient.exe set Valorant 0x00000003
```

### Get Status
```cmd
NbxHvFilterClient.exe status
```

### Clear Profile
```cmd
NbxHvFilterClient.exe clear
```

## 🔍 Debugging

### View Logs

Download [DebugView](https://learn.microsoft.com/en-us/sysinternals/downloads/debugview):

1. Run as Administrator
2. Enable "Capture Kernel"
3. Look for `[NanaBoxHvFilter]` messages

### Check Event Viewer

```cmd
eventvwr.msc
# Navigate to: Windows Logs → System
# Filter by: Service Control Manager
```

## 🛑 Quick Uninstall

```cmd
sc stop NanaBoxHvFilter
sc delete NanaBoxHvFilter
del C:\Windows\System32\drivers\nanabox_hvfilter.sys
```

## 📚 Detailed Documentation

- **Full Build Guide**: [BUILD.md](BUILD.md)
- **Full Install Guide**: [INSTALL.md](INSTALL.md)
- **Technical Docs**: [../../docs/driver-hvfilter.md](../../docs/driver-hvfilter.md)
- **Integration**: [../../examples/hvfilter-integration-example.cpp](../../examples/hvfilter-integration-example.cpp)

## ⚠️ Common Issues

### "The hash for the file is not present..."
→ Test-signing not enabled or driver not signed

### "Error 577: Windows cannot verify..."
→ Certificate not trusted or Secure Boot enabled

### "Failed to open driver device"
→ Driver not running: `sc start NanaBoxHvFilter`

### System won't boot
→ Boot Safe Mode, run: `bcdedit /set testsigning off`

## 🎯 Next Steps

1. ✅ Driver loaded and running
2. ✅ Client can communicate with driver
3. → Integrate with NanaBox configuration
4. → Wait for Phase 3B (actual CPUID/MSR interception)

---

**Quick Links**:
- [Full Installation Guide](INSTALL.md)
- [Build Documentation](BUILD.md)
- [Driver Documentation](../../docs/driver-hvfilter.md)
