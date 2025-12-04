/*
 * PROJECT:    NanaBox
 * FILE:       NanaBox.cpp
 * PURPOSE:    Implementation for NanaBox
 *
 * LICENSE:    The MIT License
 *
 * MAINTAINER: MouriNaruto (Kenji.Mouri@outlook.com)
 */

#include <Windows.h>

#include <ShlObj.h>

#include "pch.h"

#include "App.h"
#include "MainWindow.h"
#include "QuickStartPage.h"
#include "SponsorPage.h"

#include <Mile.Project.Version.h>

#include "Utils.h"
#include "NanaBoxResources.h"
#include "ConfigurationManager.h"

#include <Mile.Helpers.h>
#include <Mile.Xaml.h>
#include <algorithm>

namespace
{
    const std::wstring_view g_AppUserModelID =
        L"M2Team.NanaBox"
        L"_" MILE_PROJECT_DOT_VERSION_STRING
        L"_" MILE_PROJECT_BUILD_DATE_STRING;

    WTL::CAppModule g_Module;
}

void PrerequisiteCheck()
{
    try
    {
        NanaBox::HcsGetServiceProperties();
    }
    catch (winrt::hresult_error const& ex)
    {
        winrt::hstring InstructionText;
        winrt::hstring ContentText;

        if (ex.code() == HCS_E_ACCESS_DENIED)
        {
            InstructionText = Mile::WinRT::GetLocalizedString(
                L"Messages/AccessDeniedInstructionText");
            ContentText = Mile::WinRT::GetLocalizedString(
                L"Messages/AccessDeniedContentText");
        }
        else if (ex.code() == HCS_E_SERVICE_NOT_AVAILABLE)
        {
            InstructionText = Mile::WinRT::GetLocalizedString(
                L"Messages/HyperVNotAvailableInstructionText");
            ContentText = Mile::WinRT::GetLocalizedString(
                L"Messages/HyperVNotAvailableContentText");
        }
        else
        {
            InstructionText = ex.message();
        }

        ::ShowMessageDialog(
            nullptr,
            InstructionText,
            ContentText);

        ::ExitProcess(ex.code());
    }

    return;
}

