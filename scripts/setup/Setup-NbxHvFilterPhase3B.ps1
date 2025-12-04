<#
.SYNOPSIS
    NanaBox HvFilter Driver Phase 3B Setup and Testing Script

.DESCRIPTION
    Automated setup script for building, installing, starting, and testing the nanabox_hvfilter
    driver for Phase 3B (CPUID/MSR configuration framework).
    
    This script automates the complete Phase 3B testing flow:
    1. Validates build environment (Visual Studio, MSBuild, WDK, Windows SDK)
    2. Builds the nanabox_hvfilter driver
    3. Installs or updates the driver
    4. Starts the driver service
    5. Runs basic sanity checks using NbxHvFilterClient
    
    Designed for development/testing environments with test-signing enabled.

.PARAMETER Configuration
    Build configuration (Debug or Release). Default: Release

.PARAMETER Platform
    Build platform (x64). Default: x64

.PARAMETER DryRun
    Print steps without actually installing/starting the driver. Build still occurs.

.PARAMETER SkipBuild
    Skip building the driver (use existing built binaries).

.PARAMETER SkipInstall
    Skip driver installation step.

.PARAMETER SkipStart
    Skip starting the driver service.

.PARAMETER SkipTest
    Skip running sanity checks with NbxHvFilterClient.

.EXAMPLE
    .\Setup-NbxHvFilterPhase3B.ps1
    
    Build, install, start, and test the driver with default settings (Release, x64).

.EXAMPLE
    .\Setup-NbxHvFilterPhase3B.ps1 -Configuration Debug
    
    Build in Debug configuration.

.EXAMPLE
    .\Setup-NbxHvFilterPhase3B.ps1 -DryRun
    
    Build the driver and show what would be done without actually installing it.

.EXAMPLE
    .\Setup-NbxHvFilterPhase3B.ps1 -SkipBuild
    
    Use existing built binaries to install and test the driver.

.NOTES
    Author: NanaBox Anti-Detection Edition Contributors
    Requires: Windows 10/11, PowerShell 5.1+, Administrator privileges
    Phase: 3B - CPUID/MSR Configuration Framework
    
    IMPORTANT:
    - This script MUST be run as Administrator
    - Test signing must be enabled (bcdedit /set testsigning on)
    - Secure Boot must be disabled in BIOS/UEFI
    - This is for development/testing only
    
.LINK
    https://github.com/brunusansi/NanaBox
    See PHASE3B_TESTING.md for detailed usage instructions
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory=$false)]
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release",
    
    [Parameter(Mandatory=$false)]
    [ValidateSet("x64")]
    [string]$Platform = "x64",
    
    [Parameter(Mandatory=$false)]
    [switch]$DryRun,
    
    [Parameter(Mandatory=$false)]
    [switch]$SkipBuild,
    
    [Parameter(Mandatory=$false)]
    [switch]$SkipInstall,
    
    [Parameter(Mandatory=$false)]
    [switch]$SkipStart,
    
    [Parameter(Mandatory=$false)]
    [switch]$SkipTest
)

#Requires -RunAsAdministrator

# ============================================================================
# Constants and Configuration
# ============================================================================

$ErrorActionPreference = "Stop"
$VerbosePreference = if ($PSBoundParameters['Verbose']) { "Continue" } else { "SilentlyContinue" }

# Script configuration
$ScriptVersion = "1.0.0"
$DriverName = "nanabox_hvfilter"
$ServiceName = "NanaBoxHvFilter"
$ServiceDisplayName = "NanaBox Hypervisor Filter Driver"

# Color scheme for console output
$ColorScheme = @{
    Title = "Cyan"
    Success = "Green"
    Warning = "Yellow"
    Error = "Red"
    Info = "White"
    Highlight = "Magenta"
    Step = "Cyan"
}

# ============================================================================
# Helper Functions (Reused from Setup-NanaBoxDevEnv.ps1)
# ============================================================================

