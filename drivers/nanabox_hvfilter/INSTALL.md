# Installing nanabox_hvfilter Driver

This document provides step-by-step instructions for installing the NanaBox HvFilter driver on Windows.

## ⚠️ Important Warnings

**READ BEFORE PROCEEDING:**

- This driver is for **development and testing ONLY**
- Requires **test-signing mode** which reduces system security
- Requires **Secure Boot to be DISABLED** in BIOS/UEFI
- Modifying kernel settings incorrectly can make your system unbootable
- **Back up important data** before proceeding
- Never use test-signed drivers in production environments

## Prerequisites

### System Requirements

- **Operating System**: Windows 10 1809+ or Windows 11 (x64 only)
- **Architecture**: x64 (64-bit)
- **User Account**: Administrator privileges required
- **BIOS Settings**: Secure Boot must be disabled

### Files Required

Ensure you have these files from the build:
```
nanabox_hvfilter.sys    # Driver binary
nanabox_hvfilter.inf    # Installation INF
```

## Step 1: Disable Secure Boot

**Warning**: This step requires changing BIOS/UEFI settings.

1. Restart your computer
2. Enter BIOS/UEFI setup (usually press F2, F10, F12, or DEL during boot)
3. Navigate to Security → Secure Boot
4. Set Secure Boot to **Disabled**
5. Save settings and exit (usually F10)
6. Allow system to reboot

**Note**: The exact steps vary by manufacturer. Consult your motherboard manual if needed.

## Step 2: Enable Test-Signing Mode

Test-signing allows loading drivers signed with test certificates.

### Enable Test-Signing

Open Command Prompt **as Administrator** and run:

```cmd
bcdedit /set testsigning on
bcdedit /set nointegritychecks on
```

### Restart the System

```cmd
shutdown /r /t 0
```

After reboot, you should see a **"Test Mode"** watermark on the desktop.

### Verify Test-Signing

```cmd
bcdedit /enum | findstr -i "testsigning"
```

Output should show: `testsigning Yes`

## Step 3: Create and Install Test Certificate

### Create Test Certificate (First Time Only)

Open Command Prompt as Administrator:

```cmd
# Create self-signed certificate
makecert -r -pe -ss PrivateCertStore -n "CN=NanaBox Test Certificate" nanabox_test.cer

# Install to trusted root store
certmgr.exe /add nanabox_test.cer /s /r localMachine root

# Install to trusted publisher store
certmgr.exe /add nanabox_test.cer /s /r localMachine trustedpublisher
```

### Verify Certificate Installation

```cmd
certmgr.exe /s /r localMachine root | findstr "NanaBox"
```

## Step 4: Sign the Driver

Navigate to the driver directory:

```cmd
cd x64\Release\drivers
```

Sign the driver binary:

```cmd
signtool sign /v /s PrivateCertStore /n "NanaBox Test Certificate" /t http://timestamp.digicert.com nanabox_hvfilter.sys
```

Expected output:
```
Successfully signed: nanabox_hvfilter.sys
```

### Verify Signature

```cmd
signtool verify /v /pa nanabox_hvfilter.sys
```

## Step 5: Install the Driver

### Method 1: Manual Installation (Recommended)

#### Copy Driver to System Directory

```cmd
copy nanabox_hvfilter.sys C:\Windows\System32\drivers\
copy nanabox_hvfilter.inf C:\Windows\INF\
```

#### Create Service

```cmd
sc create NanaBoxHvFilter ^
    type= kernel ^
    start= demand ^
    error= normal ^
    binPath= C:\Windows\System32\drivers\nanabox_hvfilter.sys ^
    DisplayName= "NanaBox Hypervisor Filter Driver"
```

#### Start the Service

```cmd
sc start NanaBoxHvFilter
```

Expected output:
```
SERVICE_NAME: NanaBoxHvFilter
        TYPE               : 1  KERNEL_DRIVER
        STATE              : 4  RUNNING
```

### Method 2: Using Device Console (DevCon)

If you have DevCon installed:

```cmd
devcon install nanabox_hvfilter.inf Root\NanaBoxHvFilter
devcon enable Root\NanaBoxHvFilter
```

### Method 3: Using PnPUtil

```cmd
pnputil /add-driver nanabox_hvfilter.inf /install
```

## Step 6: Verify Installation

### Check Service Status

```cmd
sc query NanaBoxHvFilter
```

