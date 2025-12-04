# NanaBox Anti-Detection Edition - Profile System

## Overview

The profile system provides pre-configured anti-detection templates optimized for specific use cases. Each profile combines SMBIOS values, anti-detection settings, timing strategies, and hardware configurations tailored for different scenarios.

## Profile Selection

Use the `AntiDetectionProfile` field to select a high-level strategy:

- **vanilla**: No anti-detection (default)
- **balanced**: Moderate anti-detection with good performance
- **bare-metal**: Maximum anti-detection effort

Additionally, use the `Metadata.ProfileId` field to identify specific use-case profiles.

## Gaming Profiles

### Valorant Profile
**Profile ID**: `valorant`  
**Anti-Detection Level**: `bare-metal`  
**Target**: Riot Vanguard anti-cheat

**Configuration**:
- **Hardware**: ASUS ROG STRIX Z690-A GAMING WIFI D4
- **CPU Profile**: Intel Core i7-12700K (12th gen)
- **CPU Vendor**: GenuineIntel
- **Resources**: 8 vCPU, 16 GB RAM
- **TPM**: Required (2.0)
- **Secure Boot**: Enabled
- **Timing Strategy**: Strict

**Anti-Detection Features**:
- ✅ CPUID hypervisor hiding
- ✅ MSR interception (Block Hyper-V MSRs)
- ✅ TSC normalization
- ✅ ACPI device removal
- ✅ Custom SMBIOS (ASUS gaming board)
- ✅ Realistic MAC address

**SMBIOS Details**:
```
Manufacturer: ASUSTeK COMPUTER INC.
Product: ROG STRIX Z690-A GAMING WIFI D4
Version: Rev 1.xx
Serial: MB-1234567890123
UUID: 03000200-0400-0500-0006-000700080009
```

**Known Compatibility**:
- ✅ Valorant (Riot Vanguard)
- ⚠️ Requires Phase 3+ implementation for full effectiveness

**Example File**: `examples/profile-valorant.json`

---

### Roblox Profile
**Profile ID**: `roblox`  
**Anti-Detection Level**: `balanced`  
**Target**: Roblox Byfron anti-cheat

**Configuration**:
- **Hardware**: Gigabyte B550 AORUS ELITE V2
- **CPU Profile**: AMD Ryzen 5 5600X
- **CPU Vendor**: AuthenticAMD
- **Resources**: 6 vCPU, 8 GB RAM
- **TPM**: Not required
- **Secure Boot**: Enabled
- **Timing Strategy**: Relaxed

**Anti-Detection Features**:
- ✅ CPUID hypervisor hiding
- ✅ MSR interception (Block Hyper-V MSRs)
- ⚠️ TSC normalization (optional)
- ✅ ACPI device removal
- ✅ Custom SMBIOS (Gigabyte mid-range board)
- ✅ Realistic MAC address

**SMBIOS Details**:
```
Manufacturer: Gigabyte Technology Co., Ltd.
Product: B550 AORUS ELITE V2
Version: 1.0
Serial: GB-4567890123456
UUID: 12345678-90ab-cdef-1234-567890abcdef
```

**Known Compatibility**:
- ✅ Roblox (Byfron)
- ✅ Most casual games
- ⚠️ Requires Phase 3+ implementation for full effectiveness

**Example File**: `examples/profile-roblox.json`

---

### PUBG Profile
**Profile ID**: `pubg`  
**Anti-Detection Level**: `bare-metal`  
**Target**: BattlEye anti-cheat

**Configuration**:
- **Hardware**: MSI MPG Z790 EDGE WIFI
- **CPU Profile**: Intel Core i9-13900K (13th gen)
- **CPU Vendor**: GenuineIntel
- **Resources**: 8 vCPU, 16 GB RAM
- **TPM**: Required (2.0)
- **Secure Boot**: Enabled
- **Timing Strategy**: Strict

**Anti-Detection Features**:
- ✅ CPUID hypervisor hiding
- ✅ MSR interception (Block Hyper-V MSRs)
- ✅ TSC normalization
- ✅ ACPI device removal
- ✅ Custom SMBIOS (MSI high-end gaming board)
- ✅ Realistic MAC address

**SMBIOS Details**:
```
Manufacturer: MSI
Product: MPG Z790 EDGE WIFI (MS-7D91)
Version: 1.0
Serial: K8123456789012
UUID: 00020003-0004-0005-0006-000700080009
```

