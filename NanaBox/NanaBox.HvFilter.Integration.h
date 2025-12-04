/*
 * PROJECT:    NanaBox Anti-Detection Edition
 * FILE:       NanaBox.HvFilter.Integration.h
 * PURPOSE:    Integration layer between NanaBox configuration and HvFilter driver
 *
 * LICENSE:    The MIT License
 */

#ifndef NANABOX_HVFILTER_INTEGRATION
#define NANABOX_HVFILTER_INTEGRATION

#include "NanaBox.Configuration.Specification.h"
#include <Windows.h>
#include <string>

// Include shared driver definitions
#define NANABOX_HVFILTER_USER_DEVICE_NAME  "\\\\.\\NanaBoxHvFilter"

#define IOCTL_NBX_BASE                      0x8000
#define IOCTL_NBX_HVFILTER_SET_PROFILE \
    CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_NBX_BASE + 0, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_NBX_HVFILTER_GET_STATUS \
    CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_NBX_BASE + 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_NBX_HVFILTER_CLEAR_PROFILE \
    CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_NBX_BASE + 2, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define NBX_MAX_PROFILE_NAME_LENGTH     64
#define NBX_PROFILE_FLAG_CPUID          0x00000001
#define NBX_PROFILE_FLAG_MSR_INTERCEPT  0x00000002
#define NBX_PROFILE_FLAG_TIMING         0x00000004
#define NBX_PROFILE_FLAG_PCI            0x00000008

#pragma pack(push, 1)
typedef struct _NBX_SET_PROFILE_INPUT {
    CHAR ProfileName[NBX_MAX_PROFILE_NAME_LENGTH];
    DWORD Flags;
} NBX_SET_PROFILE_INPUT, *PNBX_SET_PROFILE_INPUT;

typedef struct _NBX_GET_STATUS_OUTPUT {
    CHAR ActiveProfileName[NBX_MAX_PROFILE_NAME_LENGTH];
    DWORD ActiveFlags;
    DWORD DriverVersion;
    BOOL IsActive;
} NBX_GET_STATUS_OUTPUT, *PNBX_GET_STATUS_OUTPUT;
#pragma pack(pop)

namespace NanaBox
{
    /**
     * @brief Helper class for HvFilter driver integration
     */
    class HvFilterIntegration
    {
    private:
        HANDLE m_hDevice;
        bool m_IsConnected;

    public:
        /**
         * @brief Constructor
         */
        HvFilterIntegration() : m_hDevice(INVALID_HANDLE_VALUE), m_IsConnected(false)
        {
        }

        /**
         * @brief Destructor
         */
        ~HvFilterIntegration()
        {
            Disconnect();
        }

        /**
         * @brief Check if driver is available
         * @return true if driver device exists
         */
        bool IsDriverAvailable()
        {
            HANDLE hTest = CreateFileA(
                NANABOX_HVFILTER_USER_DEVICE_NAME,
                GENERIC_READ,
                0,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL
            );

            if (hTest == INVALID_HANDLE_VALUE) {
                return false;
            }

            CloseHandle(hTest);
            return true;
        }

        /**
         * @brief Connect to the driver
         * @return true on success, false on failure
         */
        bool Connect()
        {
            if (m_IsConnected) {
                return true;
            }

            m_hDevice = CreateFileA(
                NANABOX_HVFILTER_USER_DEVICE_NAME,
                GENERIC_READ | GENERIC_WRITE,
                0,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL
            );

            if (m_hDevice == INVALID_HANDLE_VALUE) {
                return false;
            }

            m_IsConnected = true;
            return true;
        }

        /**
         * @brief Disconnect from the driver
         */
        void Disconnect()
        {
            if (m_IsConnected && m_hDevice != INVALID_HANDLE_VALUE) {
                CloseHandle(m_hDevice);
                m_hDevice = INVALID_HANDLE_VALUE;
                m_IsConnected = false;
            }
        }

