/*
 * PROJECT:    NanaBox
 * FILE:       AntiDetectionSettingsPage.cpp
 * PURPOSE:    Implementation for Anti-Detection Settings Page
 *
 * LICENSE:    The MIT License
 *
 * MAINTAINER: MouriNaruto (Kenji.Mouri@outlook.com)
 */

#include "pch.h"
#include "AntiDetectionSettingsPage.h"
#if __has_include("AntiDetectionSettingsPage.g.cpp")
#include "AntiDetectionSettingsPage.g.cpp"
#endif

#include "Utils.h"
#include "ConfigurationManager.h"
#include "NanaBoxResources.h"

namespace winrt::NanaBox::implementation
{
    AntiDetectionSettingsPage::AntiDetectionSettingsPage(
        _In_ HWND WindowHandle,
        _In_ std::wstring const* ConfigurationFilePath) :
        m_WindowHandle(WindowHandle),
        m_ConfigurationFilePath(ConfigurationFilePath)
    {
        ::SetWindowTextW(
            this->m_WindowHandle,
            Mile::WinRT::GetLocalizedString(
                L"AntiDetectionSettingsPage/GridTitleTextBlock/Text",
                L"[Anti-Detection Settings]").c_str());
    }

    void AntiDetectionSettingsPage::InitializeComponent()
    {
        AntiDetectionSettingsPageT::InitializeComponent();

        // Load current configuration and populate UI
        if (this->m_ConfigurationFilePath && !this->m_ConfigurationFilePath->empty())
        {
            try
            {
                std::string ConfigurationContent = 
                    ::ReadAllTextFromUtf8TextFile(*this->m_ConfigurationFilePath);
                NanaBox::VirtualMachineConfiguration Configuration = 
                    NanaBox::DeserializeConfiguration(ConfigurationContent);

                // Set the profile dropdown selection
                int profileIndex = 0;
                switch (Configuration.AntiDetectionProfile)
                {
                case NanaBox::AntiDetectionProfile::Vanilla:
                    profileIndex = 0;
                    break;
                case NanaBox::AntiDetectionProfile::DefaultGaming:
                    profileIndex = 1;
                    break;
                case NanaBox::AntiDetectionProfile::Valorant:
                    profileIndex = 2;
                    break;
                case NanaBox::AntiDetectionProfile::EacGeneric:
                    profileIndex = 3;
                    break;
                case NanaBox::AntiDetectionProfile::BattlEye:
                    profileIndex = 4;
                    break;
                case NanaBox::AntiDetectionProfile::Faceit:
                    profileIndex = 5;
                    break;
                case NanaBox::AntiDetectionProfile::ExpertTencent:
                    profileIndex = 6;
                    break;
                case NanaBox::AntiDetectionProfile::EaJavelin:
                    profileIndex = 7;
                    break;
                case NanaBox::AntiDetectionProfile::Balanced:
                    profileIndex = 8;
                    break;
                case NanaBox::AntiDetectionProfile::BareMetal:
                    profileIndex = 9;
                    break;
                default:
                    profileIndex = 0;
                    break;
                }

                this->ProfileComboBox().SelectedIndex(profileIndex);

                // Populate read-only summary fields
                if (Configuration.ChipsetInformation.Manufacturer.empty())
                {
                    this->SmbiosTemplateTextBlock().Text(L"(Not configured)");
                }
                else
                {
                    this->SmbiosTemplateTextBlock().Text(
                        Mile::ToWideString(CP_UTF8, Configuration.ChipsetInformation.Manufacturer + " " + 
                        Configuration.ChipsetInformation.ProductName));
                }

                if (Configuration.CpuId.Enabled)
                {
                    this->CpuIdTemplateTextBlock().Text(
                        Mile::ToWideString(CP_UTF8, Configuration.CpuId.VendorId.empty() ? 
                        "(Enabled)" : Configuration.CpuId.VendorId));
                }
                else
                {
                    this->CpuIdTemplateTextBlock().Text(L"(Disabled)");
                }

                if (Configuration.MsrIntercept.Enabled)
                {
                    this->MsrInterceptTemplateTextBlock().Text(L"Enabled");
                }
                else
                {
                    this->MsrInterceptTemplateTextBlock().Text(L"(Disabled)");
                }

                std::wstring timingMode;
                switch (Configuration.Timing.Strategy)
                {
                case NanaBox::TimingStrategy::Off:
                    timingMode = L"Off";
                    break;
                case NanaBox::TimingStrategy::Relaxed:
                    timingMode = L"Relaxed";
                    break;
                case NanaBox::TimingStrategy::Strict:
                    timingMode = L"Strict";
                    break;
                default:
                    timingMode = L"Off";
                    break;
                }
                this->TimingModeTextBlock().Text(timingMode);

                if (Configuration.Pci.Enabled)
                {
                    this->PciLayoutTemplateTextBlock().Text(
                        Mile::FormatWideString(L"Enabled (%zu devices)", Configuration.Pci.Devices.size()));
                }
                else
                {
                    this->PciLayoutTemplateTextBlock().Text(L"(Disabled)");
                }
            }
            catch (...)
            {
                // If configuration load fails, keep defaults
            }
        }
    }