**Known Compatibility**:
- ✅ PUBG (BattlEye)
- ✅ Other BattlEye-protected games
- ⚠️ Requires Phase 3+ implementation for full effectiveness

**Example File**: `examples/profile-pubg.json`

---

### EA Anti-Cheat Profile
**Profile ID**: `ea-ac`  
**Anti-Detection Level**: `bare-metal`  
**Target**: EA Anti-Cheat (Apex Legends, etc.)

**Configuration**:
- **Hardware**: ASRock Z690 Steel Legend WiFi 6E
- **CPU Profile**: Intel Core i7-13700K (13th gen)
- **CPU Vendor**: GenuineIntel
- **Resources**: 8 vCPU, 16 GB RAM
- **TPM**: Required (2.0)
- **Secure Boot**: Enabled
- **Timing Strategy**: Strict

**Anti-Detection Features**:
- ✅ CPUID hypervisor hiding
- ✅ MSR interception (Block Hyper-V MSRs)
- ✅ TSC normalization
- ✅ ACPI device removal
- ✅ Custom SMBIOS (ASRock gaming board)
- ✅ Realistic MAC address

**SMBIOS Details**:
```
Manufacturer: ASRock
Product: Z690 Steel Legend WiFi 6E
Version: 1.0
Serial: M80-AB001234567890
UUID: 48000200-8e00-11ec-ba2f-7c10c9aabbcc
```

**Known Compatibility**:
- ✅ EA Anti-Cheat games (Apex Legends, Battlefield)
- ⚠️ Requires Phase 3+ implementation for full effectiveness

**Example File**: `examples/profile-ea-ac.json`

---

## Planned Gaming Profiles (Phase 5)

### FaceIT Anti-Cheat Profile
**Profile ID**: `faceit`  
**Anti-Detection Level**: `bare-metal`  
**Target**: FaceIT anti-cheat (CS:GO, CS2, etc.)

**Status**: ⚠️ Planned for Phase 5

**Configuration** (Preliminary):
- **Hardware**: High-end gaming motherboard
- **CPU Profile**: Intel Core i9 or AMD Ryzen 9
- **CPU Vendor**: GenuineIntel or AuthenticAMD
- **Resources**: 8 vCPU, 16 GB RAM
- **TPM**: Required (2.0)
- **Secure Boot**: Enabled
- **Timing Strategy**: Strict

**Target Games**:
- Counter-Strike: Global Offensive
- Counter-Strike 2
- Other FaceIT platform games

---

### EasyAntiCheat (EAC) Generic Profile
**Profile ID**: `easyanticheat`  
**Anti-Detection Level**: `bare-metal`  
**Target**: Generic EasyAntiCheat protection

**Status**: ⚠️ Planned for Phase 5

**Configuration** (Preliminary):
- **Hardware**: Mid-to-high gaming motherboard
- **CPU Profile**: Intel Core i7 or AMD Ryzen 7
- **Resources**: 6-8 vCPU, 12-16 GB RAM
- **TPM**: Recommended
- **Secure Boot**: Enabled
- **Timing Strategy**: Strict

**Target Games**:
- Fortnite
- Apex Legends (uses both EAC and EA AC)
- Rust
- Dead by Daylight
- New World
- Many others

---

### BattlEye Generic Profile
**Profile ID**: `battleye`  
**Anti-Detection Level**: `bare-metal`  
**Target**: Generic BattlEye protection

**Status**: ⚠️ Planned for Phase 5

**Configuration** (Preliminary):
- **Hardware**: High-end gaming motherboard
- **CPU Profile**: Intel Core i7/i9 or AMD Ryzen 7/9
- **Resources**: 8 vCPU, 16 GB RAM
- **TPM**: Required
- **Secure Boot**: Enabled
- **Timing Strategy**: Strict

**Target Games**:
- PUBG (already has dedicated profile)
- Fortnite
- Rainbow Six Siege
- Arma 3
- DayZ
- Many others

---

### Tencent ACE Profile
**Profile ID**: `tencent-ace`  
**Anti-Detection Level**: `bare-metal`  
**Target**: Tencent ACE anti-cheat

**Status**: ⚠️ Planned for Phase 5

**Configuration** (Preliminary):
- **Hardware**: Popular Chinese market motherboards (ASUS ROG, MSI)
- **CPU Profile**: Intel Core i7 or AMD Ryzen 7
- **Resources**: 8 vCPU, 16 GB RAM
- **TPM**: Required
- **Secure Boot**: Enabled
- **Timing Strategy**: Strict

