# Multi-Account Isolation Guide

## Overview

Multi-account isolation is a core feature of NanaBox Anti-Detection Edition, enabling users to maintain multiple separate VM instances with unique hardware fingerprints for different accounts, games, or use cases without cross-contamination.

## Why Multi-Account Isolation?

### Use Cases

1. **Gaming**:
   - Separate VMs for different game accounts
   - Test accounts vs main accounts
   - Regional accounts (NA, EU, Asia)
   - Account recovery/ban protection

2. **Security Research**:
   - Isolated malware analysis environments
   - Separate research projects
   - Client engagement isolation

3. **Development & Testing**:
   - Separate development environments
   - Testing different game configurations
   - Isolated testing environments

4. **Privacy**:
   - Separate identities for different activities
   - Prevent fingerprinting correlation
   - Compartmentalized browsing/work

## Architecture

### Isolation Layers

```
Physical Host
└── NanaBox Installation
    └── Profiles Directory (%LOCALAPPDATA%\NanaBox\profiles\)
        ├── account-1/
        │   ├── config.json          # VM configuration
        │   ├── VM.vhdx              # Virtual disk
        │   ├── VM.vmgs              # Guest state
        │   ├── VM.vmrs              # Runtime state
        │   └── metadata.json        # Account metadata
        ├── account-2/
        │   ├── config.json
        │   ├── VM.vhdx
        │   └── ...
        └── templates/
            ├── base-windows11.vhdx  # Shared base image (read-only)
            └── ...
```

### Configuration Structure

Each account has:
- **Unique AccountId**: Identifier for the account/profile
- **Separate Storage**: Isolated VHD/VHDX files
- **Unique Hardware ID**: Different SMBIOS, UUID, MAC address
- **Independent Configuration**: Separate settings per account

## Implementation Guide

### Step 1: Directory Structure

Create profile directories:
```
%LOCALAPPDATA%\NanaBox\
├── profiles\
│   ├── valorant-main\
│   ├── valorant-alt\
│   ├── roblox-account1\
│   ├── roblox-account2\
│   └── pubg-competitive\
└── templates\
    └── windows11-base.vhdx
```

**PowerShell**:
```powershell
# Create profile directories
$ProfileRoot = "$env:LOCALAPPDATA\NanaBox\profiles"
$Accounts = @("valorant-main", "valorant-alt", "roblox-account1")

foreach ($Account in $Accounts) {
    New-Item -Path "$ProfileRoot\$Account" -ItemType Directory -Force
}

# Create templates directory
New-Item -Path "$env:LOCALAPPDATA\NanaBox\templates" -ItemType Directory -Force
```

### Step 2: Base Template Creation

Create a base Windows image that will be used as read-only parent:

1. **Install Windows** in a temporary VM
2. **Configure** the base system (updates, drivers, etc.)
3. **Sysprep** to generalize the installation
4. **Export** the VHD/VHDX as template
5. **Store** in templates directory

**PowerShell Example**:
```powershell
# After VM is prepared, convert to template
$SourceVHD = "C:\VMs\BaseWindows11.vhdx"
$TemplateVHD = "$env:LOCALAPPDATA\NanaBox\templates\windows11-base.vhdx"

Copy-Item -Path $SourceVHD -Destination $TemplateVHD
Set-ItemProperty -Path $TemplateVHD -Name IsReadOnly -Value $true
```

### Step 3: Generate Unique Hardware IDs

Each account needs unique hardware identifiers:

