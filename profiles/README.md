# NanaBox Anti-Detection Profiles

This directory contains pre-configured profiles for various anti-detection scenarios, targeting compatibility with modern kernel-level and hybrid anti-cheat systems.

## Directory Structure

```
profiles/
├── smbios/          # SMBIOS templates for different hardware configurations
├── cpuid/           # CPUID profiles for various CPU models and anti-cheat systems
├── msr/             # MSR interception rules for hiding virtualization
└── README.md        # This file
```

## SMBIOS Profiles (`smbios/`)

SMBIOS profiles define realistic hardware identities for VMs, mimicking common desktop PC configurations from major manufacturers.

### Available Templates

| Template | Description | Chipset | Year | Use Case |
|----------|-------------|---------|------|----------|
| `desktop-intel-2019.json` | Gigabyte Z390 AORUS PRO | Intel Z390 | 2019 | General gaming, workstation |
| `desktop-amd-2021.json` | ASUS ROG STRIX B550-F | AMD B550 | 2021 | Gaming, content creation |
| `desktop-intel-2022.json` | ASUS ROG STRIX Z690-A | Intel Z690 | 2022 | High-end gaming (Alder Lake) |
| `desktop-amd-2022.json` | MSI MAG X570 TOMAHAWK | AMD X570 | 2022 | High-end gaming (Ryzen 5000) |

### Usage

1. Choose a SMBIOS template matching your target use case
2. Replace placeholder strings (`"Default string"`, `"System Serial Number"`, etc.) with unique values per VM
3. Ensure consistency: manufacturer, product, and serial numbers should match the selected template's vendor
4. Use the provided `scripts/host/` PowerShell scripts to generate unique identifiers

**Important**: Each VM should have unique serial numbers, UUIDs, and MAC addresses for proper multi-account isolation.

## CPUID Profiles (`cpuid/`)

CPUID profiles control how CPU identification and features are presented to the guest OS, hiding virtualization indicators.

### Available Templates

| Template | Description | CPU Model | Anti-Cheat Target |
|----------|-------------|-----------|-------------------|
| `intel-8c-2020.json` | Intel Core i7-10700K | Comet Lake, 8C/16T | Generic gaming |
| `amd-8c-2022.json` | AMD Ryzen 7 5800X3D | Zen 3, 8C/16T | Gaming (3D V-Cache) |
| `valorant-safe.json` | Riot Vanguard optimized | Intel i7-12700K | Valorant, Vanguard |
| `eac-generic.json` | Easy Anti-Cheat generic | Intel i7-11700K | EAC-protected games |

### Key Features

- **hideHypervisorBit**: Hides CPUID.1.ECX[31] (hypervisor present bit)
- **vendorId**: CPU vendor string ("GenuineIntel" or "AuthenticAMD")
- **brandString**: Full CPU brand name shown in OS
- **featureMasks**: Masks for hiding virtualization features
- **leafOverrides**: Precise control over specific CPUID leaves

### Anti-Cheat Compatibility

| Anti-Cheat | Profile | Criticality | Notes |
|------------|---------|-------------|-------|
| Riot Vanguard | `valorant-safe.json` | **CRITICAL** | Must hide hypervisor bit and all virtualization features |
| Easy Anti-Cheat (EAC) | `eac-generic.json` | Medium-High | Hide hypervisor bit, basic feature normalization |
| BattlEye | `eac-generic.json` | High | Similar to EAC with different heuristics |
| FACEIT AC | `eac-generic.json` | Medium-High | Comprehensive checks, focus on consistency |
| Expert Anti-Cheat (Tencent) | `valorant-safe.json` | High | Aggressive checks similar to Vanguard |
| EA Javelin | `eac-generic.json` | Medium | Less aggressive than Vanguard |

## MSR Profiles (`msr/`)

MSR (Model-Specific Register) profiles define rules for intercepting and controlling access to x86 MSRs, primarily to hide Hyper-V-specific registers.

### Available Templates

| Template | Description | Anti-Cheat Target |
|----------|-------------|-------------------|
| `default-gaming-safe.json` | Conservative gaming configuration | General gaming |
| `valorant-safe.json` | Riot Vanguard optimized | Valorant, Vanguard |
| `eac-safe.json` | Easy Anti-Cheat optimized | EAC-protected games |
| `battlEye-safe.json` | BattlEye optimized | PUBG, R6S, Tarkov |

### MSR Modes

- **zero**: Returns zero when reading the MSR (hides Hyper-V indicators)
- **mirror**: Mirrors the host's MSR value (for TSC, performance counters)
- **fake**: Returns a custom fake value (for advanced scenarios)

### Hyper-V MSR Range

The Hyper-V MSR range (0x40000000-0x400000FF) is the primary target for hiding virtualization:

| MSR | Purpose | Action |
|-----|---------|--------|
| 0x40000000 | Hyper-V interface identification | Block (return zero) |
| 0x40000001 | Hyper-V version | Block (return zero) |
| 0x40000002 | Hyper-V features | Block (return zero) |
| 0x40000003 | Hyper-V enlightenment | Block (return zero) |
| 0x40000080 | Hyper-V guest OS ID | Block (return zero) |
| 0x40000081 | Hyper-V hypercall page | Block (return zero) |
| 0x40000082 | Hyper-V VP index | Block (return zero) |

### Anti-Cheat Requirements

| Anti-Cheat | MSR Blocking | TSC Requirements | Criticality |
|------------|--------------|------------------|-------------|
| Riot Vanguard | **ALL** Hyper-V MSRs | Stable, consistent | **CRITICAL** |
| Easy Anti-Cheat | Main Hyper-V MSRs | Basic stability | Medium-High |
| BattlEye | Comprehensive blocking | Stable TSC | High |
| FACEIT AC | Main Hyper-V MSRs | Consistent | Medium-High |

