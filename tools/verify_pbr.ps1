param(
    [string]$Configuration = "Debug",
    [string]$Platform = "x64",
    [switch]$SkipBuild,
    [switch]$DiscardCaptures,
    [switch]$NoLinkDebugInfo,
    [string[]]$Modes = @()
)

$ErrorActionPreference = "Stop"

$scriptPath = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Resolve-Path (Join-Path $scriptPath "..")
$outDir = Join-Path $repoRoot "out"
$summaryPath = Join-Path $outDir "pbr_verification_summary.txt"

$allModes = @(
    [pscustomobject]@{ Name = "forward"; Argument = "--verify-pbr"; Capture = "out/pbr_verification.ppm" },
    [pscustomobject]@{ Name = "forward-no-atlas"; Argument = "--verify-pbr-no-atlas"; Capture = "out/pbr_verification.ppm" },
    [pscustomobject]@{ Name = "ibl-debug"; Argument = "--verify-pbr-ibl-debug"; Capture = "out/pbr_ibl_debug_verification.ppm" },
    [pscustomobject]@{ Name = "gbuffer"; Argument = "--verify-pbr-gbuffer"; Capture = "out/pbr_gbuffer_verification.ppm" },
    [pscustomobject]@{ Name = "gbuffer-debug"; Argument = "--verify-pbr-gbuffer-debug"; Capture = "out/pbr_gbuffer_debug_verification.ppm" },
    [pscustomobject]@{ Name = "deferred"; Argument = "--verify-pbr-deferred"; Capture = "out/pbr_deferred_verification.ppm" },
    [pscustomobject]@{ Name = "deferred-no-atlas"; Argument = "--verify-pbr-deferred-no-atlas"; Capture = "out/pbr_deferred_no_atlas_verification.ppm" },
    [pscustomobject]@{ Name = "deferred-transparent"; Argument = "--verify-pbr-deferred-transparent"; Capture = "out/pbr_deferred_transparent_verification.ppm" },
    [pscustomobject]@{ Name = "deferred-emissive"; Argument = "--verify-pbr-deferred-emissive"; Capture = "out/pbr_deferred_emissive_verification.ppm" },
    [pscustomobject]@{ Name = "deferred-material-ibl"; Argument = "--verify-pbr-deferred-material-ibl"; Capture = "out/pbr_deferred_material_ibl_verification.ppm" },
    [pscustomobject]@{ Name = "deferred-alpha-mask"; Argument = "--verify-pbr-deferred-alpha-mask"; Capture = "out/pbr_deferred_alpha_mask_verification.ppm" },
    [pscustomobject]@{ Name = "deferred-tiled-lights"; Argument = "--verify-pbr-deferred-tiled-lights"; Capture = "out/pbr_deferred_tiled_lights_verification.ppm"; ExpectTiledCulling = $true },
    [pscustomobject]@{ Name = "deferred-tiled-heatmap"; Argument = "--verify-pbr-deferred-tiled-heatmap"; Capture = "out/pbr_deferred_tiled_heatmap_verification.ppm"; ExpectTiledCulling = $true; ExpectTiledHeatmap = $true },
    [pscustomobject]@{ Name = "import"; Argument = "--verify-pbr-import"; Capture = "out/pbr_import_verification.ppm" }
)

function Read-PpmToken {
    param(
        [byte[]]$Bytes,
        [ref]$Index
    )

    while ($Index.Value -lt $Bytes.Length) {
        $current = $Bytes[$Index.Value]
        if ($current -eq 35) {
            while ($Index.Value -lt $Bytes.Length -and $Bytes[$Index.Value] -ne 10) {
                $Index.Value++
            }
        }
        elseif ($current -eq 9 -or $current -eq 10 -or $current -eq 13 -or $current -eq 32) {
            $Index.Value++
        }
        else {
            break
        }
    }

    $builder = New-Object System.Text.StringBuilder
    while ($Index.Value -lt $Bytes.Length) {
        $current = $Bytes[$Index.Value]
        if ($current -eq 9 -or $current -eq 10 -or $current -eq 13 -or $current -eq 32) {
            $Index.Value++
            break
        }

        [void]$builder.Append([char]$current)
        $Index.Value++
    }

    return $builder.ToString()
}