Expected output:
```
SERVICE_NAME: NanaBoxHvFilter
        TYPE               : 1  KERNEL_DRIVER
        STATE              : 4  RUNNING
        ...
```

### Check Driver in Device Manager

```cmd
devmgmt.msc
```

Navigate to: View → Show hidden devices → System devices

Look for: "NanaBox HvFilter Driver" or "NanaBoxHvFilter"

### Test with User-Mode Client

```cmd
cd tools\NbxHvFilterClient
NbxHvFilterClient.exe status
```

Expected output:
```
===============================================================
NanaBox HvFilter Client v1.0.0
===============================================================

[INFO] Sending GET_STATUS request...
[SUCCESS] Status retrieved successfully
       Driver Version: 1.0.0
       Active Profile: None
       ...
```

### Check Kernel Debug Output

Use [DebugView](https://learn.microsoft.com/en-us/sysinternals/downloads/debugview) from Sysinternals:

1. Run DebugView as Administrator
2. Enable "Capture Kernel" in menu
3. Look for messages prefixed with `[NanaBoxHvFilter]`

## Troubleshooting

### Issue: "The hash for the file is not present in the specified catalog file"

**Cause**: Driver is not signed or test-signing not enabled

**Solution**:
1. Verify test-signing: `bcdedit /enum | findstr testsigning`
2. Ensure Secure Boot is disabled
3. Sign the driver again
4. Restart the system

### Issue: "Error 577: Windows cannot verify the digital signature for this file"

**Cause**: Certificate not trusted or test-signing not enabled

**Solution**:
1. Reinstall test certificate to trusted stores
2. Verify certificate: `certmgr.exe /s /r localMachine root | findstr NanaBox`
3. Enable test-signing: `bcdedit /set testsigning on`
4. Restart

### Issue: Service starts but driver doesn't load

**Cause**: Driver initialization error

**Solution**:
1. Check Event Viewer:
   - Windows Logs → System
   - Filter by Source: "Service Control Manager"
   - Look for NanaBoxHvFilter errors
2. View driver logs with DebugView
3. Check for conflicting drivers

### Issue: "Failed to open driver device" when using client

**Cause**: Driver not running or device not created

**Solution**:
1. Verify service is running: `sc query NanaBoxHvFilter`
2. Check DebugView for device creation messages
3. Restart driver: `sc stop NanaBoxHvFilter && sc start NanaBoxHvFilter`

### Issue: System won't boot after enabling test-signing

**Cause**: Boot configuration error

**Solution**:
1. Boot into Safe Mode
2. Disable test-signing: `bcdedit /set testsigning off`
3. Remove driver: `sc delete NanaBoxHvFilter`
4. Delete driver file: `del C:\Windows\System32\drivers\nanabox_hvfilter.sys`
5. Restart normally

## Uninstalling the Driver

### Step 1: Stop the Service

```cmd
sc stop NanaBoxHvFilter
```

### Step 2: Delete the Service

```cmd
sc delete NanaBoxHvFilter
```

### Step 3: Remove Driver Files

```cmd
del C:\Windows\System32\drivers\nanabox_hvfilter.sys
del C:\Windows\INF\nanabox_hvfilter.inf
```

### Step 4: (Optional) Disable Test-Signing

If you no longer need test-signing:

```cmd
bcdedit /set testsigning off
bcdedit /set nointegritychecks off
shutdown /r /t 0
```

### Step 5: (Optional) Re-enable Secure Boot

1. Restart and enter BIOS/UEFI
2. Navigate to Security → Secure Boot
3. Set Secure Boot to **Enabled**
4. Save and exit

## Automatic Installation with PowerShell

See `scripts/setup/Setup-NanaBoxDevEnv.ps1` for an automated installation wizard.

Usage:
```powershell
.\Setup-NanaBoxDevEnv.ps1
```

This script will:
- Check prerequisites
- Enable test-signing
- Create and install certificate
- Build the driver (if WDK is available)
- Sign and install the driver
- Verify installation

## Production Deployment (Future)

For production use, you will need:
- **EV Code Signing Certificate** (Extended Validation)
- **Windows Hardware Lab Kit (HLK)** testing
- **Microsoft Hardware Dev Center** submission
- **WHQL signature** from Microsoft

This is beyond the scope of this development guide.

## Support

For issues and questions:
- Check logs in DebugView
- Review Event Viewer for system errors
- See `docs/driver-hvfilter.md` for detailed documentation
- File issues on GitHub: https://github.com/brunusansi/NanaBox