## Profile Combinations

For complete anti-detection, combine SMBIOS, CPUID, and MSR profiles:

### Example: Valorant Gaming VM

```json
{
  "AntiDetectionProfile": "bare-metal",
  "Smbios": {
    "Enabled": true,
    "Template": "desktop-intel-2022",
    "Vendor": "ASUSTeK COMPUTER INC.",
    "Product": "ROG STRIX Z690-A GAMING WIFI D4",
    ...
  },
  "CpuId": {
    "Enabled": true,
    "Templates": ["valorant-safe"],
    "HideHypervisorBit": true,
    ...
  },
  "MsrIntercept": {
    "Enabled": true,
    "Template": "valorant-safe",
    ...
  },
  "Timing": {
    "Enabled": true,
    "Mode": "strict"
  }
}
```

### Example: Generic Gaming VM (EAC)

```json
{
  "AntiDetectionProfile": "balanced",
  "Smbios": {
    "Enabled": true,
    "Template": "desktop-amd-2021"
  },
  "CpuId": {
    "Enabled": true,
    "Templates": ["eac-generic", "amd-8c-2022"]
  },
  "MsrIntercept": {
    "Enabled": true,
    "Template": "eac-safe"
  },
  "Timing": {
    "Enabled": true,
    "Mode": "balanced"
  }
}
```

## Generating Unique Identifiers

Use the provided PowerShell scripts in `scripts/host/`:

### Extract SMBIOS from Host
```powershell
.\scripts\host\Extract-SMBIOS.ps1 -OutputPath "my-hardware.json" -IncludeSensitive
```

### Generate Unique MAC Address
```powershell
.\scripts\host\New-RandomMAC.ps1 -VendorPrefix "D8-9E-F3"  # ASUS
.\scripts\host\New-RandomMAC.ps1 -VendorPrefix "AC-DE-48"  # Gigabyte
.\scripts\host\New-RandomMAC.ps1 -VendorPrefix "00-D8-61"  # MSI
```

### Clone Profile with Unique IDs
```powershell
.\scripts\host\Copy-NanaBoxProfile.ps1 -SourceProfile "valorant-main" -TargetProfile "valorant-alt"
```

## Best Practices

### 1. Uniqueness
- **Every VM must have unique identifiers**: Serial numbers, UUIDs, MAC addresses
- Reusing identifiers across VMs can lead to detection and bans
- Use the provided scripts to generate unique values

### 2. Consistency
- Ensure all fields match the selected manufacturer (ASUS, MSI, Gigabyte, etc.)
- Don't mix manufacturers (e.g., ASUS motherboard with Dell BIOS)
- Keep serial number patterns consistent with the manufacturer

### 3. Realism
- Use real hardware models and serial number patterns
- Avoid obvious fake values like "0000000000" or "Default string"
- Study real hardware using `dmidecode` or WMI queries

### 4. Profile Selection
- **Valorant/Vanguard**: Use strictest profiles (`valorant-safe`)
- **EAC/BattlEye**: Use balanced profiles (`eac-generic`, `battlEye-safe`)
- **Casual gaming**: Use relaxed profiles (`default-gaming-safe`)

### 5. Testing
- Test with VM detection tools before deploying:
  - `scripts/guest/Test-VMDetection.ps1`
  - Pafish (https://github.com/a0rtega/pafish)
  - Al-Khaser (https://github.com/LordNoteworthy/al-khaser)

## Implementation Status

| Component | Status | Notes |
|-----------|--------|-------|
| SMBIOS Profiles | ✅ Complete | 4 templates available |
| CPUID Profiles | ✅ Complete | 4 templates (Intel, AMD, anti-cheat specific) |
| MSR Profiles | ✅ Complete | 4 templates (generic, Vanguard, EAC, BattlEye) |
| Host-side Injection | ⏳ Phase 2 | Requires Windows implementation |
| Guest-side Drivers | ⏳ Phase 3 | Kernel drivers for CPUID/MSR control |
| Profile Manager | ⏳ Phase 5 | Centralized profile selection and switching |

## Anti-Cheat System Reference

### Kernel-Level Anti-Cheats
- **Riot Vanguard** (Valorant): Most aggressive, kernel-mode driver, comprehensive checks
- **BattlEye** (PUBG, R6S, Tarkov): Kernel-mode, comprehensive MSR and CPUID checks
- **Easy Anti-Cheat** (Fortnite, Apex): Kernel-mode, widely used, medium-high detection
- **FACEIT AC** (CS:GO): Kernel-mode, tournament-grade

### Hybrid Anti-Cheats
- **Expert Anti-Cheat / Tencent** (PUBG Mobile on PC emulators): Aggressive, Vanguard-like
- **EA Javelin** (EA games): Hybrid approach, less aggressive
- **Ricochet** (Call of Duty): Kernel-level, AI-powered heuristics

## Contributing

To add new profiles:

1. Create profile JSON in appropriate directory
2. Follow existing naming conventions
3. Include comprehensive notes and anti-cheat compatibility info
4. Test with target anti-cheat system
5. Document any special requirements or limitations

## See Also

- [Anti-Detection Overview](../docs/anti-detection-overview.md)
- [Configuration Guide](../docs/configuration.md)
- [Host-Side Scripts](../scripts/host/README.md)
- [Guest-Side Tools](../scripts/guest/README.md)

## License

MIT License (maintained from upstream NanaBox project)