function Get-PpmStats {
    param([string]$Path)

    if (!(Test-Path $Path)) {
        return [pscustomobject]@{
            Path = $Path
            Exists = $false
            Valid = $false
            Error = "missing"
        }
    }

    $bytes = [System.IO.File]::ReadAllBytes($Path)
    $index = 0
    $magic = Read-PpmToken -Bytes $bytes -Index ([ref]$index)
    $widthToken = Read-PpmToken -Bytes $bytes -Index ([ref]$index)
    $heightToken = Read-PpmToken -Bytes $bytes -Index ([ref]$index)
    $maxToken = Read-PpmToken -Bytes $bytes -Index ([ref]$index)

    if ($magic -ne "P6") {
        return [pscustomobject]@{
            Path = $Path
            Exists = $true
            Valid = $false
            Error = "not P6 ppm"
            Bytes = $bytes.Length
        }
    }

    $width = [int]$widthToken
    $height = [int]$heightToken
    $maxValue = [int]$maxToken
    $pixelCount = $width * $height
    $expectedDataBytes = $pixelCount * 3
    if ($maxValue -ne 255 -or $bytes.Length -lt ($index + $expectedDataBytes)) {
        return [pscustomobject]@{
            Path = $Path
            Exists = $true
            Valid = $false
            Error = "invalid ppm payload"
            Bytes = $bytes.Length
            Width = $width
            Height = $height
        }
    }

    [double]$redSum = 0.0
    [double]$greenSum = 0.0
    [double]$blueSum = 0.0
    [int]$nonBlack = 0
    $end = $index + $expectedDataBytes
    for ($offset = $index; $offset -lt $end; $offset += 3) {
        $red = $bytes[$offset]
        $green = $bytes[$offset + 1]
        $blue = $bytes[$offset + 2]
        $redSum += $red
        $greenSum += $green
        $blueSum += $blue
        if ($red -ne 0 -or $green -ne 0 -or $blue -ne 0) {
            $nonBlack++
        }
    }

    return [pscustomobject]@{
        Path = $Path
        Exists = $true
        Valid = $true
        Bytes = $bytes.Length
        Width = $width
        Height = $height
        NonBlackPercent = [Math]::Round(($nonBlack * 100.0) / $pixelCount, 4)
        MeanRed = [Math]::Round($redSum / $pixelCount, 2)
        MeanGreen = [Math]::Round($greenSum / $pixelCount, 2)
        MeanBlue = [Math]::Round($blueSum / $pixelCount, 2)
    }
}

function Get-RegexValue {
    param(
        [string]$Text,
        [string]$Pattern,
        [int]$Group = 1
    )

    if ($Text -match $Pattern) {
        return $Matches[$Group]
    }

    return $null
}

