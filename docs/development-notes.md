# NanaBox Anti-Detection Edition - Development Notes

## Overview

This document provides guidelines for developers contributing to the NanaBox Anti-Detection Edition project. It covers architecture, implementation guidelines, security considerations, and best practices.

## Project Structure

```
NanaBox/
├── NanaBox/                          # Core application (C++/WinRT)
│   ├── NanaBox.Configuration.Specification.h
│   ├── NanaBox.Configuration.Parser.h/cpp
│   └── [Other source files]
├── docs/                             # Documentation
│   ├── roadmap.md
│   ├── anti-detection-overview.md
│   ├── configuration.md
│   ├── profiles.md
│   ├── phase1-summary.md
│   └── development-notes.md (this file)
├── examples/                         # Example configurations
│   ├── basic-vm.json
│   ├── bare-metal-profile.json
│   ├── profile-valorant.json
│   ├── profile-roblox.json
│   ├── profile-pubg.json
│   └── profile-ea-ac.json
├── scripts/                          # Automation scripts
│   ├── host/                         # Host-side automation
│   └── guest/                        # Guest-side tools
├── Documents/                        # Upstream documentation
└── README.md
```

## Architecture Layers

### Layer 1: Configuration & Management (Phase 1) ✅
**Status**: Complete  
**Language**: C++/WinRT  
**Components**:
- Configuration schema (NanaBox.Configuration.Specification.h)
- JSON parser (NanaBox.Configuration.Parser.h/cpp)
- Metadata tracking
- Profile management

**Responsibilities**:
- Parse and validate VM configurations
- Serialize/deserialize JSON
- Track VM metadata (creation time, account ID, etc.)
- Provide configuration API to upper layers

**Development Guidelines**:
- Follow existing NanaBox code style
- Add XML documentation comments
- Include phase annotations in TODO comments
- Maintain backward compatibility
- Validate all inputs

### Layer 2: Host Automation (Phase 2-4) ⚠️
**Status**: Schema ready, implementation pending  
**Language**: PowerShell, C++ helper binaries  
**Components**:
- SMBIOS extraction and injection
- ACPI table manipulation
- VHD/VHDX management
- Network configuration automation

**Responsibilities**:
- Extract hardware information from host or templates
- Generate realistic SMBIOS binaries
- Create and manage VHD/VHDX files
- Configure network adapters with custom MAC addresses
- Inject anti-detection data via Hyper-V/HCS APIs

**Development Guidelines**:
- Prioritize host-side solutions over guest-side
- Use HCS (Host Compute System) APIs where possible
- Provide robust error handling and logging
- Support both interactive and automated workflows
- Document OS version requirements

### Layer 3: Guest Tools (Phase 3-4) 🚧
**Status**: Driver skeleton implemented, interception pending  
**Language**: C (kernel drivers), PowerShell (user-mode)  
**Components**:
- nanabox_hvfilter.sys (Unified hypervisor filter driver - SKELETON ONLY)
- NbxHvFilterClient.exe (User-mode helper utility)
- Guest-side registry tweaks (future)

**Implemented (Phase 3A)**:
- ✅ Driver device creation and symbolic link
- ✅ IRP dispatch handlers (CREATE, CLOSE, DEVICE_CONTROL)
- ✅ IOCTL interface (SET_PROFILE, GET_STATUS, CLEAR_PROFILE)
- ✅ User-mode helper utility with CLI interface
- ✅ Logging and diagnostics
- ✅ Safe error handling

**Pending Implementation (Phase 3B)**:
- ⏳ CPUID instruction interception
- ⏳ MSR read/write filtering
- ⏳ Timing normalization hooks
- ⏳ PCI topology control

**Responsibilities**:
- Intercept CPUID instructions in guest OS (NOT YET IMPLEMENTED)
- Filter MSR reads/writes (NOT YET IMPLEMENTED)
- Override ACPI tables visible to guest OS (FUTURE)
- Provide user-mode control interface (IOCTLs) ✅

**Development Guidelines**:
- Follow Windows Driver Development guidelines
- Use test-signing for development
- See docs/driver-hvfilter.md for detailed documentation
- Implement proper cleanup on driver unload
- Provide safe fallbacks for failures
- Document security implications clearly
- Include uninstall procedures

