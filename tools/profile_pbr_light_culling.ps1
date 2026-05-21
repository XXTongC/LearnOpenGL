param(
    [string]$Configuration = "Debug",
    [string]$Platform = "x64",
    [switch]$SkipBuild,
    [switch]$NoLinkDebugInfo,
    [switch]$KeepCaptures,
    [string]$ReportPath = "docs/pbr_light_culling_timing_report.md"
)

$ErrorActionPreference = "Stop"

$scriptPath = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Resolve-Path (Join-Path $scriptPath "..")
$outDir = Join-Path $repoRoot "out"
$verifyScript = Join-Path $scriptPath "verify_pbr.ps1"
$timingModes = @(
    "deferred-tiled-lights-pressure-timing",
    "deferred-clustered-grid-pressure-timing"
)

function Get-StatsValue {
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

function Convert-ToInt64Value {
    param([object]$Value)

    if ($null -eq $Value -or "$Value" -eq "") {
        return 0L
    }

    return [Int64]$Value
}

function Format-Milliseconds {
    param([Int64]$Nanoseconds)

    return [string]::Format(
        [Globalization.CultureInfo]::InvariantCulture,
        "{0:0.0000}",
        ([double]$Nanoseconds / 1000000.0)
    )
}

function Format-Percent {
    param(
        [Int64]$Numerator,
        [Int64]$Denominator
    )

    if ($Denominator -le 0) {
        return "-"
    }

    return [string]::Format(
        [Globalization.CultureInfo]::InvariantCulture,
        "{0:0.00}%",
        (([double]$Numerator * 100.0) / [double]$Denominator)
    )
}

function Get-RendererStatsLine {
    param([string]$ModeName)

    $logPath = Join-Path $outDir ("pbr_verify_{0}.log" -f $ModeName)
    if (!(Test-Path $logPath)) {
        throw "Missing verification log: $logPath"
    }

    $line = Get-Content -Path $logPath | Where-Object { $_ -match "PBR verification renderer stats:" } | Select-Object -Last 1
    if (!$line) {
        throw "Missing renderer stats in log: $logPath"
    }

    return $line
}

function New-LightCullingTimingResult {
    param(
        [string]$Name,
        [string]$Kind
    )

    $line = Get-RendererStatsLine -ModeName $Name
    $indices = 0L
    $culled = 0L
    $fullIndices = 0L
    $pointLights = 0
    $grid = "-"
    $occupancy = "-"

    if ($Kind -eq "Tiled") {
        $grid = Get-StatsValue -Text $line -Pattern "pbrDeferredTiledLightGridSize=(\d+x\d+)" -Group 1
        $pointLights = [int](Convert-ToInt64Value (Get-StatsValue -Text $line -Pattern "pbrDeferredTiledLightGridPointLights=(\d+)" -Group 1))
        $fullIndices = Convert-ToInt64Value (Get-StatsValue -Text $line -Pattern "pbrDeferredTiledLightGridFullIndices=(\d+)" -Group 1)
        $indices = Convert-ToInt64Value (Get-StatsValue -Text $line -Pattern "pbrDeferredTiledLightGridIndices=(\d+)" -Group 1)
        $culled = Convert-ToInt64Value (Get-StatsValue -Text $line -Pattern "pbrDeferredTiledLightGridCulledIndices=(\d+)" -Group 1)
        $occupied = Get-StatsValue -Text $line -Pattern "pbrDeferredTiledLightGridOccupiedTiles=(\d+)/(\d+)" -Group 1
        $tileCount = Get-StatsValue -Text $line -Pattern "pbrDeferredTiledLightGridOccupiedTiles=(\d+)/(\d+)" -Group 2
        if ($null -ne $occupied -and $null -ne $tileCount) {
            $occupancy = "$occupied/$tileCount"
        }
    }
    else {
        $grid = Get-StatsValue -Text $line -Pattern "pbrDeferredClusteredLightGridSize=(\d+x\d+x\d+)" -Group 1
        $pointLights = [int](Convert-ToInt64Value (Get-StatsValue -Text $line -Pattern "pbrDeferredClusteredLightGridPointLights=(\d+)" -Group 1))
        $indices = Convert-ToInt64Value (Get-StatsValue -Text $line -Pattern "pbrDeferredClusteredLightGridIndices=(\d+)" -Group 1)
        $culled = Convert-ToInt64Value (Get-StatsValue -Text $line -Pattern "pbrDeferredClusteredLightGridCulledIndices=(\d+)" -Group 1)
        $fullIndices = $indices + $culled
        $clusters = Get-StatsValue -Text $line -Pattern "pbrDeferredClusteredLightGridClusters=(\d+)" -Group 1
        if ($null -ne $clusters) {
            $occupancy = "$clusters clusters"
        }
    }

    $frameNs = Convert-ToInt64Value (Get-StatsValue -Text $line -Pattern "rendererGpuFrameNs=(\d+)" -Group 1)
    $gbufferNs = Convert-ToInt64Value (Get-StatsValue -Text $line -Pattern "rendererGpuPbrGBufferNs=(\d+)" -Group 1)
    $deferredNs = Convert-ToInt64Value (Get-StatsValue -Text $line -Pattern "rendererGpuPbrDeferredLightingNs=(\d+)" -Group 1)
    $timedPasses = Convert-ToInt64Value (Get-StatsValue -Text $line -Pattern "rendererGpuTimedPasses=(\d+)" -Group 1)
    $pendingQueries = Convert-ToInt64Value (Get-StatsValue -Text $line -Pattern "rendererGpuTimingPendingQueries=(\d+)" -Group 1)

    return [pscustomobject]@{
        Name = $Name
        Kind = $Kind
        RendererStats = $line
        PointLights = $pointLights
        Grid = $grid
        FullIndices = $fullIndices
        LiveIndices = $indices
        CulledIndices = $culled
        CullingPercent = Format-Percent -Numerator $culled -Denominator $fullIndices
        Occupancy = $occupancy
        FrameNs = $frameNs
        GBufferNs = $gbufferNs
        DeferredLightingNs = $deferredNs
        FrameMs = Format-Milliseconds -Nanoseconds $frameNs
        GBufferMs = Format-Milliseconds -Nanoseconds $gbufferNs
        DeferredLightingMs = Format-Milliseconds -Nanoseconds $deferredNs
        TimedPasses = $timedPasses
        PendingQueries = $pendingQueries
        Log = ("out/pbr_verify_{0}.log" -f $Name)
    }
}

function Resolve-OutputPath {
    param([string]$Path)

    if ([System.IO.Path]::IsPathRooted($Path)) {
        return $Path
    }

    return Join-Path $repoRoot $Path
}

if (!(Test-Path $verifyScript)) {
    throw "Verification script was not found: $verifyScript"
}

$verifyArgs = @(
    "-NoProfile",
    "-ExecutionPolicy",
    "Bypass",
    "-File",
    $verifyScript,
    "-Configuration",
    $Configuration,
    "-Platform",
    $Platform,
    "-Modes",
    ($timingModes -join ",")
)
if ($SkipBuild) {
    $verifyArgs += "-SkipBuild"
}
if ($NoLinkDebugInfo) {
    $verifyArgs += "-NoLinkDebugInfo"
}
if (!$KeepCaptures) {
    $verifyArgs += "-DiscardCaptures"
}

& powershell.exe @verifyArgs
if ($LASTEXITCODE -ne 0) {
    throw "PBR light culling timing verification failed with exit code $LASTEXITCODE"
}

$tiled = New-LightCullingTimingResult -Name $timingModes[0] -Kind "Tiled"
$clustered = New-LightCullingTimingResult -Name $timingModes[1] -Kind "Clustered"
$reportFullPath = Resolve-OutputPath -Path $ReportPath
$reportDir = Split-Path -Parent $reportFullPath
if (!(Test-Path $reportDir)) {
    New-Item -ItemType Directory -Path $reportDir | Out-Null
}

$branch = (git -C $repoRoot rev-parse --abbrev-ref HEAD).Trim()
$commit = (git -C $repoRoot rev-parse --short HEAD).Trim()
$generated = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
$deferredDeltaNs = $clustered.DeferredLightingNs - $tiled.DeferredLightingNs
$deferredDeltaMs = Format-Milliseconds -Nanoseconds ([Math]::Abs($deferredDeltaNs))
$fasterPath = if ($deferredDeltaNs -gt 0) { "Tiled" } elseif ($deferredDeltaNs -lt 0) { "Clustered" } else { "Tie" }

$lines = New-Object System.Collections.Generic.List[string]
$lines.Add("# PBR Deferred Light Culling Timing Report")
$lines.Add("")
$lines.Add("生成时间：$generated")
$lines.Add("仓库：``$repoRoot``")
$lines.Add("分支：``$branch``")
$lines.Add("提交：``$commit``")
$lines.Add("配置：``$Configuration|$Platform``")
$lines.Add("")
$lines.Add("## 测试范围")
$lines.Add("")
$lines.Add('本报告只比较同一个 8 点光 pressure rig 下的 deferred PBR light culling 路径。两个模式都会启用 renderer pass 级 `GL_TIME_ELAPSED`，并使用跨帧 deferred query readback，避免为了当前帧 timing 主动阻塞 GPU。')
$lines.Add("")
$lines.Add("复现命令：")
$lines.Add("")
$lines.Add('```powershell')
$lines.Add("powershell -NoProfile -ExecutionPolicy Bypass -File tools\profile_pbr_light_culling.ps1 -NoLinkDebugInfo")
$lines.Add('```')
$lines.Add("")
$lines.Add("底层 verification modes：")
$lines.Add("")
$lines.Add('- `deferred-tiled-lights-pressure-timing`')
$lines.Add('- `deferred-clustered-grid-pressure-timing`')
$lines.Add("")
$lines.Add("## 结果")
$lines.Add("")
$lines.Add("| Path | Grid | Point lights | Live indices | Culled / candidate | Occupancy | G-buffer GPU ms | Deferred lighting GPU ms | Frame GPU ms | Timed passes | Pending queries | Log |")
$lines.Add("| --- | --- | ---: | ---: | --- | --- | ---: | ---: | ---: | ---: | ---: | --- |")
$lines.Add("| $($tiled.Kind) | ``$($tiled.Grid)`` | $($tiled.PointLights) | $($tiled.LiveIndices) | $($tiled.CulledIndices) / $($tiled.FullIndices) ($($tiled.CullingPercent)) | $($tiled.Occupancy) | $($tiled.GBufferMs) | $($tiled.DeferredLightingMs) | $($tiled.FrameMs) | $($tiled.TimedPasses) | $($tiled.PendingQueries) | ``$($tiled.Log)`` |")
$lines.Add("| $($clustered.Kind) | ``$($clustered.Grid)`` | $($clustered.PointLights) | $($clustered.LiveIndices) | $($clustered.CulledIndices) / $($clustered.FullIndices) ($($clustered.CullingPercent)) | $($clustered.Occupancy) | $($clustered.GBufferMs) | $($clustered.DeferredLightingMs) | $($clustered.FrameMs) | $($clustered.TimedPasses) | $($clustered.PendingQueries) | ``$($clustered.Log)`` |")
$lines.Add("")
$lines.Add("## 解释")
$lines.Add("")
if ($fasterPath -eq "Tie") {
    $lines.Add("- 两条路径本次采样的 deferred lighting GPU 时间相同。单次 timing 不足以证明长期性能结论，后续如果要做性能决策，应增加多次采样和平均值。")
}
else {
    $lines.Add("- 本次采样中 ``$fasterPath`` 的 deferred lighting pass 更短，差值约 ``$deferredDeltaMs ms``。该值是当前机器和当前压力 rig 的测量结果，不应直接外推为所有场景结论。")
}
$lines.Add("- Tiled 路径使用屏幕 2D tile list，candidate 数量较小，适合作为当前稳定 fallback。")
$lines.Add("- Clustered 路径使用 3D grid + compute assignment，candidate 空间更大，但已经具备与深度 slice 相关的 PBR 扩展基础；后续需要继续解决 overflow/fallback、多帧采样统计和真实资产 baseline。")
$lines.Add('- 两条 timing mode 都保留在 `tools/verify_pbr.ps1` 默认回归中，避免后续重构误删 profiling 入口。')
$lines.Add("")
$lines.Add("## 原始 renderer stats")
$lines.Add("")
$lines.Add("### Tiled")
$lines.Add("")
$lines.Add('```text')
$lines.Add($tiled.RendererStats)
$lines.Add('```')
$lines.Add("")
$lines.Add("### Clustered")
$lines.Add("")
$lines.Add('```text')
$lines.Add($clustered.RendererStats)
$lines.Add('```')

$lines | Set-Content -Path $reportFullPath -Encoding UTF8
Write-Host "PBR light culling timing report written to $reportFullPath"
