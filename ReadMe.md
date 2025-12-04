# ![NanaBox](Assets/NanaBox.png) NanaBox

[![GitHub Actions Build Status](https://github.com/M2Team/NanaBox/actions/workflows/BuildBinaries.yml/badge.svg?branch=main&event=push)](https://github.com/M2Team/NanaBox/actions/workflows/BuildBinaries.yml?query=event%3Apush+branch%3Amain)
[![Total Downloads](https://img.shields.io/github/downloads/M2Team/NanaBox/total)](https://github.com/M2Team/NanaBox/releases)

[![Windows Store - Release Channel](https://img.shields.io/badge/Windows%20Store-Release%20Channel-blue)](https://www.microsoft.com/store/apps/9NJXJSCB2JK0)
[![Windows Store - Preview Channel](https://img.shields.io/badge/Windows%20Store-Preview%20Channel-blue)](https://www.microsoft.com/store/apps/9NCBGTS09QJJ)

[![Latest Version - Release Channel](https://img.shields.io/github/v/release/M2Team/NanaBox?display_name=release&sort=date&color=%23a4a61d)](https://github.com/M2Team/NanaBox/releases/latest)
[![Latest Version - Preview Channel](https://img.shields.io/github/v/release/M2Team/NanaBox?include_prereleases&display_name=release&sort=date&color=%23a4a61d)](https://github.com/M2Team/NanaBox/releases)

[![Latest Release Downloads - Release Channel](https://img.shields.io/github/downloads/M2Team/NanaBox/latest/total)](https://github.com/M2Team/NanaBox/releases/latest)
[![Latest Release Downloads - Preview Channel](https://img.shields.io/github/downloads-pre/M2Team/NanaBox/latest/total)](https://github.com/M2Team/NanaBox/releases)

![Screenshot](Documents/Screenshot.png)

> **⚠️ NanaBox Anti-Detection Edition**  
> This repository is an experimental **Anti-Detection Edition** fork of [M2Team/NanaBox](https://github.com/M2Team/NanaBox), adding advanced configuration, profiles, and tooling for gaming and anti-cheat oriented scenarios (EAC, BattlEye, Vanguard, FACEIT, Tencent ACE, EA Javelin, etc.).  
>   
> **Upstream ownership and credits remain with M2Team and the original author (Kenji Mouri)**. This fork adds experimental anti-detection features on top of the upstream foundation. For the original NanaBox project, please visit [M2Team/NanaBox](https://github.com/M2Team/NanaBox).

NanaBox is a third-party lightweight XAML-based out-of-box-experience oriented
Hyper-V virtualization software based on Host Compute System API, Remote Desktop
ActiveX control, and XAML Islands.

NanaBox is not a Hyper-V client because Host Compute System API is a low-level API
of Hyper-V WMI Providers which is used in Hyper-V Manager, and Host Compute System
API is stateless which is not available to manage virtual machines listed in
Hyper-V Manager.

NanaBox chooses Host Compute System API instead of Hyper-V WMI Providers because
the author (Kenji Mouri) wants to have portable virtual machine configurations
and finds the process of registering virtual machine configurations into the system
disgusting.

NanaBox chooses to define its own JSON-based virtual machine configurations format
because Hyper-V's VMCX is a binary format with no documented format definitions
from Microsoft.

**All kinds of contributions are appreciated. All suggestions, pull 
requests, and issues are welcome.**

If you want to sponsor the development of NanaBox, please read the document about
[NanaBox Sponsor Edition](Documents/SponsorEdition.md). It's free for all
NanaBox source code repository contributors.

In general, NanaBox Sponsor Edition is more like a contributors' edition, but
we provide a way to purchase the honor and make someone appear as a source code
contributor, because NanaBox is always a community-friendly open-source project.

If you'd like me to add features or improvements ahead of time, please use
[paid services](https://github.com/MouriNaruto/MouriNaruto/blob/main/PaidServices.md).

## NanaBox Anti-Detection Edition

This **Anti-Detection Edition** extends the upstream NanaBox with features designed for bare-metal-like virtualization that can evade common anti-cheat detection methods. The primary goals are:

- **Gaming & Anti-Cheat Compatibility**: Create virtual machines that appear as close to physical hardware as possible for testing and running games protected by modern anti-cheat systems (EAC, BattlEye, Vanguard, FACEIT, Tencent ACE, EA Javelin, Byfron, mhyprot3, etc.).
- **Per-Game/Per-AC Profiles**: Provide pre-configured profiles optimized for specific games and anti-cheat engines, allowing quick deployment without manual configuration.
- **Multi-Account Isolation**: Enable safe multi-account scenarios with hardware identity isolation to prevent cross-contamination between VM instances.
- **Security Research**: Support virtualization experiments requiring detection evasion for legitimate security research and analysis.

**Important**: This project focuses **exclusively on gaming and anti-cheat oriented use cases**. Cloud provider profiles (AWS/Azure/GCP) are intentionally out of scope.

### Anti-Detection Features

The Anti-Detection Edition adds the following capabilities:

- **JSON-based Anti-Detection Configuration**: Extended configuration schema supporting SMBIOS spoofing, CPU ID masking, MSR interception, ACPI overrides, timing adjustments, and PCI device control.
- **Profile System**: Pre-configured profiles for popular games and anti-cheat systems:
  - `valorant` - Riot Vanguard optimized
  - `eac-generic` - Easy Anti-Cheat support
  - `battleye` - BattlEye anti-cheat
  - `faceit` - FaceIT anti-cheat (CS:GO, CS2)
  - `expert-tencent` - Tencent ACE
  - `ea-javelin` - EA Javelin protection
  - `default-gaming` - Basic gaming anti-detection
  - `balanced` - Moderate anti-detection with good performance
  - `bare-metal` - Maximum anti-detection effort
- **UI & CLI Integration**: Select and manage anti-detection profiles through the NanaBox UI or command-line tools.
- **Hardware Identity Spoofing**: Configurable SMBIOS, CPU features, MAC addresses, disk identifiers, GPU device IDs to mimic real hardware.
- **Multi-Account Isolation**: Profile cloning and per-VM identity management for safe multi-account scenarios.

### Anti-Detection Documentation

For detailed information about the anti-detection features, please refer to:

- **[Anti-Detection Roadmap](docs/roadmap.md)** - Development phases and feature timeline
- **[Anti-Detection Overview](docs/anti-detection-overview.md)** - Detection vectors and mitigation strategies
- **[Configuration Reference](docs/configuration.md)** - Extended JSON schema and configuration options
- **[Profile System Guide](docs/profiles.md)** - Available profiles and how to use them
- **[Development Notes](docs/development-notes.md)** - Technical implementation details
- **[Multi-Account Isolation](docs/multi-account-isolation.md)** - Multi-account setup and best practices

### Experimental Features Warning

⚠️ **The anti-detection features in this edition are experimental and under active development.** These features involve low-level system manipulation (drivers, EFI, hardware spoofing) and should only be used on test machines in controlled environments. Use at your own risk.

## Features

### Core Features (from upstream NanaBox)

- Based on Host Compute System API as virtualization backend.
- Supports Hyper-V Enhanced Session Mode for Windows Guests.
- Supports Hyper-V Nested Virtualization.
- Supports Hyper-V GPU paravirtualization solution (a.k.a GPU-PV).
- Supports exposing COM ports and physical drives to the virtual machine.
- Supports enabling Secure Boot for virtual machines.
- Uses JSON-based format to support portable virtual machines.
- Packaged with MSIX for modern deployment experience.
- Modernizes the UI with XAML Islands including dark and light mode support.
- Full High DPI and partial Accessibility support.
- Supports full screen mode.
- Multi-language Support: English and Simplified Chinese.
- Enables the relative mouse mode in Windows 11 Version 24H2 or later Host OS.
- Supports the Trusted Platform Module (TPM) 2.0 for virtual machines in Windows
  11 Version 24H2 or later Host OS. (Although Windows Server 2022 introduced
  the related Host Compute System API interfaces, it seems they are not implemented.)
- Supports customizing partial System Management BIOS (SMBIOS) information for
  virtual machines.
- Supports cursor confinement with Ctrl+Alt+Left/Right shortcut key to improve
  user experience for some applications. (Press the same shortcut key to release
  cursor confinement.)
- Supports Plan 9 protocol over Hyper-V Socket for both read-only and read/write
  file sharing between host and guest.

### Anti-Detection Edition Features

- **Extended JSON Configuration Schema**: Anti-detection configuration sections including:
  - `AntiDetectionProfile` - High-level profile selection
  - `Smbios` - System Management BIOS customization
  - `CpuId` - CPU identification and feature masking
  - `MsrIntercept` - Model-Specific Register interception
  - `AcpiOverride` - ACPI table overrides
  - `Timing` - Timing-related adjustments
  - `Pci` - PCI device configuration
- **Gaming/Anti-Cheat Focused Profiles**: Pre-configured profiles optimized for specific anti-cheat systems (Vanguard, EAC, BattlEye, FACEIT, Tencent ACE, EA Javelin, etc.)
- **UI Integration**: Anti-Detection Settings dialog for profile selection and management
- **CLI Support**: Command-line tools for automation and scripting
- **Profile Cloning**: Helpers for multi-account isolation and per-game profile management

## System Requirements

- Supported Operating Systems:
  - Windows 10, version 2004 (Build 19041) or later
  - Windows Server 2022 (Build 20348) or later
- Supported Platforms:
  - x86 (64-bit)
  - ARM (64-bit)

In general, NanaBox follows the 2025 baseline defined by Kenji Mouri's document,
read [MD23: The baselines of Windows targets for all my open-source projects]
for more information.

[MD23: The baselines of Windows targets for all my open-source projects]: https://github.com/MouriNaruto/MouriDocs/tree/main/docs/23

## Known issues

- Due to Host Compute System API and Host Compute Network API limitations, 
  there are some limitations from them that also apply to NanaBox:
  - NanaBox only supports creating UEFI Class 3 or UEFI without CSM or Hyper-V 
    Gen 2 virtual machines.
  - NanaBox needs elevated privilege via UAC.
  - The automatic switch between Enhanced Session Mode and Basic Session Mode
    won't be implemented.
  - You shouldn't put your virtual machine configuration file at the root of
    the drive, or you will encounter an access denied error.
- Due to the Microsoft Store Licensing API implementation, you will find that NanaBox
  will try to access the Internet starting with NanaBox 1.2 because NanaBox
  needs to use it to check the Sponsor Edition addon licensing status.
- To optimize the user experience, NanaBox will only check the Sponsor 
  Edition addon licensing status the first time you launch NanaBox. You
  can click the sponsor button to refresh the status after you
  have purchased or redeemed the Sponsor Edition addon.

### Anti-Detection Edition Specific Issues

- **Experimental Features**: Anti-detection capabilities (drivers, EFI helpers, advanced spoofing) are experimental and under active development. They may not work perfectly with all anti-cheat systems.
- **Test Environment Only**: Anti-detection features involve low-level system manipulation and should only be used on dedicated test machines in controlled environments.
- **Profile Effectiveness**: The effectiveness of anti-detection profiles varies by anti-cheat system and may require updates as anti-cheat technologies evolve.

Reference documents for known issues:

- [MD4: Notes for using Host Compute System API](https://github.com/MouriNaruto/MouriDocs/tree/main/docs/4)

## Tips for users who want to only enable Virtual Machine Platform feature

You need to execute the [GenerateVmmsCertificate.ps1](Documents/GenerateVmmsCertificate.ps1)
script (or from your binary packages) when using NanaBox for the first time or after
modifying your computer name.

> PowerShell -NoLogo -NoProfile -NonInteractive -InputFormat None -ExecutionPolicy Bypass "The path of the GenerateVmmsCertificate.ps1"

Also, you are unable to enable TPM support when using NanaBox with the Virtual
Machine Platform feature only.

## Development Roadmap

### Upstream NanaBox Roadmap

- Future Series (T.B.D.)
  - [ ] Add friendly virtual machine creation UI and settings UI.
  - [ ] Provide tool for Hyper-V vmcx migration. (Suggested by Belleve.)
  - [ ] Add support for setting RDP options with .rdp file. (Suggested by 
        awakecoding.)
  - [ ] Add support for connecting virtual machine with MsRdpEx from 
        Devolutions. (Suggested by awakecoding.)

### Anti-Detection Edition Roadmap

For the Anti-Detection Edition specific roadmap and development phases, please see **[docs/roadmap.md](docs/roadmap.md)**.

The Anti-Detection Edition roadmap includes:
- Phase 1: VM Creation UI & Settings UI (In Progress)
- Phase 2: SMBIOS & Basic Anti-Detection
- Phase 3: CPUID & MSR Spoofing
- Phase 4: ACPI & Advanced Spoofing
- Phase 5: Profiles & Automation

**Note**: The Anti-Detection Edition roadmap is maintained separately and may diverge from the upstream roadmap as we focus on gaming and anti-cheat specific features.

## Documents

### Upstream NanaBox Documentation

- [License](License.md)
- [Relevant People](Documents/People.md)
- [Privacy Policy](Documents/Privacy.md)
- [Code of Conduct](CODE_OF_CONDUCT.md)
- [Contributing Guide](CONTRIBUTING.md)
- [Release Notes](Documents/ReleaseNotes.md)
- [Versioning](Documents/Versioning.md)
- [NanaBox Configuration File Reference](Documents/ConfigurationReference.md)
- [NanaBox Sponsor Edition](Documents/SponsorEdition.md)
- [My Digital Life Forums](https://forums.mydigitallife.net/threads/88560)

### Anti-Detection Edition Documentation

- [Anti-Detection Roadmap](docs/roadmap.md)
- [Anti-Detection Overview](docs/anti-detection-overview.md)
- [Configuration Reference](docs/configuration.md)
- [Profile System Guide](docs/profiles.md)
- [Development Notes](docs/development-notes.md)
- [Multi-Account Isolation](docs/multi-account-isolation.md)

## Credits and Contributions

**NanaBox Anti-Detection Edition** is maintained by [brunusansi](https://github.com/brunusansi). All upstream NanaBox credits, ownership, and Sponsor Edition benefits remain with the original author **Kenji Mouri** and the **M2Team** organization.

Contributions to the Anti-Detection Edition features should be directed to this repository ([brunusansi/NanaBox](https://github.com/brunusansi/NanaBox)). For contributions to the core NanaBox functionality, please consider contributing to the upstream repository ([M2Team/NanaBox](https://github.com/M2Team/NanaBox)).
