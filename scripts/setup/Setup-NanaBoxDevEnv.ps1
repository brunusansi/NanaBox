<#
.SYNOPSIS
    NanaBox Anti-Detection Edition Development Environment Setup Wizard

.DESCRIPTION
    Interactive PowerShell script that prepares a complete development environment for
    building and installing NanaBox Anti-Detection Edition kernel drivers.
    
    This script will:
    1. Check for required tools (Visual Studio, MSBuild, WDK, Windows SDK)
    2. Enable test signing mode (requires reboot)
    3. Create and install a test certificate for driver signing
    4. Build the NanaBox kernel drivers (nanabox_hvfilter.sys)
    5. Optionally install the drivers
    
    All actions require interactive confirmation - nothing is done silently.

.PARAMETER SkipEnvironmentCheck
    Skip environment checks for Visual Studio, WDK, and Windows SDK.

.PARAMETER SkipTestSigning
    Skip test signing setup (assumes already enabled).

.PARAMETER SkipCertificate
    Skip certificate creation (assumes test certificate already exists).

.PARAMETER SkipBuild
    Skip driver building step.

.PARAMETER SkipInstall
    Skip driver installation prompt.

.PARAMETER Unattended
    Run in unattended mode with default "Yes" answers (NOT RECOMMENDED - use with caution).

.EXAMPLE
    .\Setup-NanaBoxDevEnv.ps1
    
    Run the complete interactive setup wizard.

.EXAMPLE
    .\Setup-NanaBoxDevEnv.ps1 -SkipEnvironmentCheck
    
    Skip environment checks and proceed with setup.

.EXAMPLE
    .\Setup-NanaBoxDevEnv.ps1 -SkipBuild -SkipInstall
    
    Only setup the environment (test signing, certificates) without building or installing.

.NOTES
    Author: NanaBox Anti-Detection Edition Contributors
    Requires: Windows 10/11, PowerShell 5.1+, Administrator privileges
    Phase: 3 - Driver Development Setup
    
    IMPORTANT:
    - This script MUST be run as Administrator
    - Test signing requires a system reboot to take effect
    - Secure Boot must be disabled for test-signed drivers to load
    - This is for development/testing only - production requires EV code signing

.LINK
    https://github.com/brunusansi/NanaBox
    https://learn.microsoft.com/en-us/windows-hardware/drivers/
#>

[CmdletBinding(SupportsShouldProcess=$true)]
param(
    [Parameter(Mandatory=$false)]
    [switch]$SkipEnvironmentCheck,
    
    [Parameter(Mandatory=$false)]
    [switch]$SkipTestSigning,
    
    [Parameter(Mandatory=$false)]
    [switch]$SkipCertificate,
    
    [Parameter(Mandatory=$false)]
    [switch]$SkipBuild,
    
    [Parameter(Mandatory=$false)]
    [switch]$SkipInstall,
    
    [Parameter(Mandatory=$false)]
    [switch]$Unattended
)

#Requires -RunAsAdministrator

# ============================================================================
# Constants and Configuration
# ============================================================================

$ErrorActionPreference = "Stop"
$VerbosePreference = if ($PSBoundParameters['Verbose']) { "Continue" } else { "SilentlyContinue" }

# Script configuration
$ScriptVersion = "1.0.0"
$CertificateName = "NanaBox Test Certificate"
$CertificateStoreName = "PrivateCertStore"
$DriverName = "nanabox_hvfilter"

# URLs for downloading required tools
$DownloadUrls = @{
    VisualStudio = "https://visualstudio.microsoft.com/downloads/"
    WDK = "https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk"
    WindowsSDK = "https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/"
}

# Color scheme for console output
$ColorScheme = @{
    Title = "Cyan"
    Success = "Green"
    Warning = "Yellow"
    Error = "Red"
    Info = "White"
    Prompt = "Cyan"
    Highlight = "Magenta"
}

# ============================================================================
# Helper Functions
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

function Write-Error {
    param([string]$Message)
    Write-Host "[ERROR] $Message" -ForegroundColor $ColorScheme.Error
}

function Write-Step {
    param([string]$Message)
    Write-Host ""
    Write-Host ">>> $Message" -ForegroundColor $ColorScheme.Highlight
}

