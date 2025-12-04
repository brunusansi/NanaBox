<#
.SYNOPSIS
    Clone a NanaBox profile with new unique identifiers.

.DESCRIPTION
    Creates a copy of an existing profile with new SMBIOS serial numbers, UUID, and MAC address.
    Useful for creating multiple accounts or testing different configurations.

.PARAMETER SourceProfile
    Name of the source profile to clone.

.PARAMETER TargetProfile
    Name for the new profile.

.PARAMETER ProfileRoot
    Root directory for profiles. Defaults to $env:LOCALAPPDATA\NanaBox\profiles

.PARAMETER PreserveStorage
    If specified, does not create new storage (useful for template-based profiles).

.EXAMPLE
    .\Copy-NanaBoxProfile.ps1 -SourceProfile "valorant-main" -TargetProfile "valorant-alt"

.EXAMPLE
    .\Copy-NanaBoxProfile.ps1 -SourceProfile "template-gaming" -TargetProfile "pubg-account1" -PreserveStorage

.NOTES
    Phase: 1
    This creates a configuration copy with new identifiers. Storage (VHD/VHDX) handling
    will be added in Phase 2 with differencing disk support.
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)]
    [ValidateNotNullOrEmpty()]
    [string]$SourceProfile,
    
    [Parameter(Mandatory=$true)]
    [ValidateNotNullOrEmpty()]
    [string]$TargetProfile,
    
    [Parameter(Mandatory=$false)]
    [string]$ProfileRoot = "$env:LOCALAPPDATA\NanaBox\profiles",
    
    [Parameter(Mandatory=$false)]
    [switch]$PreserveStorage
)

begin {
    Write-Verbose "Initializing profile cloning: $SourceProfile -> $TargetProfile"
    
    # Helper function to generate random serial number
    function New-RandomSerial {
        param([string]$Pattern = "XXXXXXXXXX")
        
        $chars = "0123456789ABCDEFGHJKLMNPQRSTUVWXYZ"
        $result = ""
        
        foreach ($char in $Pattern.ToCharArray()) {
            if ($char -eq 'X') {
                $result += $chars[(Get-Random -Maximum $chars.Length)]
            } else {
                $result += $char
            }
        }
        
        return $result
    }
}