function Write-Header {
    param([string]$Message)
    Write-Host ""
    Write-Host "============================================================================" -ForegroundColor $ColorScheme.Title
    Write-Host " $Message" -ForegroundColor $ColorScheme.Title
    Write-Host "============================================================================" -ForegroundColor $ColorScheme.Title
    Write-Host ""
}

function Write-Success {
    param([string]$Message)
    Write-Host "[OK] $Message" -ForegroundColor $ColorScheme.Success
}

function Write-Info {
    param([string]$Message)
    Write-Host "[INFO] $Message" -ForegroundColor $ColorScheme.Info
}

function Write-Warning {
    param([string]$Message)
    Write-Host "[WARN] $Message" -ForegroundColor $ColorScheme.Warning
}

function Write-ErrorMsg {
    param([string]$Message)
    Write-Host "[ERROR] $Message" -ForegroundColor $ColorScheme.Error
}

function Write-Step {
    param([string]$Message)
    Write-Host ""
    Write-Host ">>> $Message" -ForegroundColor $ColorScheme.Step
}

function Test-AdministratorPrivileges {
    $currentPrincipal = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
    return $currentPrincipal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Get-RepositoryRoot {
    $currentDir = $PSScriptRoot
    # Navigate up from scripts/setup to repository root
    $repoRoot = Split-Path (Split-Path $currentDir -Parent) -Parent
    return $repoRoot
}

# ============================================================================
# Environment Detection Functions
# ============================================================================

function Test-MSBuildInstallation {
    Write-Step "Checking for MSBuild..."
    
    # Try to find MSBuild via vswhere
    $vswherePath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    
    if (Test-Path $vswherePath) {
        try {
            $msbuildPath = & $vswherePath -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe | Select-Object -First 1
            
            if ($msbuildPath -and (Test-Path $msbuildPath)) {
                Write-Success "Found MSBuild at: $msbuildPath"
                return $msbuildPath
            }
        } catch {
            Write-Warning "Failed to find MSBuild via vswhere: $_"
        }
    }
    
    # Fallback: Check common paths
    $commonPaths = @(
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe",
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe",
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
    )
    
    foreach ($path in $commonPaths) {
        if (Test-Path $path) {
            Write-Success "Found MSBuild at: $path"
            return $path
        }
    }
    
    Write-Warning "MSBuild not found"
    return $null
}

function Test-WDKInstallation {
    Write-Step "Checking for Windows Driver Kit (WDK)..."
    
    $wdkPaths = @(
        "${env:ProgramFiles(x86)}\Windows Kits\10\Include",
        "${env:ProgramFiles}\Windows Kits\10\Include"
    )
    
    foreach ($path in $wdkPaths) {
        if (Test-Path $path) {
            # Check for WDF headers which are part of WDK
            $wdfPath = Join-Path $path "*\km\wdf*"
            $wdfExists = Test-Path $wdfPath
            
            if ($wdfExists) {
                Write-Success "Found WDK at: $path"
                return $path
            }
        }
    }
    
    Write-Warning "Windows Driver Kit (WDK) not found"
    return $null
}

function Test-TestSigningEnabled {
    Write-Step "Checking test signing status..."
    
    try {
        $bcdeditOutput = & bcdedit /enum "{current}" | Out-String
        
        if ($bcdeditOutput -match 'testsigning\s+Yes') {
            Write-Success "Test signing is ENABLED"
            return $true
        } else {
            Write-Warning "Test signing is DISABLED"
            return $false
        }
    } catch {
        Write-Warning "Failed to check test signing status: $_"
        return $false
    }
}

# ============================================================================
# Build Functions
# ============================================================================

function Invoke-DriverBuild {
    param(
        [string]$MSBuildPath,
        [string]$DriverPath,
        [string]$Configuration,
        [string]$Platform
    )
    
    Write-Step "Building nanabox_hvfilter driver..."
    
    Write-Info "Driver source: $DriverPath"
    Write-Info "Build configuration: $Configuration"
    Write-Info "Build platform: $Platform"
    
    try {
        # Find project file
        $projectFile = Get-ChildItem -Path $DriverPath -Filter "*.vcxproj" -File -ErrorAction SilentlyContinue | Select-Object -First 1
        
        if (-not $projectFile) {
            Write-ErrorMsg "No Visual Studio project file (.vcxproj) found in $DriverPath"
            return $false
        }
        
        Write-Info "Found project file: $($projectFile.Name)"
        Write-Info "Building with MSBuild..."
        
        $buildLog = Join-Path $DriverPath "build_phase3b.log"
        
        # Execute MSBuild
        $arguments = @(
            $projectFile.FullName,
            "/p:Configuration=$Configuration",
            "/p:Platform=$Platform",
            "/m",
            "/v:minimal",
            "/fl",
            "/flp:LogFile=$buildLog;Verbosity=detailed"
        )
        
        Write-Verbose "MSBuild command: $MSBuildPath $($arguments -join ' ')"
        
        $process = Start-Process -FilePath $MSBuildPath -ArgumentList $arguments -NoNewWindow -Wait -PassThru
        
        if ($process.ExitCode -eq 0) {
            Write-Success "Driver built successfully!"
            
            # Find output files
            $outputPath = Join-Path $DriverPath "$Platform\$Configuration"
            if (Test-Path $outputPath) {
                Write-Success "Build output location: $outputPath"
                
                $sysFile = Join-Path $outputPath "$DriverName.sys"
                $infFile = Join-Path $outputPath "$DriverName.inf"
                
                if (Test-Path $sysFile) {
                    Write-Success "Driver binary: $sysFile"
                }
                if (Test-Path $infFile) {
                    Write-Success "Driver INF: $infFile"
                }
            }
            
            return $true
        } else {
            Write-ErrorMsg "Build failed with exit code: $($process.ExitCode)"
            Write-Info "Check build log for details: $buildLog"
            return $false
        }
    } catch {
        Write-ErrorMsg "Exception during build: $_"
        return $false
    }
}

# ============================================================================
# Client Build Functions
# ============================================================================

function Invoke-ClientBuild {
    param(
        [string]$MSBuildPath,
        [string]$ClientPath,
        [string]$Configuration,
        [string]$Platform
    )
    
    Write-Step "Building NbxHvFilterClient..."
    
    Write-Info "Client source: $ClientPath"
    Write-Info "Build configuration: $Configuration"
    Write-Info "Build platform: $Platform"
    
    try {
        # Find project file
        $projectFile = Get-ChildItem -Path $ClientPath -Filter "*.vcxproj" -File -ErrorAction SilentlyContinue | Select-Object -First 1
        
        if (-not $projectFile) {
            Write-Warning "No Visual Studio project file (.vcxproj) found in $ClientPath"
            return $false
        }
        
        Write-Info "Found project file: $($projectFile.Name)"
        Write-Info "Building with MSBuild..."
        
        $buildLog = Join-Path $ClientPath "build_phase3b.log"
        
        # Execute MSBuild
        $arguments = @(
            $projectFile.FullName,
            "/p:Configuration=$Configuration",
            "/p:Platform=$Platform",
            "/m",
            "/v:minimal",
            "/fl",
            "/flp:LogFile=$buildLog;Verbosity=detailed"
        )
        
        Write-Verbose "MSBuild command: $MSBuildPath $($arguments -join ' ')"
        
        $process = Start-Process -FilePath $MSBuildPath -ArgumentList $arguments -NoNewWindow -Wait -PassThru
        
        if ($process.ExitCode -eq 0) {
            Write-Success "Client built successfully!"
            
            # Find output files
            $outputPath = Join-Path $ClientPath "$Platform\$Configuration"
            if (Test-Path $outputPath) {
                Write-Success "Build output location: $outputPath"
                
                $exeFile = Join-Path $outputPath "NbxHvFilterClient.exe"
                
                if (Test-Path $exeFile) {
                    Write-Success "Client binary: $exeFile"
                }
            }
            
            return $true
        } else {
            Write-Warning "Client build failed with exit code: $($process.ExitCode)"
            Write-Info "Check build log for details: $buildLog"
            return $false
        }
    } catch {
        Write-Warning "Exception during client build: $_"
        return $false
    }
}

# ============================================================================
# Driver Management Functions
# ============================================================================

function Get-DriverServiceStatus {
    try {
        $service = Get-Service -Name $ServiceName -ErrorAction SilentlyContinue
        return $service
    } catch {
        return $null
    }
}

function Install-Driver {
    param(
        [string]$DriverPath,
        [string]$Configuration,
        [string]$Platform
    )
    
    Write-Step "Installing/Updating nanabox_hvfilter driver..."
    
    $outputPath = Join-Path $DriverPath "$Platform\$Configuration"
    $sysFile = Join-Path $outputPath "$DriverName.sys"
    $infFile = Join-Path $outputPath "$DriverName.inf"
    
    if (-not (Test-Path $sysFile)) {
        Write-ErrorMsg "Driver binary not found: $sysFile"
        return $false
    }
    
    if (-not (Test-Path $infFile)) {
        Write-ErrorMsg "Driver INF not found: $infFile"
        return $false
    }
    
    if ($DryRun) {
        Write-Info "[DRY RUN] Would install driver from: $sysFile"
        return $true
    }
    
    try {
        # Check if driver already exists
        $destPath = Join-Path $env:SystemRoot "System32\drivers\$DriverName.sys"
        $isUpdate = Test-Path $destPath
        
        if ($isUpdate) {
            Write-Info "Driver already installed, performing update..."
            
            # Stop service if running
            $service = Get-DriverServiceStatus
            if ($service -and $service.Status -eq "Running") {
                Write-Info "Stopping driver service..."
                Stop-Service -Name $ServiceName -Force -ErrorAction SilentlyContinue
                Start-Sleep -Seconds 2
            }
        } else {
            Write-Info "Performing first-time driver installation..."
        }
        
        # Copy driver to system directory
        Write-Info "Copying driver to: $destPath"
        Copy-Item -Path $sysFile -Destination $destPath -Force
        Write-Success "Driver file copied successfully"
        
        # Create or update service
        $service = Get-DriverServiceStatus
        if ($service) {
            Write-Info "Driver service already exists"
        } else {
            Write-Info "Creating driver service..."
            $servicePath = "`"$destPath`""
            $scResult = & sc.exe create $ServiceName type= kernel start= demand error= normal binPath= $servicePath DisplayName= "`"$ServiceDisplayName`"" 2>&1
            
            if ($LASTEXITCODE -eq 0) {
                Write-Success "Driver service created successfully"
            } else {
                Write-ErrorMsg "Failed to create service: $scResult"
                return $false
            }
        }
        
        return $true
    } catch {
        Write-ErrorMsg "Exception during installation: $_"
        return $false
    }
}

function Start-DriverService {
    Write-Step "Starting driver service..."
    
    if ($DryRun) {
        Write-Info "[DRY RUN] Would start driver service: $ServiceName"
        return $true
    }
    
    try {
        $service = Get-DriverServiceStatus
        
        if (-not $service) {
            Write-ErrorMsg "Driver service not found. Install the driver first."
            return $false
        }
        
        if ($service.Status -eq "Running") {
            Write-Success "Driver service is already running"
            return $true
        }
        
        Write-Info "Starting driver service..."
        Start-Service -Name $ServiceName -ErrorAction Stop
        
        # Wait a moment for service to start
        Start-Sleep -Seconds 2
        
        # Verify it started
        $service = Get-DriverServiceStatus
        if ($service.Status -eq "Running") {
            Write-Success "Driver service started successfully"
            return $true
        } else {
            Write-ErrorMsg "Driver service failed to start. Status: $($service.Status)"
            Write-Info "Check Event Viewer (System log) for details"
            return $false
        }
    } catch {
        Write-ErrorMsg "Failed to start driver service: $_"
        Write-Info "Check Event Viewer (System log) for details"
        Write-Info "See driver-hvfilter.md > Troubleshooting for common issues"
        return $false
    }
}

function Stop-DriverService {
    Write-Step "Stopping driver service..."
    
    if ($DryRun) {
        Write-Info "[DRY RUN] Would stop driver service: $ServiceName"
        return $true
    }
    
    try {
        $service = Get-DriverServiceStatus
        
        if (-not $service) {
            Write-Info "Driver service not found (already removed or never installed)"
            return $true
        }
        
        if ($service.Status -ne "Running") {
            Write-Info "Driver service is not running"
            return $true
        }
        
        Write-Info "Stopping driver service..."
        Stop-Service -Name $ServiceName -Force -ErrorAction Stop
        
        # Wait a moment for service to stop
        Start-Sleep -Seconds 2
        
        # Verify it stopped
        $service = Get-DriverServiceStatus
        if ($service.Status -eq "Stopped") {
            Write-Success "Driver service stopped successfully"
            return $true
        } else {
            Write-Warning "Driver service may not have stopped cleanly. Status: $($service.Status)"
            return $false
        }
    } catch {
        Write-ErrorMsg "Failed to stop driver service: $_"
        return $false
    }
}

# ============================================================================
# Testing Functions
# ============================================================================

function Invoke-SanityChecks {
    param(
        [string]$ClientPath,
        [string]$Configuration,
        [string]$Platform
    )
    
    Write-Step "Running sanity checks with NbxHvFilterClient..."
    
    if ($DryRun) {
        Write-Info "[DRY RUN] Would run sanity checks with NbxHvFilterClient.exe"
        return $true
    }
    
    # Find client executable
    $outputPath = Join-Path $ClientPath "$Platform\$Configuration"
    $clientExe = Join-Path $outputPath "NbxHvFilterClient.exe"
    
    if (-not (Test-Path $clientExe)) {
        Write-Warning "NbxHvFilterClient.exe not found at: $clientExe"
        Write-Info "Skipping sanity checks (client not built)"
        return $false
    }
    
    Write-Info "Client executable: $clientExe"
    
    try {
        # Run status command
        Write-Info "Executing: NbxHvFilterClient.exe status"
        Write-Host ""
        
        $result = & $clientExe status 2>&1
        $exitCode = $LASTEXITCODE
        
        Write-Host $result
        Write-Host ""
        
        if ($exitCode -eq 0) {
            Write-Success "IOCTL test: PASSED"
            Write-Success "Driver is responding correctly to status queries"
            return $true
        } else {
            Write-ErrorMsg "IOCTL test: FAILED (exit code: $exitCode)"
            Write-Info "The driver may not be running or the IOCTL interface has issues"
            return $false
        }
    } catch {
        Write-ErrorMsg "Exception during sanity checks: $_"
        return $false
    }
}

# ============================================================================
# Summary Functions
# ============================================================================

function Write-FinalSummary {
    param(
        [bool]$BuildSuccess,
        [bool]$InstallSuccess,
        [bool]$StartSuccess,
        [bool]$TestSuccess
    )
    
    Write-Header "Phase 3B Setup Summary"
    
    Write-Host "Build:    " -NoNewline
    if ($SkipBuild) {
        Write-Host "SKIPPED" -ForegroundColor $ColorScheme.Warning
    } elseif ($BuildSuccess) {
        Write-Host "OK" -ForegroundColor $ColorScheme.Success
    } else {
        Write-Host "FAILED" -ForegroundColor $ColorScheme.Error
    }
    
    Write-Host "Install:  " -NoNewline
    if ($SkipInstall) {
        Write-Host "SKIPPED" -ForegroundColor $ColorScheme.Warning
    } elseif ($InstallSuccess) {
        Write-Host "OK" -ForegroundColor $ColorScheme.Success
    } else {
        Write-Host "FAILED" -ForegroundColor $ColorScheme.Error
    }
    
    Write-Host "Start:    " -NoNewline
    if ($SkipStart) {
        Write-Host "SKIPPED" -ForegroundColor $ColorScheme.Warning
    } elseif ($StartSuccess) {
        Write-Host "OK (Running)" -ForegroundColor $ColorScheme.Success
    } else {
        Write-Host "FAILED" -ForegroundColor $ColorScheme.Error
    }
    
    Write-Host "IOCTL Test: " -NoNewline
    if ($SkipTest) {
        Write-Host "SKIPPED" -ForegroundColor $ColorScheme.Warning
    } elseif ($TestSuccess) {
        Write-Host "OK" -ForegroundColor $ColorScheme.Success
    } else {
        Write-Host "FAILED" -ForegroundColor $ColorScheme.Error
    }
    
    Write-Host ""
    
    # Determine overall success: all non-skipped steps must succeed
    $allSucceeded = $true
    if (-not $SkipBuild -and -not $BuildSuccess) { $allSucceeded = $false }
    if (-not $SkipInstall -and -not $InstallSuccess) { $allSucceeded = $false }
    if (-not $SkipStart -and -not $StartSuccess) { $allSucceeded = $false }
    if (-not $SkipTest -and -not $TestSuccess) { $allSucceeded = $false }
    
    if ($allSucceeded) {
        Write-Success "All Phase 3B setup steps completed successfully!"
        Write-Host ""
        Write-Info "Next steps:"
        Write-Info "  1. Test CPUID/MSR configuration with NbxHvFilterClient"
        Write-Info "  2. Review driver logs in DebugView"
        Write-Info "  3. See PHASE3B_TESTING.md for detailed test scenarios"
    } else {
        Write-Warning "Some Phase 3B setup steps failed or were skipped"
        Write-Host ""
        Write-Info "Troubleshooting:"
        Write-Info "  - Check that test-signing is enabled: bcdedit /enum | findstr testsigning"
        Write-Info "  - Verify Secure Boot is disabled in BIOS/UEFI"
        Write-Info "  - Check Event Viewer (System log) for driver load errors"
        Write-Info "  - See docs/driver-hvfilter.md > Troubleshooting"
    }
    
    Write-Host ""
}

# ============================================================================
# Main Setup Flow
# ============================================================================

function Start-Phase3BSetup {
    Write-Header "NanaBox HvFilter Phase 3B Setup v$ScriptVersion"
    
    Write-Info "Configuration: $Configuration"
    Write-Info "Platform: $Platform"
    if ($DryRun) {
        Write-Warning "DRY RUN MODE - No driver installation will occur"
    }
    Write-Host ""
    
    # Check for admin privileges
    if (-not (Test-AdministratorPrivileges)) {
        Write-ErrorMsg "This script requires Administrator privileges!"
        Write-Info "Please run PowerShell as Administrator and try again."
        exit 1
    }
    
    Write-Success "Running with Administrator privileges"
    
    # Get repository paths
    $repoRoot = Get-RepositoryRoot
    $driverPath = Join-Path $repoRoot "drivers\$DriverName"
    $clientPath = Join-Path $repoRoot "tools\NbxHvFilterClient"
    
    Write-Info "Repository root: $repoRoot"
    Write-Info "Driver path: $driverPath"
    Write-Info "Client path: $clientPath"
    
    if (-not (Test-Path $driverPath)) {
        Write-ErrorMsg "Driver source directory not found: $driverPath"
        exit 1
    }
    
    # Track success of each step
    $buildSuccess = $false
    $installSuccess = $false
    $startSuccess = $false
    $testSuccess = $false
    
    # Step 1: Environment Validation
    Write-Header "Step 1: Environment Validation"
    
    $testSigningEnabled = Test-TestSigningEnabled
    if (-not $testSigningEnabled) {
        Write-Warning "Test signing is not enabled!"
        Write-Info "Enable with: bcdedit /set testsigning on"
        Write-Info "Then reboot the system"
        Write-Warning "Continuing anyway, but driver may not load..."
    }
    
    if (-not $SkipBuild) {
        $msbuildPath = Test-MSBuildInstallation
        $wdkPath = Test-WDKInstallation
        
        if (-not $msbuildPath) {
            Write-ErrorMsg "MSBuild not found. Cannot build driver."
            Write-Info "Install Visual Studio 2022 with C++ workload"
            Write-Info "Or use -SkipBuild to use pre-built binaries"
            exit 1
        }
        
        if (-not $wdkPath) {
            Write-ErrorMsg "Windows Driver Kit (WDK) not found. Cannot build driver."
            Write-Info "Download from: https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk"
            Write-Info "Or use -SkipBuild to use pre-built binaries"
            exit 1
        }
    } else {
        Write-Info "Skipping environment checks (build skipped)"
    }
    
    # Step 2: Build Driver
    Write-Header "Step 2: Build Driver and Client"
    
    if (-not $SkipBuild) {
        $buildSuccess = Invoke-DriverBuild -MSBuildPath $msbuildPath -DriverPath $driverPath -Configuration $Configuration -Platform $Platform
        
        if (-not $buildSuccess) {
            Write-ErrorMsg "Driver build failed!"
            exit 1
        }
        
        # Build client (non-fatal if it fails)
        $clientBuildSuccess = Invoke-ClientBuild -MSBuildPath $msbuildPath -ClientPath $clientPath -Configuration $Configuration -Platform $Platform
        if (-not $clientBuildSuccess) {
            Write-Warning "Client build failed, but continuing..."
        }
    } else {
        Write-Info "Skipping driver build (using existing binaries)"
        $buildSuccess = $true
    }
    
    # Step 3: Install Driver
    Write-Header "Step 3: Install/Update Driver"
    
    if (-not $SkipInstall) {
        $installSuccess = Install-Driver -DriverPath $driverPath -Configuration $Configuration -Platform $Platform
        
        if (-not $installSuccess) {
            Write-ErrorMsg "Driver installation failed!"
            Write-FinalSummary -BuildSuccess $buildSuccess -InstallSuccess $installSuccess -StartSuccess $startSuccess -TestSuccess $testSuccess
            exit 1
        }
    } else {
        Write-Info "Skipping driver installation"
        $installSuccess = $true
    }
    
    # Step 4: Start Driver Service
    Write-Header "Step 4: Start Driver Service"
    
    if (-not $SkipStart) {
        $startSuccess = Start-DriverService
        
        if (-not $startSuccess) {
            Write-ErrorMsg "Failed to start driver service!"
            Write-FinalSummary -BuildSuccess $buildSuccess -InstallSuccess $installSuccess -StartSuccess $startSuccess -TestSuccess $testSuccess
            exit 1
        }
    } else {
        Write-Info "Skipping driver start"
        # Check if already running
        $service = Get-DriverServiceStatus
        $startSuccess = ($service -and $service.Status -eq "Running")
        if ($startSuccess) {
            Write-Info "Driver service is already running"
        }
    }
    
    # Step 5: Run Sanity Checks
    Write-Header "Step 5: Run Sanity Checks"
    
    if (-not $SkipTest) {
        $testSuccess = Invoke-SanityChecks -ClientPath $clientPath -Configuration $Configuration -Platform $Platform
        
        if (-not $testSuccess) {
            Write-Warning "Sanity checks failed or client not available"
        }
    } else {
        Write-Info "Skipping sanity checks"
        $testSuccess = $true
    }
    
    # Final Summary
    Write-FinalSummary -BuildSuccess $buildSuccess -InstallSuccess $installSuccess -StartSuccess $startSuccess -TestSuccess $testSuccess
    
    # Return appropriate exit code: all non-skipped steps must succeed
    $exitCode = 0
    if (-not $SkipBuild -and -not $buildSuccess) { $exitCode = 1 }
    if (-not $SkipInstall -and -not $installSuccess) { $exitCode = 1 }
    if (-not $SkipStart -and -not $startSuccess) { $exitCode = 1 }
    if (-not $SkipTest -and -not $testSuccess) { $exitCode = 1 }
    
    exit $exitCode
}

# ============================================================================
# Script Entry Point
# ============================================================================

try {
    Start-Phase3BSetup
} catch {
    Write-ErrorMsg "Fatal error: $_"
    Write-ErrorMsg $_.Exception.Message
    Write-ErrorMsg $_.ScriptStackTrace
    exit 1
}