### Layer 4: UI & User Experience (Phase 1+) ⚠️
**Status**: Not implemented (requires Windows environment)  
**Language**: C++/WinRT, XAML  
**Components**:
- New VM Wizard (NewVirtualMachinePage)
- VM Settings UI (VirtualMachineSettingsPage)
- Profile selector
- Anti-detection configuration tabs

**Responsibilities**:
- Provide user-friendly VM creation workflow
- Expose anti-detection settings in UI
- Allow profile selection and customization
- Validate user inputs
- Display status and warnings

**Development Guidelines**:
- Follow XAML Islands patterns
- Maintain existing NanaBox UI style
- Support both light and dark themes
- Provide tooltips and help text
- Test with High DPI displays

## Code Style Guidelines

### C++ Style
Follow the existing NanaBox style:

```cpp
// Good: Use Mile library patterns
Result.Enabled = Mile::Json::ToBoolean(
    Mile::Json::GetSubKey(Value, "Enabled"),
    Result.Enabled);

// Good: XML documentation comments
/// <summary>
/// CPUID spoofing configuration
/// Phase 1: Configuration schema only (reserved for future use)
/// Phase 3: Implementation with guest-side driver
/// </summary>
struct CpuIdConfiguration
{
    bool Enabled = false;  // Enable CPUID spoofing
};

// Good: Phase annotations
// TODO(Phase3): Implement CPUID hypervisor hiding
// TODO(Phase4): Add TSC normalization support
```

### PowerShell Style
```powershell
# Good: Use approved verbs
function Get-SMBIOSData { }
function New-VMProfile { }
function Set-AntiDetectionConfig { }

# Good: Document parameters
<#
.SYNOPSIS
    Extracts SMBIOS information from the host system.
.PARAMETER OutputPath
    Path to save the extracted SMBIOS binary.
#>
```

### JSON Style
```json
{
    "NanaBox": {
        "Version": 1,
        "Metadata": {
            "Description": "User-friendly description",
            "ProfileId": "profile-type"
        }
    }
}
```

## Phase Implementation Guidelines

### Phase 1: Configuration & Schema ✅
**Deliverables**:
- ✅ Configuration structs defined
- ✅ Parser implementation complete
- ✅ Example configurations provided
- ✅ Documentation written

**Testing**:
- Validate JSON parsing
- Test backward compatibility
- Verify all example configs load

### Phase 2: SMBIOS & Host Automation ⚠️
**Deliverables**:
- PowerShell scripts for SMBIOS extraction
- SMBIOS injection via HCS API or EFI helper
- MAC address generation utilities
- VHD management scripts

**Implementation Steps**:
1. Research HCS API SMBIOS support
2. Implement extraction scripts
3. Create SMBIOS binary format writer
4. Test with various hardware configurations
5. Document limitations and workarounds

**Testing**:
- Verify SMBIOS values appear in guest OS
- Test with WMI queries in guest
- Compare against real hardware
- Test on multiple Windows versions

### Phase 3: CPUID & MSR Drivers 🚧
**Status**: Phase 3A (Skeleton) Complete ✅ | Phase 3B (Interception) Pending ⏳

**Deliverables**:
- ✅ nanabox_hvfilter.sys - Unified driver skeleton
- ✅ NbxHvFilterClient.exe - User-mode helper utility
- ✅ IOCTL interface for profile management
- ✅ Installation and signing documentation
- ✅ Automated setup scripts for Phase 3B testing
- ⏳ CPUID interception implementation (Phase 3B)
- ⏳ MSR filtering implementation (Phase 3B)

**Phase 3A (Complete)**:
1. ✅ Design driver architecture
2. ✅ Create device and symbolic link
3. ✅ Implement IOCTL interface
4. ✅ Create user-mode helper
5. ✅ Document build and installation process
6. ✅ Create automated setup/testing scripts

**Phase 3B Setup Automation (Complete)**:
1. ✅ Setup-NbxHvFilterPhase3B.ps1 - Automated build/install/test script
2. ✅ Integration with Setup-NanaBoxDevEnv.ps1 via -Phase3B parameter
3. ✅ PHASE3B_TESTING.md - Comprehensive testing guide
4. ✅ Updated documentation in driver-hvfilter.md
5. ✅ Quick Start guide in scripts/setup/README.md