        /**
         * @brief Build profile flags from VM configuration
         * @param config VM configuration
         * @return Profile flags bitmask
         */
        static DWORD BuildProfileFlags(const VirtualMachineConfiguration& config)
        {
            DWORD flags = 0;

            // Check if CPUID spoofing is enabled
            if (config.CpuId.Enabled) {
                flags |= NBX_PROFILE_FLAG_CPUID;
            }

            // Check if MSR interception is enabled
            if (config.MsrIntercept.Enabled) {
                flags |= NBX_PROFILE_FLAG_MSR_INTERCEPT;
            }

            // Check if timing normalization is enabled
            if (config.Timing.Enabled) {
                flags |= NBX_PROFILE_FLAG_TIMING;
            }

            // Check if PCI topology control is enabled
            if (config.Pci.Enabled) {
                flags |= NBX_PROFILE_FLAG_PCI;
            }

            return flags;
        }

        /**
         * @brief Get profile name from anti-detection profile enum
         * @param profile Anti-detection profile enum
         * @return Profile name string
         */
        static std::string GetProfileName(AntiDetectionProfile profile)
        {
            switch (profile) {
                case AntiDetectionProfile::Vanilla:
                    return "Vanilla";
                case AntiDetectionProfile::Balanced:
                    return "Balanced";
                case AntiDetectionProfile::BareMetal:
                    return "BareMetal";
                case AntiDetectionProfile::DefaultGaming:
                    return "DefaultGaming";
                case AntiDetectionProfile::Valorant:
                    return "Valorant";
                case AntiDetectionProfile::EacGeneric:
                    return "EacGeneric";
                case AntiDetectionProfile::BattlEye:
                    return "BattlEye";
                case AntiDetectionProfile::Faceit:
                    return "Faceit";
                case AntiDetectionProfile::ExpertTencent:
                    return "ExpertTencent";
                case AntiDetectionProfile::EaJavelin:
                    return "EaJavelin";
                default:
                    return "Unknown";
            }
        }

        /**
         * @brief Send SET_PROFILE IOCTL to driver
         * @param config VM configuration
         * @return true on success, false on failure
         */
        bool SetProfileFromConfig(const VirtualMachineConfiguration& config)
        {
            if (!m_IsConnected) {
                return false;
            }

            NBX_SET_PROFILE_INPUT input = { 0 };
            DWORD bytesReturned = 0;

            // Get profile name
            std::string profileName = GetProfileName(config.AntiDetectionProfile);
            strncpy_s(input.ProfileName, sizeof(input.ProfileName), profileName.c_str(), _TRUNCATE);

            // Build flags
            input.Flags = BuildProfileFlags(config);

            // Send IOCTL
            BOOL result = DeviceIoControl(
                m_hDevice,
                IOCTL_NBX_HVFILTER_SET_PROFILE,
                &input,
                sizeof(input),
                NULL,
                0,
                &bytesReturned,
                NULL
            );

            return result != FALSE;
        }

        /**
         * @brief Get current driver status
         * @param output Pointer to output structure
         * @return true on success, false on failure
         */
        bool GetStatus(NBX_GET_STATUS_OUTPUT* output)
        {
            if (!m_IsConnected || output == nullptr) {
                return false;
            }

            DWORD bytesReturned = 0;

            BOOL result = DeviceIoControl(
                m_hDevice,
                IOCTL_NBX_HVFILTER_GET_STATUS,
                NULL,
                0,
                output,
                sizeof(NBX_GET_STATUS_OUTPUT),
                &bytesReturned,
                NULL
            );

            return result != FALSE;
        }

        /**
         * @brief Clear active profile
         * @return true on success, false on failure
         */
        bool ClearProfile()
        {
            if (!m_IsConnected) {
                return false;
            }

            DWORD bytesReturned = 0;

            BOOL result = DeviceIoControl(
                m_hDevice,
                IOCTL_NBX_HVFILTER_CLEAR_PROFILE,
                NULL,
                0,
                NULL,
                0,
                &bytesReturned,
                NULL
            );

            return result != FALSE;
        }
    };
}

#endif // !NANABOX_HVFILTER_INTEGRATION