**SMBIOS Serials**:
```powershell
function New-RandomSerial {
    param([string]$Format = "XXXXXXXXXX")
    
    $chars = "0123456789ABCDEFGHJKLMNPQRSTUVWXYZ"
    $serial = ""
    
    foreach ($char in $Format.ToCharArray()) {
        if ($char -eq 'X') {
            $serial += $chars[(Get-Random -Maximum $chars.Length)]
        } else {
            $serial += $char
        }
    }
    
    return $serial
}

# Generate unique serials for each account
$Accounts = @{
    "valorant-main" = @{
        SerialNumber = New-RandomSerial "4ZMK3X3"
        UUID = [guid]::NewGuid().ToString()
        BaseBoardSerial = New-RandomSerial "/4ZMK3X3/CNFCP0013O0XXX/"
    }
    "valorant-alt" = @{
        SerialNumber = New-RandomSerial "5ANL4Y4"
        UUID = [guid]::NewGuid().ToString()
        BaseBoardSerial = New-RandomSerial "/5ANL4Y4/CNFCP0013O0YYY/"
    }
}

# Save to registry or metadata file
$Accounts | ConvertTo-Json | Out-File "$env:LOCALAPPDATA\NanaBox\accounts.json"
```

**MAC Addresses**:
```powershell
function New-RandomMAC {
    param([string]$VendorPrefix = "D8-9E-F3")
    
    $random = "{0:X2}-{1:X2}-{2:X2}" -f (Get-Random -Maximum 256),
                                          (Get-Random -Maximum 256),
                                          (Get-Random -Maximum 256)
    
    return "$VendorPrefix-$random"
}

# Generate unique MAC for each account
$MACAddresses = @{
    "valorant-main" = New-RandomMAC -VendorPrefix "D8-9E-F3"  # ASUS
    "valorant-alt"  = New-RandomMAC -VendorPrefix "AC-DE-48"  # Gigabyte
    "roblox-account1" = New-RandomMAC -VendorPrefix "00-D8-61"  # MSI
}
```

### Step 4: Create Differencing Disks

Use differencing disks to save space and speed up creation:

```powershell
function New-DifferencingDisk {
    param(
        [string]$ParentPath,
        [string]$ChildPath
    )
    
    # Create differencing disk
    New-VHD -Path $ChildPath -ParentPath $ParentPath -Differencing
}

# Create differencing disk for each account
$TemplatePath = "$env:LOCALAPPDATA\NanaBox\templates\windows11-base.vhdx"

$Accounts = @("valorant-main", "valorant-alt", "roblox-account1")
foreach ($Account in $Accounts) {
    $ChildPath = "$env:LOCALAPPDATA\NanaBox\profiles\$Account\VM.vhdx"
    New-DifferencingDisk -ParentPath $TemplatePath -ChildPath $ChildPath
}
```

### Step 5: Generate Account Configurations

Create configuration file for each account:

```powershell
function New-AccountConfig {
    param(
        [string]$AccountId,
        [string]$ProfileId,
        [hashtable]$SMBIOS,
        [string]$MACAddress,
        [string]$VHDXPath
    )
    
    $config = @{
        NanaBox = @{
            Type = "VirtualMachine"
            Version = 1
            Metadata = @{
                Description = "VM for $AccountId"
                AccountId = $AccountId
                ProfileId = $ProfileId
                CreationTimestamp = (Get-Date).ToUniversalTime().ToString("o")
                SchemaVersion = 1
            }
            GuestType = "Windows"
            Name = $AccountId
            ProcessorCount = 8
            MemorySize = 16384
            NetworkAdapters = @(
                @{
                    Connected = $true
                    MacAddress = $MACAddress
                    EndpointId = ""
                }
            )
            ScsiDevices = @(
                @{
                    Type = "VirtualDisk"
                    Path = $VHDXPath
                },
                @{
                    Type = "VirtualImage"
                    Path = ""
                }
            )
            SecureBoot = $true
            Tpm = $true
            GuestStateFile = "$AccountId.vmgs"
            RuntimeStateFile = "$AccountId.vmrs"
            ExposeVirtualizationExtensions = $false
            AntiDetectionProfile = "bare-metal"
            ChipsetInformation = $SMBIOS
            CpuId = @{
                Enabled = $true
                HideHypervisor = $true
                VendorString = "GenuineIntel"
                MaskVirtualizationFeatures = $true
            }
            MsrIntercept = @{
                Enabled = $true
                BlockHyperVMsrs = $true
                NormalizeTSC = $true
            }
            AcpiOverride = @{
                Enabled = $true
                RemoveHyperVDevices = $true
                CustomDSDT = ""
            }
        }
    }
    
    return $config
}

# Generate config for account
$AccountId = "valorant-main"
$config = New-AccountConfig `
    -AccountId $AccountId `
    -ProfileId "valorant" `
    -SMBIOS @{
        Manufacturer = "ASUSTeK COMPUTER INC."
        ProductName = "ROG STRIX Z690-A"
        SerialNumber = $Accounts[$AccountId].SerialNumber
        UUID = $Accounts[$AccountId].UUID
        BaseBoardSerialNumber = $Accounts[$AccountId].BaseBoardSerial
    } `
    -MACAddress $MACAddresses[$AccountId] `
    -VHDXPath "VM.vhdx"

