<#
.SYNOPSIS
    Builds Unfocused Hotkeys for one or more installed engines and zips each build into dist/.

.EXAMPLE
    .\Scripts\Package.ps1

.EXAMPLE
    .\Scripts\Package.ps1 -EngineDir "C:\Program Files\Epic Games\UE_5.7", "C:\Program Files\Epic Games\UE_5.8"
#>
param(
    [string[]]$EngineDir = @("C:\Program Files\Epic Games\UE_5.8")
)

$ErrorActionPreference = "Stop"

$PluginName = "UnfocusedHotkeys"
$RepoRoot = Split-Path -Parent $PSScriptRoot
$PluginFile = Join-Path $RepoRoot "$PluginName.uplugin"
$DistDir = Join-Path $RepoRoot "dist"
$PluginVersion = (Get-Content $PluginFile -Raw | ConvertFrom-Json).VersionName

New-Item -ItemType Directory -Force $DistDir | Out-Null

foreach ($Engine in $EngineDir) {
    $RunUAT = Join-Path $Engine "Engine\Build\BatchFiles\RunUAT.bat"
    if (-not (Test-Path $RunUAT)) { throw "RunUAT.bat not found under '$Engine'." }

    $Build = Get-Content (Join-Path $Engine "Engine\Build\Build.version") -Raw | ConvertFrom-Json
    $EngineVersion = "$($Build.MajorVersion).$($Build.MinorVersion)"

    # UAT won't package into a folder inside the plugin, so stage in %TEMP%.
    $StageDir = Join-Path $env:TEMP "$PluginName-UE$EngineVersion"
    $PackageDir = Join-Path $StageDir $PluginName
    if (Test-Path $StageDir) { Remove-Item -Recurse -Force $StageDir }

    Write-Host "Building $PluginName $PluginVersion for UE $EngineVersion..." -ForegroundColor Cyan
    & $RunUAT BuildPlugin "-Plugin=$PluginFile" "-Package=$PackageDir" -TargetPlatforms=Win64
    if ($LASTEXITCODE -ne 0) { throw "BuildPlugin failed for UE $EngineVersion (exit code $LASTEXITCODE)." }

    # Stamp the engine version so other engine versions show a clear "built for 5.x" prompt.
    $PackagedPluginFile = Join-Path $PackageDir "$PluginName.uplugin"
    $Descriptor = Get-Content $PackagedPluginFile -Raw | ConvertFrom-Json
    $Descriptor | Add-Member -NotePropertyName EngineVersion -NotePropertyValue "$EngineVersion.0" -Force
    $Descriptor | ConvertTo-Json -Depth 10 | Set-Content $PackagedPluginFile -Encoding utf8

    # Debug symbols are ~60 MB and users don't need them.
    Get-ChildItem (Join-Path $PackageDir "Binaries") -Recurse -Filter *.pdb | Remove-Item -Force

    $ZipPath = Join-Path $DistDir "$PluginName-$PluginVersion-UE$EngineVersion-Win64.zip"
    if (Test-Path $ZipPath) { Remove-Item $ZipPath }
    Compress-Archive -Path $PackageDir -DestinationPath $ZipPath
    Remove-Item -Recurse -Force $StageDir

    Write-Host "Packaged $ZipPath" -ForegroundColor Green
}