int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    winrt::init_apartment(winrt::apartment_type::single_threaded);

    winrt::check_hresult(::SetCurrentProcessExplicitAppUserModelID(
        g_AppUserModelID.data()));

    ::SetProcessShutdownParameters(0x3FF, 0);

    winrt::com_ptr<winrt::NanaBox::implementation::App> App =
        winrt::make_self<winrt::NanaBox::implementation::App>();
    auto ExitHandler = Mile::ScopeExitTaskHandler([&]()
    {
        App->Close();
    });

    std::wstring ApplicationName;
    std::map<std::wstring, std::wstring> OptionsAndParameters;
    std::wstring UnresolvedCommandLine;

    ::SplitCommandLineEx(
        std::wstring(::GetCommandLineW()),
        std::vector<std::wstring>{ L"-", L"/", L"--" },
        std::vector<std::wstring>{ L"=", L":" },
        ApplicationName,
        OptionsAndParameters,
        UnresolvedCommandLine);

    bool AcquireSponsorEdition = false;
    bool ShowConfigMode = false;
    bool SetProfileMode = false;
    std::wstring CommandArg;
    std::wstring ProfileArg;

    for (auto& Current : OptionsAndParameters)
    {
        if (0 == _wcsicmp(Current.first.c_str(), L"AcquireSponsorEdition"))
        {
            AcquireSponsorEdition = true;
        }
        else if (0 == _wcsicmp(Current.first.c_str(), L"show-config"))
        {
            ShowConfigMode = true;
            CommandArg = Current.second;
        }
        else if (0 == _wcsicmp(Current.first.c_str(), L"set-profile"))
        {
            SetProfileMode = true;
            CommandArg = Current.second;
        }
        else if (0 == _wcsicmp(Current.first.c_str(), L"profile"))
        {
            ProfileArg = Current.second;
        }
    }

    // CLI Mode: show-config
    if (ShowConfigMode && !CommandArg.empty())
    {
        try
        {
            std::wstring ConfigFilePath = ::GetAbsolutePath(CommandArg);
            std::string ConfigContent = ::ReadAllTextFromUtf8TextFile(ConfigFilePath);
            NanaBox::VirtualMachineConfiguration Config = 
                NanaBox::DeserializeConfiguration(ConfigContent);

            // Output configuration to console
            std::wstring output = L"\nNanaBox VM Configuration: " + CommandArg + L"\n";
            output += L"======================================\n";
            output += L"Name: " + Mile::ToWideString(CP_UTF8, Config.Name) + L"\n";
            output += L"Guest Type: " + Mile::ToWideString(
                CP_UTF8, NanaBox::FromGuestType(Config.GuestType)) + L"\n";
            output += L"Processors: " + std::to_wstring(Config.ProcessorCount) + L"\n";
            output += L"Memory: " + std::to_wstring(Config.MemorySize) + L" MB\n";
            output += L"Anti-Detection Profile: " + Mile::ToWideString(
                CP_UTF8, NanaBox::FromAntiDetectionProfile(Config.AntiDetectionProfile)) + L"\n";
            
            if (Config.ChipsetInformation.Manufacturer.empty())
            {
                output += L"SMBIOS: (Not configured)\n";
            }
            else
            {
                output += L"SMBIOS: " + Mile::ToWideString(CP_UTF8, 
                    Config.ChipsetInformation.Manufacturer + " " + 
                    Config.ChipsetInformation.ProductName) + L"\n";
            }
            
            output += L"CPUID Enabled: " + std::wstring(Config.CpuId.Enabled ? L"Yes" : L"No") + L"\n";
            output += L"MSR Intercept: " + std::wstring(Config.MsrIntercept.Enabled ? L"Yes" : L"No") + L"\n";
            output += L"Timing Strategy: " + Mile::ToWideString(
                CP_UTF8, NanaBox::FromTimingStrategy(Config.Timing.Strategy)) + L"\n";
            output += L"PCI Layout: " + std::wstring(Config.Pci.Enabled ? L"Enabled" : L"Disabled") + L"\n";
            
            ::MessageBoxW(nullptr, output.c_str(), L"NanaBox Configuration", MB_OK | MB_ICONINFORMATION);
        }
        catch (...)
        {
            winrt::hresult_error Exception = Mile::WinRT::ToHResultError();
            std::wstring errorMsg = L"Failed to read configuration: " + Exception.message();
            ::MessageBoxW(nullptr, errorMsg.c_str(), L"Error", MB_OK | MB_ICONERROR);
        }
        ::ExitProcess(0);
    }

    // CLI Mode: set-profile
    if (SetProfileMode && !CommandArg.empty() && !ProfileArg.empty())
    {
        try
        {
            std::wstring ConfigFilePath = ::GetAbsolutePath(CommandArg);
            std::string ConfigContent = ::ReadAllTextFromUtf8TextFile(ConfigFilePath);
            NanaBox::VirtualMachineConfiguration Config = 
                NanaBox::DeserializeConfiguration(ConfigContent);

            // Validate and set profile
            std::wstring profileLower = ProfileArg;
            std::transform(profileLower.begin(), profileLower.end(), profileLower.begin(), ::towlower);
            
            if (profileLower == L"vanilla")
                Config.AntiDetectionProfile = NanaBox::AntiDetectionProfile::Vanilla;
            else if (profileLower == L"default-gaming")
                Config.AntiDetectionProfile = NanaBox::AntiDetectionProfile::DefaultGaming;
            else if (profileLower == L"valorant")
                Config.AntiDetectionProfile = NanaBox::AntiDetectionProfile::Valorant;
            else if (profileLower == L"eac-generic")
                Config.AntiDetectionProfile = NanaBox::AntiDetectionProfile::EacGeneric;
            else if (profileLower == L"battleye")
                Config.AntiDetectionProfile = NanaBox::AntiDetectionProfile::BattlEye;
            else if (profileLower == L"faceit")
                Config.AntiDetectionProfile = NanaBox::AntiDetectionProfile::Faceit;
            else if (profileLower == L"expert-tencent")
                Config.AntiDetectionProfile = NanaBox::AntiDetectionProfile::ExpertTencent;
            else if (profileLower == L"ea-javelin")
                Config.AntiDetectionProfile = NanaBox::AntiDetectionProfile::EaJavelin;
            else if (profileLower == L"balanced")
                Config.AntiDetectionProfile = NanaBox::AntiDetectionProfile::Balanced;
            else if (profileLower == L"bare-metal")
                Config.AntiDetectionProfile = NanaBox::AntiDetectionProfile::BareMetal;
            else
            {
                std::wstring errorMsg = L"Invalid profile: " + ProfileArg + L"\n\n";
                errorMsg += L"Valid profiles:\n";
                errorMsg += L"  - vanilla\n";
                errorMsg += L"  - default-gaming\n";
                errorMsg += L"  - valorant\n";
                errorMsg += L"  - eac-generic\n";
                errorMsg += L"  - battleye\n";
                errorMsg += L"  - faceit\n";
                errorMsg += L"  - expert-tencent\n";
                errorMsg += L"  - ea-javelin\n";
                errorMsg += L"  - balanced\n";
                errorMsg += L"  - bare-metal\n";
                ::MessageBoxW(nullptr, errorMsg.c_str(), L"Error", MB_OK | MB_ICONERROR);
                ::ExitProcess(1);
            }

            std::string NewContent = NanaBox::SerializeConfiguration(Config);
            ::WriteAllTextToUtf8TextFile(ConfigFilePath, NewContent);

            std::wstring successMsg = L"Profile set to: " + ProfileArg + L"\n\n";
            successMsg += L"Please restart the VM for changes to take effect.";
            ::MessageBoxW(nullptr, successMsg.c_str(), L"Success", MB_OK | MB_ICONINFORMATION);
        }
        catch (...)
        {
            winrt::hresult_error Exception = Mile::WinRT::ToHResultError();
            std::wstring errorMsg = L"Failed to set profile: " + Exception.message();
            ::MessageBoxW(nullptr, errorMsg.c_str(), L"Error", MB_OK | MB_ICONERROR);
            ::ExitProcess(1);
        }
        ::ExitProcess(0);
    }

    if (AcquireSponsorEdition)
    {
        HWND WindowHandle = ::CreateWindowExW(
            WS_EX_STATICEDGE | WS_EX_DLGMODALFRAME,
            L"Mile.Xaml.ContentWindow",
            nullptr,
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            CW_USEDEFAULT,
            0,
            CW_USEDEFAULT,
            0,
            nullptr,
            nullptr,
            nullptr,
            nullptr);
        if (!WindowHandle)
        {
            return -1;
        }

        if (FAILED(::MileAllowNonClientDefaultDrawingForWindow(
            WindowHandle,
            FALSE)))
        {
            return -1;
        }

        winrt::NanaBox::SponsorPage Window =
            winrt::make<winrt::NanaBox::implementation::SponsorPage>(
                WindowHandle);
        ::ShowXamlWindow(
            WindowHandle,
            460,
            320,
            winrt::get_abi(Window),
            nullptr);

        ::ExitProcess(0);
    }

    bool PackagedMode = Mile::WinRT::IsPackagedMode();
    std::wstring TargetBinaryPath;

    if (PackagedMode)
    {
        try
        {
            std::wstring AppBinaryPath;
            {
                std::wstring RawPath = ::GetCurrentProcessModulePath();
                std::wcsrchr(&RawPath[0], L'\\')[0] = L'\0';
                RawPath.resize(std::wcslen(RawPath.c_str()));
                AppBinaryPath = RawPath;
            }

            std::wstring TempBinaryPath;
            {
                std::wstring RawPath = ::GetLocalStateFolderPath();
                GUID TempFolderGuid;
                winrt::check_hresult(::CoCreateGuid(&TempFolderGuid));
                RawPath.append(L"\\");
                RawPath.append(::FromGuid(TempFolderGuid));
                TempBinaryPath = RawPath;
            }

            winrt::check_bool(::CreateDirectoryW(
                TempBinaryPath.c_str(),
                nullptr));

            winrt::check_bool(::CopyFileW(
                (AppBinaryPath + L"\\NanaBox.exe").c_str(),
                (TempBinaryPath + L"\\NanaBox.exe").c_str(),
                FALSE));

            winrt::check_bool(::CopyFileW(
                (AppBinaryPath + L"\\resources.pri").c_str(),
                (TempBinaryPath + L"\\resources.pri").c_str(),
                FALSE));

            winrt::check_bool(::CopyFileW(
                (AppBinaryPath + L"\\Mile.Xaml.Styles.SunValley.xbf").c_str(),
                (TempBinaryPath + L"\\Mile.Xaml.Styles.SunValley.xbf").c_str(),
                FALSE));

            TargetBinaryPath = TempBinaryPath;
        }
        catch (...)
        {
            winrt::hresult_error Exception = Mile::WinRT::ToHResultError();
            ::ShowErrorMessageDialog(Exception);
            ::ExitProcess(Exception.code());
        }
    }

    if (!::MileIsCurrentProcessElevated() || PackagedMode)
    {
        try
        {
            if (PackagedMode && !TargetBinaryPath.empty())
            {
                ApplicationName = TargetBinaryPath + L"\\NanaBox.exe";
            }
            else
            {
                ApplicationName = ::GetCurrentProcessModulePath();
            }

            SHELLEXECUTEINFOW Information = {};
            Information.cbSize = sizeof(SHELLEXECUTEINFOW);
            Information.fMask = SEE_MASK_NOCLOSEPROCESS;
            Information.lpVerb = L"runas";
            Information.nShow = nShowCmd;
            Information.lpFile = ApplicationName.c_str();
            Information.lpParameters = UnresolvedCommandLine.c_str();
            winrt::check_bool(::ShellExecuteExW(&Information));
            ::WaitForSingleObjectEx(Information.hProcess, INFINITE, FALSE);
            ::CloseHandle(Information.hProcess);

            if (PackagedMode && !TargetBinaryPath.empty())
            {
                ::SimpleRemoveDirectory(TargetBinaryPath.c_str());
            }
        }
        catch (...)
        {
            winrt::hresult_error Exception = Mile::WinRT::ToHResultError();

            if (Exception.code() != ::HRESULT_FROM_WIN32(ERROR_CANCELLED))
            {
                ::ShowErrorMessageDialog(Exception);
            }

            if (PackagedMode && !TargetBinaryPath.empty())
            {
                ::SimpleRemoveDirectory(TargetBinaryPath.c_str());
            }

            ::ExitProcess(Exception.code());
        }

        ::ExitProcess(0);
    }

    ::PrerequisiteCheck();

    std::wstring ConfigurationFilePath;

    if (!UnresolvedCommandLine.empty())
    {
        ConfigurationFilePath = ::GetAbsolutePath(UnresolvedCommandLine);
    }
    else
    {
        HWND WindowHandle = ::CreateWindowExW(
            WS_EX_STATICEDGE | WS_EX_DLGMODALFRAME,
            L"Mile.Xaml.ContentWindow",
            nullptr,
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            CW_USEDEFAULT,
            0,
            CW_USEDEFAULT,
            0,
            nullptr,
            nullptr,
            nullptr,
            nullptr);
        if (!WindowHandle)
        {
            return -1;
        }

        if (FAILED(::MileAllowNonClientDefaultDrawingForWindow(
            WindowHandle,
            FALSE)))
        {
            return -1;
        }

        winrt::NanaBox::QuickStartPage Window =
            winrt::make<winrt::NanaBox::implementation::QuickStartPage>(
                WindowHandle,
                &ConfigurationFilePath);
        ::ShowXamlWindow(
            WindowHandle,
            460,
            460,
            winrt::get_abi(Window),
            nullptr);
        if (ConfigurationFilePath.empty())
        {
            return 0;
        }
    }

    {
        std::wstring CurrentPath = ConfigurationFilePath;
        std::wcsrchr(&CurrentPath[0], L'\\')[0] = L'\0';
        CurrentPath.resize(std::wcslen(CurrentPath.c_str()));
        ::SetCurrentDirectoryW(CurrentPath.c_str());
    }

    WTL::CMessageLoop MessageLoop;

    ATL::AtlAxWinInit();

    g_Module.Init(nullptr, hInstance);
    g_Module.AddMessageLoop(&MessageLoop);

    NanaBox::MainWindow MainWindow(ConfigurationFilePath);
    if (!MainWindow.Create(
        nullptr,
        MainWindow.rcDefault,
        L"NanaBox",
        WS_OVERLAPPEDWINDOW))
    {
        return -1;
    }

    MainWindow.ShowWindow(nShowCmd);
    MainWindow.UpdateWindow();

    int Result = MessageLoop.Run();

    g_Module.RemoveMessageLoop();
    g_Module.Term();

    return Result;
}