**Target Games**:
- PUBG Mobile (PC version)
- Call of Duty Mobile (PC version)
- Arena of Valor
- Other Tencent games

---

## Profile Comparison Matrix

| Profile | Anti-Detection | TPM | Resources | Primary Use Case | Status |
|---------|----------------|-----|-----------|------------------|--------|
| vanilla | None | Optional | Flexible | Development | ✅ Ready |
| Valorant | bare-metal | Required | High | Riot Vanguard | ✅ Ready |
| Roblox | balanced | No | Medium | Byfron | ✅ Ready |
| PUBG | bare-metal | Required | High | BattlEye | ✅ Ready |
| EA-AC | bare-metal | Required | High | EA Anti-Cheat | ✅ Ready |
| FaceIT | bare-metal | Required | High | FaceIT AC | ⚠️ Planned |
| EasyAntiCheat | bare-metal | Recommended | High | EAC Games | ⚠️ Planned |
| BattlEye Generic | bare-metal | Required | High | BattlEye Games | ⚠️ Planned |
| Tencent ACE | bare-metal | Required | High | Tencent Games | ⚠️ Planned |

## Creating Custom Profiles

### Step 1: Choose Base Profile
Start with the closest matching profile from examples.

### Step 2: Customize SMBIOS
Extract real hardware values using:

**Windows**:
```powershell
Get-WmiObject Win32_ComputerSystem
Get-WmiObject Win32_BIOS
Get-WmiObject Win32_BaseBoard
```

**Linux**:
```bash
sudo dmidecode -t system
sudo dmidecode -t bios
sudo dmidecode -t baseboard
```

### Step 3: Generate Unique Identifiers
- **MAC Address**: Use realistic vendor prefixes
- **Serial Numbers**: Randomize but keep format consistent
- **UUID**: Generate new GUID for each VM

### Step 4: Set Anti-Detection Level
- **Development/Testing**: vanilla
- **General Gaming**: balanced
- **Anti-Cheat Gaming**: bare-metal
- **Cloud Work**: balanced (provider-specific)

### Step 5: Test Incrementally
1. Start with vanilla profile
2. Enable anti-detection features one by one
3. Test with detection tools (Pafish, Al-Khaser)
4. Test with target application/game

## Profile Switching

### Manual Switching
1. Stop the VM
2. Edit configuration file
3. Change `AntiDetectionProfile` and `Metadata.ProfileId`
4. Update SMBIOS values if needed
5. Start the VM

### Future: Automatic Switching (Phase 5)
- Profile auto-detection based on running applications
- One-click profile switching via UI
- Profile inheritance and layering
- Profile hot-swapping for running VMs

## Profile Best Practices

### For Gaming
1. **Use bare-metal profile** for anti-cheat games
2. **Enable TPM 2.0** if supported by game
3. **Match CPU vendor** to realistic hardware
4. **Use strict timing** for kernel-level anti-cheats
5. **Test with detection tools** before using with real accounts

### For Development & Testing
1. **Use vanilla profile** for maximum compatibility
2. **Disable anti-detection** for better performance
3. **Use minimal resources** to conserve host capacity
4. **Enable debugging features** as needed
5. **Document test configurations** in metadata

### For Multi-Account Isolation
1. **Unique AccountId** per VM instance
2. **Unique SMBIOS serials** per account
3. **Unique MAC addresses** per account
4. **Separate storage paths** per account
5. **Document account mappings** in metadata

## Implementation Status

| Feature | Status | Phase |
|---------|--------|-------|
| Profile schema | ✅ Complete | Phase 1 |
| Example configs | ✅ Complete | Phase 1 |
| SMBIOS support | ✅ Complete | Phase 1 |
| Metadata tracking | ✅ Complete | Phase 1 |
| CPUID spoofing | ⚠️ Schema only | Phase 3 |
| MSR interception | ⚠️ Schema only | Phase 3 |
| ACPI overrides | ⚠️ Schema only | Phase 4 |
| Timing normalization | ⚠️ Schema only | Phase 4 |
| PCI topology | ⚠️ Schema only | Phase 4 |
| Profile switching UI | ❌ Not started | Phase 5 |
| Auto-detection | ❌ Not started | Phase 5 |

## See Also

- [Configuration Guide](configuration.md) - Full configuration documentation
- [Anti-Detection Overview](anti-detection-overview.md) - Detection vectors
- [Examples](../examples/README.md) - Example configuration files
- [Development Notes](development-notes.md) - Contributing to profiles
