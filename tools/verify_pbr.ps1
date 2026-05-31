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
    [pscustomobject]@{ Name = "forward"; Argument = "--verify-pbr"; Capture = "out/pbr_verification.ppm"; ExpectLegacyWorldMirror = $true },
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
    [pscustomobject]@{ Name = "deferred-untiled-lights"; Argument = "--verify-pbr-deferred-untiled-lights"; Capture = "out/pbr_deferred_untiled_lights_verification.ppm"; ExpectUntiledFallback = $true },
    [pscustomobject]@{ Name = "deferred-tiled-lights"; Argument = "--verify-pbr-deferred-tiled-lights"; Capture = "out/pbr_deferred_tiled_lights_verification.ppm"; ExpectTiledCulling = $true },
    [pscustomobject]@{ Name = "deferred-tiled-lights-pressure"; Argument = "--verify-pbr-deferred-tiled-lights-pressure"; Capture = "out/pbr_deferred_tiled_lights_pressure_verification.ppm"; ExpectTiledCulling = $true; ExpectPointLightPressure = 8 },
    [pscustomobject]@{ Name = "deferred-tiled-lights-pressure-timing"; Argument = "--verify-pbr-deferred-tiled-lights-pressure-timing"; Capture = "out/pbr_deferred_tiled_lights_pressure_timing_verification.ppm"; ExpectTiledCulling = $true; ExpectPointLightPressure = 8; ExpectGpuTiming = $true },
    [pscustomobject]@{ Name = "deferred-tiled-lights-32"; Argument = "--verify-pbr-deferred-tiled-lights-32"; Capture = "out/pbr_deferred_tiled_lights_32_verification.ppm"; ExpectTiledCulling = $true; ExpectTileSize = 32 },
    [pscustomobject]@{ Name = "deferred-tiled-lights-cutoff-005"; Argument = "--verify-pbr-deferred-tiled-lights-cutoff-005"; Capture = "out/pbr_deferred_tiled_lights_cutoff_005_verification.ppm"; ExpectTiledCulling = $true; ExpectLightCutoff = 0.05 },
    [pscustomobject]@{ Name = "deferred-tiled-heatmap"; Argument = "--verify-pbr-deferred-tiled-heatmap"; Capture = "out/pbr_deferred_tiled_heatmap_verification.ppm"; ExpectTiledCulling = $true; ExpectTiledHeatmap = $true },
    [pscustomobject]@{ Name = "deferred-clustered-heatmap"; Argument = "--verify-pbr-deferred-clustered-heatmap"; Capture = "out/pbr_deferred_clustered_heatmap_verification.ppm"; ExpectClusteredHeatmap = $true },
    [pscustomobject]@{ Name = "deferred-clustered-layout"; Argument = "--verify-pbr-deferred-clustered-layout"; Capture = "out/pbr_deferred_clustered_layout_verification.ppm"; ExpectClusteredLayout = $true },
    [pscustomobject]@{ Name = "deferred-clustered-grid"; Argument = "--verify-pbr-deferred-clustered-grid"; Capture = "out/pbr_deferred_clustered_grid_verification.ppm"; ExpectClusteredGrid = $true },
    [pscustomobject]@{ Name = "deferred-clustered-grid-timing"; Argument = "--verify-pbr-deferred-clustered-grid-timing"; Capture = "out/pbr_deferred_clustered_grid_timing_verification.ppm"; ExpectClusteredGrid = $true; ExpectGpuTiming = $true },
    [pscustomobject]@{ Name = "deferred-clustered-grid-pressure"; Argument = "--verify-pbr-deferred-clustered-grid-pressure"; Capture = "out/pbr_deferred_clustered_grid_pressure_verification.ppm"; ExpectClusteredGrid = $true; ExpectPointLightPressure = 8 },
    [pscustomobject]@{ Name = "deferred-clustered-grid-pressure-timing"; Argument = "--verify-pbr-deferred-clustered-grid-pressure-timing"; Capture = "out/pbr_deferred_clustered_grid_pressure_timing_verification.ppm"; ExpectClusteredGrid = $true; ExpectPointLightPressure = 8; ExpectGpuTiming = $true },
    [pscustomobject]@{ Name = "deferred-clustered-grid-no-readback"; Argument = "--verify-pbr-deferred-clustered-grid-no-readback"; Capture = "out/pbr_deferred_clustered_grid_no_readback_verification.ppm"; ExpectClusteredGridNoReadback = $true },
    [pscustomobject]@{ Name = "import"; Argument = "--verify-pbr-import"; Capture = "out/pbr_import_verification.ppm"; ExpectImportedAssetProbe = $true },
    [pscustomobject]@{ Name = "texture-set"; Argument = "--verify-pbr-texture-set"; Capture = "out/pbr_texture_set_verification.ppm"; ExpectTexturedProbe = $true },
    [pscustomobject]@{ Name = "deferred-texture-set"; Argument = "--verify-pbr-deferred-texture-set"; Capture = "out/pbr_deferred_texture_set_verification.ppm"; ExpectTexturedProbe = $true; ExpectDeferredLighting = $true },
    [pscustomobject]@{ Name = "showcase-spheres"; Argument = "--verify-pbr-showcase-spheres"; Capture = "out/pbr_showcase_spheres_verification.ppm"; ExpectShowcaseSpheres = 6; ExpectDeferredLighting = $true; ExpectTiledCulling = $true; ExpectPointLightPressure = 8 },
    [pscustomobject]@{ Name = "engine-world-scene-probe"; Argument = "--verify-engine-world-scene-probe"; Capture = "out/engine_world_scene_probe_verification.ppm"; ExpectEngineWorldSceneProbe = $true; ExpectEngineWorldTransformSnapshot = $true; ExpectDeferredLighting = $true },
    [pscustomobject]@{ Name = "engine-world-minimal-scene"; Argument = "--verify-engine-world-minimal-scene"; Capture = "out/engine_world_minimal_scene_verification.ppm"; ExpectEngineWorldMinimalScene = $true; ExpectEngineWorldTransformSnapshot = $true; ExpectDeferredLighting = $true },
    [pscustomobject]@{ Name = "engine-world-editor-create"; Argument = "--verify-engine-world-editor-create"; Capture = "out/engine_world_editor_create_verification.ppm"; ExpectEngineWorldMinimalScene = $true; ExpectEngineWorldEditorCreate = $true; ExpectEngineWorldTransformSnapshot = $true; ExpectDeferredLighting = $true },
    [pscustomobject]@{ Name = "engine-world-scene-package"; Argument = "--verify-engine-world-scene-package"; Capture = "out/engine_world_scene_package_verification.ppm"; ExpectEngineWorldMinimalScene = $true; ExpectEngineWorldScenePackage = $true; ExpectEngineWorldTransformSnapshot = $true; ExpectDeferredLighting = $true },
    [pscustomobject]@{ Name = "renderer-backend-registry-noop"; Argument = "--verify-renderer-backend-registry-noop"; Capture = "out/renderer_backend_registry_noop_verification.ppm"; ExpectNoOpRendererBackend = $true }
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
    $expectNoOpRendererBackend = $mode.PSObject.Properties.Name -contains "ExpectNoOpRendererBackend" -and $mode.ExpectNoOpRendererBackend
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
    $engineWorldPreparedSceneLine = ($output | Where-Object { $_ -match "Engine world prepared scene stats:" } | Select-Object -Last 1)
    $runtimeEngineLifecycleSnapshotLine = ($output | Where-Object { $_ -match "Runtime engine lifecycle snapshot stats:" } | Select-Object -Last 1)
    $runtimeEngineSubsystemSummaryLine = ($output | Where-Object { $_ -match "Runtime engine subsystem summary stats:" } | Select-Object -Last 1)
    $runtimeEngineTickLine = ($output | Where-Object { $_ -match "Runtime engine tick stats:" } | Select-Object -Last 1)
    $runtimeSubsystemHealthLine = ($output | Where-Object { $_ -match "Runtime subsystem health stats:" } | Select-Object -Last 1)
    $runtimeRendererSubsystemLine = ($output | Where-Object { $_ -match "Runtime renderer subsystem stats:" } | Select-Object -Last 1)
    $runtimeRendererBackendContractLine = ($output | Where-Object { $_ -match "Runtime renderer backend contract stats:" } | Select-Object -Last 1)
    $runtimeRendererSubsystemCleanupLine = ($output | Where-Object { $_ -match "Runtime renderer subsystem cleanup stats:" } | Select-Object -Last 1)
    $runtimeRendererBackendContractCleanupLine = ($output | Where-Object { $_ -match "Runtime renderer backend contract cleanup stats:" } | Select-Object -Last 1)
    $runtimeEngineWorldCleanupLine = ($output | Where-Object { $_ -match "Runtime engine world cleanup stats:" } | Select-Object -Last 1)
    $runtimeEngineSubsystemCleanupLine = ($output | Where-Object { $_ -match "Runtime engine subsystem cleanup stats:" } | Select-Object -Last 1)
    $legacyWorldMirrorLine = ($output | Where-Object { $_ -match "Engine world legacy mirror stats:" } | Select-Object -Last 1)
    $importedAssetWorldImportLine = ($output | Where-Object { $_ -match "PBR imported asset engine world import stats:" } | Select-Object -Last 1)
    $runtimeAssetRegistryLine = ($output | Where-Object { $_ -match "Runtime asset registry stats:" } | Select-Object -Last 1)
    $importedAssetScenePackageLine = ($output | Where-Object { $_ -match "PBR imported asset scene package stats:" } | Select-Object -Last 1)
    $engineWorldProbeLine = ($output | Where-Object { $_ -match "Engine world scene probe stats:" } | Select-Object -Last 1)
    $engineWorldMinimalLine = ($output | Where-Object { $_ -match "Engine world minimal scene stats:" } | Select-Object -Last 1)
    $engineWorldEditorCreateLine = ($output | Where-Object { $_ -match "Engine world editor create stats:" } | Select-Object -Last 1)
    $engineWorldScenePackageLine = ($output | Where-Object { $_ -match "Engine world scene package stats:" } | Select-Object -Last 1)
    $engineWorldScenePackageNegativeLine = ($output | Where-Object { $_ -match "Engine world scene package negative stats:" } | Select-Object -Last 1)
    $engineWorldSnapshotLine = ($output | Where-Object { $_ -match "Engine world transform snapshot stats:" } | Select-Object -Last 1)
    $engineWorldSnapshotApplyLine = ($output | Where-Object { $_ -match "Engine world transform snapshot apply stats:" } | Select-Object -Last 1)

    $result = [pscustomobject]@{
        Name = $mode.Name
        Argument = $mode.Argument
        ExitCode = $exitCode
        Capture = $mode.Capture
        Log = ("out/" + [System.IO.Path]::GetFileName($logPath))
        Ppm = $stats
        SceneStats = $sceneLine
        EngineWorldPreparedSceneStats = $engineWorldPreparedSceneLine
        RendererStats = $rendererLine
        RuntimeEngineLifecycleSnapshotStats = $runtimeEngineLifecycleSnapshotLine
        RuntimeEngineSubsystemSummaryStats = $runtimeEngineSubsystemSummaryLine
        RuntimeEngineTickStats = $runtimeEngineTickLine
        RuntimeSubsystemHealthStats = $runtimeSubsystemHealthLine
        RuntimeRendererSubsystemStats = $runtimeRendererSubsystemLine
        RuntimeRendererBackendContractStats = $runtimeRendererBackendContractLine
        RuntimeRendererSubsystemCleanupStats = $runtimeRendererSubsystemCleanupLine
        RuntimeRendererBackendContractCleanupStats = $runtimeRendererBackendContractCleanupLine
        RuntimeEngineWorldCleanupStats = $runtimeEngineWorldCleanupLine
        RuntimeEngineSubsystemCleanupStats = $runtimeEngineSubsystemCleanupLine
        LegacyWorldMirrorStats = $legacyWorldMirrorLine
        ImportedAssetWorldImportStats = $importedAssetWorldImportLine
        RuntimeAssetRegistryStats = $runtimeAssetRegistryLine
        ImportedAssetScenePackageStats = $importedAssetScenePackageLine
        EngineWorldProbeStats = $engineWorldProbeLine
        EngineWorldMinimalStats = $engineWorldMinimalLine
        EngineWorldEditorCreateStats = $engineWorldEditorCreateLine
        EngineWorldScenePackageStats = $engineWorldScenePackageLine
        EngineWorldScenePackageNegativeStats = $engineWorldScenePackageNegativeLine
        EngineWorldSnapshotStats = $engineWorldSnapshotLine
        EngineWorldSnapshotApplyStats = $engineWorldSnapshotApplyLine
    }
    $results.Add($result)

    if ($exitCode -ne 0) {
        $failures.Add("$($mode.Name): process exited with $exitCode")
    }
    if (!$stats.Valid) {
        $failures.Add("$($mode.Name): capture invalid ($($stats.Error))")
    }
    if (!$engineWorldPreparedSceneLine) {
        $failures.Add("$($mode.Name): missing engine world prepared scene stats")
    }
    if ($expectNoOpRendererBackend -and $stats.Valid -and [double]$stats.NonBlackPercent -lt 99.0) {
        $failures.Add("$($mode.Name): no-op renderer backend did not clear the full framebuffer")
    }
    if (!$runtimeEngineLifecycleSnapshotLine) {
        $failures.Add("$($mode.Name): missing runtime engine lifecycle snapshot stats")
    }
    else {
        $snapshotViewportWidth = Get-RegexValue -Text $runtimeEngineLifecycleSnapshotLine -Pattern "viewport=(\d+)x(\d+)" -Group 1
        $snapshotViewportHeight = Get-RegexValue -Text $runtimeEngineLifecycleSnapshotLine -Pattern "viewport=(\d+)x(\d+)" -Group 2
        $snapshotActiveWorldActors = Get-RegexValue -Text $runtimeEngineLifecycleSnapshotLine -Pattern "activeWorldActors=(\d+)" -Group 1
        $snapshotActiveWorldTicks = Get-RegexValue -Text $runtimeEngineLifecycleSnapshotLine -Pattern "activeWorldTicks=(\d+)" -Group 1
        $snapshotEngineTicks = Get-RegexValue -Text $runtimeEngineLifecycleSnapshotLine -Pattern "engineTicks=(\d+)" -Group 1
        $snapshotSubsystemCount = Get-RegexValue -Text $runtimeEngineLifecycleSnapshotLine -Pattern "subsystemCount=(\d+)" -Group 1
        $snapshotInitializedSubsystemCount = Get-RegexValue -Text $runtimeEngineLifecycleSnapshotLine -Pattern "initializedSubsystems=(\d+)" -Group 1
        $snapshotTickedSubsystemCount = Get-RegexValue -Text $runtimeEngineLifecycleSnapshotLine -Pattern "tickedSubsystems=(\d+)" -Group 1
        $snapshotTimeSeconds = Get-RegexValue -Text $runtimeEngineLifecycleSnapshotLine -Pattern "timeSeconds=([0-9.]+)" -Group 1
        $snapshotDeltaSeconds = Get-RegexValue -Text $runtimeEngineLifecycleSnapshotLine -Pattern "deltaSeconds=([0-9.]+)" -Group 1
        if ($runtimeEngineLifecycleSnapshotLine -notmatch "initialized=yes") {
            $failures.Add("$($mode.Name): Engine lifecycle snapshot was not initialized")
        }
        if ($runtimeEngineLifecycleSnapshotLine -notmatch "runMode=Verification") {
            $failures.Add("$($mode.Name): Engine lifecycle snapshot did not report Verification run mode")
        }
        if ($runtimeEngineLifecycleSnapshotLine -notmatch "activeWorld=yes") {
            $failures.Add("$($mode.Name): Engine lifecycle snapshot had no active World")
        }
        if ($runtimeEngineLifecycleSnapshotLine -notmatch "activeWorldPlaying=yes") {
            $failures.Add("$($mode.Name): Engine lifecycle snapshot active World was not playing")
        }
        if ($null -eq $snapshotViewportWidth -or [int]$snapshotViewportWidth -le 0) {
            $failures.Add("$($mode.Name): Engine lifecycle snapshot viewport width was not positive")
        }
        if ($null -eq $snapshotViewportHeight -or [int]$snapshotViewportHeight -le 0) {
            $failures.Add("$($mode.Name): Engine lifecycle snapshot viewport height was not positive")
        }
        if ($null -eq $snapshotEngineTicks -or [int]$snapshotEngineTicks -le 0) {
            $failures.Add("$($mode.Name): Engine lifecycle snapshot engine tick count did not advance")
        }
        if ($null -eq $snapshotActiveWorldTicks -or [int]$snapshotActiveWorldTicks -le 0) {
            $failures.Add("$($mode.Name): Engine lifecycle snapshot active World tick count did not advance")
        }
        if ($null -ne $snapshotEngineTicks -and $null -ne $snapshotActiveWorldTicks -and [int]$snapshotEngineTicks -ne [int]$snapshotActiveWorldTicks) {
            $failures.Add("$($mode.Name): Engine lifecycle snapshot Engine and active World tick counters diverged")
        }
        if ($null -eq $snapshotSubsystemCount -or [int]$snapshotSubsystemCount -lt 2) {
            $failures.Add("$($mode.Name): Engine lifecycle snapshot subsystem count was lower than expected")
        }
        if ($null -eq $snapshotInitializedSubsystemCount -or [int]$snapshotInitializedSubsystemCount -lt 2) {
            $failures.Add("$($mode.Name): Engine lifecycle snapshot initialized subsystem count was lower than expected")
        }
        if ($null -eq $snapshotTickedSubsystemCount -or [int]$snapshotTickedSubsystemCount -lt 2) {
            $failures.Add("$($mode.Name): Engine lifecycle snapshot ticked subsystem count was lower than expected")
        }
        if ($null -eq $snapshotTimeSeconds -or [double]$snapshotTimeSeconds -le 0.0) {
            $failures.Add("$($mode.Name): Engine lifecycle snapshot timeSeconds did not advance")
        }
        if ($null -eq $snapshotDeltaSeconds -or [double]$snapshotDeltaSeconds -le 0.0) {
            $failures.Add("$($mode.Name): Engine lifecycle snapshot deltaSeconds was not positive")
        }
        if ($engineWorldPreparedSceneLine) {
            $sceneRuntimeWorldActors = Get-RegexValue -Text $engineWorldPreparedSceneLine -Pattern "runtimeWorldActors=(\d+)" -Group 1
            if ($null -ne $sceneRuntimeWorldActors -and $null -ne $snapshotActiveWorldActors -and [int]$snapshotActiveWorldActors -lt [int]$sceneRuntimeWorldActors) {
                $failures.Add("$($mode.Name): Engine lifecycle snapshot activeWorldActors was lower than prepared-scene runtimeWorldActors")
            }
        }
    }
    if (!$runtimeEngineSubsystemSummaryLine) {
        $failures.Add("$($mode.Name): missing runtime engine subsystem summary stats")
    }
    else {
        $summarySubsystemCount = Get-RegexValue -Text $runtimeEngineSubsystemSummaryLine -Pattern "subsystems=(\d+)" -Group 1
        $summaryInitializedSubsystemCount = Get-RegexValue -Text $runtimeEngineSubsystemSummaryLine -Pattern "initializedSubsystems=(\d+)" -Group 1
        $summaryTickedSubsystemCount = Get-RegexValue -Text $runtimeEngineSubsystemSummaryLine -Pattern "tickedSubsystems=(\d+)" -Group 1
        $summaryTicks = Get-RegexValue -Text $runtimeEngineSubsystemSummaryLine -Pattern "ticks=([^,\s]+)" -Group 1
        if ($null -eq $summarySubsystemCount -or [int]$summarySubsystemCount -lt 2) {
            $failures.Add("$($mode.Name): Engine subsystem summary count was lower than expected")
        }
        if ($null -eq $summaryInitializedSubsystemCount -or [int]$summaryInitializedSubsystemCount -lt 2) {
            $failures.Add("$($mode.Name): Engine subsystem summary initialized count was lower than expected")
        }
        if ($null -eq $summaryTickedSubsystemCount -or [int]$summaryTickedSubsystemCount -lt 2) {
            $failures.Add("$($mode.Name): Engine subsystem summary ticked count was lower than expected")
        }
        if ($runtimeEngineSubsystemSummaryLine -notmatch "names=.*AssetSubsystem") {
            $failures.Add("$($mode.Name): Engine subsystem summary did not include AssetSubsystem")
        }
        if ($runtimeEngineSubsystemSummaryLine -notmatch "names=.*RendererSubsystem") {
            $failures.Add("$($mode.Name): Engine subsystem summary did not include RendererSubsystem")
        }
        if ($null -ne $summaryTicks) {
            foreach ($tickText in ($summaryTicks -split "\|")) {
                if ([int]$tickText -le 0) {
                    $failures.Add("$($mode.Name): Engine subsystem summary contained a non-positive runtime tick count")
                    break
                }
            }
        }
    }
    if (!$runtimeEngineTickLine) {
        $failures.Add("$($mode.Name): missing runtime engine tick stats")
    }
    else {
        $engineTimeSeconds = Get-RegexValue -Text $runtimeEngineTickLine -Pattern "timeSeconds=([0-9.]+)" -Group 1
        $engineDeltaSeconds = Get-RegexValue -Text $runtimeEngineTickLine -Pattern "deltaSeconds=([0-9.]+)" -Group 1
        if ($runtimeEngineTickLine -notmatch "initialized=yes") {
            $failures.Add("$($mode.Name): Engine was not initialized at rendered-frame report")
        }
        if ($runtimeEngineTickLine -notmatch "runtimeContextEngineAttached=yes") {
            $failures.Add("$($mode.Name): AppRuntimeContext did not expose the runtime Engine")
        }
        if ($runtimeEngineTickLine -notmatch "activeWorld=yes") {
            $failures.Add("$($mode.Name): Engine had no active World at rendered-frame report")
        }
        if ($runtimeEngineTickLine -notmatch "activeWorldPlaying=yes") {
            $failures.Add("$($mode.Name): Engine active World was not playing at rendered-frame report")
        }
        if ($null -eq $engineTimeSeconds -or [double]$engineTimeSeconds -le 0.0) {
            $failures.Add("$($mode.Name): Engine timeSeconds did not advance")
        }
        if ($null -eq $engineDeltaSeconds -or [double]$engineDeltaSeconds -le 0.0) {
            $failures.Add("$($mode.Name): Engine deltaSeconds was not positive")
        }
    }
    if (!$runtimeSubsystemHealthLine) {
        $failures.Add("$($mode.Name): missing runtime subsystem health stats")
    }
    else {
        $engineTicks = Get-RegexValue -Text $runtimeSubsystemHealthLine -Pattern "engineTicks=(\d+)" -Group 1
        $worldTicks = Get-RegexValue -Text $runtimeSubsystemHealthLine -Pattern "worldTicks=(\d+)" -Group 1
        $assetSubsystemTicks = Get-RegexValue -Text $runtimeSubsystemHealthLine -Pattern "assetSubsystemTicks=(\d+)" -Group 1
        $rendererSubsystemTicks = Get-RegexValue -Text $runtimeSubsystemHealthLine -Pattern "rendererSubsystemTicks=(\d+)" -Group 1
        $healthWorldActors = Get-RegexValue -Text $runtimeSubsystemHealthLine -Pattern "worldActors=(\d+)" -Group 1
        if ($runtimeSubsystemHealthLine -notmatch "contextWorldMatchesActive=yes") {
            $failures.Add("$($mode.Name): runtime context World did not match Engine active World")
        }
        if ($null -eq $engineTicks -or [int]$engineTicks -le 0) {
            $failures.Add("$($mode.Name): Engine tick counter did not advance")
        }
        if ($null -eq $worldTicks -or [int]$worldTicks -le 0) {
            $failures.Add("$($mode.Name): World tick counter did not advance")
        }
        if ($null -eq $assetSubsystemTicks -or [int]$assetSubsystemTicks -le 0) {
            $failures.Add("$($mode.Name): AssetSubsystem tick counter did not advance")
        }
        if ($null -eq $rendererSubsystemTicks -or [int]$rendererSubsystemTicks -le 0) {
            $failures.Add("$($mode.Name): RendererSubsystem tick counter did not advance")
        }
        if ($null -ne $engineTicks -and $null -ne $worldTicks -and [int]$engineTicks -ne [int]$worldTicks) {
            $failures.Add("$($mode.Name): Engine and World tick counters diverged")
        }
        if ($null -ne $engineTicks -and $null -ne $assetSubsystemTicks -and [int]$engineTicks -ne [int]$assetSubsystemTicks) {
            $failures.Add("$($mode.Name): Engine and AssetSubsystem tick counters diverged")
        }
        if ($null -ne $engineTicks -and $null -ne $rendererSubsystemTicks -and [int]$engineTicks -ne [int]$rendererSubsystemTicks) {
            $failures.Add("$($mode.Name): Engine and RendererSubsystem tick counters diverged")
        }
        if ($engineWorldPreparedSceneLine) {
            $sceneRuntimeWorldActors = Get-RegexValue -Text $engineWorldPreparedSceneLine -Pattern "runtimeWorldActors=(\d+)" -Group 1
            if ($null -ne $sceneRuntimeWorldActors -and $null -ne $healthWorldActors -and [int]$healthWorldActors -lt [int]$sceneRuntimeWorldActors) {
                $failures.Add("$($mode.Name): subsystem health worldActors was lower than prepared-scene runtimeWorldActors")
            }
        }
    }
    if (!$runtimeRendererSubsystemLine) {
        $failures.Add("$($mode.Name): missing runtime renderer subsystem stats")
    }
    else {
        $renderFrameBridgeCalls = Get-RegexValue -Text $runtimeRendererSubsystemLine -Pattern "renderFrameBridgeCalls=(\d+)" -Group 1
        $rendererBackendFrameCalls = Get-RegexValue -Text $runtimeRendererSubsystemLine -Pattern "rendererBackendFrameCalls=(\d+)" -Group 1
        $framebufferWidth = Get-RegexValue -Text $runtimeRendererSubsystemLine -Pattern "framebuffer=(\d+)x(\d+)" -Group 1
        $framebufferHeight = Get-RegexValue -Text $runtimeRendererSubsystemLine -Pattern "framebuffer=(\d+)x(\d+)" -Group 2
        $rendererBackendKey = Get-RegexValue -Text $runtimeRendererSubsystemLine -Pattern "rendererBackendKey=([^,\s]+)" -Group 1
        $rendererBackendState = Get-RegexValue -Text $runtimeRendererSubsystemLine -Pattern "rendererBackendState=([^,\s]+)" -Group 1
        $rendererBackendOwnerKey = Get-RegexValue -Text $runtimeRendererSubsystemLine -Pattern "rendererBackendOwnerKey=([^,\s]+)" -Group 1
        $rendererBackendOwnership = Get-RegexValue -Text $runtimeRendererSubsystemLine -Pattern "rendererBackendOwnership=([^,\s]+)" -Group 1
        $rendererBackendRegistryKey = Get-RegexValue -Text $runtimeRendererSubsystemLine -Pattern "rendererBackendRegistryKey=([^,\s]+)" -Group 1
        $rendererBackendRegistryCount = Get-RegexValue -Text $runtimeRendererSubsystemLine -Pattern "rendererBackendRegistryCount=(\d+)" -Group 1
        $rendererBackendAttachCount = Get-RegexValue -Text $runtimeRendererSubsystemLine -Pattern "rendererBackendAttachCount=(\d+)" -Group 1
        $rendererBackendDetachCount = Get-RegexValue -Text $runtimeRendererSubsystemLine -Pattern "rendererBackendDetachCount=(\d+)" -Group 1
        $rendererBackendReadyFrames = Get-RegexValue -Text $runtimeRendererSubsystemLine -Pattern "rendererBackendReadyFrames=(\d+)" -Group 1
        $rendererBackendNotReadyFrames = Get-RegexValue -Text $runtimeRendererSubsystemLine -Pattern "rendererBackendNotReadyFrames=(\d+)" -Group 1
        $framePlanKey = Get-RegexValue -Text $runtimeRendererSubsystemLine -Pattern "framePlanKey=([^,\s]+)" -Group 1
        $plannedPassCount = Get-RegexValue -Text $runtimeRendererSubsystemLine -Pattern "framePasses=(\d+)/(\d+)/(\d+)" -Group 1
        $executedPassCount = Get-RegexValue -Text $runtimeRendererSubsystemLine -Pattern "framePasses=(\d+)/(\d+)/(\d+)" -Group 2
        $skippedPassCount = Get-RegexValue -Text $runtimeRendererSubsystemLine -Pattern "framePasses=(\d+)/(\d+)/(\d+)" -Group 3
        $beginFrames = Get-RegexValue -Text $runtimeRendererSubsystemLine -Pattern "beginFrames=(\d+)" -Group 1
        $completedFrames = Get-RegexValue -Text $runtimeRendererSubsystemLine -Pattern "completedFrames=(\d+)" -Group 1
        $observedRendererPasses = Get-RegexValue -Text $runtimeRendererSubsystemLine -Pattern "observedRendererPasses=(\d+)" -Group 1
        $bridgeTimeSeconds = Get-RegexValue -Text $runtimeRendererSubsystemLine -Pattern "engineTimeSeconds=([0-9.]+)" -Group 1
        $bridgeDeltaSeconds = Get-RegexValue -Text $runtimeRendererSubsystemLine -Pattern "engineDeltaSeconds=([0-9.]+)" -Group 1
        if ($runtimeRendererSubsystemLine -notmatch "initialized=yes") {
            $failures.Add("$($mode.Name): RendererSubsystem was not initialized at rendered-frame report")
        }
        if ($runtimeRendererSubsystemLine -notmatch "hasRenderer=yes") {
            $failures.Add("$($mode.Name): RendererSubsystem had no renderer at rendered-frame report")
        }
        if ($runtimeRendererSubsystemLine -notmatch "runtimeRendererAttached=yes") {
            $failures.Add("$($mode.Name): RendererSubsystem renderer was not attached to the runtime renderer")
        }
        if ($runtimeRendererSubsystemLine -notmatch "runtimeContextRendererSubsystemAttached=yes") {
            $failures.Add("$($mode.Name): AppRuntimeContext did not expose the Engine-owned RendererSubsystem")
        }
        if ($runtimeRendererSubsystemLine -notmatch "frameBridgeActive=yes") {
            $failures.Add("$($mode.Name): RendererSubsystem frame bridge was not active")
        }
        if ($runtimeRendererSubsystemLine -notmatch "rendererBackendAttached=yes") {
            $failures.Add("$($mode.Name): RendererSubsystem did not receive a renderer backend")
        }
        if ($runtimeRendererSubsystemLine -notmatch "rendererBackendReady=yes") {
            $failures.Add("$($mode.Name): RendererSubsystem renderer backend was not ready")
        }
        if ($runtimeRendererSubsystemLine -notmatch "frameConfigValid=yes") {
            $failures.Add("$($mode.Name): RendererSubsystem frame config was not valid")
        }
        if ($null -eq $framebufferWidth -or [int]$framebufferWidth -le 0) {
            $failures.Add("$($mode.Name): RendererSubsystem framebuffer width was not positive")
        }
        if ($null -eq $framebufferHeight -or [int]$framebufferHeight -le 0) {
            $failures.Add("$($mode.Name): RendererSubsystem framebuffer height was not positive")
        }
        if ([string]::IsNullOrWhiteSpace($framePlanKey) -or $framePlanKey -eq "none") {
            $failures.Add("$($mode.Name): RendererSubsystem did not receive a frame plan key")
        }
        if ([string]::IsNullOrWhiteSpace($rendererBackendKey) -or $rendererBackendKey -eq "none") {
            $failures.Add("$($mode.Name): RendererSubsystem did not receive a renderer backend key")
        }
        if ($rendererBackendState -ne "ready") {
            $failures.Add("$($mode.Name): RendererSubsystem renderer backend state was not ready")
        }
        if ([string]::IsNullOrWhiteSpace($rendererBackendOwnerKey) -or $rendererBackendOwnerKey -eq "none") {
            $failures.Add("$($mode.Name): RendererSubsystem did not receive a renderer backend owner key")
        }
        if ($rendererBackendOwnerKey -ne "engine-renderer-subsystem") {
            $failures.Add("$($mode.Name): RendererSubsystem renderer backend owner key was not engine-renderer-subsystem")
        }
        if ($rendererBackendOwnership -ne "engine-owned") {
            $failures.Add("$($mode.Name): RendererSubsystem renderer backend ownership was not engine-owned")
        }
        if ([string]::IsNullOrWhiteSpace($rendererBackendRegistryKey) -or $rendererBackendRegistryKey -eq "none") {
            $failures.Add("$($mode.Name): RendererSubsystem did not receive a renderer backend registry key")
        }
        if ($null -eq $rendererBackendRegistryCount -or [int]$rendererBackendRegistryCount -le 0) {
            $failures.Add("$($mode.Name): RendererSubsystem did not report registered renderer backends")
        }
        if ($expectNoOpRendererBackend -and $rendererBackendKey -ne "test-noop-renderer-backend") {
            $failures.Add("$($mode.Name): RendererSubsystem did not select the no-op renderer backend")
        }
        if ($expectNoOpRendererBackend -and $rendererBackendRegistryKey -ne "test-noop-renderer-backend") {
            $failures.Add("$($mode.Name): RendererSubsystem registry key did not report the no-op renderer backend")
        }
        if ($expectNoOpRendererBackend -and ($null -eq $rendererBackendRegistryCount -or [int]$rendererBackendRegistryCount -lt 2)) {
            $failures.Add("$($mode.Name): RendererSubsystem registry did not expose multiple renderer backends")
        }
        if ($null -ne $rendererBackendRegistryKey -and $null -ne $rendererBackendKey -and $rendererBackendRegistryKey -ne $rendererBackendKey) {
            $failures.Add("$($mode.Name): RendererSubsystem registry key did not match the selected backend key")
        }
        if ($null -eq $rendererBackendAttachCount -or [int]$rendererBackendAttachCount -le 0) {
            $failures.Add("$($mode.Name): RendererSubsystem did not record a renderer backend attach")
        }
        if ($null -eq $rendererBackendDetachCount) {
            $failures.Add("$($mode.Name): RendererSubsystem did not report renderer backend detach count")
        }
        if ($null -eq $rendererBackendReadyFrames -or [int]$rendererBackendReadyFrames -le 0) {
            $failures.Add("$($mode.Name): RendererSubsystem did not record ready backend frames")
        }
        if ($null -eq $rendererBackendNotReadyFrames) {
            $failures.Add("$($mode.Name): RendererSubsystem did not report not-ready backend frames")
        }
        if ($null -ne $rendererBackendNotReadyFrames -and [int]$rendererBackendNotReadyFrames -ne 0) {
            $failures.Add("$($mode.Name): RendererSubsystem recorded not-ready backend frames")
        }
        if ($null -eq $plannedPassCount -or [int]$plannedPassCount -le 0) {
            $failures.Add("$($mode.Name): RendererSubsystem did not observe planned frame passes")
        }
        if ($null -eq $executedPassCount -or [int]$executedPassCount -le 0) {
            $failures.Add("$($mode.Name): RendererSubsystem did not observe executed frame passes")
        }
        if ($null -eq $skippedPassCount) {
            $failures.Add("$($mode.Name): RendererSubsystem did not report skipped frame passes")
        }
        if ($null -ne $plannedPassCount -and $null -ne $executedPassCount -and $null -ne $skippedPassCount) {
            $observedFramePasses = [int]$executedPassCount + [int]$skippedPassCount
            if ([int]$plannedPassCount -ne $observedFramePasses) {
                $failures.Add("$($mode.Name): RendererSubsystem planned/executed/skipped frame pass counts diverged")
            }
        }
        if ($expectNoOpRendererBackend) {
            if ($framePlanKey -ne "test-noop-renderer-backend:clear") {
                $failures.Add("$($mode.Name): no-op renderer backend frame plan key was not reported")
            }
            if ($plannedPassCount -ne "1" -or $executedPassCount -ne "1" -or $skippedPassCount -ne "0") {
                $failures.Add("$($mode.Name): no-op renderer backend frame pass counts were not 1/1/0")
            }
        }
        if ($null -eq $renderFrameBridgeCalls -or [int]$renderFrameBridgeCalls -le 0) {
            $failures.Add("$($mode.Name): RendererSubsystem renderFrameBridge was not called")
        }
        if ($null -eq $rendererBackendFrameCalls -or [int]$rendererBackendFrameCalls -le 0) {
            $failures.Add("$($mode.Name): RendererSubsystem renderer backend was not called")
        }
        if ($null -eq $beginFrames -or [int]$beginFrames -le 0) {
            $failures.Add("$($mode.Name): RendererSubsystem frame bridge did not begin any frames")
        }
        if ($null -eq $completedFrames -or [int]$completedFrames -le 0) {
            $failures.Add("$($mode.Name): RendererSubsystem frame bridge did not complete any frames")
        }
        if ($null -ne $beginFrames -and $null -ne $completedFrames -and [int]$completedFrames -ne [int]$beginFrames) {
            $failures.Add("$($mode.Name): RendererSubsystem frame bridge begin/completed frame counts diverged")
        }
        if ($null -ne $renderFrameBridgeCalls -and $null -ne $beginFrames -and [int]$renderFrameBridgeCalls -ne [int]$beginFrames) {
            $failures.Add("$($mode.Name): RendererSubsystem renderFrameBridge/begin frame counts diverged")
        }
        if ($null -ne $rendererBackendFrameCalls -and $null -ne $renderFrameBridgeCalls -and [int]$rendererBackendFrameCalls -ne [int]$renderFrameBridgeCalls) {
            $failures.Add("$($mode.Name): RendererSubsystem renderer backend/render bridge counts diverged")
        }
        if ($null -ne $rendererBackendReadyFrames -and $null -ne $rendererBackendNotReadyFrames -and $null -ne $renderFrameBridgeCalls) {
            $backendFrameObservations = [int]$rendererBackendReadyFrames + [int]$rendererBackendNotReadyFrames
            if ($backendFrameObservations -ne [int]$renderFrameBridgeCalls) {
                $failures.Add("$($mode.Name): RendererSubsystem backend frame observations/render bridge counts diverged")
            }
        }
        if ($null -ne $rendererBackendReadyFrames -and $null -ne $renderFrameBridgeCalls -and [int]$rendererBackendReadyFrames -ne [int]$renderFrameBridgeCalls) {
            $failures.Add("$($mode.Name): RendererSubsystem ready backend frame count did not match render bridge calls")
        }
        if ($expectNoOpRendererBackend -and $null -ne $observedRendererPasses -and [int]$observedRendererPasses -ne 0) {
            $failures.Add("$($mode.Name): no-op renderer backend unexpectedly invoked legacy renderer passes")
        }
        if (!$expectNoOpRendererBackend -and ($null -eq $observedRendererPasses -or [int]$observedRendererPasses -le 0)) {
            $failures.Add("$($mode.Name): RendererSubsystem did not observe renderer passes")
        }
        if ($null -eq $bridgeTimeSeconds -or [double]$bridgeTimeSeconds -le 0.0) {
            $failures.Add("$($mode.Name): RendererSubsystem bridge engineTimeSeconds did not advance")
        }
        if ($null -eq $bridgeDeltaSeconds -or [double]$bridgeDeltaSeconds -le 0.0) {
            $failures.Add("$($mode.Name): RendererSubsystem bridge engineDeltaSeconds was not positive")
        }
    }
    if (!$runtimeRendererBackendContractLine) {
        $failures.Add("$($mode.Name): missing runtime renderer backend contract stats")
    }
    else {
        $contractRenderFrameBridgeCalls = Get-RegexValue -Text $runtimeRendererBackendContractLine -Pattern "renderFrameBridgeCalls=(\d+)" -Group 1
        $contractRendererBackendFrameCalls = Get-RegexValue -Text $runtimeRendererBackendContractLine -Pattern "rendererBackendFrameCalls=(\d+)" -Group 1
        $contractFramebufferWidth = Get-RegexValue -Text $runtimeRendererBackendContractLine -Pattern "framebuffer=(\d+)x(\d+)" -Group 1
        $contractFramebufferHeight = Get-RegexValue -Text $runtimeRendererBackendContractLine -Pattern "framebuffer=(\d+)x(\d+)" -Group 2
        $contractRendererBackendKey = Get-RegexValue -Text $runtimeRendererBackendContractLine -Pattern "rendererBackendKey=([^,\s]+)" -Group 1
        $contractRendererBackendState = Get-RegexValue -Text $runtimeRendererBackendContractLine -Pattern "rendererBackendState=([^,\s]+)" -Group 1
        $contractRendererBackendOwnerKey = Get-RegexValue -Text $runtimeRendererBackendContractLine -Pattern "rendererBackendOwnerKey=([^,\s]+)" -Group 1
        $contractRendererBackendOwnership = Get-RegexValue -Text $runtimeRendererBackendContractLine -Pattern "rendererBackendOwnership=([^,\s]+)" -Group 1
        $contractRendererBackendRegistryKey = Get-RegexValue -Text $runtimeRendererBackendContractLine -Pattern "rendererBackendRegistryKey=([^,\s]+)" -Group 1
        $contractRendererBackendRegistryCount = Get-RegexValue -Text $runtimeRendererBackendContractLine -Pattern "rendererBackendRegistryCount=(\d+)" -Group 1
        $contractRendererBackendReadyFrames = Get-RegexValue -Text $runtimeRendererBackendContractLine -Pattern "rendererBackendReadyFrames=(\d+)" -Group 1
        $contractRendererBackendNotReadyFrames = Get-RegexValue -Text $runtimeRendererBackendContractLine -Pattern "rendererBackendNotReadyFrames=(\d+)" -Group 1
        $contractFramePlanKey = Get-RegexValue -Text $runtimeRendererBackendContractLine -Pattern "framePlanKey=([^,\s]+)" -Group 1
        $contractPlannedPassCount = Get-RegexValue -Text $runtimeRendererBackendContractLine -Pattern "framePasses=(\d+)/(\d+)/(\d+)" -Group 1
        $contractExecutedPassCount = Get-RegexValue -Text $runtimeRendererBackendContractLine -Pattern "framePasses=(\d+)/(\d+)/(\d+)" -Group 2
        $contractSkippedPassCount = Get-RegexValue -Text $runtimeRendererBackendContractLine -Pattern "framePasses=(\d+)/(\d+)/(\d+)" -Group 3
        $contractObservedRendererPasses = Get-RegexValue -Text $runtimeRendererBackendContractLine -Pattern "observedRendererPasses=(\d+)" -Group 1
        if ($runtimeRendererBackendContractLine -notmatch "rendererBackendAttached=yes") {
            $failures.Add("$($mode.Name): renderer backend contract did not report an attached backend")
        }
        if ($runtimeRendererBackendContractLine -notmatch "rendererBackendReady=yes") {
            $failures.Add("$($mode.Name): renderer backend contract was not ready")
        }
        if ($runtimeRendererBackendContractLine -notmatch "frameConfigValid=yes") {
            $failures.Add("$($mode.Name): renderer backend contract frame config was not valid")
        }
        if ($null -eq $contractFramebufferWidth -or [int]$contractFramebufferWidth -le 0) {
            $failures.Add("$($mode.Name): renderer backend contract framebuffer width was not positive")
        }
        if ($null -eq $contractFramebufferHeight -or [int]$contractFramebufferHeight -le 0) {
            $failures.Add("$($mode.Name): renderer backend contract framebuffer height was not positive")
        }
        if ([string]::IsNullOrWhiteSpace($contractRendererBackendKey) -or $contractRendererBackendKey -eq "none") {
            $failures.Add("$($mode.Name): renderer backend contract did not expose a backend key")
        }
        if ($contractRendererBackendState -ne "ready") {
            $failures.Add("$($mode.Name): renderer backend contract state was not ready")
        }
        if ($contractRendererBackendOwnerKey -ne "engine-renderer-subsystem") {
            $failures.Add("$($mode.Name): renderer backend contract owner was not engine-renderer-subsystem")
        }
        if ($contractRendererBackendOwnership -ne "engine-owned") {
            $failures.Add("$($mode.Name): renderer backend contract ownership was not engine-owned")
        }
        if ([string]::IsNullOrWhiteSpace($contractRendererBackendRegistryKey) -or $contractRendererBackendRegistryKey -eq "none") {
            $failures.Add("$($mode.Name): renderer backend contract did not expose a registry key")
        }
        if ($contractRendererBackendRegistryKey -ne $contractRendererBackendKey) {
            $failures.Add("$($mode.Name): renderer backend contract registry key did not match selected backend key")
        }
        if ($null -eq $contractRendererBackendRegistryCount -or [int]$contractRendererBackendRegistryCount -le 0) {
            $failures.Add("$($mode.Name): renderer backend contract did not expose registered backends")
        }
        if ($expectNoOpRendererBackend -and $null -ne $contractRendererBackendRegistryCount -and [int]$contractRendererBackendRegistryCount -lt 2) {
            $failures.Add("$($mode.Name): renderer backend contract no-op mode did not expose multiple registered backends")
        }
        if ($expectNoOpRendererBackend -and $contractRendererBackendKey -ne "test-noop-renderer-backend") {
            $failures.Add("$($mode.Name): renderer backend contract did not select the no-op backend")
        }
        if ($expectNoOpRendererBackend -and $contractFramePlanKey -ne "test-noop-renderer-backend:clear") {
            $failures.Add("$($mode.Name): renderer backend contract no-op frame plan key was not reported")
        }
        if ($null -eq $contractRendererBackendReadyFrames -or [int]$contractRendererBackendReadyFrames -le 0) {
            $failures.Add("$($mode.Name): renderer backend contract did not record ready frames")
        }
        if ($null -eq $contractRendererBackendNotReadyFrames -or [int]$contractRendererBackendNotReadyFrames -ne 0) {
            $failures.Add("$($mode.Name): renderer backend contract recorded not-ready frames")
        }
        if ($null -ne $contractRendererBackendReadyFrames -and $null -ne $contractRendererBackendNotReadyFrames -and $null -ne $contractRenderFrameBridgeCalls) {
            $contractBackendFrameObservations = [int]$contractRendererBackendReadyFrames + [int]$contractRendererBackendNotReadyFrames
            if ($contractBackendFrameObservations -ne [int]$contractRenderFrameBridgeCalls) {
                $failures.Add("$($mode.Name): renderer backend contract frame observations/render bridge counts diverged")
            }
        }
        if ($null -ne $contractRendererBackendReadyFrames -and $null -ne $contractRenderFrameBridgeCalls -and [int]$contractRendererBackendReadyFrames -ne [int]$contractRenderFrameBridgeCalls) {
            $failures.Add("$($mode.Name): renderer backend contract ready frame count did not match render bridge calls")
        }
        if ([string]::IsNullOrWhiteSpace($contractFramePlanKey) -or $contractFramePlanKey -eq "none") {
            $failures.Add("$($mode.Name): renderer backend contract did not expose a frame plan key")
        }
        if ($null -ne $contractPlannedPassCount -and $null -ne $contractExecutedPassCount -and $null -ne $contractSkippedPassCount) {
            $contractObservedFramePasses = [int]$contractExecutedPassCount + [int]$contractSkippedPassCount
            if ([int]$contractPlannedPassCount -ne $contractObservedFramePasses) {
                $failures.Add("$($mode.Name): renderer backend contract frame pass counts diverged")
            }
        }
        else {
            $failures.Add("$($mode.Name): renderer backend contract did not expose frame pass counts")
        }
        if ($expectNoOpRendererBackend -and ($contractPlannedPassCount -ne "1" -or $contractExecutedPassCount -ne "1" -or $contractSkippedPassCount -ne "0")) {
            $failures.Add("$($mode.Name): renderer backend contract no-op frame pass counts were not 1/1/0")
        }
        if ($null -eq $contractRenderFrameBridgeCalls -or [int]$contractRenderFrameBridgeCalls -le 0) {
            $failures.Add("$($mode.Name): renderer backend contract render bridge was not called")
        }
        if ($null -eq $contractRendererBackendFrameCalls -or [int]$contractRendererBackendFrameCalls -le 0) {
            $failures.Add("$($mode.Name): renderer backend contract backend was not called")
        }
        if ($null -ne $contractRendererBackendFrameCalls -and $null -ne $contractRenderFrameBridgeCalls -and [int]$contractRendererBackendFrameCalls -ne [int]$contractRenderFrameBridgeCalls) {
            $failures.Add("$($mode.Name): renderer backend contract backend/render bridge counts diverged")
        }
        if ($expectNoOpRendererBackend -and $null -ne $contractObservedRendererPasses -and [int]$contractObservedRendererPasses -ne 0) {
            $failures.Add("$($mode.Name): renderer backend contract no-op path unexpectedly observed renderer passes")
        }
        if (!$expectNoOpRendererBackend -and ($null -eq $contractObservedRendererPasses -or [int]$contractObservedRendererPasses -le 0)) {
            $failures.Add("$($mode.Name): renderer backend contract did not observe renderer passes")
        }
    }
    if (!$runtimeRendererSubsystemCleanupLine) {
        $failures.Add("$($mode.Name): missing runtime renderer subsystem cleanup stats")
    }
    else {
        $cleanupBackendKey = Get-RegexValue -Text $runtimeRendererSubsystemCleanupLine -Pattern "rendererBackendKey=([^,\s]+)" -Group 1
        $cleanupBackendState = Get-RegexValue -Text $runtimeRendererSubsystemCleanupLine -Pattern "rendererBackendState=([^,\s]+)" -Group 1
        $cleanupBackendOwnerKey = Get-RegexValue -Text $runtimeRendererSubsystemCleanupLine -Pattern "rendererBackendOwnerKey=([^,\s]+)" -Group 1
        $cleanupBackendOwnership = Get-RegexValue -Text $runtimeRendererSubsystemCleanupLine -Pattern "rendererBackendOwnership=([^,\s]+)" -Group 1
        $cleanupBackendRegistryKey = Get-RegexValue -Text $runtimeRendererSubsystemCleanupLine -Pattern "rendererBackendRegistryKey=([^,\s]+)" -Group 1
        $cleanupBackendRegistryCount = Get-RegexValue -Text $runtimeRendererSubsystemCleanupLine -Pattern "rendererBackendRegistryCount=(\d+)" -Group 1
        $cleanupBackendAttachCount = Get-RegexValue -Text $runtimeRendererSubsystemCleanupLine -Pattern "rendererBackendAttachCount=(\d+)" -Group 1
        $cleanupBackendDetachCount = Get-RegexValue -Text $runtimeRendererSubsystemCleanupLine -Pattern "rendererBackendDetachCount=(\d+)" -Group 1
        $cleanupBackendReadyFrames = Get-RegexValue -Text $runtimeRendererSubsystemCleanupLine -Pattern "rendererBackendReadyFrames=(\d+)" -Group 1
        $cleanupBackendNotReadyFrames = Get-RegexValue -Text $runtimeRendererSubsystemCleanupLine -Pattern "rendererBackendNotReadyFrames=(\d+)" -Group 1
        if ($runtimeRendererSubsystemCleanupLine -notmatch "initialized=yes") {
            $failures.Add("$($mode.Name): RendererSubsystem was not initialized during cleanup report")
        }
        if ($runtimeRendererSubsystemCleanupLine -notmatch "runtimeContextRendererSubsystemAttached=yes") {
            $failures.Add("$($mode.Name): runtime context did not still reference RendererSubsystem during cleanup report")
        }
        if ($runtimeRendererSubsystemCleanupLine -notmatch "rendererBackendAttached=no") {
            $failures.Add("$($mode.Name): RendererSubsystem renderer backend was not detached during cleanup")
        }
        if ($runtimeRendererSubsystemCleanupLine -notmatch "rendererBackendReady=no") {
            $failures.Add("$($mode.Name): RendererSubsystem backend still reported ready during cleanup")
        }
        if ($cleanupBackendKey -ne "none") {
            $failures.Add("$($mode.Name): RendererSubsystem cleanup backend key was not none")
        }
        if ($cleanupBackendState -ne "detached") {
            $failures.Add("$($mode.Name): RendererSubsystem cleanup backend state was not detached")
        }
        if ($cleanupBackendOwnerKey -ne "none") {
            $failures.Add("$($mode.Name): RendererSubsystem cleanup backend owner key was not cleared")
        }
        if ($cleanupBackendOwnership -ne "detached") {
            $failures.Add("$($mode.Name): RendererSubsystem cleanup backend ownership was not detached")
        }
        if ($cleanupBackendRegistryKey -ne "none") {
            $failures.Add("$($mode.Name): RendererSubsystem cleanup backend registry key was not cleared")
        }
        if ($null -eq $cleanupBackendRegistryCount -or [int]$cleanupBackendRegistryCount -ne 0) {
            $failures.Add("$($mode.Name): RendererSubsystem cleanup backend registry count was not reset")
        }
        if ($null -eq $cleanupBackendAttachCount -or [int]$cleanupBackendAttachCount -le 0) {
            $failures.Add("$($mode.Name): RendererSubsystem cleanup did not preserve backend attach count")
        }
        if ($null -eq $cleanupBackendDetachCount -or [int]$cleanupBackendDetachCount -le 0) {
            $failures.Add("$($mode.Name): RendererSubsystem cleanup did not record backend detach")
        }
        if ($null -ne $cleanupBackendAttachCount -and $null -ne $cleanupBackendDetachCount -and [int]$cleanupBackendAttachCount -ne [int]$cleanupBackendDetachCount) {
            $failures.Add("$($mode.Name): RendererSubsystem cleanup attach/detach counts diverged")
        }
        if ($null -eq $cleanupBackendReadyFrames -or [int]$cleanupBackendReadyFrames -le 0) {
            $failures.Add("$($mode.Name): RendererSubsystem cleanup did not preserve ready backend frame count")
        }
        if ($null -eq $cleanupBackendNotReadyFrames -or [int]$cleanupBackendNotReadyFrames -ne 0) {
            $failures.Add("$($mode.Name): RendererSubsystem cleanup reported not-ready backend frames")
        }
    }
    if (!$runtimeRendererBackendContractCleanupLine) {
        $failures.Add("$($mode.Name): missing runtime renderer backend contract cleanup stats")
    }
    else {
        $contractCleanupBackendKey = Get-RegexValue -Text $runtimeRendererBackendContractCleanupLine -Pattern "rendererBackendKey=([^,\s]+)" -Group 1
        $contractCleanupBackendState = Get-RegexValue -Text $runtimeRendererBackendContractCleanupLine -Pattern "rendererBackendState=([^,\s]+)" -Group 1
        $contractCleanupBackendOwnerKey = Get-RegexValue -Text $runtimeRendererBackendContractCleanupLine -Pattern "rendererBackendOwnerKey=([^,\s]+)" -Group 1
        $contractCleanupBackendOwnership = Get-RegexValue -Text $runtimeRendererBackendContractCleanupLine -Pattern "rendererBackendOwnership=([^,\s]+)" -Group 1
        $contractCleanupBackendRegistryKey = Get-RegexValue -Text $runtimeRendererBackendContractCleanupLine -Pattern "rendererBackendRegistryKey=([^,\s]+)" -Group 1
        $contractCleanupBackendRegistryCount = Get-RegexValue -Text $runtimeRendererBackendContractCleanupLine -Pattern "rendererBackendRegistryCount=(\d+)" -Group 1
        $contractCleanupBackendAttachCount = Get-RegexValue -Text $runtimeRendererBackendContractCleanupLine -Pattern "rendererBackendAttachCount=(\d+)" -Group 1
        $contractCleanupBackendDetachCount = Get-RegexValue -Text $runtimeRendererBackendContractCleanupLine -Pattern "rendererBackendDetachCount=(\d+)" -Group 1
        $contractCleanupBackendReadyFrames = Get-RegexValue -Text $runtimeRendererBackendContractCleanupLine -Pattern "rendererBackendReadyFrames=(\d+)" -Group 1
        $contractCleanupBackendNotReadyFrames = Get-RegexValue -Text $runtimeRendererBackendContractCleanupLine -Pattern "rendererBackendNotReadyFrames=(\d+)" -Group 1
        if ($runtimeRendererBackendContractCleanupLine -notmatch "rendererBackendAttached=no") {
            $failures.Add("$($mode.Name): renderer backend contract cleanup did not detach backend")
        }
        if ($runtimeRendererBackendContractCleanupLine -notmatch "rendererBackendReady=no") {
            $failures.Add("$($mode.Name): renderer backend contract cleanup still reported ready")
        }
        if ($contractCleanupBackendKey -ne "none") {
            $failures.Add("$($mode.Name): renderer backend contract cleanup backend key was not none")
        }
        if ($contractCleanupBackendState -ne "detached") {
            $failures.Add("$($mode.Name): renderer backend contract cleanup state was not detached")
        }
        if ($contractCleanupBackendOwnerKey -ne "none") {
            $failures.Add("$($mode.Name): renderer backend contract cleanup owner key was not cleared")
        }
        if ($contractCleanupBackendOwnership -ne "detached") {
            $failures.Add("$($mode.Name): renderer backend contract cleanup ownership was not detached")
        }
        if ($contractCleanupBackendRegistryKey -ne "none") {
            $failures.Add("$($mode.Name): renderer backend contract cleanup registry key was not cleared")
        }
        if ($null -eq $contractCleanupBackendRegistryCount -or [int]$contractCleanupBackendRegistryCount -ne 0) {
            $failures.Add("$($mode.Name): renderer backend contract cleanup registry count was not reset")
        }
        if ($null -eq $contractCleanupBackendAttachCount -or [int]$contractCleanupBackendAttachCount -le 0) {
            $failures.Add("$($mode.Name): renderer backend contract cleanup did not preserve attach count")
        }
        if ($null -eq $contractCleanupBackendDetachCount -or [int]$contractCleanupBackendDetachCount -le 0) {
            $failures.Add("$($mode.Name): renderer backend contract cleanup did not record detach")
        }
        if ($null -ne $contractCleanupBackendAttachCount -and $null -ne $contractCleanupBackendDetachCount -and [int]$contractCleanupBackendAttachCount -ne [int]$contractCleanupBackendDetachCount) {
            $failures.Add("$($mode.Name): renderer backend contract cleanup attach/detach counts diverged")
        }
        if ($null -eq $contractCleanupBackendReadyFrames -or [int]$contractCleanupBackendReadyFrames -le 0) {
            $failures.Add("$($mode.Name): renderer backend contract cleanup did not preserve ready frames")
        }
        if ($null -eq $contractCleanupBackendNotReadyFrames -or [int]$contractCleanupBackendNotReadyFrames -ne 0) {
            $failures.Add("$($mode.Name): renderer backend contract cleanup reported not-ready frames")
        }
    }
    if (!$runtimeEngineWorldCleanupLine) {
        $failures.Add("$($mode.Name): missing runtime engine world cleanup stats")
    }
    else {
        $cleanupEngineTicks = Get-RegexValue -Text $runtimeEngineWorldCleanupLine -Pattern "engineTicks=(\d+)" -Group 1
        $cleanupEngineTimeSeconds = Get-RegexValue -Text $runtimeEngineWorldCleanupLine -Pattern "engineTimeSeconds=([0-9.]+)" -Group 1
        $cleanupEngineDeltaSeconds = Get-RegexValue -Text $runtimeEngineWorldCleanupLine -Pattern "engineDeltaSeconds=([0-9.]+)" -Group 1
        if ($runtimeEngineWorldCleanupLine -notmatch "engineInitialized=no") {
            $failures.Add("$($mode.Name): Engine remained initialized during world cleanup report")
        }
        if ($runtimeEngineWorldCleanupLine -notmatch "runtimeContextEngineNull=yes") {
            $failures.Add("$($mode.Name): runtime context engine pointer was not cleared during world cleanup")
        }
        if ($runtimeEngineWorldCleanupLine -notmatch "runtimeContextEngineAttached=no") {
            $failures.Add("$($mode.Name): runtime context still reported attached Engine during world cleanup")
        }
        if ($runtimeEngineWorldCleanupLine -notmatch "activeWorld=no") {
            $failures.Add("$($mode.Name): Engine active World was not reset during cleanup")
        }
        if ($runtimeEngineWorldCleanupLine -notmatch "runtimeContextWorldNull=yes") {
            $failures.Add("$($mode.Name): runtime context World pointer was not cleared during cleanup")
        }
        if ($runtimeEngineWorldCleanupLine -notmatch "runtimeContextWorldAttached=no") {
            $failures.Add("$($mode.Name): runtime context still reported attached World during cleanup")
        }
        if ($runtimeEngineWorldCleanupLine -notmatch "runtimeContextWorldEditable=no") {
            $failures.Add("$($mode.Name): runtime context World editable flag was not cleared during cleanup")
        }
        if ($runtimeEngineWorldCleanupLine -notmatch "runtimeContextAssetSubsystemNull=yes") {
            $failures.Add("$($mode.Name): runtime context AssetSubsystem pointer was not cleared during cleanup")
        }
        if ($runtimeEngineWorldCleanupLine -notmatch "runtimeContextAssetSubsystemAttached=no") {
            $failures.Add("$($mode.Name): runtime context still reported attached AssetSubsystem during cleanup")
        }
        if ($runtimeEngineWorldCleanupLine -notmatch "runtimeContextRendererSubsystemNull=yes") {
            $failures.Add("$($mode.Name): runtime context RendererSubsystem pointer was not cleared during cleanup")
        }
        if ($runtimeEngineWorldCleanupLine -notmatch "runtimeContextRendererSubsystemAttached=no") {
            $failures.Add("$($mode.Name): runtime context still reported attached RendererSubsystem during cleanup")
        }
        if ($null -eq $cleanupEngineTicks -or [int]$cleanupEngineTicks -le 0) {
            $failures.Add("$($mode.Name): Engine cleanup report did not preserve tick count")
        }
        if ($null -eq $cleanupEngineTimeSeconds -or [double]$cleanupEngineTimeSeconds -le 0.0) {
            $failures.Add("$($mode.Name): Engine cleanup report did not preserve advanced timeSeconds")
        }
        if ($null -eq $cleanupEngineDeltaSeconds -or [double]$cleanupEngineDeltaSeconds -le 0.0) {
            $failures.Add("$($mode.Name): Engine cleanup report did not preserve positive deltaSeconds")
        }
    }
    if (!$runtimeEngineSubsystemCleanupLine) {
        $failures.Add("$($mode.Name): missing runtime engine subsystem cleanup stats")
    }
    else {
        $cleanupAssetSubsystemTicks = Get-RegexValue -Text $runtimeEngineSubsystemCleanupLine -Pattern "assetSubsystemTicks=(\d+)" -Group 1
        $cleanupAssetRegistryAssets = Get-RegexValue -Text $runtimeEngineSubsystemCleanupLine -Pattern "assetRegistryAssets=(\d+)" -Group 1
        $cleanupRendererSubsystemTicks = Get-RegexValue -Text $runtimeEngineSubsystemCleanupLine -Pattern "rendererSubsystemTicks=(\d+)" -Group 1
        $cleanupRendererBackendKey = Get-RegexValue -Text $runtimeEngineSubsystemCleanupLine -Pattern "rendererBackendKey=([^,\s]+)" -Group 1
        $cleanupRendererBackendState = Get-RegexValue -Text $runtimeEngineSubsystemCleanupLine -Pattern "rendererBackendState=([^,\s]+)" -Group 1
        $cleanupRendererBackendOwnerKey = Get-RegexValue -Text $runtimeEngineSubsystemCleanupLine -Pattern "rendererBackendOwnerKey=([^,\s]+)" -Group 1
        $cleanupRendererBackendOwnership = Get-RegexValue -Text $runtimeEngineSubsystemCleanupLine -Pattern "rendererBackendOwnership=([^,\s]+)" -Group 1
        $cleanupRendererBackendRegistryKey = Get-RegexValue -Text $runtimeEngineSubsystemCleanupLine -Pattern "rendererBackendRegistryKey=([^,\s]+)" -Group 1
        $cleanupRendererBackendRegistryCount = Get-RegexValue -Text $runtimeEngineSubsystemCleanupLine -Pattern "rendererBackendRegistryCount=(\d+)" -Group 1
        $cleanupRendererBackendAttachCount = Get-RegexValue -Text $runtimeEngineSubsystemCleanupLine -Pattern "rendererBackendAttachCount=(\d+)" -Group 1
        $cleanupRendererBackendDetachCount = Get-RegexValue -Text $runtimeEngineSubsystemCleanupLine -Pattern "rendererBackendDetachCount=(\d+)" -Group 1
        $cleanupSnapshotSubsystemCount = Get-RegexValue -Text $runtimeEngineSubsystemCleanupLine -Pattern "snapshotSubsystems=(\d+)" -Group 1
        $cleanupSnapshotInitializedSubsystemCount = Get-RegexValue -Text $runtimeEngineSubsystemCleanupLine -Pattern "snapshotInitializedSubsystems=(\d+)" -Group 1
        $cleanupSnapshotTickedSubsystemCount = Get-RegexValue -Text $runtimeEngineSubsystemCleanupLine -Pattern "snapshotTickedSubsystems=(\d+)" -Group 1
        $cleanupSnapshotSubsystemTicks = Get-RegexValue -Text $runtimeEngineSubsystemCleanupLine -Pattern "snapshotSubsystemTicks=([^,\s]+)" -Group 1
        if ($runtimeEngineSubsystemCleanupLine -notmatch "runtimeContextAssetSubsystemAttached=no") {
            $failures.Add("$($mode.Name): runtime context still reported attached AssetSubsystem during subsystem cleanup")
        }
        if ($runtimeEngineSubsystemCleanupLine -notmatch "runtimeContextRendererSubsystemAttached=no") {
            $failures.Add("$($mode.Name): runtime context still reported attached RendererSubsystem during subsystem cleanup")
        }
        if ($runtimeEngineSubsystemCleanupLine -notmatch "assetSubsystemPresent=yes") {
            $failures.Add("$($mode.Name): Engine-owned AssetSubsystem was not available for post-shutdown cleanup report")
        }
        if ($runtimeEngineSubsystemCleanupLine -notmatch "assetSubsystemInitialized=no") {
            $failures.Add("$($mode.Name): AssetSubsystem remained initialized after Engine shutdown")
        }
        if ($null -eq $cleanupAssetSubsystemTicks -or [int]$cleanupAssetSubsystemTicks -ne 0) {
            $failures.Add("$($mode.Name): AssetSubsystem tick count was not reset during shutdown")
        }
        if ($null -eq $cleanupAssetRegistryAssets -or [int]$cleanupAssetRegistryAssets -ne 0) {
            $failures.Add("$($mode.Name): AssetSubsystem registry was not cleared during shutdown")
        }
        if ($runtimeEngineSubsystemCleanupLine -notmatch "rendererSubsystemPresent=yes") {
            $failures.Add("$($mode.Name): Engine-owned RendererSubsystem was not available for post-shutdown cleanup report")
        }
        if ($runtimeEngineSubsystemCleanupLine -notmatch "rendererSubsystemInitialized=no") {
            $failures.Add("$($mode.Name): RendererSubsystem remained initialized after Engine shutdown")
        }
        if ($null -eq $cleanupRendererSubsystemTicks -or [int]$cleanupRendererSubsystemTicks -ne 0) {
            $failures.Add("$($mode.Name): RendererSubsystem tick count was not reset during shutdown")
        }
        if ($runtimeEngineSubsystemCleanupLine -notmatch "rendererHasRenderer=no") {
            $failures.Add("$($mode.Name): RendererSubsystem still held a runtime renderer after shutdown")
        }
        if ($runtimeEngineSubsystemCleanupLine -notmatch "rendererBackendAttached=no") {
            $failures.Add("$($mode.Name): RendererSubsystem still held a renderer backend after shutdown")
        }
        if ($runtimeEngineSubsystemCleanupLine -notmatch "rendererBackendReady=no") {
            $failures.Add("$($mode.Name): RendererSubsystem backend remained ready after shutdown")
        }
        if ($cleanupRendererBackendKey -ne "none") {
            $failures.Add("$($mode.Name): RendererSubsystem post-shutdown backend key was not none")
        }
        if ($cleanupRendererBackendState -ne "detached") {
            $failures.Add("$($mode.Name): RendererSubsystem post-shutdown backend state was not detached")
        }
        if ($cleanupRendererBackendOwnerKey -ne "none") {
            $failures.Add("$($mode.Name): RendererSubsystem post-shutdown backend owner key was not cleared")
        }
        if ($cleanupRendererBackendOwnership -ne "detached") {
            $failures.Add("$($mode.Name): RendererSubsystem post-shutdown backend ownership was not detached")
        }
        if ($cleanupRendererBackendRegistryKey -ne "none") {
            $failures.Add("$($mode.Name): RendererSubsystem post-shutdown backend registry key was not cleared")
        }
        if ($null -eq $cleanupRendererBackendRegistryCount -or [int]$cleanupRendererBackendRegistryCount -ne 0) {
            $failures.Add("$($mode.Name): RendererSubsystem post-shutdown backend registry count was not reset")
        }
        if ($null -eq $cleanupRendererBackendAttachCount -or [int]$cleanupRendererBackendAttachCount -le 0) {
            $failures.Add("$($mode.Name): RendererSubsystem post-shutdown cleanup did not preserve backend attach count")
        }
        if ($null -eq $cleanupRendererBackendDetachCount -or [int]$cleanupRendererBackendDetachCount -le 0) {
            $failures.Add("$($mode.Name): RendererSubsystem post-shutdown cleanup did not preserve backend detach count")
        }
        if ($null -ne $cleanupRendererBackendAttachCount -and $null -ne $cleanupRendererBackendDetachCount -and [int]$cleanupRendererBackendAttachCount -ne [int]$cleanupRendererBackendDetachCount) {
            $failures.Add("$($mode.Name): RendererSubsystem post-shutdown attach/detach counts diverged")
        }
        if ($null -eq $cleanupSnapshotSubsystemCount -or [int]$cleanupSnapshotSubsystemCount -lt 2) {
            $failures.Add("$($mode.Name): Engine lifecycle snapshot did not preserve subsystem summaries after shutdown")
        }
        if ($null -eq $cleanupSnapshotInitializedSubsystemCount -or [int]$cleanupSnapshotInitializedSubsystemCount -ne 0) {
            $failures.Add("$($mode.Name): Engine lifecycle snapshot reported initialized subsystems after shutdown")
        }
        if ($null -eq $cleanupSnapshotTickedSubsystemCount -or [int]$cleanupSnapshotTickedSubsystemCount -ne 0) {
            $failures.Add("$($mode.Name): Engine lifecycle snapshot reported ticked subsystems after shutdown")
        }
        if ($runtimeEngineSubsystemCleanupLine -notmatch "snapshotSubsystemNames=.*AssetSubsystem") {
            $failures.Add("$($mode.Name): Engine lifecycle cleanup snapshot did not include AssetSubsystem")
        }
        if ($runtimeEngineSubsystemCleanupLine -notmatch "snapshotSubsystemNames=.*RendererSubsystem") {
            $failures.Add("$($mode.Name): Engine lifecycle cleanup snapshot did not include RendererSubsystem")
        }
        if ($null -ne $cleanupSnapshotSubsystemTicks) {
            foreach ($tickText in ($cleanupSnapshotSubsystemTicks -split "\|")) {
                if ([int]$tickText -ne 0) {
                    $failures.Add("$($mode.Name): Engine lifecycle cleanup snapshot contained a non-zero subsystem tick count")
                    break
                }
            }
        }
    }
    if ($expectNoOpRendererBackend) {
        if (!$rendererLine) {
            $failures.Add("$($mode.Name): missing renderer stats")
        }
        else {
            $rendererPasses = Get-RegexValue -Text $rendererLine -Pattern "rendererPasses=(\d+)" -Group 1
            if ($null -eq $rendererPasses -or [int]$rendererPasses -ne 0) {
                $failures.Add("$($mode.Name): no-op renderer backend did not leave legacy renderer pass stats at zero")
            }
        }
    }
    if ($mode.PSObject.Properties.Name -contains "ExpectLegacyWorldMirror" -and $mode.ExpectLegacyWorldMirror) {
        if (!$engineWorldPreparedSceneLine) {
            $failures.Add("$($mode.Name): missing engine world prepared scene stats")
        }
        else {
            $runtimeWorldActors = Get-RegexValue -Text $engineWorldPreparedSceneLine -Pattern "runtimeWorldActors=(\d+)" -Group 1
            if ($null -eq $runtimeWorldActors -or [int]$runtimeWorldActors -le 0) {
                $failures.Add("$($mode.Name): legacy scene was not mirrored into runtime engineWorld")
            }
        }
        if (!$legacyWorldMirrorLine) {
            $failures.Add("$($mode.Name): missing engine world legacy mirror stats")
        }
        else {
            $mirrorActors = Get-RegexValue -Text $legacyWorldMirrorLine -Pattern "actors=(\d+)" -Group 1
            $meshActors = Get-RegexValue -Text $legacyWorldMirrorLine -Pattern "meshActors=(\d+)" -Group 1
            $persistentActors = Get-RegexValue -Text $legacyWorldMirrorLine -Pattern "persistentActors=(\d+)" -Group 1
            $persistentSceneComponents = Get-RegexValue -Text $legacyWorldMirrorLine -Pattern "persistentSceneComponents=(\d+)" -Group 1
            if ($null -eq $mirrorActors -or [int]$mirrorActors -le 0) {
                $failures.Add("$($mode.Name): legacy world mirror did not import any actors")
            }
            if ($null -eq $meshActors -or [int]$meshActors -le 0) {
                $failures.Add("$($mode.Name): legacy world mirror did not import any mesh actors")
            }
            if ($legacyWorldMirrorLine -notmatch "persistentIdSource=legacy-mirror") {
                $failures.Add("$($mode.Name): legacy world mirror did not report the legacy-mirror persistent id source")
            }
            if ($null -eq $persistentActors -or [int]$persistentActors -lt [int]$mirrorActors) {
                $failures.Add("$($mode.Name): legacy world mirror did not assign persistent ids to every imported actor")
            }
            if ($null -eq $persistentSceneComponents -or [int]$persistentSceneComponents -lt [int]$mirrorActors) {
                $failures.Add("$($mode.Name): legacy world mirror did not assign persistent ids to imported SceneComponents")
            }
        }
    }
    if ($mode.PSObject.Properties.Name -contains "ExpectGpuTiming" -and $mode.ExpectGpuTiming) {
        if (!$rendererLine) {
            $failures.Add("$($mode.Name): missing renderer stats")
        }
        elseif ($rendererLine -notmatch "rendererGpuTimingEnabled=yes" -or $rendererLine -notmatch "rendererGpuTimingAvailable=yes" -or $rendererLine -notmatch "rendererGpuTimingDeferredReadback=yes") {
            $failures.Add("$($mode.Name): GPU timing was not enabled and available")
        }
        else {
            $timedPasses = Get-RegexValue -Text $rendererLine -Pattern "rendererGpuTimedPasses=(\d+)" -Group 1
            $pendingQueries = Get-RegexValue -Text $rendererLine -Pattern "rendererGpuTimingPendingQueries=(\d+)" -Group 1
            $frameNs = Get-RegexValue -Text $rendererLine -Pattern "rendererGpuFrameNs=(\d+)" -Group 1
            $gbufferNs = Get-RegexValue -Text $rendererLine -Pattern "rendererGpuPbrGBufferNs=(\d+)" -Group 1
            $deferredNs = Get-RegexValue -Text $rendererLine -Pattern "rendererGpuPbrDeferredLightingNs=(\d+)" -Group 1
            if ($null -eq $timedPasses -or [int]$timedPasses -lt 3) {
                $failures.Add("$($mode.Name): GPU timing did not report enough timed passes")
            }
            if ($null -eq $pendingQueries -or [int]$pendingQueries -le 0) {
                $failures.Add("$($mode.Name): GPU timing did not leave deferred pending queries")
            }
            if ($null -eq $frameNs -or [UInt64]$frameNs -le 0) {
                $failures.Add("$($mode.Name): GPU frame time was not positive")
            }
            if ($null -eq $gbufferNs -or [UInt64]$gbufferNs -le 0) {
                $failures.Add("$($mode.Name): GPU G-buffer time was not positive")
            }
            if ($null -eq $deferredNs -or [UInt64]$deferredNs -le 0) {
                $failures.Add("$($mode.Name): GPU deferred lighting time was not positive")
            }
        }
    }
    if ($mode.PSObject.Properties.Name -contains "ExpectPointLightPressure") {
        if (!$rendererLine) {
            $failures.Add("$($mode.Name): missing renderer stats")
        }
        else {
            $expectedPressureLights = [int]$mode.ExpectPointLightPressure
            $lightBufferPointLights = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredLightBufferPointLights=(\d+)/" -Group 1
            $tiledPointLights = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredTiledLightGridPointLights=(\d+)" -Group 1
            $clusteredPointLights = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridPointLights=(\d+)" -Group 1
            $reportedPressureLights = 0
            if ($null -ne $lightBufferPointLights) {
                $reportedPressureLights = [Math]::Max($reportedPressureLights, [int]$lightBufferPointLights)
            }
            if ($null -ne $tiledPointLights) {
                $reportedPressureLights = [Math]::Max($reportedPressureLights, [int]$tiledPointLights)
            }
            if ($null -ne $clusteredPointLights) {
                $reportedPressureLights = [Math]::Max($reportedPressureLights, [int]$clusteredPointLights)
            }
            if ($reportedPressureLights -lt $expectedPressureLights) {
                $failures.Add("$($mode.Name): pressure scene did not report enough point lights ($reportedPressureLights < $expectedPressureLights)")
            }
        }
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
            $tileSize = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredTiledLightGridTileSize=(\d+)" -Group 1
            $lightCutoff = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredTiledLightGridCutoff=([0-9.]+)" -Group 1
            $gridPointLights = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredTiledLightGridPointLights=(\d+)" -Group 1
            $fullIndices = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredTiledLightGridFullIndices=(\d+)" -Group 1
            $indices = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredTiledLightGridIndices=(\d+)" -Group 1
            $culledIndices = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredTiledLightGridCulledIndices=(\d+)" -Group 1
            $occupiedTiles = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredTiledLightGridOccupiedTiles=(\d+)/(\d+)" -Group 1
            $reportedTileCount = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredTiledLightGridOccupiedTiles=(\d+)/(\d+)" -Group 2
            $pointLights = $gridPointLights
            if ($null -eq $pointLights -or [int]$pointLights -le 0) {
                $pointLights = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredLightBufferPointLights=(\d+)/" -Group 1
            }
            if ($null -eq $pointLights -or [int]$pointLights -le 0) {
                $pointLights = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredTiledLightGridMaxTileLights=(\d+)" -Group 1
            }
            if ($null -eq $columns -or $null -eq $rows -or $null -eq $tileSize -or $null -eq $lightCutoff -or $null -eq $indices -or $null -eq $occupiedTiles -or $null -eq $reportedTileCount -or $null -eq $pointLights -or $null -eq $fullIndices -or $null -eq $culledIndices) {
                $failures.Add("$($mode.Name): tiled culling stats were incomplete")
            }
            else {
                $fullGlobalLoopIndexCount = [int]$columns * [int]$rows * [int]$pointLights
                $tileCount = [int]$columns * [int]$rows
                $expectedCulledIndices = [int]$fullIndices - [int]$indices
                if ($mode.PSObject.Properties.Name -contains "ExpectTileSize" -and [int]$tileSize -ne [int]$mode.ExpectTileSize) {
                    $failures.Add("$($mode.Name): tiled light grid tile size did not match expected override ($tileSize != $($mode.ExpectTileSize))")
                }
                if ($mode.PSObject.Properties.Name -contains "ExpectLightCutoff" -and [Math]::Abs(([double]$lightCutoff) - ([double]$mode.ExpectLightCutoff)) -gt 0.0001) {
                    $failures.Add("$($mode.Name): tiled light cutoff did not match expected override ($lightCutoff != $($mode.ExpectLightCutoff))")
                }
                if ([int]$fullIndices -ne $fullGlobalLoopIndexCount) {
                    $failures.Add("$($mode.Name): tiled full index count did not match grid dimensions and point lights ($fullIndices != $fullGlobalLoopIndexCount)")
                }
                if ([int]$indices -le 0) {
                    $failures.Add("$($mode.Name): tiled light grid produced no light indices")
                }
                if ([int]$indices -ge [int]$fullIndices) {
                    $failures.Add("$($mode.Name): tiled light grid did not reduce the global point-light loop ($indices >= $fullIndices)")
                }
                if ([int]$culledIndices -ne $expectedCulledIndices) {
                    $failures.Add("$($mode.Name): tiled culled index count was inconsistent ($culledIndices != $expectedCulledIndices)")
                }
                if ([int]$culledIndices -le 0) {
                    $failures.Add("$($mode.Name): tiled light grid reported no culled indices")
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
    if ($mode.PSObject.Properties.Name -contains "ExpectUntiledFallback" -and $mode.ExpectUntiledFallback) {
        if (!$rendererLine) {
            $failures.Add("$($mode.Name): missing renderer stats")
        }
        else {
            $drawCalls = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredLightingDrawCalls=(\d+)" -Group 1
            $pointLights = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredLightBufferPointLights=(\d+)/" -Group 1
            if ($null -eq $drawCalls -or [int]$drawCalls -le 0) {
                $failures.Add("$($mode.Name): deferred lighting fallback did not draw")
            }
            if ($rendererLine -notmatch "pbrDeferredLightBufferBound=yes") {
                $failures.Add("$($mode.Name): deferred light buffer was not bound")
            }
            if ($null -eq $pointLights -or [int]$pointLights -le 0) {
                $failures.Add("$($mode.Name): deferred light buffer reported no point lights")
            }
            if ($rendererLine -notmatch "pbrDeferredTiledLightsEnabled=no") {
                $failures.Add("$($mode.Name): tiled point-light path was not disabled")
            }
            if ($rendererLine -notmatch "pbrDeferredTiledLightGridBound=no") {
                $failures.Add("$($mode.Name): tiled light grid should not be bound in untiled fallback mode")
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
    if ($mode.PSObject.Properties.Name -contains "ExpectClusteredHeatmap" -and $mode.ExpectClusteredHeatmap) {
        if (!$rendererLine) {
            $failures.Add("$($mode.Name): missing renderer stats")
        }
        else {
            $drawCalls = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightDebugDrawCalls=(\d+)" -Group 1
            $columns = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridSize=(\d+)x(\d+)x(\d+)" -Group 1
            $rows = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridSize=(\d+)x(\d+)x(\d+)" -Group 2
            $depthSlices = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridSize=(\d+)x(\d+)x(\d+)" -Group 3
            $clusters = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridClusters=(\d+)" -Group 1
            $maxLights = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridMaxLightsPerCluster=(\d+)" -Group 1
            $maxIndices = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridMaxIndices=(\d+)" -Group 1
            if ($null -eq $drawCalls -or [int]$drawCalls -ne 1) {
                $failures.Add("$($mode.Name): clustered light heatmap debug pass did not draw")
            }
            if ($rendererLine -notmatch "pbrDeferredClusteredLightGridEnabled=yes") {
                $failures.Add("$($mode.Name): clustered heatmap did not enable clustered grid")
            }
            if ($rendererLine -notmatch "pbrDeferredClusteredLightGridBound=yes") {
                $failures.Add("$($mode.Name): clustered heatmap did not bind clustered grid buffers")
            }
            if ($rendererLine -notmatch "pbrDeferredClusteredLightGridCompute=yes") {
                $failures.Add("$($mode.Name): clustered heatmap did not dispatch compute assignment")
            }
            if ($rendererLine -notmatch "pbrDeferredClusteredLightGridStatsReadback=no") {
                $failures.Add("$($mode.Name): clustered heatmap should not enable CPU stats readback")
            }
            if ($rendererLine -notmatch "pbrDeferredClusteredLightGridLightIndexStats=no") {
                $failures.Add("$($mode.Name): clustered heatmap should not require CPU light index stats")
            }
            if ($rendererLine -notmatch "pbrDeferredTiledLightsEnabled=no") {
                $failures.Add("$($mode.Name): CPU tiled lights should be disabled for clustered heatmap mode")
            }
            if ($rendererLine -notmatch "pbrDeferredTiledLightGridBound=no") {
                $failures.Add("$($mode.Name): CPU tiled grid should not bind in clustered heatmap mode")
            }
            if ($null -eq $columns -or $null -eq $rows -or $null -eq $depthSlices -or $null -eq $clusters -or $null -eq $maxLights -or $null -eq $maxIndices) {
                $failures.Add("$($mode.Name): clustered heatmap layout stats were incomplete")
            }
            else {
                $expectedClusters = [int]$columns * [int]$rows * [int]$depthSlices
                $expectedMaxIndices = $expectedClusters * [int]$maxLights
                if ([int]$clusters -ne $expectedClusters) {
                    $failures.Add("$($mode.Name): clustered heatmap count did not match dimensions ($clusters != $expectedClusters)")
                }
                if ([int]$maxIndices -ne $expectedMaxIndices) {
                    $failures.Add("$($mode.Name): clustered heatmap max index capacity did not match dimensions and capacity ($maxIndices != $expectedMaxIndices)")
                }
            }
        }
    }
    if ($mode.PSObject.Properties.Name -contains "ExpectClusteredLayout" -and $mode.ExpectClusteredLayout) {
        if (!$rendererLine) {
            $failures.Add("$($mode.Name): missing renderer stats")
        }
        else {
            $drawCalls = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredLightingDrawCalls=(\d+)" -Group 1
            $columns = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridSize=(\d+)x(\d+)x(\d+)" -Group 1
            $rows = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridSize=(\d+)x(\d+)x(\d+)" -Group 2
            $depthSlices = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridSize=(\d+)x(\d+)x(\d+)" -Group 3
            $tileSize = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridTileSize=(\d+)" -Group 1
            $clusters = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridClusters=(\d+)" -Group 1
            $maxLights = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridMaxLightsPerCluster=(\d+)" -Group 1
            $maxIndices = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridMaxIndices=(\d+)" -Group 1
            $pointLights = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridPointLights=(\d+)" -Group 1
            if ($null -eq $drawCalls -or [int]$drawCalls -le 0) {
                $failures.Add("$($mode.Name): deferred lighting did not draw")
            }
            if ($rendererLine -notmatch "pbrDeferredClusteredLightGridEnabled=yes") {
                $failures.Add("$($mode.Name): clustered layout stats were not enabled")
            }
            if ($rendererLine -notmatch "pbrDeferredClusteredLightGridBound=no") {
                $failures.Add("$($mode.Name): clustered layout-only mode should not bind GPU clustered buffers")
            }
            if ($rendererLine -notmatch "pbrDeferredTiledLightsEnabled=no") {
                $failures.Add("$($mode.Name): CPU tiled lights should be disabled for clustered layout-only mode")
            }
            if ($null -eq $columns -or $null -eq $rows -or $null -eq $depthSlices -or $null -eq $tileSize -or $null -eq $clusters -or $null -eq $maxLights -or $null -eq $maxIndices -or $null -eq $pointLights) {
                $failures.Add("$($mode.Name): clustered layout stats were incomplete")
            }
            else {
                $expectedClusters = [int]$columns * [int]$rows * [int]$depthSlices
                $expectedMaxIndices = $expectedClusters * [int]$maxLights
                if ([int]$tileSize -ne 16) {
                    $failures.Add("$($mode.Name): clustered tile size did not match default profile ($tileSize != 16)")
                }
                if ([int]$depthSlices -ne 24) {
                    $failures.Add("$($mode.Name): clustered depth slices did not match default profile ($depthSlices != 24)")
                }
                if ([int]$maxLights -ne 64) {
                    $failures.Add("$($mode.Name): clustered max lights per cluster did not match default profile ($maxLights != 64)")
                }
                if ([int]$clusters -ne $expectedClusters) {
                    $failures.Add("$($mode.Name): clustered count did not match dimensions ($clusters != $expectedClusters)")
                }
                if ([int]$maxIndices -ne $expectedMaxIndices) {
                    $failures.Add("$($mode.Name): clustered max index capacity did not match dimensions and capacity ($maxIndices != $expectedMaxIndices)")
                }
                if ([int]$pointLights -le 0) {
                    $failures.Add("$($mode.Name): clustered layout reported no point lights")
                }
            }
        }
    }
    if ($mode.PSObject.Properties.Name -contains "ExpectClusteredGrid" -and $mode.ExpectClusteredGrid) {
        if (!$rendererLine) {
            $failures.Add("$($mode.Name): missing renderer stats")
        }
        else {
            $drawCalls = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredLightingDrawCalls=(\d+)" -Group 1
            $columns = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridSize=(\d+)x(\d+)x(\d+)" -Group 1
            $rows = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridSize=(\d+)x(\d+)x(\d+)" -Group 2
            $depthSlices = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridSize=(\d+)x(\d+)x(\d+)" -Group 3
            $tileSize = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridTileSize=(\d+)" -Group 1
            $clusters = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridClusters=(\d+)" -Group 1
            $maxLights = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridMaxLightsPerCluster=(\d+)" -Group 1
            $maxIndices = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridMaxIndices=(\d+)" -Group 1
            $pointLights = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridPointLights=(\d+)" -Group 1
            $indices = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridIndices=(\d+)" -Group 1
            $culledIndices = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridCulledIndices=(\d+)" -Group 1
            if ($null -eq $drawCalls -or [int]$drawCalls -le 0) {
                $failures.Add("$($mode.Name): deferred lighting did not draw")
            }
            if ($rendererLine -notmatch "pbrDeferredClusteredLightGridEnabled=yes") {
                $failures.Add("$($mode.Name): clustered grid was not enabled")
            }
            if ($rendererLine -notmatch "pbrDeferredClusteredLightGridBound=yes") {
                $failures.Add("$($mode.Name): clustered grid buffers were not bound")
            }
            if ($rendererLine -notmatch "pbrDeferredClusteredLightGridCompute=yes") {
                $failures.Add("$($mode.Name): clustered grid compute shader was not dispatched")
            }
            if ($rendererLine -notmatch "pbrDeferredClusteredLightGridStatsReadback=yes") {
                $failures.Add("$($mode.Name): clustered grid verification did not enable stats readback")
            }
            if ($rendererLine -notmatch "pbrDeferredClusteredLightGridLightIndexStats=yes") {
                $failures.Add("$($mode.Name): clustered grid light index stats were not available")
            }
            if ($rendererLine -notmatch "pbrDeferredTiledLightsEnabled=no") {
                $failures.Add("$($mode.Name): CPU tiled lights should be disabled for clustered grid mode")
            }
            if ($rendererLine -notmatch "pbrDeferredTiledLightGridBound=no") {
                $failures.Add("$($mode.Name): CPU tiled grid should not bind in clustered grid mode")
            }
            if ($null -eq $columns -or $null -eq $rows -or $null -eq $depthSlices -or $null -eq $tileSize -or $null -eq $clusters -or $null -eq $maxLights -or $null -eq $maxIndices -or $null -eq $pointLights -or $null -eq $indices -or $null -eq $culledIndices) {
                $failures.Add("$($mode.Name): clustered grid stats were incomplete")
            }
            else {
                $expectedClusters = [int]$columns * [int]$rows * [int]$depthSlices
                $expectedMaxIndices = $expectedClusters * [int]$maxLights
                $fullGlobalLoopIndexCount = $expectedClusters * [int]$pointLights
                $expectedCulledIndices = $fullGlobalLoopIndexCount - [int]$indices
                if ([int]$tileSize -ne 16) {
                    $failures.Add("$($mode.Name): clustered tile size did not match default profile ($tileSize != 16)")
                }
                if ([int]$depthSlices -ne 24) {
                    $failures.Add("$($mode.Name): clustered depth slices did not match default profile ($depthSlices != 24)")
                }
                if ([int]$maxLights -ne 64) {
                    $failures.Add("$($mode.Name): clustered max lights per cluster did not match default profile ($maxLights != 64)")
                }
                if ([int]$clusters -ne $expectedClusters) {
                    $failures.Add("$($mode.Name): clustered count did not match dimensions ($clusters != $expectedClusters)")
                }
                if ([int]$maxIndices -ne $expectedMaxIndices) {
                    $failures.Add("$($mode.Name): clustered max index capacity did not match dimensions and capacity ($maxIndices != $expectedMaxIndices)")
                }
                if ([int]$pointLights -le 0) {
                    $failures.Add("$($mode.Name): clustered grid reported no point lights")
                }
                if ([int]$indices -le 0) {
                    $failures.Add("$($mode.Name): clustered grid produced no light indices")
                }
                if ([int]$indices -ge $fullGlobalLoopIndexCount) {
                    $failures.Add("$($mode.Name): clustered grid did not reduce the global point-light loop ($indices >= $fullGlobalLoopIndexCount)")
                }
                if ([int]$culledIndices -ne $expectedCulledIndices) {
                    $failures.Add("$($mode.Name): clustered culled index count was inconsistent ($culledIndices != $expectedCulledIndices)")
                }
                if ([int]$culledIndices -le 0) {
                    $failures.Add("$($mode.Name): clustered grid reported no culled indices")
                }
            }
        }
    }
    if ($mode.PSObject.Properties.Name -contains "ExpectClusteredGridNoReadback" -and $mode.ExpectClusteredGridNoReadback) {
        if (!$rendererLine) {
            $failures.Add("$($mode.Name): missing renderer stats")
        }
        else {
            $drawCalls = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredLightingDrawCalls=(\d+)" -Group 1
            $columns = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridSize=(\d+)x(\d+)x(\d+)" -Group 1
            $rows = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridSize=(\d+)x(\d+)x(\d+)" -Group 2
            $depthSlices = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridSize=(\d+)x(\d+)x(\d+)" -Group 3
            $clusters = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridClusters=(\d+)" -Group 1
            $maxLights = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridMaxLightsPerCluster=(\d+)" -Group 1
            $maxIndices = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredClusteredLightGridMaxIndices=(\d+)" -Group 1
            if ($null -eq $drawCalls -or [int]$drawCalls -le 0) {
                $failures.Add("$($mode.Name): deferred lighting did not draw")
            }
            if ($rendererLine -notmatch "pbrDeferredClusteredLightGridEnabled=yes") {
                $failures.Add("$($mode.Name): clustered grid was not enabled")
            }
            if ($rendererLine -notmatch "pbrDeferredClusteredLightGridBound=yes") {
                $failures.Add("$($mode.Name): clustered grid buffers were not bound")
            }
            if ($rendererLine -notmatch "pbrDeferredClusteredLightGridCompute=yes") {
                $failures.Add("$($mode.Name): clustered grid compute shader was not dispatched")
            }
            if ($rendererLine -notmatch "pbrDeferredClusteredLightGridStatsReadback=no") {
                $failures.Add("$($mode.Name): clustered grid no-readback mode unexpectedly enabled stats readback")
            }
            if ($rendererLine -notmatch "pbrDeferredClusteredLightGridLightIndexStats=no") {
                $failures.Add("$($mode.Name): clustered grid no-readback mode unexpectedly reported light index stats")
            }
            if ($rendererLine -notmatch "pbrDeferredTiledLightsEnabled=no") {
                $failures.Add("$($mode.Name): CPU tiled lights should be disabled for clustered no-readback mode")
            }
            if ($rendererLine -notmatch "pbrDeferredTiledLightGridBound=no") {
                $failures.Add("$($mode.Name): CPU tiled grid should not bind in clustered no-readback mode")
            }
            if ($null -eq $columns -or $null -eq $rows -or $null -eq $depthSlices -or $null -eq $clusters -or $null -eq $maxLights -or $null -eq $maxIndices) {
                $failures.Add("$($mode.Name): clustered grid no-readback layout stats were incomplete")
            }
            else {
                $expectedClusters = [int]$columns * [int]$rows * [int]$depthSlices
                $expectedMaxIndices = $expectedClusters * [int]$maxLights
                if ([int]$clusters -ne $expectedClusters) {
                    $failures.Add("$($mode.Name): clustered count did not match dimensions ($clusters != $expectedClusters)")
                }
                if ([int]$maxIndices -ne $expectedMaxIndices) {
                    $failures.Add("$($mode.Name): clustered max index capacity did not match dimensions and capacity ($maxIndices != $expectedMaxIndices)")
                }
            }
        }
    }
    if ($mode.PSObject.Properties.Name -contains "ExpectTexturedProbe" -and $mode.ExpectTexturedProbe) {
        if (!$sceneLine) {
            $failures.Add("$($mode.Name): missing scene stats")
        }
        else {
            $texturedMeshes = Get-RegexValue -Text $sceneLine -Pattern "pbrTexturedMeshes=(\d+)" -Group 1
            if ($null -eq $texturedMeshes -or [int]$texturedMeshes -le 0) {
                $failures.Add("$($mode.Name): textured PBR probe was not added")
            }
        }
        if (!$rendererLine) {
            $failures.Add("$($mode.Name): missing renderer stats")
        }
        else {
            $pbrDrawCalls = Get-RegexValue -Text $rendererLine -Pattern "pbrDrawCalls=(\d+)" -Group 1
            $pbrGBufferDrawCalls = Get-RegexValue -Text $rendererLine -Pattern "pbrGBufferDrawCalls=(\d+)" -Group 1
            $pbrDrawPathCalls = 0
            if ($null -ne $pbrDrawCalls) {
                $pbrDrawPathCalls = [Math]::Max($pbrDrawPathCalls, [int]$pbrDrawCalls)
            }
            if ($null -ne $pbrGBufferDrawCalls) {
                $pbrDrawPathCalls = [Math]::Max($pbrDrawPathCalls, [int]$pbrGBufferDrawCalls)
            }
            if ($pbrDrawPathCalls -lt 26) {
                $failures.Add("$($mode.Name): textured PBR probe did not increase a PBR draw path")
            }
        }
    }
    if ($mode.PSObject.Properties.Name -contains "ExpectImportedAssetProbe" -and $mode.ExpectImportedAssetProbe) {
        if (!$sceneLine) {
            $failures.Add("$($mode.Name): missing scene stats")
        }
        else {
            $importedMeshes = Get-RegexValue -Text $sceneLine -Pattern "pbrImportedMeshes=(\d+)" -Group 1
            if ($null -eq $importedMeshes -or [int]$importedMeshes -le 0) {
                $failures.Add("$($mode.Name): imported PBR asset probe was not added to the render scene")
            }
        }
        if (!$engineWorldPreparedSceneLine) {
            $failures.Add("$($mode.Name): missing engine world prepared scene stats")
        }
        else {
            $runtimeWorldActors = Get-RegexValue -Text $engineWorldPreparedSceneLine -Pattern "runtimeWorldActors=(\d+)" -Group 1
            if ($null -eq $runtimeWorldActors -or [int]$runtimeWorldActors -le 0) {
                $failures.Add("$($mode.Name): imported PBR asset probe was not retained in runtime engineWorld")
            }
        }
        if (!$importedAssetWorldImportLine) {
            $failures.Add("$($mode.Name): missing imported asset engine world import stats")
        }
        else {
            $importedActors = Get-RegexValue -Text $importedAssetWorldImportLine -Pattern "actors=(\d+)" -Group 1
            $meshActors = Get-RegexValue -Text $importedAssetWorldImportLine -Pattern "meshActors=(\d+)" -Group 1
            $persistentActors = Get-RegexValue -Text $importedAssetWorldImportLine -Pattern "persistentActors=(\d+)" -Group 1
            $persistentSceneComponents = Get-RegexValue -Text $importedAssetWorldImportLine -Pattern "persistentSceneComponents=(\d+)" -Group 1
            $assetHandles = Get-RegexValue -Text $importedAssetWorldImportLine -Pattern "assetHandles=(\d+)" -Group 1
            $meshAssetHandles = Get-RegexValue -Text $importedAssetWorldImportLine -Pattern "meshAssetHandles=(\d+)" -Group 1
            $materialAssetHandles = Get-RegexValue -Text $importedAssetWorldImportLine -Pattern "materialAssetHandles=(\d+)" -Group 1
            $textureAssetHandles = Get-RegexValue -Text $importedAssetWorldImportLine -Pattern "textureAssetHandles=(\d+)" -Group 1
            if ($importedAssetWorldImportLine -notmatch "persistentIdSource=imported-asset") {
                $failures.Add("$($mode.Name): imported asset engine world import did not report the imported-asset persistent id source")
            }
            if ($importedAssetWorldImportLine -notmatch "assetHandleSource=imported-asset") {
                $failures.Add("$($mode.Name): imported asset engine world import did not report imported-asset handle source")
            }
            if ($null -eq $importedActors -or [int]$importedActors -le 0) {
                $failures.Add("$($mode.Name): imported asset engine world import did not create actors")
            }
            if ($null -eq $meshActors -or [int]$meshActors -le 0) {
                $failures.Add("$($mode.Name): imported asset engine world import did not create mesh actors")
            }
            if ($null -eq $persistentActors -or $null -eq $importedActors -or [int]$persistentActors -lt [int]$importedActors) {
                $failures.Add("$($mode.Name): imported asset engine world import did not assign persistent ids to every actor")
            }
            if ($null -eq $persistentSceneComponents -or $null -eq $importedActors -or [int]$persistentSceneComponents -lt [int]$importedActors) {
                $failures.Add("$($mode.Name): imported asset engine world import did not assign persistent ids to imported SceneComponents")
            }
            if ($null -eq $assetHandles -or [int]$assetHandles -le 0) {
                $failures.Add("$($mode.Name): imported asset engine world import did not register stable asset handles")
            }
            if ($null -eq $meshAssetHandles -or [int]$meshAssetHandles -le 0) {
                $failures.Add("$($mode.Name): imported asset engine world import did not register mesh asset handles")
            }
            if ($null -eq $materialAssetHandles -or [int]$materialAssetHandles -le 0) {
                $failures.Add("$($mode.Name): imported asset engine world import did not register material asset handles")
            }
            if ($null -eq $textureAssetHandles -or [int]$textureAssetHandles -le 0) {
                $failures.Add("$($mode.Name): imported asset engine world import did not register texture asset handles")
            }
        }
        if (!$runtimeAssetRegistryLine) {
            $failures.Add("$($mode.Name): missing runtime asset registry stats")
        }
        else {
            $registryAssets = Get-RegexValue -Text $runtimeAssetRegistryLine -Pattern "assets=(\d+)" -Group 1
            $registryMeshAssets = Get-RegexValue -Text $runtimeAssetRegistryLine -Pattern "meshAssets=(\d+)" -Group 1
            $registryMaterialAssets = Get-RegexValue -Text $runtimeAssetRegistryLine -Pattern "materialAssets=(\d+)" -Group 1
            $registryTextureAssets = Get-RegexValue -Text $runtimeAssetRegistryLine -Pattern "textureAssets=(\d+)" -Group 1
            $registryImportedHandles = Get-RegexValue -Text $runtimeAssetRegistryLine -Pattern "importedAssetHandles=(\d+)" -Group 1
            if ($null -eq $registryAssets -or [int]$registryAssets -le 0) {
                $failures.Add("$($mode.Name): runtime asset registry did not retain registered assets")
            }
            if ($null -eq $registryMeshAssets -or [int]$registryMeshAssets -le 0) {
                $failures.Add("$($mode.Name): runtime asset registry did not expose mesh assets")
            }
            if ($null -eq $registryMaterialAssets -or [int]$registryMaterialAssets -le 0) {
                $failures.Add("$($mode.Name): runtime asset registry did not expose material assets")
            }
            if ($null -eq $registryTextureAssets -or [int]$registryTextureAssets -le 0) {
                $failures.Add("$($mode.Name): runtime asset registry did not expose texture assets")
            }
            if ($null -eq $registryImportedHandles -or [int]$registryImportedHandles -le 0) {
                $failures.Add("$($mode.Name): runtime asset registry did not expose imported asset handles")
            }
        }
        if (!$importedAssetScenePackageLine) {
            $failures.Add("$($mode.Name): missing imported asset scene package stats")
        }
        else {
            $assetManifest = Get-RegexValue -Text $importedAssetScenePackageLine -Pattern "assetManifest=(\d+)" -Group 1
            $meshAssets = Get-RegexValue -Text $importedAssetScenePackageLine -Pattern "meshAssets=(\d+)" -Group 1
            $materialAssets = Get-RegexValue -Text $importedAssetScenePackageLine -Pattern "materialAssets=(\d+)" -Group 1
            $textureAssets = Get-RegexValue -Text $importedAssetScenePackageLine -Pattern "textureAssets=(\d+)" -Group 1
            $loadedAssetManifest = Get-RegexValue -Text $importedAssetScenePackageLine -Pattern "loadedAssetManifest=(\d+)" -Group 1
            $loadedMeshAssets = Get-RegexValue -Text $importedAssetScenePackageLine -Pattern "loadedMeshAssets=(\d+)" -Group 1
            $loadedMaterialAssets = Get-RegexValue -Text $importedAssetScenePackageLine -Pattern "loadedMaterialAssets=(\d+)" -Group 1
            $loadedTextureAssets = Get-RegexValue -Text $importedAssetScenePackageLine -Pattern "loadedTextureAssets=(\d+)" -Group 1
            $registeredAssetManifest = Get-RegexValue -Text $importedAssetScenePackageLine -Pattern "registeredAssetManifest=(\d+)" -Group 1
            $registeredMeshAssets = Get-RegexValue -Text $importedAssetScenePackageLine -Pattern "registeredMeshAssets=(\d+)" -Group 1
            $registeredMaterialAssets = Get-RegexValue -Text $importedAssetScenePackageLine -Pattern "registeredMaterialAssets=(\d+)" -Group 1
            $registeredTextureAssets = Get-RegexValue -Text $importedAssetScenePackageLine -Pattern "registeredTextureAssets=(\d+)" -Group 1
            $importedAssetPackagePath = Get-RegexValue -Text $importedAssetScenePackageLine -Pattern "path=([^,]+)" -Group 1
            if ($importedAssetScenePackageLine -notmatch "saved=yes") {
                $failures.Add("$($mode.Name): imported asset scene package was not saved")
            }
            if ($importedAssetScenePackageLine -notmatch "loaded=yes") {
                $failures.Add("$($mode.Name): imported asset scene package was not loaded")
            }
            if ($null -eq $assetManifest -or [int]$assetManifest -le 0) {
                $failures.Add("$($mode.Name): imported asset scene package did not persist asset manifest")
            }
            if ($null -eq $meshAssets -or [int]$meshAssets -le 0) {
                $failures.Add("$($mode.Name): imported asset scene package did not persist mesh assets")
            }
            if ($null -eq $materialAssets -or [int]$materialAssets -le 0) {
                $failures.Add("$($mode.Name): imported asset scene package did not persist material assets")
            }
            if ($null -eq $textureAssets -or [int]$textureAssets -le 0) {
                $failures.Add("$($mode.Name): imported asset scene package did not persist texture assets")
            }
            if ($null -eq $loadedAssetManifest -or $null -eq $assetManifest -or [int]$loadedAssetManifest -lt [int]$assetManifest) {
                $failures.Add("$($mode.Name): imported asset scene package did not reload asset manifest")
            }
            if ($null -eq $loadedMeshAssets -or [int]$loadedMeshAssets -lt 1) {
                $failures.Add("$($mode.Name): imported asset scene package did not reload mesh assets")
            }
            if ($null -eq $loadedMaterialAssets -or [int]$loadedMaterialAssets -lt 1) {
                $failures.Add("$($mode.Name): imported asset scene package did not reload material assets")
            }
            if ($null -eq $loadedTextureAssets -or [int]$loadedTextureAssets -lt 1) {
                $failures.Add("$($mode.Name): imported asset scene package did not reload texture assets")
            }
            if ($null -eq $registeredAssetManifest -or $null -eq $assetManifest -or [int]$registeredAssetManifest -lt [int]$assetManifest) {
                $failures.Add("$($mode.Name): imported asset scene package did not register reloaded asset manifest into runtime registry")
            }
            if ($null -eq $registeredMeshAssets -or [int]$registeredMeshAssets -lt 1) {
                $failures.Add("$($mode.Name): imported asset scene package did not register mesh manifest assets")
            }
            if ($null -eq $registeredMaterialAssets -or [int]$registeredMaterialAssets -lt 1) {
                $failures.Add("$($mode.Name): imported asset scene package did not register material manifest assets")
            }
            if ($null -eq $registeredTextureAssets -or [int]$registeredTextureAssets -lt 1) {
                $failures.Add("$($mode.Name): imported asset scene package did not register texture manifest assets")
            }

            $importedAssetPackageFullPath = $null
            if ($null -ne $importedAssetPackagePath) {
                $importedAssetPackageFullPath = Join-Path $repoRoot $importedAssetPackagePath
            }
            if ($null -eq $importedAssetPackageFullPath -or !(Test-Path $importedAssetPackageFullPath)) {
                $failures.Add("$($mode.Name): imported asset scene package file was not written")
            }
            else {
                if (!(Select-String -Path $importedAssetPackageFullPath -Pattern "assetManifest\.\d+\.handle=asset:mesh:imported-asset:" -Quiet)) {
                    $failures.Add("$($mode.Name): imported asset scene package did not write imported mesh handles")
                }
                if (!(Select-String -Path $importedAssetPackageFullPath -Pattern "assetManifest\.\d+\.handle=asset:material:imported-asset:" -Quiet)) {
                    $failures.Add("$($mode.Name): imported asset scene package did not write imported material handles")
                }
                if (!(Select-String -Path $importedAssetPackageFullPath -Pattern "assetManifest\.\d+\.handle=asset:texture:imported-asset:" -Quiet)) {
                    $failures.Add("$($mode.Name): imported asset scene package did not write imported texture handles")
                }
            }
        }
    }
    if ($mode.PSObject.Properties.Name -contains "ExpectShowcaseSpheres") {
        if (!$sceneLine) {
            $failures.Add("$($mode.Name): missing scene stats")
        }
        else {
            $expectedShowcaseSpheres = [int]$mode.ExpectShowcaseSpheres
            $showcaseSpheres = Get-RegexValue -Text $sceneLine -Pattern "pbrShowcaseSpheres=(\d+)" -Group 1
            if ($null -eq $showcaseSpheres -or [int]$showcaseSpheres -lt $expectedShowcaseSpheres) {
                $failures.Add("$($mode.Name): showcase sphere scene did not add enough PBR spheres ($showcaseSpheres < $expectedShowcaseSpheres)")
            }
        }
        if (!$rendererLine) {
            $failures.Add("$($mode.Name): missing renderer stats")
        }
        else {
            $pbrGBufferDrawCalls = Get-RegexValue -Text $rendererLine -Pattern "pbrGBufferDrawCalls=(\d+)" -Group 1
            if ($null -eq $pbrGBufferDrawCalls -or [int]$pbrGBufferDrawCalls -lt 26) {
                $failures.Add("$($mode.Name): showcase sphere scene did not render through the PBR G-buffer path")
            }
        }
    }
    if ($mode.PSObject.Properties.Name -contains "ExpectEngineWorldSceneProbe" -and $mode.ExpectEngineWorldSceneProbe) {
        if (!$engineWorldPreparedSceneLine) {
            $failures.Add("$($mode.Name): missing engine world prepared scene stats")
        }
        else {
            $engineWorldMeshes = Get-RegexValue -Text $engineWorldPreparedSceneLine -Pattern "engineWorldProbeMeshes=(\d+)" -Group 1
            $runtimeWorldActors = Get-RegexValue -Text $engineWorldPreparedSceneLine -Pattern "runtimeWorldActors=(\d+)" -Group 1
            if ($null -eq $engineWorldMeshes -or [int]$engineWorldMeshes -le 0) {
                $failures.Add("$($mode.Name): engine world scene probe mesh was not exported into the render scene")
            }
            if ($null -eq $runtimeWorldActors -or [int]$runtimeWorldActors -lt 2) {
                $failures.Add("$($mode.Name): engine world scene probe was not retained in the runtime World")
            }
        }
        if (!$engineWorldProbeLine) {
            $failures.Add("$($mode.Name): missing engine world scene probe bridge stats")
        }
        else {
            $worldActors = Get-RegexValue -Text $engineWorldProbeLine -Pattern "actors=(\d+)" -Group 1
            $sceneComponents = Get-RegexValue -Text $engineWorldProbeLine -Pattern "sceneComponents=(\d+)" -Group 1
            $exportedObjects = Get-RegexValue -Text $engineWorldProbeLine -Pattern "exportedObjects=(\d+)" -Group 1
            $exportedMeshes = Get-RegexValue -Text $engineWorldProbeLine -Pattern "exportedMeshes=(\d+)" -Group 1
            $sceneRootObjects = Get-RegexValue -Text $engineWorldProbeLine -Pattern "sceneRootObjects=(\d+)" -Group 1
            $objectAttachments = Get-RegexValue -Text $engineWorldProbeLine -Pattern "objectAttachments=(\d+)" -Group 1
            if ($null -eq $worldActors -or [int]$worldActors -lt 2) {
                $failures.Add("$($mode.Name): engine world probe did not visit enough actors")
            }
            if ($null -eq $sceneComponents -or [int]$sceneComponents -lt 2) {
                $failures.Add("$($mode.Name): engine world probe did not visit enough scene components")
            }
            if ($null -eq $exportedObjects -or [int]$exportedObjects -lt 2) {
                $failures.Add("$($mode.Name): engine world probe did not export enough objects")
            }
            if ($null -eq $exportedMeshes -or [int]$exportedMeshes -lt 1) {
                $failures.Add("$($mode.Name): engine world probe did not export a mesh")
            }
            if ($null -eq $sceneRootObjects -or [int]$sceneRootObjects -lt 1) {
                $failures.Add("$($mode.Name): engine world probe did not export a scene root object")
            }
            if ($null -eq $objectAttachments -or [int]$objectAttachments -lt 1) {
                $failures.Add("$($mode.Name): engine world probe did not preserve object attachment")
            }
        }
        if (!$rendererLine) {
            $failures.Add("$($mode.Name): missing renderer stats")
        }
        else {
            $pbrGBufferDrawCalls = Get-RegexValue -Text $rendererLine -Pattern "pbrGBufferDrawCalls=(\d+)" -Group 1
            if ($null -eq $pbrGBufferDrawCalls -or [int]$pbrGBufferDrawCalls -lt 26) {
                $failures.Add("$($mode.Name): engine world scene probe did not render through the PBR G-buffer path")
            }
        }
    }
    if ($mode.PSObject.Properties.Name -contains "ExpectEngineWorldMinimalScene" -and $mode.ExpectEngineWorldMinimalScene) {
        if (!$sceneLine) {
            $failures.Add("$($mode.Name): missing scene stats")
        }
        else {
            $pbrPreviewMeshes = Get-RegexValue -Text $sceneLine -Pattern "pbrPreviewMeshes=(\d+)" -Group 1
            if ($null -eq $pbrPreviewMeshes -or [int]$pbrPreviewMeshes -ne 0) {
                $failures.Add("$($mode.Name): engine world minimal scene should not include the legacy PBR preview grid")
            }
        }
        if (!$engineWorldPreparedSceneLine) {
            $failures.Add("$($mode.Name): missing engine world prepared scene stats")
        }
        else {
            $engineWorldMeshes = Get-RegexValue -Text $engineWorldPreparedSceneLine -Pattern "engineWorldMinimalMeshes=(\d+)" -Group 1
            $runtimeWorldActors = Get-RegexValue -Text $engineWorldPreparedSceneLine -Pattern "runtimeWorldActors=(\d+)" -Group 1
            if ($null -eq $engineWorldMeshes -or [int]$engineWorldMeshes -lt 4) {
                $failures.Add("$($mode.Name): engine world minimal scene did not export enough PBR meshes ($engineWorldMeshes < 4)")
            }
            if ($null -eq $runtimeWorldActors -or [int]$runtimeWorldActors -lt 5) {
                $failures.Add("$($mode.Name): engine world minimal scene was not retained in the runtime World")
            }
        }
        if (!$engineWorldMinimalLine) {
            $failures.Add("$($mode.Name): missing engine world minimal scene bridge stats")
        }
        else {
            $worldActors = Get-RegexValue -Text $engineWorldMinimalLine -Pattern "actors=(\d+)" -Group 1
            $sceneComponents = Get-RegexValue -Text $engineWorldMinimalLine -Pattern "sceneComponents=(\d+)" -Group 1
            $exportedObjects = Get-RegexValue -Text $engineWorldMinimalLine -Pattern "exportedObjects=(\d+)" -Group 1
            $exportedMeshes = Get-RegexValue -Text $engineWorldMinimalLine -Pattern "exportedMeshes=(\d+)" -Group 1
            $sceneRootObjects = Get-RegexValue -Text $engineWorldMinimalLine -Pattern "sceneRootObjects=(\d+)" -Group 1
            $objectAttachments = Get-RegexValue -Text $engineWorldMinimalLine -Pattern "objectAttachments=(\d+)" -Group 1
            $createdActors = Get-RegexValue -Text $engineWorldMinimalLine -Pattern "createdActors=(\d+)" -Group 1
            $createdMeshes = Get-RegexValue -Text $engineWorldMinimalLine -Pattern "createdMeshes=(\d+)" -Group 1
            if ($null -eq $worldActors -or [int]$worldActors -lt 5) {
                $failures.Add("$($mode.Name): engine world minimal scene did not visit enough actors")
            }
            if ($null -eq $sceneComponents -or [int]$sceneComponents -lt 5) {
                $failures.Add("$($mode.Name): engine world minimal scene did not visit enough scene components")
            }
            if ($null -eq $exportedObjects -or [int]$exportedObjects -lt 5) {
                $failures.Add("$($mode.Name): engine world minimal scene did not export enough objects")
            }
            if ($null -eq $exportedMeshes -or [int]$exportedMeshes -lt 4) {
                $failures.Add("$($mode.Name): engine world minimal scene did not export enough meshes")
            }
            if ($null -eq $sceneRootObjects -or [int]$sceneRootObjects -lt 1) {
                $failures.Add("$($mode.Name): engine world minimal scene did not export a scene root object")
            }
            if ($null -eq $objectAttachments -or [int]$objectAttachments -lt 4) {
                $failures.Add("$($mode.Name): engine world minimal scene did not preserve mesh attachments")
            }
            if ($null -eq $createdActors -or [int]$createdActors -lt 5) {
                $failures.Add("$($mode.Name): engine world minimal scene did not create enough actors")
            }
            if ($null -eq $createdMeshes -or [int]$createdMeshes -lt 4) {
                $failures.Add("$($mode.Name): engine world minimal scene did not create enough meshes")
            }
        }
        if (!$rendererLine) {
            $failures.Add("$($mode.Name): missing renderer stats")
        }
        else {
            $pbrGBufferDrawCalls = Get-RegexValue -Text $rendererLine -Pattern "pbrGBufferDrawCalls=(\d+)" -Group 1
            if ($null -eq $pbrGBufferDrawCalls -or [int]$pbrGBufferDrawCalls -lt 4) {
                $failures.Add("$($mode.Name): engine world minimal scene did not render enough meshes through the PBR G-buffer path")
            }
        }
    }
    if ($mode.PSObject.Properties.Name -contains "ExpectEngineWorldEditorCreate" -and $mode.ExpectEngineWorldEditorCreate) {
        if (!$engineWorldPreparedSceneLine) {
            $failures.Add("$($mode.Name): missing engine world prepared scene stats")
        }
        else {
            $runtimeWorldActors = Get-RegexValue -Text $engineWorldPreparedSceneLine -Pattern "runtimeWorldActors=(\d+)" -Group 1
            if ($null -eq $runtimeWorldActors -or [int]$runtimeWorldActors -lt 6) {
                $failures.Add("$($mode.Name): editor-created actor was not retained in runtime World")
            }
        }
        if (!$engineWorldEditorCreateLine) {
            $failures.Add("$($mode.Name): missing engine world editor create stats")
        }
        else {
            $editorCreatedActors = Get-RegexValue -Text $engineWorldEditorCreateLine -Pattern "editorCreatedActors=(\d+)" -Group 1
            $editorCreatedSceneComponents = Get-RegexValue -Text $engineWorldEditorCreateLine -Pattern "editorCreatedSceneComponents=(\d+)" -Group 1
            $actorPersistentId = Get-RegexValue -Text $engineWorldEditorCreateLine -Pattern "actorPersistentId=([^,]+)" -Group 1
            $componentPersistentId = Get-RegexValue -Text $engineWorldEditorCreateLine -Pattern "componentPersistentId=([^,]+)" -Group 1
            if ($engineWorldEditorCreateLine -notmatch "created=yes") {
                $failures.Add("$($mode.Name): editor-created actor creation did not succeed")
            }
            if ($null -eq $editorCreatedActors -or [int]$editorCreatedActors -le 0) {
                $failures.Add("$($mode.Name): editor-created actor count was not reported")
            }
            if ($null -eq $editorCreatedSceneComponents -or [int]$editorCreatedSceneComponents -le 0) {
                $failures.Add("$($mode.Name): editor-created SceneComponent count was not reported")
            }
            if ($null -eq $actorPersistentId -or $actorPersistentId -notmatch "^actor:editor-created:") {
                $failures.Add("$($mode.Name): editor-created actor persistent id did not use editor-created source")
            }
            if ($null -eq $componentPersistentId -or $componentPersistentId -notmatch "^component:editor-created:") {
                $failures.Add("$($mode.Name): editor-created SceneComponent persistent id did not use editor-created source")
            }
        }
    }
    if ($mode.PSObject.Properties.Name -contains "ExpectEngineWorldScenePackage" -and $mode.ExpectEngineWorldScenePackage) {
        if (!$engineWorldScenePackageLine) {
            $failures.Add("$($mode.Name): missing engine world scene package stats")
        }
        else {
            $packageActors = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "actors=(\d+)" -Group 1
            $packageSceneComponents = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "sceneComponents=(\d+)" -Group 1
            $typedActors = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "typedActors=(\d+)" -Group 1
            $typedSceneComponents = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "typedSceneComponents=(\d+)" -Group 1
            $adapterDescriptors = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "adapterDescriptors=(\d+)" -Group 1
            $assetHandles = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "assetHandles=(\d+)" -Group 1
            $loadedActors = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "loadedActors=(\d+)" -Group 1
            $loadedSceneComponents = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "loadedSceneComponents=(\d+)" -Group 1
            $loadedTypedActors = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "loadedTypedActors=(\d+)" -Group 1
            $loadedActorsWithPersistentIds = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "loadedActorsWithPersistentIds=(\d+)" -Group 1
            $loadedSceneComponentsWithPersistentIds = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "loadedSceneComponentsWithPersistentIds=(\d+)" -Group 1
            $loadedTypedSceneComponents = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "loadedTypedSceneComponents=(\d+)" -Group 1
            $packageUnknownActorTypes = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "unknownActorTypes=(\d+)" -Group 1
            $packageUnknownSceneComponentTypes = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "unknownSceneComponentTypes=(\d+)" -Group 1
            $packageInvalidSceneComponentTransforms = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "invalidSceneComponentTransforms=(\d+)" -Group 1
            $packageDuplicateActorPersistentIds = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "duplicateActorPersistentIds=(\d+)" -Group 1
            $packageDuplicateSceneComponentPersistentIds = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "duplicateSceneComponentPersistentIds=(\d+)" -Group 1
            $packageInvalidRootSceneComponentIndices = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "invalidRootSceneComponentIndices=(\d+)" -Group 1
            $packageUnresolvedParentReferences = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "unresolvedParentReferences=(\d+)" -Group 1
            $packageInvalidParentSceneComponentIndices = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "invalidParentSceneComponentIndices=(\d+)" -Group 1
            $packageSelfParentReferences = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "selfParentReferences=(\d+)" -Group 1
            $packageCyclicParentReferences = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "cyclicParentReferences=(\d+)" -Group 1
            $loadedAdapterReferences = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "loadedAdapterReferences=(\d+)" -Group 1
            $loadedAssetHandles = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "loadedAssetHandles=(\d+)" -Group 1
            $resolvedAdapterReferences = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern ", resolvedAdapterReferences=(\d+)" -Group 1
            $resolvedAssetHandles = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern ", resolvedAssetHandles=(\d+)" -Group 1
            $unresolvedAdapterReferences = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern ", unresolvedAdapterReferences=(\d+)" -Group 1
            $unresolvedAssetHandles = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern ", unresolvedAssetHandles=(\d+)" -Group 1
            $restoredRootComponents = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "restoredRootComponents=(\d+)" -Group 1
            $restoredCrossActorParentReferences = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "restoredCrossActorParentReferences=(\d+)" -Group 1
            $exportedLoadedWorldObjects = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "exportedLoadedWorldObjects=(\d+)" -Group 1
            $exportedLoadedWorldMeshes = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "exportedLoadedWorldMeshes=(\d+)" -Group 1
            $exportedLoadedWorldLights = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "exportedLoadedWorldLights=(\d+)" -Group 1
            $exportedLoadedWorldLegacyObjects = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "exportedLoadedWorldLegacyObjects=(\d+)" -Group 1
            $exportedLoadedWorldAttachments = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "exportedLoadedWorldAttachments=(\d+)" -Group 1
            $packagePath = Get-RegexValue -Text $engineWorldScenePackageLine -Pattern "path=([^,]+)" -Group 1
            if ($engineWorldScenePackageLine -notmatch "saved=yes") {
                $failures.Add("$($mode.Name): engine world scene package was not saved")
            }
            if ($engineWorldScenePackageLine -notmatch "loaded=yes") {
                $failures.Add("$($mode.Name): engine world scene package was not loaded")
            }
            if ($null -eq $packageActors -or [int]$packageActors -lt 5) {
                $failures.Add("$($mode.Name): engine world scene package did not save enough actors")
            }
            if ($null -eq $packageSceneComponents -or [int]$packageSceneComponents -lt 5) {
                $failures.Add("$($mode.Name): engine world scene package did not save enough SceneComponents")
            }
            if ($null -eq $typedActors -or [int]$typedActors -lt 5) {
                $failures.Add("$($mode.Name): engine world scene package did not save typed Actor metadata")
            }
            if ($null -eq $typedSceneComponents -or [int]$typedSceneComponents -lt 5) {
                $failures.Add("$($mode.Name): engine world scene package did not save typed SceneComponent metadata")
            }
            if ($null -eq $adapterDescriptors -or [int]$adapterDescriptors -lt 5) {
                $failures.Add("$($mode.Name): engine world scene package did not save adapter descriptors")
            }
            if ($null -eq $assetHandles -or [int]$assetHandles -lt 5) {
                $failures.Add("$($mode.Name): engine world scene package did not save stable asset handles")
            }
            if ($null -eq $loadedActors -or $null -eq $packageActors -or [int]$loadedActors -ne [int]$packageActors) {
                $failures.Add("$($mode.Name): engine world scene package actor count did not roundtrip")
            }
            if ($null -eq $loadedSceneComponents -or $null -eq $packageSceneComponents -or [int]$loadedSceneComponents -ne [int]$packageSceneComponents) {
                $failures.Add("$($mode.Name): engine world scene package SceneComponent count did not roundtrip")
            }
            if ($null -eq $loadedTypedActors -or $null -eq $typedActors -or [int]$loadedTypedActors -lt [int]$typedActors) {
                $failures.Add("$($mode.Name): engine world scene package did not restore typed Actors")
            }
            if ($null -eq $loadedActorsWithPersistentIds -or $null -eq $packageActors -or [int]$loadedActorsWithPersistentIds -lt [int]$packageActors) {
                $failures.Add("$($mode.Name): engine world scene package did not preserve Actor persistent ids")
            }
            if ($null -eq $loadedSceneComponentsWithPersistentIds -or $null -eq $packageSceneComponents -or [int]$loadedSceneComponentsWithPersistentIds -lt [int]$packageSceneComponents) {
                $failures.Add("$($mode.Name): engine world scene package did not preserve SceneComponent persistent ids")
            }
            if ($null -eq $loadedTypedSceneComponents -or [int]$loadedTypedSceneComponents -lt 5) {
                $failures.Add("$($mode.Name): engine world scene package did not restore typed SceneComponents")
            }
            if ($null -eq $packageUnknownActorTypes -or [int]$packageUnknownActorTypes -ne 0) {
                $failures.Add("$($mode.Name): valid engine world scene package reported unknown Actor types")
            }
            if ($null -eq $packageUnknownSceneComponentTypes -or [int]$packageUnknownSceneComponentTypes -ne 0) {
                $failures.Add("$($mode.Name): valid engine world scene package reported unknown SceneComponent types")
            }
            if ($null -eq $packageInvalidSceneComponentTransforms -or [int]$packageInvalidSceneComponentTransforms -ne 0) {
                $failures.Add("$($mode.Name): valid engine world scene package reported invalid SceneComponent transforms")
            }
            if ($null -eq $packageDuplicateActorPersistentIds -or [int]$packageDuplicateActorPersistentIds -ne 0) {
                $failures.Add("$($mode.Name): valid engine world scene package reported duplicate Actor persistent ids")
            }
            if ($null -eq $packageDuplicateSceneComponentPersistentIds -or [int]$packageDuplicateSceneComponentPersistentIds -ne 0) {
                $failures.Add("$($mode.Name): valid engine world scene package reported duplicate SceneComponent persistent ids")
            }
            if ($null -eq $packageInvalidRootSceneComponentIndices -or [int]$packageInvalidRootSceneComponentIndices -ne 0) {
                $failures.Add("$($mode.Name): valid engine world scene package reported invalid root component indices")
            }
            if ($null -eq $packageUnresolvedParentReferences -or [int]$packageUnresolvedParentReferences -ne 0) {
                $failures.Add("$($mode.Name): valid engine world scene package reported unresolved parent references")
            }
            if ($null -eq $packageInvalidParentSceneComponentIndices -or [int]$packageInvalidParentSceneComponentIndices -ne 0) {
                $failures.Add("$($mode.Name): valid engine world scene package reported invalid parent component indices")
            }
            if ($null -eq $packageSelfParentReferences -or [int]$packageSelfParentReferences -ne 0) {
                $failures.Add("$($mode.Name): valid engine world scene package reported self-parent references")
            }
            if ($null -eq $packageCyclicParentReferences -or [int]$packageCyclicParentReferences -ne 0) {
                $failures.Add("$($mode.Name): valid engine world scene package reported cyclic parent references")
            }
            if ($null -eq $loadedAdapterReferences -or [int]$loadedAdapterReferences -lt 5) {
                $failures.Add("$($mode.Name): engine world scene package did not read adapter references")
            }
            if ($null -eq $loadedAssetHandles -or [int]$loadedAssetHandles -lt 5) {
                $failures.Add("$($mode.Name): engine world scene package did not read stable asset handles")
            }
            if ($null -eq $resolvedAdapterReferences -or [int]$resolvedAdapterReferences -lt 5) {
                $failures.Add("$($mode.Name): engine world scene package did not resolve adapter references")
            }
            if ($null -eq $resolvedAssetHandles -or [int]$resolvedAssetHandles -lt 5) {
                $failures.Add("$($mode.Name): engine world scene package did not resolve stable asset handles")
            }
            if ($null -eq $unresolvedAdapterReferences -or [int]$unresolvedAdapterReferences -ne 0) {
                $failures.Add("$($mode.Name): engine world scene package left unresolved adapter references")
            }
            if ($null -eq $unresolvedAssetHandles -or [int]$unresolvedAssetHandles -ne 0) {
                $failures.Add("$($mode.Name): engine world scene package left unresolved stable asset handles")
            }
            if ($null -eq $restoredRootComponents -or $null -eq $packageActors -or [int]$restoredRootComponents -lt [int]$packageActors) {
                $failures.Add("$($mode.Name): engine world scene package did not restore root components")
            }
            if ($null -eq $restoredCrossActorParentReferences -or [int]$restoredCrossActorParentReferences -lt 1) {
                $failures.Add("$($mode.Name): engine world scene package did not report valid cross-actor parent restore")
            }
            if ($null -eq $exportedLoadedWorldObjects -or [int]$exportedLoadedWorldObjects -lt 5) {
                $failures.Add("$($mode.Name): loaded engine world package did not export enough objects")
            }
            if ($null -eq $exportedLoadedWorldMeshes -or [int]$exportedLoadedWorldMeshes -lt 4) {
                $failures.Add("$($mode.Name): loaded engine world package did not export mesh adapters")
            }
            if ($null -eq $exportedLoadedWorldLights -or [int]$exportedLoadedWorldLights -lt 1) {
                $failures.Add("$($mode.Name): loaded engine world package did not export light adapters")
            }
            if ($null -eq $exportedLoadedWorldLegacyObjects -or [int]$exportedLoadedWorldLegacyObjects -lt 1) {
                $failures.Add("$($mode.Name): loaded engine world package did not export legacy object adapters")
            }
            if ($null -eq $exportedLoadedWorldAttachments -or [int]$exportedLoadedWorldAttachments -lt 5) {
                $failures.Add("$($mode.Name): loaded engine world package did not restore exported attachments")
            }

            $packageFullPath = $null
            if ($null -ne $packagePath) {
                $packageFullPath = Join-Path $repoRoot $packagePath
            }
            if ($null -eq $packageFullPath -or !(Test-Path $packageFullPath)) {
                $failures.Add("$($mode.Name): engine world scene package file was not written")
            }
            else {
                if (!(Select-String -Path $packageFullPath -Pattern "schema=engine\.world\.scenePackage\.v1" -Quiet)) {
                    $failures.Add("$($mode.Name): engine world scene package did not write the expected schema")
                }
                if (!(Select-String -Path $packageFullPath -Pattern "world\.persistentId=world:preset:" -Quiet)) {
                    $failures.Add("$($mode.Name): engine world scene package did not write preset-sourced World persistent id")
                }
                if (!(Select-String -Path $packageFullPath -Pattern "actor\.\d+\.persistentId=actor:preset:" -Quiet)) {
                    $failures.Add("$($mode.Name): engine world scene package did not write preset-sourced Actor persistent ids")
                }
                if (!(Select-String -Path $packageFullPath -Pattern "sceneComponent\.\d+\.persistentId=component:preset:" -Quiet)) {
                    $failures.Add("$($mode.Name): engine world scene package did not write preset-sourced SceneComponent persistent ids")
                }
                if (!(Select-String -Path $packageFullPath -Pattern "actor\.\d+\.type=MeshActor" -Quiet)) {
                    $failures.Add("$($mode.Name): engine world scene package did not write MeshActor type metadata")
                }
                if (!(Select-String -Path $packageFullPath -Pattern "actor\.\d+\.type=LightActor" -Quiet)) {
                    $failures.Add("$($mode.Name): engine world scene package did not write LightActor type metadata")
                }
                if (!(Select-String -Path $packageFullPath -Pattern "sceneComponent\.\d+\.type=MeshComponent" -Quiet)) {
                    $failures.Add("$($mode.Name): engine world scene package did not write MeshComponent type metadata")
                }
                if (!(Select-String -Path $packageFullPath -Pattern "sceneComponent\.\d+\.type=LightComponent" -Quiet)) {
                    $failures.Add("$($mode.Name): engine world scene package did not write LightComponent type metadata")
                }
                if (!(Select-String -Path $packageFullPath -Pattern "sceneComponent\.\d+\.type=LegacyObjectComponent" -Quiet)) {
                    $failures.Add("$($mode.Name): engine world scene package did not write LegacyObjectComponent type metadata")
                }
                if (!(Select-String -Path $packageFullPath -Pattern "sceneComponent\.\d+\.adapter\.kind=mesh" -Quiet)) {
                    $failures.Add("$($mode.Name): engine world scene package did not write mesh adapter descriptors")
                }
                if (!(Select-String -Path $packageFullPath -Pattern "sceneComponent\.\d+\.adapter\.assetHandle=asset:mesh:runtime-generated:" -Quiet)) {
                    $failures.Add("$($mode.Name): engine world scene package did not write mesh asset handles")
                }
                if (!(Select-String -Path $packageFullPath -Pattern "sceneComponent\.\d+\.adapter\.assetHandle=asset:legacy-object:runtime-generated:" -Quiet)) {
                    $failures.Add("$($mode.Name): engine world scene package did not write legacy object asset handles")
                }
                if (!(Select-String -Path $packageFullPath -Pattern "sceneComponent\.\d+\.adapter\.assetHandle=asset:light:runtime-generated:" -Quiet)) {
                    $failures.Add("$($mode.Name): engine world scene package did not write light asset handles")
                }
                if (!(Select-String -Path $packageFullPath -Pattern "sceneComponent\.\d+\.adapter\.assetReference=runtime-generated:mesh:" -Quiet)) {
                    $failures.Add("$($mode.Name): engine world scene package did not keep mesh asset reference fallback")
                }
                if (!(Select-String -Path $packageFullPath -Pattern "sceneComponent\.\d+\.adapter\.assetReference=runtime-generated:light:" -Quiet)) {
                    $failures.Add("$($mode.Name): engine world scene package did not keep light asset reference fallback")
                }
                if (!(Select-String -Path $packageFullPath -Pattern "sceneComponent\.\d+\.adapter\.materialType=PBRMaterial" -Quiet)) {
                    $failures.Add("$($mode.Name): engine world scene package did not write PBR material adapter metadata")
                }
            }
        }
        if (!$engineWorldScenePackageNegativeLine) {
            $failures.Add("$($mode.Name): missing engine world scene package negative stats")
        }
        else {
            $unknownActorTypes = Get-RegexValue -Text $engineWorldScenePackageNegativeLine -Pattern "unknownActorTypes=(\d+)" -Group 1
            $unknownSceneComponentTypes = Get-RegexValue -Text $engineWorldScenePackageNegativeLine -Pattern "unknownSceneComponentTypes=(\d+)" -Group 1
            $invalidSceneComponentTransforms = Get-RegexValue -Text $engineWorldScenePackageNegativeLine -Pattern "invalidSceneComponentTransforms=(\d+)" -Group 1
            $duplicateActorPersistentIds = Get-RegexValue -Text $engineWorldScenePackageNegativeLine -Pattern "duplicateActorPersistentIds=(\d+)" -Group 1
            $duplicateSceneComponentPersistentIds = Get-RegexValue -Text $engineWorldScenePackageNegativeLine -Pattern "duplicateSceneComponentPersistentIds=(\d+)" -Group 1
            $unresolvedParentReferences = Get-RegexValue -Text $engineWorldScenePackageNegativeLine -Pattern "unresolvedParentReferences=(\d+)" -Group 1
            $invalidParentSceneComponentIndices = Get-RegexValue -Text $engineWorldScenePackageNegativeLine -Pattern "invalidParentSceneComponentIndices=(\d+)" -Group 1
            $selfParentReferences = Get-RegexValue -Text $engineWorldScenePackageNegativeLine -Pattern "selfParentReferences=(\d+)" -Group 1
            $cyclicParentReferences = Get-RegexValue -Text $engineWorldScenePackageNegativeLine -Pattern "cyclicParentReferences=(\d+)" -Group 1
            $negativeRestoredCrossActorParentReferences = Get-RegexValue -Text $engineWorldScenePackageNegativeLine -Pattern "restoredCrossActorParentReferences=(\d+)" -Group 1
            if ($engineWorldScenePackageNegativeLine -notmatch "missingSchemaRejected=yes") {
                $failures.Add("$($mode.Name): scene package negative probe did not reject missing schema")
            }
            if ($engineWorldScenePackageNegativeLine -notmatch "invalidLineRejected=yes") {
                $failures.Add("$($mode.Name): scene package negative probe did not reject invalid line")
            }
            if ($engineWorldScenePackageNegativeLine -notmatch "missingCountRejected=yes") {
                $failures.Add("$($mode.Name): scene package negative probe did not reject missing count")
            }
            if ($engineWorldScenePackageNegativeLine -notmatch "unknownTypesLoaded=yes") {
                $failures.Add("$($mode.Name): scene package negative probe did not load unknown-type fallback package")
            }
            if ($engineWorldScenePackageNegativeLine -notmatch "duplicateActorIdRejected=yes") {
                $failures.Add("$($mode.Name): scene package graph probe did not reject duplicate Actor persistent ids")
            }
            if ($engineWorldScenePackageNegativeLine -notmatch "duplicateSceneComponentIdRejected=yes") {
                $failures.Add("$($mode.Name): scene package graph probe did not reject duplicate SceneComponent persistent ids")
            }
            if ($engineWorldScenePackageNegativeLine -notmatch "unresolvedParentRejected=yes") {
                $failures.Add("$($mode.Name): scene package graph probe did not reject unresolved parent references")
            }
            if ($engineWorldScenePackageNegativeLine -notmatch "invalidParentIndexRejected=yes") {
                $failures.Add("$($mode.Name): scene package graph probe did not reject invalid parent indices")
            }
            if ($engineWorldScenePackageNegativeLine -notmatch "selfParentRejected=yes") {
                $failures.Add("$($mode.Name): scene package graph probe did not reject self-parent references")
            }
            if ($engineWorldScenePackageNegativeLine -notmatch "cycleRejected=yes") {
                $failures.Add("$($mode.Name): scene package graph probe did not reject cyclic parent references")
            }
            if ($engineWorldScenePackageNegativeLine -notmatch "validCrossActorParentLoaded=yes") {
                $failures.Add("$($mode.Name): scene package graph probe did not preserve a valid cross-actor parent reference")
            }
            if ($null -eq $unknownActorTypes -or [int]$unknownActorTypes -lt 1) {
                $failures.Add("$($mode.Name): scene package negative probe did not report unknown Actor types")
            }
            if ($null -eq $unknownSceneComponentTypes -or [int]$unknownSceneComponentTypes -lt 1) {
                $failures.Add("$($mode.Name): scene package negative probe did not report unknown SceneComponent types")
            }
            if ($null -eq $invalidSceneComponentTransforms -or [int]$invalidSceneComponentTransforms -lt 1) {
                $failures.Add("$($mode.Name): scene package negative probe did not report invalid SceneComponent transforms")
            }
            if ($null -eq $duplicateActorPersistentIds -or [int]$duplicateActorPersistentIds -lt 1) {
                $failures.Add("$($mode.Name): scene package graph probe did not report duplicate Actor persistent ids")
            }
            if ($null -eq $duplicateSceneComponentPersistentIds -or [int]$duplicateSceneComponentPersistentIds -lt 1) {
                $failures.Add("$($mode.Name): scene package graph probe did not report duplicate SceneComponent persistent ids")
            }
            if ($null -eq $unresolvedParentReferences -or [int]$unresolvedParentReferences -lt 1) {
                $failures.Add("$($mode.Name): scene package graph probe did not report unresolved parent references")
            }
            if ($null -eq $invalidParentSceneComponentIndices -or [int]$invalidParentSceneComponentIndices -lt 1) {
                $failures.Add("$($mode.Name): scene package graph probe did not report invalid parent indices")
            }
            if ($null -eq $selfParentReferences -or [int]$selfParentReferences -lt 1) {
                $failures.Add("$($mode.Name): scene package graph probe did not report self-parent references")
            }
            if ($null -eq $cyclicParentReferences -or [int]$cyclicParentReferences -lt 1) {
                $failures.Add("$($mode.Name): scene package graph probe did not report cyclic parent references")
            }
            if ($null -eq $negativeRestoredCrossActorParentReferences -or [int]$negativeRestoredCrossActorParentReferences -lt 1) {
                $failures.Add("$($mode.Name): scene package graph probe did not report restored cross-actor parent references")
            }
        }
    }
    if ($mode.PSObject.Properties.Name -contains "ExpectEngineWorldTransformSnapshot" -and $mode.ExpectEngineWorldTransformSnapshot) {
        if (!$engineWorldSnapshotLine) {
            $failures.Add("$($mode.Name): missing engine world transform snapshot stats")
        }
        else {
            $snapshotSaved = Get-RegexValue -Text $engineWorldSnapshotLine -Pattern "saved=([^,]+)" -Group 1
            $snapshotActors = Get-RegexValue -Text $engineWorldSnapshotLine -Pattern "actors=(\d+)" -Group 1
            $snapshotSceneComponents = Get-RegexValue -Text $engineWorldSnapshotLine -Pattern "sceneComponents=(\d+)" -Group 1
            $snapshotPath = Get-RegexValue -Text $engineWorldSnapshotLine -Pattern "path=([^,]+)" -Group 1
            if ($snapshotSaved -ne "yes") {
                $failures.Add("$($mode.Name): engine world transform snapshot was not saved")
            }
            if ($null -eq $snapshotActors -or [int]$snapshotActors -le 0) {
                $failures.Add("$($mode.Name): engine world transform snapshot did not include actors")
            }
            if ($null -eq $snapshotSceneComponents -or [int]$snapshotSceneComponents -le 0) {
                $failures.Add("$($mode.Name): engine world transform snapshot did not include SceneComponents")
            }
            $snapshotFullPath = $null
            if ($null -ne $snapshotPath) {
                $snapshotFullPath = Join-Path $repoRoot $snapshotPath
            }
            if ($null -eq $snapshotFullPath -or !(Test-Path $snapshotFullPath)) {
                $failures.Add("$($mode.Name): engine world transform snapshot file was not written")
            }
            else {
                if (!(Select-String -Path $snapshotFullPath -Pattern "world\.persistentId=world:preset:" -Quiet)) {
                    $failures.Add("$($mode.Name): engine world transform snapshot did not write preset-sourced World persistent id")
                }
                if (!(Select-String -Path $snapshotFullPath -Pattern "sceneComponent\.0\.persistentId=component:preset:" -Quiet)) {
                    $failures.Add("$($mode.Name): engine world transform snapshot did not write preset-sourced SceneComponent persistent ids")
                }
                if ($mode.PSObject.Properties.Name -contains "ExpectEngineWorldEditorCreate" -and $mode.ExpectEngineWorldEditorCreate) {
                    if (!(Select-String -Path $snapshotFullPath -Pattern "actor\.\d+\.persistentId=actor:editor-created:" -Quiet)) {
                        $failures.Add("$($mode.Name): engine world transform snapshot did not write editor-created Actor persistent ids")
                    }
                    if (!(Select-String -Path $snapshotFullPath -Pattern "sceneComponent\.\d+\.persistentId=component:editor-created:" -Quiet)) {
                        $failures.Add("$($mode.Name): engine world transform snapshot did not write editor-created SceneComponent persistent ids")
                    }
                }
            }
        }
        if (!$engineWorldSnapshotApplyLine) {
            $failures.Add("$($mode.Name): missing engine world transform snapshot apply stats")
        }
        else {
            $snapshotApplied = Get-RegexValue -Text $engineWorldSnapshotApplyLine -Pattern "applied=([^,]+)" -Group 1
            $snapshotMatchedByPersistentId = Get-RegexValue -Text $engineWorldSnapshotApplyLine -Pattern "matchedByPersistentId=(\d+)" -Group 1
            $snapshotAppliedSceneComponents = Get-RegexValue -Text $engineWorldSnapshotApplyLine -Pattern "appliedSceneComponents=(\d+)" -Group 1
            $snapshotChangedSceneComponents = Get-RegexValue -Text $engineWorldSnapshotApplyLine -Pattern "changedSceneComponents=(\d+)" -Group 1
            $snapshotVerificationPerturbed = Get-RegexValue -Text $engineWorldSnapshotApplyLine -Pattern "verificationPerturbed=([^,]+)" -Group 1
            if ($snapshotApplied -ne "yes") {
                $failures.Add("$($mode.Name): engine world transform snapshot was not applied")
            }
            if ($null -eq $snapshotMatchedByPersistentId -or [int]$snapshotMatchedByPersistentId -le 0) {
                $failures.Add("$($mode.Name): engine world transform snapshot did not match by persistent id")
            }
            if ($null -eq $snapshotAppliedSceneComponents -or [int]$snapshotAppliedSceneComponents -le 0) {
                $failures.Add("$($mode.Name): engine world transform snapshot did not apply any SceneComponents")
            }
            if ($null -eq $snapshotChangedSceneComponents -or [int]$snapshotChangedSceneComponents -le 0) {
                $failures.Add("$($mode.Name): engine world transform snapshot apply did not restore a perturbed SceneComponent")
            }
            if ($snapshotVerificationPerturbed -ne "yes") {
                $failures.Add("$($mode.Name): engine world transform snapshot verification did not perturb a SceneComponent")
            }
        }
    }
    if ($mode.PSObject.Properties.Name -contains "ExpectDeferredLighting" -and $mode.ExpectDeferredLighting) {
        if (!$rendererLine) {
            $failures.Add("$($mode.Name): missing renderer stats")
        }
        else {
            $deferredDrawCalls = Get-RegexValue -Text $rendererLine -Pattern "pbrDeferredLightingDrawCalls=(\d+)" -Group 1
            if ($null -eq $deferredDrawCalls -or [int]$deferredDrawCalls -le 0) {
                $failures.Add("$($mode.Name): deferred lighting pass did not draw")
            }
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
    if ($result.EngineWorldPreparedSceneStats) {
        $summaryLines.Add("  engineWorldPreparedScene: $($result.EngineWorldPreparedSceneStats)")
    }
    if ($result.RendererStats) {
        $summaryLines.Add("  renderer: $($result.RendererStats)")
    }
    if ($result.RuntimeEngineLifecycleSnapshotStats) {
        $summaryLines.Add("  runtimeEngineLifecycleSnapshot: $($result.RuntimeEngineLifecycleSnapshotStats)")
    }
    if ($result.RuntimeEngineSubsystemSummaryStats) {
        $summaryLines.Add("  runtimeEngineSubsystemSummary: $($result.RuntimeEngineSubsystemSummaryStats)")
    }
    if ($result.RuntimeEngineTickStats) {
        $summaryLines.Add("  runtimeEngineTick: $($result.RuntimeEngineTickStats)")
    }
    if ($result.RuntimeSubsystemHealthStats) {
        $summaryLines.Add("  runtimeSubsystemHealth: $($result.RuntimeSubsystemHealthStats)")
    }
    if ($result.RuntimeRendererSubsystemStats) {
        $summaryLines.Add("  runtimeRendererSubsystem: $($result.RuntimeRendererSubsystemStats)")
    }
    if ($result.RuntimeRendererBackendContractStats) {
        $summaryLines.Add("  runtimeRendererBackendContract: $($result.RuntimeRendererBackendContractStats)")
    }
    if ($result.RuntimeRendererSubsystemCleanupStats) {
        $summaryLines.Add("  runtimeRendererSubsystemCleanup: $($result.RuntimeRendererSubsystemCleanupStats)")
    }
    if ($result.RuntimeRendererBackendContractCleanupStats) {
        $summaryLines.Add("  runtimeRendererBackendContractCleanup: $($result.RuntimeRendererBackendContractCleanupStats)")
    }
    if ($result.RuntimeEngineWorldCleanupStats) {
        $summaryLines.Add("  runtimeEngineWorldCleanup: $($result.RuntimeEngineWorldCleanupStats)")
    }
    if ($result.RuntimeEngineSubsystemCleanupStats) {
        $summaryLines.Add("  runtimeEngineSubsystemCleanup: $($result.RuntimeEngineSubsystemCleanupStats)")
    }
    if ($result.LegacyWorldMirrorStats) {
        $summaryLines.Add("  legacyWorldMirror: $($result.LegacyWorldMirrorStats)")
    }
    if ($result.ImportedAssetWorldImportStats) {
        $summaryLines.Add("  importedAssetWorldImport: $($result.ImportedAssetWorldImportStats)")
    }
    if ($result.RuntimeAssetRegistryStats) {
        $summaryLines.Add("  runtimeAssetRegistry: $($result.RuntimeAssetRegistryStats)")
    }
    if ($result.ImportedAssetScenePackageStats) {
        $summaryLines.Add("  importedAssetScenePackage: $($result.ImportedAssetScenePackageStats)")
    }
    if ($result.EngineWorldProbeStats) {
        $summaryLines.Add("  engineWorldProbe: $($result.EngineWorldProbeStats)")
    }
    if ($result.EngineWorldMinimalStats) {
        $summaryLines.Add("  engineWorldMinimal: $($result.EngineWorldMinimalStats)")
    }
    if ($result.EngineWorldEditorCreateStats) {
        $summaryLines.Add("  engineWorldEditorCreate: $($result.EngineWorldEditorCreateStats)")
    }
    if ($result.EngineWorldScenePackageStats) {
        $summaryLines.Add("  engineWorldScenePackage: $($result.EngineWorldScenePackageStats)")
    }
    if ($result.EngineWorldScenePackageNegativeStats) {
        $summaryLines.Add("  engineWorldScenePackageNegative: $($result.EngineWorldScenePackageNegativeStats)")
    }
    if ($result.EngineWorldSnapshotStats) {
        $summaryLines.Add("  engineWorldSnapshot: $($result.EngineWorldSnapshotStats)")
    }
    if ($result.EngineWorldSnapshotApplyStats) {
        $summaryLines.Add("  engineWorldSnapshotApply: $($result.EngineWorldSnapshotApplyStats)")
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
