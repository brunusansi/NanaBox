# Anti-Detection Overview

## Introduction

This document provides a comprehensive overview of virtualization detection vectors and the mitigation strategies employed by NanaBox Anti-Detection Edition.

## Detection Vectors

### 1. SMBIOS (System Management BIOS)

**What It Is**: Firmware data structure that provides information about system hardware.

**Detection Methods**:
- Reading SMBIOS tables directly from firmware
- WMI queries (Win32_BIOS, Win32_ComputerSystem, Win32_BaseBoard)
- Registry keys containing SMBIOS information

**Common Hypervisor Signatures**:
- Manufacturer: "Microsoft Corporation"
- Product: "Virtual Machine"
- Version: Contains "Hyper-V" or "VRTUAL"
- Serial numbers with obvious virtual patterns
- Asset tags indicating virtualization

**Mitigation Strategy**:
- **Phase 1**: Configuration schema to store custom SMBIOS values
- **Phase 2**: Host-side SMBIOS injection via HCS API where available
- **Phase 2**: VHD EFI helper disk for boot-time SMBIOS override
- **Phase 2**: Mimic real hardware SMBIOS from popular manufacturers (Dell, HP, Lenovo, ASUS)

### 2. CPUID Instructions

**What It Is**: x86 instruction that returns processor identification and feature information.

**Detection Methods**:
- Checking hypervisor bit (CPUID.0x1.ECX bit 31)
- Reading hypervisor vendor string via CPUID.0x40000000
- Feature flags indicating virtualization (VMX, SVM)
- CPU model and family detection

**Common Hypervisor Signatures**:
- Hypervisor present bit set
- Vendor string "Microsoft Hv" for Hyper-V
- Reduced CPU features compared to bare metal

**Mitigation Strategy**:
- **Phase 1**: Configuration schema for CPUID masks
- **Phase 3**: Hide hypervisor present bit where Hyper-V allows
- **Phase 3**: Spoof vendor string to match real CPUs
- **Phase 3**: Guest-side CPUID driver for advanced spoofing when needed

### 3. MSR (Model-Specific Registers)

**What It Is**: Control registers specific to x86 processors, accessible via RDMSR/WRMSR instructions.

**Detection Methods**:
- Reading Hyper-V-specific MSRs (0x40000000-0x400000FF range)
- Checking virtualization feature MSRs
- Timing attacks using TSC (Time Stamp Counter) MSR

**Common Hypervisor Signatures**:
- Hyper-V Hypercall MSRs present and functional
- Virtual TSC behavior (synthetic timer, irregular frequency)
- VM-specific performance counters

**Mitigation Strategy**:
- **Phase 1**: Configuration schema for MSR intercepts
- **Phase 3**: Block access to Hyper-V MSR ranges via HCS
- **Phase 3**: Guest-side MSR filter driver when needed
- **Phase 4**: TSC frequency stabilization and normalization

### 4. ACPI (Advanced Configuration and Power Interface)

**What It Is**: Industry standard for power management and hardware configuration.

**Detection Methods**:
- Parsing ACPI tables (DSDT, SSDT, MADT, etc.)
- Looking for virtualization-specific devices and methods
- Checking for synthetic devices in ACPI namespace

**Common Hypervisor Signatures**:
- ACPI device IDs containing "Hyper-V", "VMBus", or "VMGEN"
- Synthetic ACPI tables with obvious virtual signatures
- Virtual device drivers and interfaces

**Mitigation Strategy**:
- **Phase 1**: Configuration schema for ACPI overrides
- **Phase 4**: Extract and modify ACPI tables
- **Phase 4**: Inject clean ACPI tables via EFI helper
- **Phase 4**: Remove or rename synthetic ACPI devices

### 5. PCI/PCIe Devices

**What It Is**: Peripheral Component Interconnect devices exposed to the guest OS.

**Detection Methods**:
- Enumerating PCI devices via configuration space
- Checking vendor/device IDs
- Looking for virtual device controllers

**Common Hypervisor Signatures**:
- Microsoft Virtual devices (vendor ID 0x1414)
- Synthetic network adapters
- Virtual GPU controllers
- VMBus and other para-virtualization devices

**Mitigation Strategy**:
- **Phase 2**: Hide integration services devices where possible
- **Phase 4**: Spoof PCI vendor/device IDs for synthetic devices
- **Phase 4**: Remove or hide VMBus and para-virtualization buses
- **Phase 5**: Physical device passthrough (GPU-PV, NIC-PV)

### 6. Timing Attacks

**What It Is**: Detecting virtualization through timing inconsistencies.

**Detection Methods**:
- RDTSC (Read Time-Stamp Counter) frequency analysis
- Instruction timing differences
- Hypercall detection via timing
- Cache timing side-channels

**Common Indicators**:
- Irregular TSC frequency
- VM exits causing timing spikes
- Synthetic timers with different behavior

**Mitigation Strategy**:
- **Phase 4**: TSC frequency normalization
- **Phase 4**: Reduce VM exits through optimizations
- **Phase 4**: Guest-side TSC spoofing driver
- **Phase 5**: Performance tuning and profiling