function Confirm-Action {
    param(
        [string]$Message,
        [switch]$DefaultYes
    )
    
    if ($Unattended) {
        Write-Info "Unattended mode: Auto-accepting"
        return $true
    }
    
    $prompt = if ($DefaultYes) { "$Message [Y/n]" } else { "$Message [y/N]" }
    
    Write-Host ""
    $response = Read-Host $prompt
    
    if ([string]::IsNullOrWhiteSpace($response)) {
        return $DefaultYes
    }
    
    return $response -match '^[Yy]'
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

function Test-VisualStudioInstallation {
    Write-Step "Checking for Visual Studio installation..."
    
    $vswherePath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    
    if (-not (Test-Path $vswherePath)) {
        Write-Warning "vswhere.exe not found"
        return $null
    }
    
    try {
        $vsInstances = & $vswherePath -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath -format value
        
        if ($vsInstances) {
            Write-Success "Found Visual Studio at: $vsInstances"
            return $vsInstances
        }
    } catch {
        Write-Warning "Failed to query Visual Studio installations: $_"
    }
    
    return $null
}

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

function Test-WindowsSDKInstallation {
    Write-Step "Checking for Windows SDK..."
    
    $sdkPaths = @(
        "${env:ProgramFiles(x86)}\Windows Kits\10",
        "${env:ProgramFiles}\Windows Kits\10"
    )
    
    foreach ($path in $sdkPaths) {
        if (Test-Path $path) {
            # Check for SDK version directories
            $includePath = Join-Path $path "Include"
            if (Test-Path $includePath) {
                $versions = Get-ChildItem $includePath -Directory | Where-Object { $_.Name -match '^\d+\.' } | Sort-Object Name -Descending
                
                if ($versions.Count -gt 0) {
                    $latestVersion = $versions[0].Name
                    Write-Success "Found Windows SDK version $latestVersion at: $path"
                    return @{
                        Path = $path
                        Version = $latestVersion
                    }
                }
            }
        }
    }
    
    Write-Warning "Windows SDK not found"
    return $null
}

# ============================================================================
# Test Signing Functions
# ============================================================================

function Get-TestSigningStatus {
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

function Enable-TestSigning {
    Write-Step "Enabling test signing mode..."
    
    Write-Info "Test signing allows loading of test-signed kernel-mode drivers."
    Write-Info "This is required for development and testing of NanaBox drivers."
    Write-Warning "IMPORTANT: This requires a system reboot to take effect!"
    Write-Warning "IMPORTANT: Secure Boot must be disabled in BIOS/UEFI for test-signed drivers."
    
    if (-not (Confirm-Action "Enable test signing mode? (requires reboot)" -DefaultYes)) {
        Write-Info "Skipping test signing setup"
        return $false
    }
    
    try {
        Write-Info "Running: bcdedit /set testsigning on"
        $result = & bcdedit /set testsigning on 2>&1
        
        if ($LASTEXITCODE -eq 0) {
            Write-Success "Test signing enabled successfully"
            Write-Warning "A system reboot is required for changes to take effect!"
            
            if (Confirm-Action "Reboot now?" -DefaultYes:$false) {
                Write-Warning "System will reboot in 30 seconds... (Press Ctrl+C to cancel)"
                Write-Warning "Please save any open work before the reboot!"
                for ($i = 30; $i -gt 0; $i--) {
                    Write-Host "`rRebooting in $i seconds... " -NoNewline -ForegroundColor Yellow
                    Start-Sleep -Seconds 1
                }
                Write-Host ""
                Restart-Computer
                exit 0
            } else {
                Write-Info "Please reboot manually for test signing to take effect"
                Write-Info "After reboot, run this script again to continue"
            }
            
            return $true
        } else {
            Write-Error "Failed to enable test signing: $result"
            return $false
        }
    } catch {
        Write-Error "Exception while enabling test signing: $_"
        return $false
    }
}

# ============================================================================
# Certificate Functions
# ============================================================================

function Test-TestCertificate {
    param([string]$CertName = $CertificateName)
    
    Write-Step "Checking for existing test certificate..."
    
    try {
        $cert = Get-ChildItem -Path Cert:\LocalMachine\My | Where-Object { $_.Subject -like "*$CertName*" } | Select-Object -First 1
        
        if ($cert) {
            Write-Success "Found test certificate: $($cert.Subject)"
            Write-Info "Thumbprint: $($cert.Thumbprint)"
            Write-Info "Valid until: $($cert.NotAfter)"
            return $cert
        } else {
            Write-Warning "Test certificate not found"
            return $null
        }
    } catch {
        Write-Warning "Failed to check for test certificate: $_"
        return $null
    }
}

function New-TestCertificate {
    param([string]$CertName = $CertificateName)
    
    Write-Step "Creating self-signed test certificate..."
    
    Write-Info "This creates a self-signed certificate for signing test drivers."
    Write-Info "Certificate name: $CertName"
    Write-Info "Validity period: 10 years"
    
    if (-not (Confirm-Action "Create test certificate?" -DefaultYes)) {
        Write-Info "Skipping certificate creation"
        return $null
    }
    
    try {
        # Check if makecert is available (older method) or use New-SelfSignedCertificate
        Write-Info "Creating self-signed certificate using PowerShell cmdlet..."
        
        $notBefore = (Get-Date).AddDays(-1)
        $notAfter = (Get-Date).AddYears(10)
        
        # Create certificate
        $cert = New-SelfSignedCertificate `
            -Subject "CN=$CertName" `
            -Type CodeSigningCert `
            -KeyUsage DigitalSignature `
            -KeyAlgorithm RSA `
            -KeyLength 2048 `
            -NotBefore $notBefore `
            -NotAfter $notAfter `
            -CertStoreLocation "Cert:\LocalMachine\My"
        
        if ($cert) {
            Write-Success "Certificate created successfully"
            Write-Info "Thumbprint: $($cert.Thumbprint)"
            
            # Install to Trusted Root and Trusted Publishers
            Write-Step "Installing certificate to trusted stores..."
            
            $certBytes = $cert.Export([System.Security.Cryptography.X509Certificates.X509ContentType]::Cert)
            
            # Install to Root
            $rootStore = New-Object System.Security.Cryptography.X509Certificates.X509Store("Root", "LocalMachine")
            $rootStore.Open("ReadWrite")
            $rootStore.Add($cert)
            $rootStore.Close()
            Write-Success "Installed to Trusted Root Certification Authorities"
            
            # Install to TrustedPublisher
            $pubStore = New-Object System.Security.Cryptography.X509Certificates.X509Store("TrustedPublisher", "LocalMachine")
            $pubStore.Open("ReadWrite")
            $pubStore.Add($cert)
            $pubStore.Close()
            Write-Success "Installed to Trusted Publishers"
            
            return $cert
        } else {
            Write-Error "Failed to create certificate"
            return $null
        }
    } catch {
        Write-Error "Exception while creating certificate: $_"
        Write-Error $_.Exception.Message
        return $null
    }
}

# ============================================================================
# Build Functions
# ============================================================================

function Invoke-DriverBuild {
    param(
        [string]$MSBuildPath,
        [string]$DriverPath
    )
    
    Write-Step "Building NanaBox kernel driver..."
    
    Write-Info "This will build the nanabox_hvfilter.sys kernel driver."
    Write-Info "Driver source: $DriverPath"
    Write-Info "Build configuration: Release"
    Write-Info "Build platform: x64"
    
    if (-not (Confirm-Action "Build driver now?" -DefaultYes)) {
        Write-Info "Skipping driver build"
        return $false
    }
    
    try {
        # Check if driver project file exists
        $projectFile = Get-ChildItem -Path $DriverPath -Filter "*.vcxproj" -File -ErrorAction SilentlyContinue | Select-Object -First 1
        
        if (-not $projectFile) {
            Write-Warning "No Visual Studio project file (.vcxproj) found in $DriverPath"
            Write-Info "The driver project may need to be created first."
            Write-Info "Please ensure the driver source code includes a proper .vcxproj file."
            return $false
        }
        
        Write-Info "Found project file: $($projectFile.Name)"
        Write-Info "Building with MSBuild..."
        
        $buildLog = Join-Path $DriverPath "build.log"
        
        # Execute MSBuild
        $arguments = @(
            $projectFile.FullName,
            "/p:Configuration=Release",
            "/p:Platform=x64",
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
            $outputPath = Join-Path $DriverPath "x64\Release"
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
            Write-Error "Build failed with exit code: $($process.ExitCode)"
            Write-Info "Check build log for details: $buildLog"
            return $false
        }
    } catch {
        Write-Error "Exception during build: $_"
        Write-Error $_.Exception.Message
        return $false
    }
}

function Invoke-DriverSigning {
    param(
        [string]$DriverPath,
        [object]$Certificate
    )
    
    Write-Step "Signing driver binary..."
    
    $sysFile = Join-Path $DriverPath "x64\Release\$DriverName.sys"
    
    if (-not (Test-Path $sysFile)) {
        Write-Warning "Driver binary not found: $sysFile"
        return $false
    }
    
    Write-Info "This will sign the driver with the test certificate."
    Write-Info "Driver: $sysFile"
    Write-Info "Certificate: $($Certificate.Subject)"
    
    if (-not (Confirm-Action "Sign driver binary?" -DefaultYes)) {
        Write-Info "Skipping driver signing"
        return $false
    }
    
    try {
        # Check if signtool is available
        $signtoolPath = Get-Command "signtool.exe" -ErrorAction SilentlyContinue
        
        if (-not $signtoolPath) {
            # Try to find in Windows SDK
            $sdkPaths = @(
                "${env:ProgramFiles(x86)}\Windows Kits\10\bin\*\x64\signtool.exe",
                "${env:ProgramFiles}\Windows Kits\10\bin\*\x64\signtool.exe"
            )
            
            foreach ($path in $sdkPaths) {
                $foundPath = Get-Item $path -ErrorAction SilentlyContinue | Select-Object -First 1
                if ($foundPath) {
                    $signtoolPath = $foundPath.FullName
                    break
                }
            }
        } else {
            $signtoolPath = $signtoolPath.Source
        }
        
        if (-not $signtoolPath) {
            Write-Warning "signtool.exe not found. Driver signing skipped."
            Write-Info "Install Windows SDK to get signtool.exe"
            return $false
        }
        
        Write-Info "Using signtool: $signtoolPath"
        
        # Sign the driver
        $arguments = @(
            "sign",
            "/v",
            "/sha1", $Certificate.Thumbprint,
            "/tr", "http://timestamp.digicert.com",
            "/td", "SHA256",
            "/fd", "SHA256",
            $sysFile
        )
        
        Write-Verbose "Signtool command: $signtoolPath $($arguments -join ' ')"
        
        $process = Start-Process -FilePath $signtoolPath -ArgumentList $arguments -NoNewWindow -Wait -PassThru
        
        if ($process.ExitCode -eq 0) {
            Write-Success "Driver signed successfully!"
            return $true
        } else {
            Write-Warning "Driver signing failed with exit code: $($process.ExitCode)"
            Write-Info "Driver may still work if test signing is enabled"
            return $false
        }
    } catch {
        Write-Warning "Exception during signing: $_"
        Write-Info "Driver may still work if test signing is enabled"
        return $false
    }
}

# ============================================================================
# Installation Functions
# ============================================================================

function Install-NanaBoxDriver {
    param([string]$DriverPath)
    
    Write-Step "Installing NanaBox driver..."
    
    $infFile = Join-Path $DriverPath "x64\Release\$DriverName.inf"
    $sysFile = Join-Path $DriverPath "x64\Release\$DriverName.sys"
    
    if (-not (Test-Path $infFile) -or -not (Test-Path $sysFile)) {
        Write-Error "Driver files not found"
        return $false
    }
    
    Write-Warning "IMPORTANT: Driver installation will:"
    Write-Warning "  1. Copy driver files to Windows system directories"
    Write-Warning "  2. Create a kernel driver service"
    Write-Warning "  3. Start the driver service"
    Write-Warning ""
    Write-Warning "This affects your system at kernel level!"
    Write-Warning "Only proceed if you understand the implications."
    
    if (-not (Confirm-Action "Install and start the driver?" -DefaultYes:$false)) {
        Write-Info "Skipping driver installation"
        Write-Info "You can manually install later using:"
        Write-Info "  pnputil /add-driver $infFile /install"
        return $false
    }
    
    try {
        # Install driver using pnputil
        Write-Info "Installing driver with pnputil..."
        $result = & pnputil /add-driver $infFile /install 2>&1
        
        if ($LASTEXITCODE -eq 0) {
            Write-Success "Driver installed successfully"
            
            # Copy driver to system32\drivers
            $destPath = Join-Path $env:SystemRoot "System32\drivers\$DriverName.sys"
            if (Test-Path $destPath) {
                Write-Warning "Driver file already exists at: $destPath"
                if (-not (Confirm-Action "Overwrite existing driver file?" -DefaultYes:$false)) {
                    Write-Info "Skipping driver file copy"
                    return $false
                }
            }
            Write-Info "Copying driver to: $destPath"
            Copy-Item -Path $sysFile -Destination $destPath -Force
            
            # Create service
            Write-Info "Creating driver service..."
            $servicePath = "`"$destPath`""
            $scResult = & sc.exe create $DriverName type= kernel binPath= $servicePath 2>&1
            
            if ($LASTEXITCODE -eq 0 -or $scResult -like "*already exists*") {
                Write-Success "Driver service created/exists"
                
                # Start service
                if (Confirm-Action "Start the driver service now?" -DefaultYes) {
                    Write-Info "Starting driver service..."
                    $startResult = & sc.exe start $DriverName 2>&1
                    
                    if ($LASTEXITCODE -eq 0 -or $startResult -like "*already*running*") {
                        Write-Success "Driver service started successfully!"
                        Write-Success "NanaBox driver is now running"
                    } else {
                        Write-Warning "Failed to start driver service: $startResult"
                        Write-Info "You can start it manually: sc start $DriverName"
                    }
                } else {
                    Write-Info "Driver installed but not started"
                    Write-Info "Start manually with: sc start $DriverName"
                }
                
                return $true
            } else {
                Write-Error "Failed to create service: $scResult"
                return $false
            }
        } else {
            Write-Error "Driver installation failed: $result"
            return $false
        }
    } catch {
        Write-Error "Exception during installation: $_"
        Write-Error $_.Exception.Message
        return $false
    }
}

# ============================================================================
# Main Wizard Flow
# ============================================================================

function Start-SetupWizard {
    Write-Header "NanaBox Anti-Detection Edition - Development Environment Setup Wizard v$ScriptVersion"
    
    Write-Info "This wizard will set up your development environment for building NanaBox kernel drivers."
    Write-Info "All steps require confirmation - nothing is done automatically."
    Write-Host ""
    
    # Check for admin privileges
    if (-not (Test-AdministratorPrivileges)) {
        Write-Error "This script requires Administrator privileges!"
        Write-Info "Please run PowerShell as Administrator and try again."
        exit 1
    }
    
    Write-Success "Running with Administrator privileges"
    
    # Get repository root
    $repoRoot = Get-RepositoryRoot
    $driverPath = Join-Path $repoRoot "drivers\$DriverName"
    
    Write-Info "Repository root: $repoRoot"
    Write-Info "Driver path: $driverPath"
    
    if (-not (Test-Path $driverPath)) {
        Write-Error "Driver source directory not found: $driverPath"
        Write-Info "Please ensure you're running this script from the NanaBox repository."
        exit 1
    }
    
    # Step 1: Environment Checks
    Write-Header "Step 1: Environment Check"
    
    $envStatus = @{
        VisualStudio = $null
        MSBuild = $null
        WDK = $null
        WindowsSDK = $null
    }
    
    if (-not $SkipEnvironmentCheck) {
        $envStatus.VisualStudio = Test-VisualStudioInstallation
        $envStatus.MSBuild = Test-MSBuildInstallation
        $envStatus.WDK = Test-WDKInstallation
        $envStatus.WindowsSDK = Test-WindowsSDKInstallation
        
        # Check for missing components
        $missingComponents = @()
        
        if (-not $envStatus.VisualStudio) {
            $missingComponents += "Visual Studio with C++ Desktop Development"
        }
        if (-not $envStatus.MSBuild) {
            $missingComponents += "MSBuild"
        }
        if (-not $envStatus.WDK) {
            $missingComponents += "Windows Driver Kit (WDK)"
        }
        if (-not $envStatus.WindowsSDK) {
            $missingComponents += "Windows SDK"
        }
        
        if ($missingComponents.Count -gt 0) {
            Write-Warning "Missing required components:"
            foreach ($component in $missingComponents) {
                Write-Warning "  - $component"
            }
            Write-Host ""
            Write-Info "Required downloads:"
            Write-Info "  Visual Studio: $($DownloadUrls.VisualStudio)"
            Write-Info "  Windows Driver Kit: $($DownloadUrls.WDK)"
            Write-Info "  Windows SDK: $($DownloadUrls.WindowsSDK)"
            Write-Host ""
            
            if (Confirm-Action "Open download pages in browser?" -DefaultYes) {
                foreach ($url in $DownloadUrls.Values) {
                    Start-Process $url
                }
            }
            
            Write-Info "Please install missing components and run this script again."
            exit 1
        } else {
            Write-Success "All required components are installed!"
        }
    } else {
        Write-Info "Skipping environment checks"
    }
    
    # Step 2: Test Signing
    Write-Header "Step 2: Test Signing Setup"
    
    if (-not $SkipTestSigning) {
        $testSigningEnabled = Get-TestSigningStatus
        
        if (-not $testSigningEnabled) {
            $enabled = Enable-TestSigning
            
            if ($enabled -and -not (Get-TestSigningStatus)) {
                Write-Warning "Test signing was enabled but requires a reboot"
                Write-Info "Please reboot and run this script again to continue"
                exit 0
            }
        } else {
            Write-Success "Test signing is already enabled"
        }
    } else {
        Write-Info "Skipping test signing setup"
    }
    
    # Step 3: Certificate
    Write-Header "Step 3: Test Certificate Setup"
    
    $certificate = $null
    
    if (-not $SkipCertificate) {
        $certificate = Test-TestCertificate
        
        if (-not $certificate) {
            $certificate = New-TestCertificate
            
            if (-not $certificate) {
                Write-Warning "Certificate creation failed or was skipped"
                Write-Info "Driver can still be built but may not be signed"
            }
        } else {
            Write-Success "Test certificate is already installed"
        }
    } else {
        Write-Info "Skipping certificate setup"
        $certificate = Test-TestCertificate
    }
    
    # Step 4: Build Driver
    Write-Header "Step 4: Build Driver"
    
    if (-not $SkipBuild) {
        if (-not $envStatus.MSBuild) {
            $envStatus.MSBuild = Test-MSBuildInstallation
        }
        
        if ($envStatus.MSBuild) {
            $buildSuccess = Invoke-DriverBuild -MSBuildPath $envStatus.MSBuild -DriverPath $driverPath
            
            if ($buildSuccess -and $certificate) {
                # Optional: Sign the driver
                if (Confirm-Action "Sign the driver with test certificate?" -DefaultYes) {
                    Invoke-DriverSigning -DriverPath $driverPath -Certificate $certificate
                }
            }
        } else {
            Write-Warning "MSBuild not found - cannot build driver"
            Write-Info "Please install Visual Studio and run this script again"
        }
    } else {
        Write-Info "Skipping driver build"
    }
    
    # Step 5: Install Driver
    Write-Header "Step 5: Driver Installation (Optional)"
    
    if (-not $SkipInstall) {
        if (Confirm-Action "Do you want to install the driver now?" -DefaultYes:$false) {
            Install-NanaBoxDriver -DriverPath $driverPath
        } else {
            Write-Info "Driver installation skipped"
            Write-Info "You can install manually later using:"
            Write-Info "  cd $driverPath"
            Write-Info "  pnputil /add-driver x64\Release\$DriverName.inf /install"
        }
    } else {
        Write-Info "Skipping driver installation"
    }
    
    # Final Summary
    Write-Header "Setup Complete!"
    
    Write-Success "NanaBox development environment setup finished!"
    Write-Host ""
    Write-Info "Next steps:"
    Write-Info "  1. Review driver source code in: $driverPath"
    Write-Info "  2. Modify driver as needed for your use case"
    Write-Info "  3. Rebuild using: msbuild /p:Configuration=Release /p:Platform=x64"
    Write-Info "  4. Test driver functionality"
    Write-Host ""
    Write-Info "Documentation:"
    Write-Info "  - Driver README: $repoRoot\drivers\README.md"
    Write-Info "  - Anti-Detection Docs: $repoRoot\docs\anti-detection-overview.md"
    Write-Host ""
    Write-Warning "REMINDER: This is for development/testing only!"
    Write-Warning "Production deployment requires proper EV code signing."
    Write-Host ""
}

# ============================================================================
# Script Entry Point
# ============================================================================

try {
    Start-SetupWizard
    exit 0
} catch {
    Write-Error "Fatal error: $_"
    Write-Error $_.Exception.Message
    Write-Error $_.ScriptStackTrace
    exit 1
}