**Phase 3B Interception (Pending)**:
1. ⏳ Implement CPUID instruction hooking
2. ⏳ Implement MSR read/write interception
3. ⏳ Add profile-based configuration loading
4. ⏳ Test with anti-cheat detection tools
5. ⏳ Performance optimization

**Testing**:
- ✅ Verify driver loads and unloads cleanly
- ✅ Test IOCTL communication with user-mode
- ✅ Validate input buffer handling
- ✅ Automated setup/test with Setup-NbxHvFilterPhase3B.ps1
- ✅ See PHASE3B_TESTING.md for complete test scenarios
- ⏳ Test CPUID spoofing with CPU-Z (Phase 3B interception)
- ⏳ Test MSR blocking with RWEverything (Phase 3B interception)
- ⏳ Test with Pafish, Al-Khaser (Phase 3B interception)
- ⏳ Verify stability under load (Phase 3B interception)

**Security Considerations**:
- ⚠️ Kernel drivers require extreme care
- ⚠️ Improper implementation can cause BSOD
- ⚠️ Use test-signing during development
- ⚠️ Document all security implications
- ⚠️ Provide safe uninstall procedures
- ✅ Current skeleton is safe - no CPU modification

### Phase 4: ACPI & Timing ⚠️
**Deliverables**:
- ACPI table extraction and modification tools
- TSC normalization implementation
- APIC/HPET timing adjustments
- EFI helper disk for boot-time injection

**Implementation Steps**:
1. Extract ACPI tables from running system
2. Implement table modification tools
3. Create EFI helper bootloader
4. Implement timing normalization
5. Test with timing-sensitive applications

**Testing**:
- Verify ACPI tables in guest OS
- Test timing accuracy
- Measure performance impact
- Test boot compatibility

### Phase 5: Profile System & Automation ⚠️
**Deliverables**:
- Profile database
- One-click profile switching
- Profile inheritance system
- Auto-detection hooks (stretch goal)

**Implementation Steps**:
1. Design profile database format
2. Implement profile switching logic
3. Create UI for profile management
4. Test profile inheritance
5. Document profile creation process

## Testing Strategy

### Unit Testing
- Configuration parsing and validation
- SMBIOS data extraction
- Profile loading and application

### Integration Testing
- End-to-end VM creation workflow
- Profile switching with running VMs
- Anti-detection feature combinations

### Detection Testing
Use these tools to validate anti-detection:
- **Pafish**: Basic VM detection
- **Al-Khaser**: Comprehensive detection tests
- **CPU-Z**: CPU identification verification
- **HWiNFO**: Hardware information validation
- **Game anti-cheats**: Real-world validation

### Compatibility Testing
- Windows 10 20H2+ (Host)
- Windows 11 24H2+ (Host, for TPM)
- Windows 10/11 (Guest)
- Various CPU vendors (Intel, AMD)
- Multiple motherboard manufacturers

## Security Considerations

### Kernel-Level Components
⚠️ **WARNING**: Kernel drivers have severe security implications.

**Guidelines**:
1. **Never run untrusted code in kernel mode**
2. **Use test-signing only on dedicated test machines**
3. **Implement proper input validation**
4. **Use safe memory operations**
5. **Provide emergency disable mechanisms**
6. **Document all security risks clearly**

### Ethical Use
This project is intended for:
- ✅ Security research
- ✅ Anti-cheat testing laboratories
- ✅ Educational purposes
- ✅ Legitimate gaming use cases

**Not intended for**:
- ❌ Cheating in online games
- ❌ Bypassing software licensing
- ❌ Malicious purposes
- ❌ Violation of terms of service

### Responsible Disclosure
If you discover security vulnerabilities:
1. Do not disclose publicly
2. Report privately to maintainers
3. Allow time for fixes before disclosure
4. Follow responsible disclosure practices

## Build System

### Requirements
- Visual Studio 2022
- Windows SDK 10.0.20348.0+
- C++/WinRT support
- MSBuild or VS Build Tools

### Building
```cmd
# Build all targets
BuildAllTargets.cmd

# Build specific configuration
msbuild NanaBox.slnx /p:Configuration=Release /p:Platform=x64
```