process {
    try {
        $sourcePath = Join-Path $ProfileRoot $SourceProfile
        $targetPath = Join-Path $ProfileRoot $TargetProfile
        
        # Validate source
        if (-not (Test-Path $sourcePath)) {
            throw "Source profile not found: $sourcePath"
        }
        
        # Check target doesn't exist
        if (Test-Path $targetPath) {
            throw "Target profile already exists: $targetPath"
        }
        
        Write-Host "Cloning profile: $SourceProfile -> $TargetProfile" -ForegroundColor Cyan
        
        # Create target directory
        New-Item -Path $targetPath -ItemType Directory -Force | Out-Null
        Write-Verbose "Created directory: $targetPath"
        
        # Load source configuration
        $configPath = Join-Path $sourcePath "config.json"
        if (-not (Test-Path $configPath)) {
            throw "Source configuration not found: $configPath"
        }
        
        $config = Get-Content $configPath -Raw | ConvertFrom-Json
        
        # Generate new unique identifiers
        Write-Host "Generating new unique identifiers..." -ForegroundColor Yellow
        
        # Generate new serial number (preserve format from source if possible)
        $oldSerial = $config.NanaBox.ChipsetInformation.SerialNumber
        $newSerial = if ($oldSerial -and $oldSerial.Length -gt 0) {
            # Try to preserve pattern
            $pattern = $oldSerial -replace '[0-9A-Z]', 'X'
            New-RandomSerial -Pattern $pattern
        } else {
            "SN-" + (New-RandomSerial -Pattern "XXXXXXXXXX")
        }
        
        # Generate new UUID
        $newUUID = [guid]::NewGuid().ToString()
        
        # Generate new MAC address (preserve vendor if possible)
        $vendorPrefix = "D8-9E-F3"  # Default to ASUS
        if ($config.NanaBox.NetworkAdapters -and $config.NanaBox.NetworkAdapters.Count -gt 0) {
            $oldMAC = $config.NanaBox.NetworkAdapters[0].MacAddress
            if ($oldMAC -and $oldMAC.Length -ge 8) {
                $vendorPrefix = $oldMAC.Substring(0, 8)
            }
        }
        
        # Use New-RandomMAC.ps1 if available
        $scriptPath = Join-Path $PSScriptRoot "New-RandomMAC.ps1"
        if (Test-Path $scriptPath) {
            $newMAC = & $scriptPath -VendorPrefix $vendorPrefix
        } else {
            # Fallback: generate inline
            $r1 = "{0:X2}" -f (Get-Random -Maximum 256)
            $r2 = "{0:X2}" -f (Get-Random -Maximum 256)
            $r3 = "{0:X2}" -f (Get-Random -Maximum 256)
            $newMAC = "$vendorPrefix-$r1-$r2-$r3"
        }
        
        # Generate new baseboard serial
        $newBaseBoardSerial = if ($config.NanaBox.ChipsetInformation.BaseBoardSerialNumber) {
            "/" + $newSerial + "/CNFCP0013O0" + (New-RandomSerial -Pattern "XXX") + "/"
        } else {
            ""
        }
        
        # Update configuration with new identifiers
        Write-Host "Updating configuration..." -ForegroundColor Yellow
        
        # Update metadata
        $config.NanaBox.Metadata.AccountId = $TargetProfile
        $config.NanaBox.Metadata.CreationTimestamp = (Get-Date).ToUniversalTime().ToString("o")
        $config.NanaBox.Metadata.LastUpdatedTimestamp = (Get-Date).ToUniversalTime().ToString("o")
        
        # Update basic info
        $config.NanaBox.Name = $TargetProfile
        
        # Update SMBIOS
        $config.NanaBox.ChipsetInformation.SerialNumber = $newSerial
        $config.NanaBox.ChipsetInformation.UUID = $newUUID
        if ($newBaseBoardSerial) {
            $config.NanaBox.ChipsetInformation.BaseBoardSerialNumber = $newBaseBoardSerial
        }
        $config.NanaBox.ChipsetInformation.ChassisSerialNumber = $newSerial
        
        # Update network
        if ($config.NanaBox.NetworkAdapters -and $config.NanaBox.NetworkAdapters.Count -gt 0) {
            $config.NanaBox.NetworkAdapters[0].MacAddress = $newMAC
        } else {
            Write-Warning "No network adapters found in source configuration"
        }
        
        # Update storage paths
        if (-not $PreserveStorage) {
            $config.NanaBox.GuestStateFile = "$TargetProfile.vmgs"
            $config.NanaBox.RuntimeStateFile = "$TargetProfile.vmrs"
            if ($config.NanaBox.SaveStateFile) {
                $config.NanaBox.SaveStateFile = "$TargetProfile.SaveState.vmrs"
            }
        }
        
        # Save configuration
        $targetConfigPath = Join-Path $targetPath "config.json"
        $config | ConvertTo-Json -Depth 10 | Out-File -FilePath $targetConfigPath -Encoding UTF8
        
        Write-Host ""
        Write-Host "✓ Profile cloned successfully!" -ForegroundColor Green
        Write-Host "  Target Profile: $TargetProfile"
        Write-Host "  Location: $targetPath"
        Write-Host ""
        Write-Host "New Identifiers:" -ForegroundColor Cyan
        Write-Host "  Serial Number: $newSerial"
        Write-Host "  UUID: $newUUID"
        Write-Host "  MAC Address: $newMAC"
        Write-Host ""
        
        if (-not $PreserveStorage) {
            Write-Warning "Storage files not created. You need to:"
            Write-Warning "  1. Create/copy VHD/VHDX file"
            Write-Warning "  2. Create guest state file (.vmgs)"
            Write-Warning "  Or use differencing disks (see New-DifferencingDisk.ps1)"
        }
    }
    catch {
        Write-Error "Failed to clone profile: $_"
        
        # Cleanup on error
        if (Test-Path $targetPath) {
            Write-Verbose "Cleaning up failed clone..."
            Remove-Item -Path $targetPath -Recurse -Force -ErrorAction SilentlyContinue
        }
        
        throw
    }
}

end {
    Write-Verbose "Profile cloning completed."
}
