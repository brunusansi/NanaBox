/*
 * PROJECT:    NanaBox
 * FILE:       AntiDetectionSettingsPage.h
 * PURPOSE:    Definition for Anti-Detection Settings Page
 *
 * LICENSE:    The MIT License
 *
 * MAINTAINER: MouriNaruto (Kenji.Mouri@outlook.com)
 */

#pragma once

#include "AntiDetectionSettingsPage.g.h"

#include <Windows.h>
#include <string>

namespace winrt
{
    using Windows::Foundation::IInspectable;
    using Windows::UI::Xaml::RoutedEventArgs;
}

namespace winrt::NanaBox::implementation
{
    struct AntiDetectionSettingsPage : AntiDetectionSettingsPageT<AntiDetectionSettingsPage>
    {
    public:

        AntiDetectionSettingsPage(
            _In_ HWND WindowHandle = nullptr,
            _In_ std::wstring const* ConfigurationFilePath = nullptr);

        void InitializeComponent();

        void ProfileComboBoxSelectionChanged(
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const& e);

        void SaveButtonClick(
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const& e);

        void CancelButtonClick(
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const& e);

    private:

        HWND m_WindowHandle;
        std::wstring const* m_ConfigurationFilePath;
    };
}

namespace winrt::NanaBox::factory_implementation
{
    struct AntiDetectionSettingsPage :
        AntiDetectionSettingsPageT<AntiDetectionSettingsPage, implementation::AntiDetectionSettingsPage>
    {
    };
}