### Testing Configuration Parsing
```cpp
// Load and validate configuration
auto config = NanaBox::ToVirtualMachineConfiguration(jsonValue);

// Verify anti-detection fields
assert(config.AntiDetectionProfile == NanaBox::AntiDetectionProfile::BareMetal);
assert(config.CpuId.Enabled == true);
assert(config.Metadata.ProfileId == "valorant");
```

## Documentation Standards

### Code Documentation
- XML comments for all public APIs
- Phase annotations in TODO comments
- Security warnings for dangerous operations
- Examples in comments where helpful

### User Documentation
- Clear use case explanations
- Step-by-step guides
- Security and ethical considerations
- Troubleshooting sections
- Links to related documentation

### Example Documentation
- Purpose and use case
- Configuration highlights
- Expected behavior
- Compatibility notes
- Testing recommendations

## Contributing Guidelines

### Before Contributing
1. Read this document completely
2. Understand the phase roadmap
3. Review existing code style
4. Test your changes thoroughly
5. Update documentation

### Pull Request Checklist
- [ ] Code follows existing style
- [ ] XML documentation added
- [ ] Phase annotations included
- [ ] Security implications documented
- [ ] Tests added/updated
- [ ] Documentation updated
- [ ] Examples provided (if applicable)
- [ ] Backward compatibility maintained

### Commit Message Format
```
[Phase X] Brief description

Detailed explanation of changes.

- Bullet points for major changes
- Reference issues if applicable
- Note any breaking changes
```

## Debugging Tips

### Configuration Parsing
- Enable verbose JSON parsing errors
- Validate against schema before parsing
- Test with minimal and maximal configs
- Check for missing required fields

### SMBIOS Issues
- Verify values in guest with WMI queries
- Compare against real hardware
- Check HCS API version support
- Test on different Windows versions

### Driver Issues
- Use automated setup script: `scripts\setup\Setup-NbxHvFilterPhase3B.ps1`
- Use WinDbg for kernel debugging
- Enable Driver Verifier for testing
- Check Event Viewer for errors
- Use test VMs, never production systems
- See PHASE3B_TESTING.md for troubleshooting guide

### Anti-Detection Failures
- Test with detection tools first
- Enable features incrementally
- Check for conflicting settings
- Verify OS/HCS API support

## Resources

### Documentation
- [NanaBox Upstream](https://github.com/M2Team/NanaBox)
- [Host Compute System API](https://docs.microsoft.com/en-us/virtualization/api/)
- [Windows Driver Development](https://docs.microsoft.com/en-us/windows-hardware/drivers/)

### Detection Research
- [Pafish](https://github.com/a0rtega/pafish)
- [Al-Khaser](https://github.com/LordNoteworthy/al-khaser)
- [pve-anti-detection](https://github.com/evilscript/pve-anti-detection)

### Community
- GitHub Issues for bug reports
- GitHub Discussions for questions
- Pull Requests for contributions

## FAQ

**Q: Can I contribute without Windows?**  
A: Yes! You can help with documentation, configuration examples, testing plans, and design discussions. Code contributions require Windows + Visual Studio.

**Q: How do I test kernel drivers safely?**  
A: Always use test-signing, dedicated test VMs, and be prepared for system crashes. Never test on production systems.

**Q: What if my anti-detection isn't working?**  
A: Phase 1 only includes configuration schema. Anti-detection implementation comes in Phase 2-4. Test incrementally when those are implemented.

**Q: Can I add support for more games/anti-cheats?**  
A: Yes! Create new profile configurations and test them. Document your findings and submit a PR.

**Q: Is this legal?**  
A: Creating VMs and anti-detection technology is legal for security research and educational purposes. However, using it to violate terms of service or cheat in games is not. Use responsibly.

## Getting Help

- **Configuration Issues**: Check examples/ and docs/configuration.md
- **Profile Questions**: See docs/profiles.md
- **Bug Reports**: Open GitHub issue with reproduction steps
- **Feature Requests**: Open GitHub issue with use case
- **Security Issues**: Contact maintainers privately

## License

MIT License - See LICENSE.md for details.

---

**Last Updated**: 2025-12-04  
**Document Version**: 1.0  
**Project Phase**: Phase 1 Complete, Phase 2+ Planned