### 7. Network & MAC Addresses

**What It Is**: Network adapter identification and configuration.

**Detection Methods**:
- Checking MAC address OUI (first 3 bytes)
- Looking for synthetic network driver signatures
- Network timing and behavior analysis

**Common Hypervisor Signatures**:
- Microsoft OUI: 00-15-5D (Hyper-V default)
- Synthetic network adapter driver names
- Virtual switch behaviors

**Mitigation Strategy**:
- **Phase 1**: Configuration for custom MAC addresses
- **Phase 2**: Use real hardware OUIs (Intel, Realtek, etc.)
- **Phase 2**: Random MAC generation from common vendors
- **Phase 3**: Hide or disable synthetic network services

### 8. Registry & Files

**What It Is**: Windows registry keys and files indicating virtualization.

**Detection Methods**:
- Checking for Hyper-V integration services registry keys
- Looking for VM-specific services and drivers
- Scanning for VMBus and synthetic device drivers
- File system artifacts from VM tools

**Common Indicators**:
- HKLM\SOFTWARE\Microsoft\Virtual Machine\*
- Services: vmicheartbeat, vmickvpexchange, vmicshutdown, etc.
- Drivers: vmbus.sys, storvsc.sys, netvsc.sys, etc.
- Files in System32\drivers\

**Mitigation Strategy**:
- **Phase 3**: Guest-side PowerShell scripts to clean registry
- **Phase 3**: Disable or remove integration services
- **Phase 3**: Hide or rename VM-specific drivers
- **Phase 4**: Automated cleanup on VM first boot

### 9. Instruction Behavior

**What It Is**: CPU instruction behavior differences in VMs.

**Detection Methods**:
- Privileged instruction behavior (INT, CPUID, RDMSR)
- Invalid or unusual instruction results
- Exception handling differences
- Hypercall instruction presence (VMCALL, VMMCALL)

**Mitigation Strategy**:
- **Phase 3**: Guest-side instruction interception
- **Phase 4**: Normalize instruction timing and behavior
- **Phase 5**: Advanced CPU emulation for edge cases

### 10. Memory Artifacts

**What It Is**: Memory patterns and structures unique to VMs.

**Detection Methods**:
- Scanning memory for hypervisor signatures
- Checking for synthetic memory devices
- Looking for VM-specific memory management
- NUMA topology analysis

**Common Indicators**:
- Hyper-V hypercall pages in memory
- Synthetic interrupt controller structures
- Virtual NUMA nodes with suspicious topology

**Mitigation Strategy**:
- **Phase 4**: Memory layout normalization
- **Phase 4**: Hide hypercall pages
- **Phase 5**: Realistic NUMA topology spoofing

## Anti-Detection Profiles

### Vanilla Profile
- No anti-detection measures
- Standard Hyper-V VM configuration
- Maximum compatibility and performance
- Use for: Non-sensitive workloads, development

### Balanced Profile
- Basic SMBIOS spoofing
- MAC address randomization
- Integration services kept for functionality
- Moderate anti-detection with good performance
- Use for: Most gaming, light anti-cheat testing

### Bare-Metal Profile
- Maximum anti-detection effort
- All detection vectors addressed
- Integration services disabled
- Guest-side drivers and scripts deployed
- Performance may be impacted
- Use for: Advanced anti-cheat, security research

## Detection Testing

To verify anti-detection effectiveness, use these tools:

### Windows Tools
- **Pafish**: Popular VM detection tool
- **Al-Khaser**: Comprehensive malware analysis evasion tests
- **CPUID**: Raw CPUID information
- **HWiNFO**: Detailed hardware information
- **CPU-Z**: CPU and motherboard details

### Custom Scripts
- WMI queries for SMBIOS
- CPUID checks
- Registry enumeration
- Driver and service detection

### Anti-Cheat Systems
- Test with actual games and their anti-cheats
- Document detection methods used by each
- Create per-game profiles based on findings

## Best Practices

1. **Start Conservative**: Use balanced profile first, increase measures as needed
2. **Test Incrementally**: Enable one anti-detection feature at a time
3. **Monitor Performance**: Track VM performance impact of each measure
4. **Document Findings**: Keep notes on what works for specific games/anti-cheats
5. **Stay Updated**: Anti-cheat systems evolve, so must anti-detection

## References

- [pve-anti-detection](https://github.com/evilscript/pve-anti-detection)
- [Pafish - Paranoid Fish](https://github.com/a0rtega/pafish)
- [Al-Khaser](https://github.com/LordNoteworthy/al-khaser)
- Intel and AMD processor manuals for CPUID and MSR documentation
- ACPI Specification
- PCI/PCIe Specifications

## Contributing Detection Vectors

If you discover new detection methods, please document them:
1. Detection method and how it works
2. Specific indicators or signatures
3. Which anti-cheat/software uses it
4. Proposed mitigation strategy
5. Testing results

Submit via pull request or issue to help improve NanaBox Anti-Detection Edition.
