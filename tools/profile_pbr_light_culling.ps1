param(
    [string]$Configuration = "Debug",
    [string]$Platform = "x64",
    [switch]$SkipBuild,
    [switch]$NoLinkDebugInfo,
    [switch]$KeepCaptures,
    [ValidateRange(1, 100)]
    [int]$Samples = 1,
    [string]$SamplesCsvPath = "docs/pbr_light_culling_timing_samples.csv",
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

function Format-MillisecondsValue {
    param([double]$Nanoseconds)
    return [string]::Format(
        [Globalization.CultureInfo]::InvariantCulture,
        "{0:0.0000}",
        ($Nanoseconds / 1000000.0)
    )
}

function Format-Milliseconds {
    param([Int64]$Nanoseconds)

    return Format-MillisecondsValue -Nanoseconds ([double]$Nanoseconds)
}

function Format-MillisecondsSummary {
    param(
        [double]$Average,
        [double]$Minimum,
        [double]$Maximum
    )

    return "{0} / {1} / {2}" -f `
        (Format-MillisecondsValue -Nanoseconds $Average),
        (Format-MillisecondsValue -Nanoseconds $Minimum),
        (Format-MillisecondsValue -Nanoseconds $Maximum)
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

function New-TimingAggregate {
    param(
        [object[]]$Samples,
        [string]$Kind
    )

    if ($Samples.Count -le 0) {
        throw "Cannot aggregate an empty timing sample set for $Kind"
    }

    $first = $Samples | Select-Object -First 1
    $frameStats = $Samples | Measure-Object -Property FrameNs -Average -Minimum -Maximum
    $gbufferStats = $Samples | Measure-Object -Property GBufferNs -Average -Minimum -Maximum
    $deferredStats = $Samples | Measure-Object -Property DeferredLightingNs -Average -Minimum -Maximum

    return [pscustomobject]@{
        Kind = $Kind
        SampleCount = $Samples.Count
        Grid = $first.Grid
        PointLights = $first.PointLights
        FullIndices = $first.FullIndices
        LiveIndices = $first.LiveIndices
        CulledIndices = $first.CulledIndices
        CullingPercent = $first.CullingPercent
        Occupancy = $first.Occupancy
        FrameAverageNs = [double]$frameStats.Average
        FrameMinimumNs = [double]$frameStats.Minimum
        FrameMaximumNs = [double]$frameStats.Maximum
        GBufferAverageNs = [double]$gbufferStats.Average
        GBufferMinimumNs = [double]$gbufferStats.Minimum
        GBufferMaximumNs = [double]$gbufferStats.Maximum
        DeferredLightingAverageNs = [double]$deferredStats.Average
        DeferredLightingMinimumNs = [double]$deferredStats.Minimum
        DeferredLightingMaximumNs = [double]$deferredStats.Maximum
        FrameSummaryMs = Format-MillisecondsSummary -Average ([double]$frameStats.Average) -Minimum ([double]$frameStats.Minimum) -Maximum ([double]$frameStats.Maximum)
        GBufferSummaryMs = Format-MillisecondsSummary -Average ([double]$gbufferStats.Average) -Minimum ([double]$gbufferStats.Minimum) -Maximum ([double]$gbufferStats.Maximum)
        DeferredLightingSummaryMs = Format-MillisecondsSummary -Average ([double]$deferredStats.Average) -Minimum ([double]$deferredStats.Minimum) -Maximum ([double]$deferredStats.Maximum)
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

function Invoke-PbrTimingVerification {
    param([bool]$SkipBuildForSample)

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
    if ($SkipBuildForSample) {
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
}

$sampleResults = New-Object System.Collections.Generic.List[object]
for ($sampleIndex = 1; $sampleIndex -le $Samples; ++$sampleIndex) {
    Write-Host "Running PBR light culling timing sample $sampleIndex/$Samples..."
    $skipBuildForSample = $SkipBuild -or $sampleIndex -gt 1
    Invoke-PbrTimingVerification -SkipBuildForSample $skipBuildForSample

    $tiledSample = New-LightCullingTimingResult -Name $timingModes[0] -Kind "Tiled"
    $clusteredSample = New-LightCullingTimingResult -Name $timingModes[1] -Kind "Clustered"
    $tiledSample | Add-Member -NotePropertyName Sample -NotePropertyValue $sampleIndex
    $tiledSample | Add-Member -NotePropertyName PathOrder -NotePropertyValue 1
    $clusteredSample | Add-Member -NotePropertyName Sample -NotePropertyValue $sampleIndex
    $clusteredSample | Add-Member -NotePropertyName PathOrder -NotePropertyValue 2
    $sampleResults.Add($tiledSample)
    $sampleResults.Add($clusteredSample)
}

$tiledSamples = @($sampleResults | Where-Object { $_.Kind -eq "Tiled" })
$clusteredSamples = @($sampleResults | Where-Object { $_.Kind -eq "Clustered" })
$tiled = $tiledSamples | Select-Object -Last 1
$clustered = $clusteredSamples | Select-Object -Last 1
$tiledAggregate = New-TimingAggregate -Samples $tiledSamples -Kind "Tiled"
$clusteredAggregate = New-TimingAggregate -Samples $clusteredSamples -Kind "Clustered"
$reportFullPath = Resolve-OutputPath -Path $ReportPath
$reportDir = Split-Path -Parent $reportFullPath
if (!(Test-Path $reportDir)) {
    New-Item -ItemType Directory -Path $reportDir | Out-Null
}
$samplesCsvFullPath = Resolve-OutputPath -Path $SamplesCsvPath
$samplesCsvDir = Split-Path -Parent $samplesCsvFullPath
if (!(Test-Path $samplesCsvDir)) {
    New-Item -ItemType Directory -Path $samplesCsvDir | Out-Null
}
$sampleResults |
    Select-Object Sample, Kind, Grid, PointLights, LiveIndices, FullIndices, CulledIndices, CullingPercent, Occupancy, GBufferMs, DeferredLightingMs, FrameMs, TimedPasses, PendingQueries, Log |
    Export-Csv -Path $samplesCsvFullPath -NoTypeInformation -Encoding UTF8

$branch = (git -C $repoRoot rev-parse --abbrev-ref HEAD).Trim()
$commit = (git -C $repoRoot rev-parse --short HEAD).Trim()
$generated = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
$deferredDeltaNs = $clusteredAggregate.DeferredLightingAverageNs - $tiledAggregate.DeferredLightingAverageNs
$deferredDeltaMs = Format-MillisecondsValue -Nanoseconds ([Math]::Abs($deferredDeltaNs))
$fasterPath = if ($deferredDeltaNs -gt 0) { "Tiled" } elseif ($deferredDeltaNs -lt 0) { "Clustered" } else { "Tie" }

$lines = New-Object System.Collections.Generic.List[string]
$lines.Add("# PBR Deferred Light Culling Timing Report")
$lines.Add("")
$lines.Add("生成时间：$generated")
$lines.Add("仓库：``$repoRoot``")
$lines.Add("分支：``$branch``")
$lines.Add("提交：``$commit``")
$lines.Add("配置：``$Configuration|$Platform``")
$lines.Add("样本数：``$Samples``")
$lines.Add("样本 CSV：``$SamplesCsvPath``")
$lines.Add("")
$lines.Add("## 测试范围")
$lines.Add("")
$lines.Add('本报告只比较同一个 8 点光 pressure rig 下的 deferred PBR light culling 路径。两个模式都会启用 renderer pass 级 `GL_TIME_ELAPSED`，并使用跨帧 deferred query readback，避免为了当前帧 timing 主动阻塞 GPU。')
$lines.Add("")
$lines.Add("复现命令：")
$lines.Add("")
$lines.Add('```powershell')
$lines.Add("powershell -NoProfile -ExecutionPolicy Bypass -File tools\profile_pbr_light_culling.ps1 -NoLinkDebugInfo -Samples $Samples")
$lines.Add('```')
$lines.Add("")
$lines.Add("底层 verification modes：")
$lines.Add("")
$lines.Add('- `deferred-tiled-lights-pressure-timing`')
$lines.Add('- `deferred-clustered-grid-pressure-timing`')
$lines.Add("")
$lines.Add("## 结果")
$lines.Add("")
$lines.Add("| Path | Grid | Point lights | Live indices | Culled / candidate | Occupancy | G-buffer avg/min/max ms | Deferred lighting avg/min/max ms | Frame avg/min/max ms | Samples |")
$lines.Add("| --- | --- | ---: | ---: | --- | --- | --- | --- | --- | ---: |")
$lines.Add("| $($tiledAggregate.Kind) | ``$($tiledAggregate.Grid)`` | $($tiledAggregate.PointLights) | $($tiledAggregate.LiveIndices) | $($tiledAggregate.CulledIndices) / $($tiledAggregate.FullIndices) ($($tiledAggregate.CullingPercent)) | $($tiledAggregate.Occupancy) | $($tiledAggregate.GBufferSummaryMs) | $($tiledAggregate.DeferredLightingSummaryMs) | $($tiledAggregate.FrameSummaryMs) | $($tiledAggregate.SampleCount) |")
$lines.Add("| $($clusteredAggregate.Kind) | ``$($clusteredAggregate.Grid)`` | $($clusteredAggregate.PointLights) | $($clusteredAggregate.LiveIndices) | $($clusteredAggregate.CulledIndices) / $($clusteredAggregate.FullIndices) ($($clusteredAggregate.CullingPercent)) | $($clusteredAggregate.Occupancy) | $($clusteredAggregate.GBufferSummaryMs) | $($clusteredAggregate.DeferredLightingSummaryMs) | $($clusteredAggregate.FrameSummaryMs) | $($clusteredAggregate.SampleCount) |")
$lines.Add("")
$lines.Add("## 样本")
$lines.Add("")
$lines.Add("| Sample | Path | G-buffer GPU ms | Deferred lighting GPU ms | Frame GPU ms | Pending queries | Log |")
$lines.Add("| ---: | --- | ---: | ---: | ---: | ---: | --- |")
foreach ($sample in ($sampleResults | Sort-Object Sample, PathOrder)) {
    $lines.Add("| $($sample.Sample) | $($sample.Kind) | $($sample.GBufferMs) | $($sample.DeferredLightingMs) | $($sample.FrameMs) | $($sample.PendingQueries) | ``$($sample.Log)`` |")
}
$lines.Add("")
$lines.Add("## 解释")
$lines.Add("")
if ($fasterPath -eq "Tie") {
    $lines.Add("- 两条路径本次样本集的 deferred lighting GPU 平均时间相同。")
}
else {
    $lines.Add("- 本次样本集中 ``$fasterPath`` 的 deferred lighting pass 平均时间更短，差值约 ``$deferredDeltaMs ms``。该值是当前机器和当前 pressure rig 的测量结果，不应直接外推为所有场景结论。")
}
$lines.Add("- Tiled 路径使用屏幕 2D tile list，candidate 数量较小，适合作为当前稳定 fallback。")
$lines.Add("- Clustered 路径使用 3D grid + compute assignment，candidate 空间更大，但已经具备与深度 slice 相关的 PBR 扩展基础；后续需要继续解决 overflow/fallback、多帧采样统计和真实资产 baseline。")
$lines.Add('- 两条 timing mode 都保留在 `tools/verify_pbr.ps1` 默认回归中，避免后续重构误删 profiling 入口。')
$lines.Add("")
$lines.Add("## 最后一轮原始 renderer stats")
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
