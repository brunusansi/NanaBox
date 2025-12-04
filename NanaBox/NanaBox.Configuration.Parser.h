/*
 * PROJECT:    Mouri Internal Library Essentials
 * FILE:       NanaBox.Configuration.Parser.h
 * PURPOSE:    Definition for Virtual Machine Configuration Parser
 *
 * LICENSE:    The MIT License
 *
 * MAINTAINER: MouriNaruto (Kenji.Mouri@outlook.com)
 */

#ifndef NANABOX_CONFIGURATION_PARSER
#define NANABOX_CONFIGURATION_PARSER

#include <NanaBox.Configuration.Specification.h>

#include <Mile.Json.h>

namespace NanaBox
{
    nlohmann::json FromGuestType(
        GuestType const& Value);

    GuestType ToGuestType(
        nlohmann::json const& Value);

    nlohmann::json FromUefiConsoleMode(
        UefiConsoleMode const& Value);

    UefiConsoleMode ToUefiConsoleMode(
        nlohmann::json const& Value);

    nlohmann::json FromGpuAssignmentMode(
        GpuAssignmentMode const& Value);

    GpuAssignmentMode ToGpuAssignmentMode(
        nlohmann::json const& Value);

    nlohmann::json FromScsiDeviceType(
        ScsiDeviceType const& Value);

    ScsiDeviceType ToScsiDeviceType(
        nlohmann::json const& Value);

    nlohmann::json FromComPortsConfiguration(
        ComPortsConfiguration const& Value);

    ComPortsConfiguration ToComPortsConfiguration(
        nlohmann::json const& Value);

    nlohmann::json FromGpuConfiguration(
        GpuConfiguration const& Value);

    GpuConfiguration ToGpuConfiguration(
        nlohmann::json const& Value);

    nlohmann::json FromNetworkAdapterConfiguration(
        NetworkAdapterConfiguration const& Value);

    NetworkAdapterConfiguration ToNetworkAdapterConfiguration(
        nlohmann::json const& Value);

    nlohmann::json FromScsiDeviceConfiguration(
        ScsiDeviceConfiguration const& Value);

    ScsiDeviceConfiguration ToScsiDeviceConfiguration(
        nlohmann::json const& Value);

    nlohmann::json FromKeyboardConfiguration(
        KeyboardConfiguration const& Value);

    KeyboardConfiguration ToKeyboardConfiguration(
        nlohmann::json const& Value);

    nlohmann::json FromEnhancedSessionConfiguration(
        EnhancedSessionConfiguration const& Value);

    EnhancedSessionConfiguration ToEnhancedSessionConfiguration(
        nlohmann::json const& Value);

    nlohmann::json FromChipsetInformationConfiguration(
        ChipsetInformationConfiguration const& Value);

    ChipsetInformationConfiguration ToChipsetInformationConfiguration(
        nlohmann::json const& Value);

    nlohmann::json FromVideoMonitorConfiguration(
        VideoMonitorConfiguration const& Value);

    VideoMonitorConfiguration ToVideoMonitorConfiguration(
        nlohmann::json const& Value);

    nlohmann::json FromPlan9ShareConfiguration(
        Plan9ShareConfiguration const& Value);

    Plan9ShareConfiguration ToPlan9ShareConfiguration(
        nlohmann::json const& Value);

    nlohmann::json FromAntiDetectionProfile(
        AntiDetectionProfile const& Value);

    AntiDetectionProfile ToAntiDetectionProfile(
        nlohmann::json const& Value);

    // Phase 2: SMBIOS parsers
    nlohmann::json FromSmbiosBaseboardConfiguration(
        SmbiosBaseboardConfiguration const& Value);

    SmbiosBaseboardConfiguration ToSmbiosBaseboardConfiguration(
        nlohmann::json const& Value);

    nlohmann::json FromSmbiosChassisConfiguration(
        SmbiosChassisConfiguration const& Value);

    SmbiosChassisConfiguration ToSmbiosChassisConfiguration(
        nlohmann::json const& Value);

    nlohmann::json FromSmbiosConfiguration(
        SmbiosConfiguration const& Value);

    SmbiosConfiguration ToSmbiosConfiguration(
        nlohmann::json const& Value);

    // Phase 2: ACPI parsers
    nlohmann::json FromAcpiConfiguration(
        AcpiConfiguration const& Value);

    AcpiConfiguration ToAcpiConfiguration(
        nlohmann::json const& Value);

    // Phase 3: CPUID parsers
    nlohmann::json FromCpuIdLeafOverride(
        CpuIdLeafOverride const& Value);

    CpuIdLeafOverride ToCpuIdLeafOverride(
        nlohmann::json const& Value);

    nlohmann::json FromCpuIdConfiguration(
        CpuIdConfiguration const& Value);

    CpuIdConfiguration ToCpuIdConfiguration(
        nlohmann::json const& Value);

    // Phase 3: MSR parsers
    nlohmann::json FromMsrRule(
        MsrRule const& Value);

    MsrRule ToMsrRule(
        nlohmann::json const& Value);

    nlohmann::json FromMsrInterceptConfiguration(
        MsrInterceptConfiguration const& Value);

    MsrInterceptConfiguration ToMsrInterceptConfiguration(
        nlohmann::json const& Value);

    nlohmann::json FromAcpiOverrideConfiguration(
        AcpiOverrideConfiguration const& Value);

    AcpiOverrideConfiguration ToAcpiOverrideConfiguration(
        nlohmann::json const& Value);

    // Phase 4: Timing parsers
    nlohmann::json FromTimingMode(
        TimingMode const& Value);

    TimingMode ToTimingMode(
        nlohmann::json const& Value);

    nlohmann::json FromTscConfiguration(
        TscConfiguration const& Value);

    TscConfiguration ToTscConfiguration(
        nlohmann::json const& Value);

    nlohmann::json FromQpcConfiguration(
        QpcConfiguration const& Value);

    QpcConfiguration ToQpcConfiguration(
        nlohmann::json const& Value);

    nlohmann::json FromTimingConfiguration(
        TimingConfiguration const& Value);

    TimingConfiguration ToTimingConfiguration(
        nlohmann::json const& Value);

    // Legacy timing strategy (backward compatibility)
    nlohmann::json FromTimingStrategy(
        TimingStrategy const& Value);

    TimingStrategy ToTimingStrategy(
        nlohmann::json const& Value);

    nlohmann::json FromPciDeviceConfiguration(
        PciDeviceConfiguration const& Value);

    PciDeviceConfiguration ToPciDeviceConfiguration(
        nlohmann::json const& Value);

    nlohmann::json FromPciConfiguration(
        PciConfiguration const& Value);

    PciConfiguration ToPciConfiguration(
        nlohmann::json const& Value);

    nlohmann::json FromVirtualMachineMetadata(
        VirtualMachineMetadata const& Value);

    VirtualMachineMetadata ToVirtualMachineMetadata(
        nlohmann::json const& Value);

    nlohmann::json FromVirtualMachineConfiguration(
        VirtualMachineConfiguration const& Value);

    VirtualMachineConfiguration ToVirtualMachineConfiguration(
        nlohmann::json const& Value);
}

#endif // !NANABOX_CONFIGURATION_PARSER