    void AntiDetectionSettingsPage::ProfileComboBoxSelectionChanged(
        winrt::IInspectable const& sender,
        winrt::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);

        // Could update summary fields based on profile selection
        // For now, just note that changes require restart
    }

    void AntiDetectionSettingsPage::SaveButtonClick(
        winrt::IInspectable const& sender,
        winrt::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);

        if (this->m_ConfigurationFilePath && !this->m_ConfigurationFilePath->empty())
        {
            try
            {
                std::string ConfigurationContent = 
                    ::ReadAllTextFromUtf8TextFile(*this->m_ConfigurationFilePath);
                NanaBox::VirtualMachineConfiguration Configuration = 
                    NanaBox::DeserializeConfiguration(ConfigurationContent);

                // Update the profile based on selection
                int selectedIndex = this->ProfileComboBox().SelectedIndex();
                switch (selectedIndex)
                {
                case 0:
                    Configuration.AntiDetectionProfile = NanaBox::AntiDetectionProfile::Vanilla;
                    break;
                case 1:
                    Configuration.AntiDetectionProfile = NanaBox::AntiDetectionProfile::DefaultGaming;
                    break;
                case 2:
                    Configuration.AntiDetectionProfile = NanaBox::AntiDetectionProfile::Valorant;
                    break;
                case 3:
                    Configuration.AntiDetectionProfile = NanaBox::AntiDetectionProfile::EacGeneric;
                    break;
                case 4:
                    Configuration.AntiDetectionProfile = NanaBox::AntiDetectionProfile::BattlEye;
                    break;
                case 5:
                    Configuration.AntiDetectionProfile = NanaBox::AntiDetectionProfile::Faceit;
                    break;
                case 6:
                    Configuration.AntiDetectionProfile = NanaBox::AntiDetectionProfile::ExpertTencent;
                    break;
                case 7:
                    Configuration.AntiDetectionProfile = NanaBox::AntiDetectionProfile::EaJavelin;
                    break;
                case 8:
                    Configuration.AntiDetectionProfile = NanaBox::AntiDetectionProfile::Balanced;
                    break;
                case 9:
                    Configuration.AntiDetectionProfile = NanaBox::AntiDetectionProfile::BareMetal;
                    break;
                default:
                    Configuration.AntiDetectionProfile = NanaBox::AntiDetectionProfile::Vanilla;
                    break;
                }

                std::string Content = NanaBox::SerializeConfiguration(Configuration);
                ::WriteAllTextToUtf8TextFile(*this->m_ConfigurationFilePath, Content);

                ::ShowMessageDialog(
                    this->m_WindowHandle,
                    Mile::WinRT::GetLocalizedString(
                        L"AntiDetectionSettingsPage/SuccessInstructionText",
                        L"[Settings Saved]").c_str(),
                    Mile::WinRT::GetLocalizedString(
                        L"AntiDetectionSettingsPage/SuccessContentText",
                        L"[Anti-detection profile has been updated. Please restart the VM for changes to take effect.]").c_str());
            }
            catch (...)
            {
                winrt::hresult_error Exception = Mile::WinRT::ToHResultError();
                ::ShowErrorMessageDialog(this->m_WindowHandle, Exception);
            }
        }

        ::PostMessageW(this->m_WindowHandle, WM_CLOSE, 0, 0);
    }

    void AntiDetectionSettingsPage::CancelButtonClick(
        winrt::IInspectable const& sender,
        winrt::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);

        ::PostMessageW(this->m_WindowHandle, WM_CLOSE, 0, 0);
    }
}
