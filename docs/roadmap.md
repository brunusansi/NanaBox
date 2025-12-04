# NanaBox Anti-Detection Edition - Roadmap

## Overview

This document outlines the roadmap for evolving NanaBox into a 100% automated, anti-detection-focused Hyper-V UI and orchestration tool, inspired by pve-anti-detection.

## Goals

- **Repository**: brunusansi/NanaBox (fork of M2Team/NanaBox)
- **Objective**: Create an advanced virtualization tool optimized for anti-detection use cases
- **Target Use Cases**: 
  - Gaming and anti-cheat testing laboratories (EAC, BattlEye, Vanguard, Byfron, mhyprot3, EA AC, etc.)
  - Security research and analysis
  - Virtualization experiments requiring bare-metal detection evasion

## Core Requirements

1. **Compatibility**: Maintain compatibility with upstream NanaBox UX where possible
2. **Declarative Configuration**: JSON-based configuration model with anti-detection extensions
3. **Dual Interface**: Both graphical UI and scriptable CLI surface
4. **Host-Side Automation**: Maximize host automation, use guest-side tooling only when necessary
5. **Modularity**: Anti-detection features should be optional and composable

## Architecture

### Core (C#/.NET)

The core NanaBox application will be extended to:
- Understand an enriched VM configuration schema including:
  - `smbios`: System Management BIOS information
  - `cpuId`: CPU identification and feature masking
  - `msrIntercept`: Model-Specific Register interception
  - `acpiOverride`: Advanced Configuration and Power Interface overrides
  - `network`: Network adapter configuration with MAC spoofing
  - `disk`: Disk configuration (physical vs virtual)
  - `gpu`: GPU configuration with device ID spoofing
  - `antiDetectionProfile`: High-level preset selection
- Map configuration sections to underlying Hyper-V/HCS APIs
- Integrate with PowerShell helpers for advanced scenarios

### Host Automation Layer (PowerShell + Helper Binaries)

Located in `scripts/host/`, responsibilities include:
- Hardware identity discovery (SMBIOS, ACPI, CPUID, GPU)
- Spoof configuration generation (smbios_clean.bin, dsdt_clean.aml)
- VHD EFI helper disk creation for boot-time injection
- VM-level settings: MAC addresses, integration services, GPU-PV configuration

### Guest Tools (Optional)

Located in `drivers/` and `scripts/guest/`:
- Kernel drivers: cpuid-spoof.sys, msr-spoof.sys, acpi-spoof.sys, tsc-spoof.sys
- PowerShell scripts for registry tweaks and driver management
- Modular and optional, used only when host-side spoofing is insufficient

### Profiles and Presets

The `antiDetectionProfile` field supports presets:
- **vanilla**: Plain NanaBox/Hyper-V behavior (default)
- **balanced**: Minimal anti-detection with maximum stability/performance
- **bare-metal**: Maximum anti-detection with feature/performance tradeoffs

Future phases will add per-game/anti-cheat profiles (e.g., "valorant", "roblox", "pubg").

## Phase Breakdown

### Phase 0 - Preparation

**Status**: In Progress

**Deliverables**:
- Directory structure established
- Core documentation written
- Example configurations provided

### Phase 1 - VM Creation UI & Settings UI

**Status**: Not Started

**Goals**:
- Provide friendly UI for VM creation and management
- Expose configuration for future anti-detection features
- Keep UI clean and usable

**Deliverables**:
- Working "New VM" wizard
- Working VM Settings UI
- Extended configuration schema with anti-detection placeholders
- No anti-detection logic implemented yet (structure only)

### Phase 2 - SMBIOS & Basic Anti-Detection

**Status**: Not Started

**Goals**:
- Implement SMBIOS spoofing
- Basic hardware identity masking
- Host-side automation scripts

### Phase 3 - CPUID & MSR Spoofing

**Status**: Not Started

**Goals**:
- Implement CPUID masking
- MSR interception where possible via Hyper-V
- Guest drivers for advanced scenarios

### Phase 4 - ACPI & Advanced Spoofing

**Status**: Not Started

**Goals**:
- ACPI table override support
- Advanced timing attack mitigation
- PCI device hiding

### Phase 5 - Profiles & Automation

**Status**: Not Started

**Goals**:
- Complete game/anti-cheat profile system
- One-click profiles for major anti-cheat systems (Vanguard, EAC, BattlEye, FACEIT, Tencent ACE, EA Javelin, etc.)
- Profile auto-detection and switching
- Full automation pipeline

**Profile Focus**:
- Game-specific profiles: `valorant`, `roblox`, `pubg`, `ea-ac`, etc.
- Anti-cheat generic profiles: `eac-generic`, `battleye`, `faceit`, `expert-tencent`, `ea-javelin`, etc.
- Custom user profiles: `my-custom-fps`, `default-gaming`, etc.

**Note**: Cloud provider profiles (AWS/Azure/GCP) are intentionally out of scope. This project focuses exclusively on gaming and anti-cheat use cases.

## References

- [pve-anti-detection](https://github.com/evilscript/pve-anti-detection) - Inspiration
- [Hyper-V Host Compute System API](https://docs.microsoft.com/en-us/virtualization/api/)
- NanaBox upstream documentation

## License

MIT License (maintained from upstream NanaBox)