function Invoke-Build {
    $vsDevCmd = Join-Path ${env:ProgramFiles} "Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
    if (!(Test-Path $vsDevCmd)) {
        throw "VsDevCmd.bat was not found at $vsDevCmd"
    }

    $noDebugInfoArg = ""
    if ($NoLinkDebugInfo) {
        $noDebugInfoTarget = Join-Path $scriptPath "msbuild_no_link_debug.targets"
        if (!(Test-Path $noDebugInfoTarget)) {
            throw "No-link-debug-info MSBuild target was not found at $noDebugInfoTarget"
        }
        $noDebugInfoArg = " /p:ForceImportAfterCppTargets=`"$noDebugInfoTarget`""
    }

    $buildCommand = "`"$vsDevCmd`" -arch=x64 -host_arch=x64 >nul && msbuild text2.sln /m:1 /p:Configuration=$Configuration /p:Platform=$Platform /p:LinkIncremental=false$noDebugInfoArg /v:minimal"
    Push-Location $repoRoot
    try {
        & cmd.exe /d /s /c $buildCommand
        if ($LASTEXITCODE -ne 0) {
            throw "MSBuild failed with exit code $LASTEXITCODE"
        }
    }
    finally {
        Pop-Location
    }
}

function Get-ExecutablePath {
    if ($Platform -eq "x64") {
        return Join-Path $repoRoot "x64\$Configuration\text2.exe"
    }

    return Join-Path $repoRoot "$Configuration\text2.exe"
}

if (!(Test-Path $outDir)) {
    New-Item -ItemType Directory -Path $outDir | Out-Null
}

$selectedModes = $allModes
if ($Modes.Count -gt 0) {
    $normalizedModes = @(
        foreach ($mode in $Modes) {
            $mode -split "," | ForEach-Object { $_.Trim() } | Where-Object { $_.Length -gt 0 }
        }
    )
    $requested = @{}
    foreach ($mode in $normalizedModes) {
        $requested[$mode] = $true
    }

    $selectedModes = @($allModes | Where-Object { $requested.ContainsKey($_.Name) -or $requested.ContainsKey($_.Argument) })
    if ($selectedModes.Count -ne $normalizedModes.Count) {
        $available = ($allModes | ForEach-Object { $_.Name }) -join ", "
        throw "Unknown mode. Available modes: $available"
    }
}

$summaryLines = New-Object System.Collections.Generic.List[string]
$summaryLines.Add("PBR verification summary")
$summaryLines.Add("Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')")
$summaryLines.Add("Repository: $repoRoot")
$summaryLines.Add("Configuration: $Configuration")
$summaryLines.Add("Platform: $Platform")
$summaryLines.Add("DiscardCaptures: $($DiscardCaptures.IsPresent)")
$summaryLines.Add("NoLinkDebugInfo: $($NoLinkDebugInfo.IsPresent)")
$summaryLines.Add("")

if (!$SkipBuild) {
    Write-Host "Building $Configuration|$Platform..."
    Invoke-Build
    $summaryLines.Add("Build: passed")
}
else {
    $summaryLines.Add("Build: skipped")
}

$exePath = Get-ExecutablePath
if (!(Test-Path $exePath)) {
    throw "Executable was not found: $exePath"
}

$failures = New-Object System.Collections.Generic.List[string]
$results = New-Object System.Collections.Generic.List[object]

foreach ($mode in $selectedModes) {
    $logPath = Join-Path $outDir ("pbr_verify_{0}.log" -f $mode.Name)
    $capturePath = Join-Path $repoRoot $mode.Capture
    Write-Host "Running $($mode.Argument)..."

    Push-Location $repoRoot
    try {
        $oldErrorActionPreference = $ErrorActionPreference
        $ErrorActionPreference = "Continue"
        try {
            $output = & $exePath $mode.Argument 2>&1 | ForEach-Object { $_.ToString() }
            $exitCode = $LASTEXITCODE
        }
        finally {
            $ErrorActionPreference = $oldErrorActionPreference
        }
    }
    finally {
        Pop-Location
    }

    $output | Set-Content -Path $logPath -Encoding UTF8
    $stats = Get-PpmStats -Path $capturePath
    if ($DiscardCaptures -and (Test-Path $capturePath)) {
        Remove-Item -Path $capturePath -Force
    }
    $rendererLine = ($output | Where-Object { $_ -match "PBR verification renderer stats:" } | Select-Object -Last 1)
    $sceneLine = ($output | Where-Object { $_ -match "PBR verification scene stats:" } | Select-Object -Last 1)

    $result = [pscustomobject]@{
        Name = $mode.Name
        Argument = $mode.Argument
        ExitCode = $exitCode
        Capture = $mode.Capture
        Log = ("out/" + [System.IO.Path]::GetFileName($logPath))
        Ppm = $stats
        SceneStats = $sceneLine
        RendererStats = $rendererLine
    }
    $results.Add($result)

    if ($exitCode -ne 0) {
        $failures.Add("$($mode.Name): process exited with $exitCode")
    }
    if (!$stats.Valid) {
        $failures.Add("$($mode.Name): capture invalid ($($stats.Error))")
    }
    if ($mode.PSObject.Properties.Name -contains "ExpectTiledCulling" -and $mode.ExpectTiledCulling) {
        if (!$rendererLine) {
            $failures.Add("$($mode.Name): missing renderer stats")
        }
        elseif ($rendererLine -notmatch "pbrDeferredTiledLightGridBound=yes") {
            $failures.Add("$($mode.Name): tiled light grid was not bound")
        }
        else {
            $columns = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredTiledLightGridSize=(\d+)x(\d+)" -Group 1
            $rows = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredTiledLightGridSize=(\d+)x(\d+)" -Group 2
            $indices = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredTiledLightGridIndices=(\d+)" -Group 1
            $occupiedTiles = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredTiledLightGridOccupiedTiles=(\d+)/(\d+)" -Group 1
            $reportedTileCount = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredTiledLightGridOccupiedTiles=(\d+)/(\d+)" -Group 2
            $pointLights = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredLightBufferPointLights=(\d+)/" -Group 1
            if ($null -eq $pointLights -or [int]$pointLights -le 0) {
                $pointLights = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredTiledLightGridMaxTileLights=(\d+)" -Group 1
            }
            if ($null -eq $columns -or $null -eq $rows -or $null -eq $indices -or $null -eq $occupiedTiles -or $null -eq $reportedTileCount -or $null -eq $pointLights) {
                $failures.Add("$($mode.Name): tiled culling stats were incomplete")
            }
            else {
                $fullGlobalLoopIndexCount = [int]$columns * [int]$rows * [int]$pointLights
                $tileCount = [int]$columns * [int]$rows
                if ([int]$indices -le 0) {
                    $failures.Add("$($mode.Name): tiled light grid produced no light indices")
                }
                if ([int]$indices -ge $fullGlobalLoopIndexCount) {
                    $failures.Add("$($mode.Name): tiled light grid did not reduce the global point-light loop ($indices >= $fullGlobalLoopIndexCount)")
                }
                if ([int]$reportedTileCount -ne $tileCount) {
                    $failures.Add("$($mode.Name): reported tile count did not match grid dimensions ($reportedTileCount != $tileCount)")
                }
                if ([int]$occupiedTiles -le 0) {
                    $failures.Add("$($mode.Name): tiled light grid reported no occupied tiles")
                }
                if ([int]$occupiedTiles -ge $tileCount) {
                    $failures.Add("$($mode.Name): tiled light grid occupied every tile ($occupiedTiles >= $tileCount)")
                }
            }
        }
    }
    if ($mode.PSObject.Properties.Name -contains "ExpectTiledHeatmap" -and $mode.ExpectTiledHeatmap) {
        if (!$rendererLine) {
            $failures.Add("$($mode.Name): missing renderer stats")
        }
        elseif ($rendererLine -notmatch "pbrDeferredTiledLightDebugDrawCalls=1") {
            $failures.Add("$($mode.Name): tiled light heatmap debug pass did not draw")
        }
    }
}

$summaryLines.Add("Modes:")
foreach ($result in $results) {
    $stats = $result.Ppm
    if ($stats.Valid) {
        $summaryLines.Add(("- {0}: exit={1}, capture={2}, size={3}x{4}, bytes={5}, nonblack={6}%, meanRGB={7}/{8}/{9}, log={10}" -f `
            $result.Name,
            $result.ExitCode,
            $result.Capture,
            $stats.Width,
            $stats.Height,
            $stats.Bytes,
            $stats.NonBlackPercent,
            $stats.MeanRed,
            $stats.MeanGreen,
            $stats.MeanBlue,
            $result.Log))
    }
    else {
        $summaryLines.Add(("- {0}: exit={1}, capture={2}, invalid={3}, log={4}" -f `
            $result.Name,
            $result.ExitCode,
            $result.Capture,
            $stats.Error,
            $result.Log))
    }

    if ($result.SceneStats) {
        $summaryLines.Add("  scene: $($result.SceneStats)")
    }
    if ($result.RendererStats) {
        $summaryLines.Add("  renderer: $($result.RendererStats)")
    }
}

$summaryLines.Add("")
if ($failures.Count -gt 0) {
    $summaryLines.Add("Result: failed")
    foreach ($failure in $failures) {
        $summaryLines.Add("- $failure")
    }
}
else {
    $summaryLines.Add("Result: passed")
}

$summaryLines | Set-Content -Path $summaryPath -Encoding UTF8
$summaryLines | ForEach-Object { Write-Host $_ }

if ($failures.Count -gt 0) {
    exit 1
}

exit 0
