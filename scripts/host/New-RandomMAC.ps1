<#
.SYNOPSIS
    Generate a random MAC address with realistic vendor prefix.

.DESCRIPTION
    Creates a MAC address using common NIC vendor prefixes for anti-detection purposes.
    Supports multiple vendor formats and validates output.

.PARAMETER VendorPrefix
    Vendor prefix (XX-XX-XX format). Common values:
    - D8-9E-F3 (ASUS)
    - AC-DE-48 (Gigabyte)  
    - 00-D8-61 (MSI)
    - 00-15-5D (Microsoft/Intel)
    - 54-BF-64 (ASRock)
    - E4-5F-01 (Realtek)

.PARAMETER Format
    Output format: Hyphen (XX-XX-XX-XX-XX-XX) or Colon (XX:XX:XX:XX:XX:XX)

.PARAMETER Count
    Number of MAC addresses to generate.

.EXAMPLE
    .\New-RandomMAC.ps1 -VendorPrefix "D8-9E-F3"
    
    Generates: D8-9E-F3-AB-CD-EF

.EXAMPLE
    .\New-RandomMAC.ps1 -VendorPrefix "AC:DE:48" -Format Colon -Count 5

.NOTES
    Phase: 1
    For multi-account isolation, ensure each VM gets a unique MAC address.
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory=$false, HelpMessage="Vendor prefix (XX-XX-XX or XX:XX:XX format)")]
    [ValidatePattern('^([0-9A-F]{2}[-:]){2}[0-9A-F]{2}$')]
    [string]$VendorPrefix = "D8-9E-F3",
    
    [Parameter(Mandatory=$false)]
    [ValidateSet("Hyphen", "Colon")]
    [string]$Format = "Hyphen",
    
    [Parameter(Mandatory=$false)]
    [ValidateRange(1, 100)]
    [int]$Count = 1
)

begin {
    # Normalize vendor prefix format
    $VendorPrefix = $VendorPrefix.Replace(":", "-").ToUpper()
    
    # Common vendor prefixes with descriptions
    $knownVendors = @{
        "D8-9E-F3" = "ASUS"
        "AC-DE-48" = "Gigabyte"
        "00-D8-61" = "MSI"
        "00-15-5D" = "Microsoft/Intel"
        "54-BF-64" = "ASRock"
        "E4-5F-01" = "Realtek"
        "00-E0-4C" = "Realtek"
        "00-1B-21" = "Intel"
        "B8-27-EB" = "Raspberry Pi Foundation"
    }
    
    $vendorName = if ($knownVendors.ContainsKey($VendorPrefix)) {
        $knownVendors[$VendorPrefix]
    } else {
        "Unknown"
    }
    
    Write-Verbose "Generating MAC address(es) with vendor: $vendorName ($VendorPrefix)"
}

process {
    $macAddresses = @()
    
    for ($i = 0; $i -lt $Count; $i++) {
        # Generate random bytes for the last 3 octets
        $random1 = "{0:X2}" -f (Get-Random -Minimum 0 -Maximum 256)
        $random2 = "{0:X2}" -f (Get-Random -Minimum 0 -Maximum 256)
        $random3 = "{0:X2}" -f (Get-Random -Minimum 0 -Maximum 256)
        
        # Build MAC address
        $macAddress = "$VendorPrefix-$random1-$random2-$random3"
        
        # Convert format if needed
        if ($Format -eq "Colon") {
            $macAddress = $macAddress.Replace("-", ":")
        }
        
        $macAddresses += $macAddress
    }
    
    # Output results
    if ($Count -eq 1) {
        Write-Output $macAddresses[0]
    } else {
        Write-Output $macAddresses
    }
    
    # Verbose output
    if ($Count -le 10) {
        foreach ($mac in $macAddresses) {
            Write-Verbose "Generated: $mac (Vendor: $vendorName)"
        }
    } else {
        Write-Verbose "Generated $Count MAC addresses with vendor prefix: $VendorPrefix"
    }
}

<#
.LINK
    https://standards-oui.ieee.org/ - IEEE OUI Database
#>
