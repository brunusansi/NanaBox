/*
 * PROJECT:    NanaBox Anti-Detection Edition
 * FILE:       examples/hvfilter-integration-example.cpp
 * PURPOSE:    Example of integrating HvFilter driver with NanaBox VM configuration
 *
 * LICENSE:    The MIT License
 *
 * DESCRIPTION:
 *   This example demonstrates how to use the HvFilterIntegration class to
 *   communicate with the nanabox_hvfilter.sys driver from within NanaBox.
 *
 *   This code can be integrated into:
 *   - VM startup routines (to load anti-detection profile when VM starts)
 *   - Configuration manager (to sync driver settings with VM config)
 *   - CLI tools (to manually control driver settings)
 *
 * USAGE:
 *   This is example code only - not meant to be compiled standalone.
 *   Copy relevant sections into NanaBox source files as needed.
 */

#include "NanaBox.Configuration.Specification.h"
#include "NanaBox.HvFilter.Integration.h"
#include <iostream>

namespace NanaBox
{
    /**
     * @brief Example: Load anti-detection profile when starting a VM
     *
     * This function would be called during VM initialization, after the VM
     * configuration has been loaded from JSON.
     */
    bool LoadAntiDetectionProfile(const VirtualMachineConfiguration& vmConfig)
    {
        // Create HvFilter integration instance
        HvFilterIntegration hvFilter;

        // Check if driver is available
        if (!hvFilter.IsDriverAvailable()) {
            // Driver not installed or not running
            // This is not an error - just means anti-detection features are unavailable
            std::cout << "[INFO] HvFilter driver not available - anti-detection features disabled\n";
            return false;
        }

        // Connect to driver
        if (!hvFilter.Connect()) {
            std::cerr << "[ERROR] Failed to connect to HvFilter driver\n";
            return false;
        }

        // Check if any anti-detection features are enabled
        DWORD flags = HvFilterIntegration::BuildProfileFlags(vmConfig);
        if (flags == 0) {
            std::cout << "[INFO] No anti-detection features enabled in configuration\n";
            hvFilter.Disconnect();
            return true;
        }

        // Send profile to driver
        if (!hvFilter.SetProfileFromConfig(vmConfig)) {
            std::cerr << "[ERROR] Failed to set anti-detection profile in driver\n";
            hvFilter.Disconnect();
            return false;
        }

        std::cout << "[SUCCESS] Anti-detection profile loaded successfully\n";

        // Query status to verify
        NBX_GET_STATUS_OUTPUT status;
        if (hvFilter.GetStatus(&status)) {
            std::cout << "[INFO] Active Profile: " << status.ActiveProfileName << "\n";
            std::cout << "[INFO] Flags: 0x" << std::hex << status.ActiveFlags << std::dec << "\n";
            std::cout << "[INFO]   - CPUID: " << ((status.ActiveFlags & NBX_PROFILE_FLAG_CPUID) ? "Yes" : "No") << "\n";
            std::cout << "[INFO]   - MSR Intercept: " << ((status.ActiveFlags & NBX_PROFILE_FLAG_MSR_INTERCEPT) ? "Yes" : "No") << "\n";
            std::cout << "[INFO]   - Timing: " << ((status.ActiveFlags & NBX_PROFILE_FLAG_TIMING) ? "Yes" : "No") << "\n";
            std::cout << "[INFO]   - PCI: " << ((status.ActiveFlags & NBX_PROFILE_FLAG_PCI) ? "Yes" : "No") << "\n";
        }

        hvFilter.Disconnect();
        return true;
    }

    /**
     * @brief Example: Clear anti-detection profile when stopping a VM
     *
     * This function would be called during VM shutdown, to reset the driver
     * to its default state.
     */
    bool ClearAntiDetectionProfile()
    {
        HvFilterIntegration hvFilter;

        if (!hvFilter.IsDriverAvailable()) {
            // Driver not available - nothing to clear
            return true;
        }

        if (!hvFilter.Connect()) {
            std::cerr << "[ERROR] Failed to connect to HvFilter driver\n";
            return false;
        }

        if (!hvFilter.ClearProfile()) {
            std::cerr << "[ERROR] Failed to clear anti-detection profile\n";
            hvFilter.Disconnect();
            return false;
        }

        std::cout << "[INFO] Anti-detection profile cleared\n";
        hvFilter.Disconnect();
        return true;
    }