# Save configuration
$ConfigPath = "$env:LOCALAPPDATA\NanaBox\profiles\$AccountId\config.json"
$config | ConvertTo-Json -Depth 10 | Out-File -Encoding UTF8 $ConfigPath
```

### Step 6: Account Registry

Maintain a registry of accounts to prevent identifier reuse:

```powershell
# Account registry structure
$AccountRegistry = @{
    Version = 1
    Accounts = @{
        "valorant-main" = @{
            ProfileId = "valorant"
            SerialNumber = "4ZMK3X3"
            UUID = "03000200-0400-0500-0006-000700080009"
            MACAddress = "D8-9E-F3-12-34-56"
            CreatedDate = "2025-12-04T10:30:00Z"
            LastUsed = "2025-12-04T15:45:00Z"
        }
        "valorant-alt" = @{
            ProfileId = "valorant"
            SerialNumber = "5ANL4Y4"
            UUID = "04000300-0500-0600-0007-000800090010"
            MACAddress = "D8-9E-F3-AB-CD-EF"
            CreatedDate = "2025-12-04T11:00:00Z"
            LastUsed = "2025-12-04T14:30:00Z"
        }
    }
}

# Save registry
$RegistryPath = "$env:LOCALAPPDATA\NanaBox\account-registry.json"
$AccountRegistry | ConvertTo-Json -Depth 10 | Out-File -Encoding UTF8 $RegistryPath
```

## Account Cloning

### Quick Clone Script

```powershell
function Copy-AccountProfile {
    param(
        [string]$SourceAccountId,
        [string]$TargetAccountId
    )
    
    $ProfileRoot = "$env:LOCALAPPDATA\NanaBox\profiles"
    $SourcePath = "$ProfileRoot\$SourceAccountId"
    $TargetPath = "$ProfileRoot\$TargetAccountId"
    
    # Create target directory
    New-Item -Path $TargetPath -ItemType Directory -Force
    
    # Load source config
    $SourceConfig = Get-Content "$SourcePath\config.json" | ConvertFrom-Json
    
    # Generate new unique IDs
    $NewSerial = New-RandomSerial
    $NewUUID = [guid]::NewGuid().ToString()
    $NewMAC = New-RandomMAC
    
    # Update config
    $SourceConfig.NanaBox.Metadata.AccountId = $TargetAccountId
    $SourceConfig.NanaBox.Metadata.CreationTimestamp = (Get-Date).ToUniversalTime().ToString("o")
    $SourceConfig.NanaBox.Name = $TargetAccountId
    $SourceConfig.NanaBox.ChipsetInformation.SerialNumber = $NewSerial
    $SourceConfig.NanaBox.ChipsetInformation.UUID = $NewUUID
    $SourceConfig.NanaBox.NetworkAdapters[0].MacAddress = $NewMAC
    
    # Save new config
    $SourceConfig | ConvertTo-Json -Depth 10 | Out-File "$TargetPath\config.json"
    
    # Create differencing disk from same parent
    $ParentDisk = (Get-VHD "$SourcePath\VM.vhdx").ParentPath
    New-DifferencingDisk -ParentPath $ParentDisk -ChildPath "$TargetPath\VM.vhdx"
    
    Write-Host "Account $SourceAccountId cloned to $TargetAccountId"
}