    /**
     * @brief Example: Query current driver status
     *
     * This function can be called at any time to check the current state
     * of the HvFilter driver.
     */
    bool QueryDriverStatus()
    {
        HvFilterIntegration hvFilter;

        if (!hvFilter.IsDriverAvailable()) {
            std::cout << "[INFO] HvFilter driver is not available\n";
            return false;
        }

        if (!hvFilter.Connect()) {
            std::cerr << "[ERROR] Failed to connect to HvFilter driver\n";
            return false;
        }

        NBX_GET_STATUS_OUTPUT status;
        if (!hvFilter.GetStatus(&status)) {
            std::cerr << "[ERROR] Failed to query driver status\n";
            hvFilter.Disconnect();
            return false;
        }

        std::cout << "\n========================================\n";
        std::cout << "HvFilter Driver Status\n";
        std::cout << "========================================\n";
        std::cout << "Driver Version: "
                  << ((status.DriverVersion >> 16) & 0xFF) << "."
                  << ((status.DriverVersion >> 8) & 0xFF) << "."
                  << (status.DriverVersion & 0xFF) << "\n";
        std::cout << "Is Active: " << (status.IsActive ? "Yes" : "No") << "\n";
        std::cout << "Active Profile: " << status.ActiveProfileName << "\n";
        std::cout << "Active Flags: 0x" << std::hex << status.ActiveFlags << std::dec << "\n";
        std::cout << "  - CPUID Spoofing: " << ((status.ActiveFlags & NBX_PROFILE_FLAG_CPUID) ? "Enabled" : "Disabled") << "\n";
        std::cout << "  - MSR Interception: " << ((status.ActiveFlags & NBX_PROFILE_FLAG_MSR_INTERCEPT) ? "Enabled" : "Disabled") << "\n";
        std::cout << "  - Timing Normalization: " << ((status.ActiveFlags & NBX_PROFILE_FLAG_TIMING) ? "Enabled" : "Disabled") << "\n";
        std::cout << "  - PCI Topology: " << ((status.ActiveFlags & NBX_PROFILE_FLAG_PCI) ? "Enabled" : "Disabled") << "\n";
        std::cout << "========================================\n\n";

        hvFilter.Disconnect();
        return true;
    }

    /**
     * @brief Example: Integration point for VM startup
     *
     * This shows where you would call LoadAntiDetectionProfile during
     * VM initialization. This is pseudo-code showing the integration point.
     */
    void ExampleVMStartupIntegration()
    {
        // Pseudo-code showing integration
        /*
        void StartVirtualMachine(const VirtualMachineConfiguration& config)
        {
            // ... existing VM startup code ...

            // Create compute system
            CreateComputeSystem(config);

            // ... configure network, storage, etc. ...

            // NEW: Load anti-detection profile if driver is available
            if (config.AntiDetectionProfile != AntiDetectionProfile::Vanilla) {
                LoadAntiDetectionProfile(config);
            }

            // Start the VM
            StartComputeSystem();

            // ... rest of startup code ...
        }
        */
    }

    /**
     * @brief Example: Integration point for VM shutdown
     *
     * This shows where you would call ClearAntiDetectionProfile during
     * VM shutdown. This is pseudo-code showing the integration point.
     */
    void ExampleVMShutdownIntegration()
    {
        // Pseudo-code showing integration
        /*
        void StopVirtualMachine()
        {
            // ... existing VM shutdown code ...

            // Stop the compute system
            StopComputeSystem();

            // NEW: Clear anti-detection profile
            ClearAntiDetectionProfile();

            // ... rest of shutdown code ...
        }
        */
    }
}

/*
 * INTEGRATION NOTES:
 *
 * 1. Where to add this code:
 *    - Add LoadAntiDetectionProfile() call in VM startup routine
 *      (likely in HostCompute.cpp or MainWindow.cpp)
 *    - Add ClearAntiDetectionProfile() call in VM shutdown routine
 *    - Add QueryDriverStatus() to status/info commands
 *
 * 2. Error handling:
 *    - Driver not available is NOT an error - features just won't be active
 *    - Log warnings if driver communication fails
 *    - Don't prevent VM from starting if driver is unavailable
 *
 * 3. User feedback:
 *    - Show driver status in UI (e.g., "Anti-Detection: Active")
 *    - Provide clear messages when driver is not installed
 *    - Document that driver requires admin privileges and test-signing
 *
 * 4. Future enhancements:
 *    - Add automatic driver installation prompt
 *    - Add driver status indicator in UI
 *    - Add real-time profile switching (if VM is running)
 *    - Add profile validation before sending to driver
 */