# Clone account
Copy-AccountProfile -SourceAccountId "valorant-main" -TargetAccountId "valorant-alt"
```

## Best Practices

### 1. Identifier Uniqueness
- ✅ Generate random UUIDs for each account
- ✅ Use realistic but unique serial numbers
- ✅ Maintain unique MAC addresses per account
- ✅ Keep registry of used identifiers
- ❌ Don't reuse identifiers across accounts

### 2. Storage Organization
- ✅ Use differencing disks to save space
- ✅ Keep base template read-only
- ✅ Separate storage per account
- ✅ Regular backups of account data
- ❌ Don't share VHD/VHDX files between accounts

### 3. Network Isolation
- ✅ Unique MAC per account
- ✅ Separate network switches if needed
- ✅ Consider VLAN tagging for isolation
- ❌ Don't use same MAC across accounts

### 4. Metadata Tracking
- ✅ Document account purpose
- ✅ Track creation and usage dates
- ✅ Maintain account registry
- ✅ Note any special configurations
- ❌ Don't lose track of account mappings

### 5. Security
- ✅ Encrypt sensitive account data
- ✅ Secure storage of credentials
- ✅ Regular security audits
- ✅ Backup account configurations
- ❌ Don't expose account registry publicly

## Troubleshooting

### Issue: Accounts Detected as Same Hardware
**Solution**: Verify unique SMBIOS, UUID, and MAC addresses.

```powershell
# Check uniqueness in guest OS
Get-WmiObject Win32_ComputerSystemProduct | Select UUID
Get-WmiObject Win32_BIOS | Select SerialNumber
Get-NetAdapter | Select MacAddress
```

### Issue: Differencing Disk Chain Broken
**Solution**: Ensure base template exists and is accessible.

```powershell
# Check disk chain
Get-VHD -Path "$env:LOCALAPPDATA\NanaBox\profiles\account-1\VM.vhdx" | Select ParentPath
```

### Issue: Account Registry Out of Sync
**Solution**: Rebuild registry from existing configs.

```powershell
# Rebuild registry
$ProfileRoot = "$env:LOCALAPPDATA\NanaBox\profiles"
$Accounts = Get-ChildItem -Path $ProfileRoot -Directory

$Registry = @{ Accounts = @{} }
foreach ($Account in $Accounts) {
    $ConfigPath = "$($Account.FullName)\config.json"
    if (Test-Path $ConfigPath) {
        $Config = Get-Content $ConfigPath | ConvertFrom-Json
        $Registry.Accounts[$Account.Name] = @{
            ProfileId = $Config.NanaBox.Metadata.ProfileId
            SerialNumber = $Config.NanaBox.ChipsetInformation.SerialNumber
            UUID = $Config.NanaBox.ChipsetInformation.UUID
            MACAddress = $Config.NanaBox.NetworkAdapters[0].MacAddress
        }
    }
}

$Registry | ConvertTo-Json -Depth 10 | Out-File "$ProfileRoot\..\account-registry.json"
```

## Future Enhancements (Phase 5)

- **UI for Account Management**: Create, clone, delete accounts from GUI
- **Automatic Profile Assignment**: Auto-select profile based on account type
- **Account Synchronization**: Sync configurations across machines
- **Bulk Operations**: Manage multiple accounts at once
- **Account Groups**: Organize accounts by game, purpose, or client

## See Also

- [Configuration Guide](configuration.md) - Configuration schema details
- [Profiles](profiles.md) - Profile system documentation
- [Examples](../examples/README.md) - Example configurations
- [Development Notes](development-notes.md) - Contributing guidelines
