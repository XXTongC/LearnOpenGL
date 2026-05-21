param(
    [string]$Configuration = "Debug",
    [string]$Platform = "x64",
    [switch]$SkipBuild,
    [switch]$NoLinkDebugInfo,
    [switch]$UpdateBaseline,
    [switch]$StrictHash,
    [switch]$DiscardCaptures,
    [string]$BaselinePath = "docs/pbr_golden_baselines.json",
    [string]$ReportPath = "docs/pbr_golden_verification_report.md"
)

$ErrorActionPreference = "Stop"

$scriptPath = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Resolve-Path (Join-Path $scriptPath "..")
$outDir = Join-Path $repoRoot "out"
$verifyScript = Join-Path $scriptPath "verify_pbr.ps1"
$goldenModes = @(
    [pscustomobject]@{
        Name = "import"
        Capture = "out/pbr_import_verification.ppm"
        RequiredScenePatterns = @("pbrImportedMeshes=1")
        RequiredRendererPatterns = @("pbrDrawCalls=26")
    },
    [pscustomobject]@{
        Name = "texture-set"
        Capture = "out/pbr_texture_set_verification.ppm"
        RequiredScenePatterns = @("pbrTexturedMeshes=1")
        RequiredRendererPatterns = @("pbrDrawCalls=26")
    },
    [pscustomobject]@{
        Name = "deferred-texture-set"
        Capture = "out/pbr_deferred_texture_set_verification.ppm"
        RequiredScenePatterns = @("pbrTexturedMeshes=1")
        RequiredRendererPatterns = @("pbrGBufferDrawCalls=26", "pbrDeferredLightingDrawCalls=1")
    }
)

function Resolve-RepoPath {
    param([string]$Path)

    if ([System.IO.Path]::IsPathRooted($Path)) {
        return $Path
    }

    return Join-Path $repoRoot $Path
}

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
        throw "Missing capture: $Path"
    }

    $bytes = [System.IO.File]::ReadAllBytes($Path)
    $index = 0
    $magic = Read-PpmToken -Bytes $bytes -Index ([ref]$index)
    $widthToken = Read-PpmToken -Bytes $bytes -Index ([ref]$index)
    $heightToken = Read-PpmToken -Bytes $bytes -Index ([ref]$index)
    $maxToken = Read-PpmToken -Bytes $bytes -Index ([ref]$index)

    if ($magic -ne "P6") {
        throw "Capture is not P6 PPM: $Path"
    }

    $width = [int]$widthToken
    $height = [int]$heightToken
    $maxValue = [int]$maxToken
    $pixelCount = $width * $height
    $expectedDataBytes = $pixelCount * 3
    if ($maxValue -ne 255 -or $bytes.Length -lt ($index + $expectedDataBytes)) {
        throw "Invalid PPM payload: $Path"
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
        Width = $width
        Height = $height
        Bytes = $bytes.Length
        NonBlackPercent = [Math]::Round(($nonBlack * 100.0) / $pixelCount, 4)
        MeanRed = [Math]::Round($redSum / $pixelCount, 2)
        MeanGreen = [Math]::Round($greenSum / $pixelCount, 2)
        MeanBlue = [Math]::Round($blueSum / $pixelCount, 2)
        Sha256 = (Get-FileHash -Path $Path -Algorithm SHA256).Hash.ToLowerInvariant()
    }
}

function Get-ModeResult {
    param([object]$Mode)

    $capturePath = Resolve-RepoPath -Path $Mode.Capture
    $logPath = Join-Path $outDir ("pbr_verify_{0}.log" -f $Mode.Name)
    if (!(Test-Path $logPath)) {
        throw "Missing verification log: $logPath"
    }

    $logLines = Get-Content -Path $logPath
    $sceneLine = $logLines | Where-Object { $_ -match "PBR verification scene stats:" } | Select-Object -Last 1
    $rendererLine = $logLines | Where-Object { $_ -match "PBR verification renderer stats:" } | Select-Object -Last 1
    if (!$sceneLine) {
        throw "Missing scene stats in $logPath"
    }
    if (!$rendererLine) {
        throw "Missing renderer stats in $logPath"
    }

    return [pscustomobject]@{
        Name = $Mode.Name
        Capture = $Mode.Capture
        CapturePath = $capturePath
        Log = ("out/" + [System.IO.Path]::GetFileName($logPath))
        Stats = Get-PpmStats -Path $capturePath
        SceneStats = $sceneLine
        RendererStats = $rendererLine
        RequiredScenePatterns = $Mode.RequiredScenePatterns
        RequiredRendererPatterns = $Mode.RequiredRendererPatterns
    }
}

function Invoke-PbrGoldenModes {
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
        (($goldenModes | ForEach-Object { $_.Name }) -join ",")
    )
    if ($SkipBuild) {
        $verifyArgs += "-SkipBuild"
    }
    if ($NoLinkDebugInfo) {
        $verifyArgs += "-NoLinkDebugInfo"
    }

    & powershell.exe @verifyArgs
    if ($LASTEXITCODE -ne 0) {
        throw "PBR golden verification modes failed with exit code $LASTEXITCODE"
    }
}

function New-BaselineDocument {
    param([object[]]$Results)

    $commit = (git -C $repoRoot rev-parse --short HEAD).Trim()
    return [ordered]@{
        generated = (Get-Date -Format "yyyy-MM-dd HH:mm:ss")
        repository = "$repoRoot"
        commit = $commit
        configuration = $Configuration
        platform = $Platform
        tolerances = [ordered]@{
            nonBlackPercent = 0.25
            meanRgb = 0.75
        }
        modes = @(
            foreach ($result in $Results) {
                [ordered]@{
                    name = $result.Name
                    capture = $result.Capture
                    width = $result.Stats.Width
                    height = $result.Stats.Height
                    bytes = $result.Stats.Bytes
                    nonBlackPercent = $result.Stats.NonBlackPercent
                    meanRed = $result.Stats.MeanRed
                    meanGreen = $result.Stats.MeanGreen
                    meanBlue = $result.Stats.MeanBlue
                    sha256 = $result.Stats.Sha256
                    requiredScenePatterns = @($result.RequiredScenePatterns)
                    requiredRendererPatterns = @($result.RequiredRendererPatterns)
                }
            }
        )
    }
}

function Test-NumericNear {
    param(
        [double]$Actual,
        [double]$Expected,
        [double]$Tolerance
    )

    return [Math]::Abs($Actual - $Expected) -le $Tolerance
}

function Compare-ResultToBaseline {
    param(
        [object]$Result,
        [object]$BaselineMode,
        [object]$Tolerances
    )

    $failures = New-Object System.Collections.Generic.List[string]
    if ($Result.Stats.Width -ne [int]$BaselineMode.width -or $Result.Stats.Height -ne [int]$BaselineMode.height) {
        $failures.Add("size $($Result.Stats.Width)x$($Result.Stats.Height) != $($BaselineMode.width)x$($BaselineMode.height)")
    }
    if ($Result.Stats.Bytes -ne [int]$BaselineMode.bytes) {
        $failures.Add("bytes $($Result.Stats.Bytes) != $($BaselineMode.bytes)")
    }
    if (!(Test-NumericNear -Actual $Result.Stats.NonBlackPercent -Expected ([double]$BaselineMode.nonBlackPercent) -Tolerance ([double]$Tolerances.nonBlackPercent))) {
        $failures.Add("nonblack $($Result.Stats.NonBlackPercent) outside baseline $($BaselineMode.nonBlackPercent) +/- $($Tolerances.nonBlackPercent)")
    }
    if (!(Test-NumericNear -Actual $Result.Stats.MeanRed -Expected ([double]$BaselineMode.meanRed) -Tolerance ([double]$Tolerances.meanRgb))) {
        $failures.Add("mean red $($Result.Stats.MeanRed) outside baseline $($BaselineMode.meanRed) +/- $($Tolerances.meanRgb)")
    }
    if (!(Test-NumericNear -Actual $Result.Stats.MeanGreen -Expected ([double]$BaselineMode.meanGreen) -Tolerance ([double]$Tolerances.meanRgb))) {
        $failures.Add("mean green $($Result.Stats.MeanGreen) outside baseline $($BaselineMode.meanGreen) +/- $($Tolerances.meanRgb)")
    }
    if (!(Test-NumericNear -Actual $Result.Stats.MeanBlue -Expected ([double]$BaselineMode.meanBlue) -Tolerance ([double]$Tolerances.meanRgb))) {
        $failures.Add("mean blue $($Result.Stats.MeanBlue) outside baseline $($BaselineMode.meanBlue) +/- $($Tolerances.meanRgb)")
    }
    if ($StrictHash -and $Result.Stats.Sha256 -ne $BaselineMode.sha256) {
        $failures.Add("sha256 $($Result.Stats.Sha256) != $($BaselineMode.sha256)")
    }

    foreach ($pattern in @($BaselineMode.requiredScenePatterns)) {
        if ($Result.SceneStats -notmatch $pattern) {
            $failures.Add("scene stats missing pattern '$pattern'")
        }
    }
    foreach ($pattern in @($BaselineMode.requiredRendererPatterns)) {
        if ($Result.RendererStats -notmatch $pattern) {
            $failures.Add("renderer stats missing pattern '$pattern'")
        }
    }

    return $failures
}

function Write-GoldenReport {
    param(
        [object[]]$Results,
        [hashtable]$FailuresByMode,
        [bool]$BaselineUpdated
    )

    $reportFullPath = Resolve-RepoPath -Path $ReportPath
    $reportDir = Split-Path -Parent $reportFullPath
    if (!(Test-Path $reportDir)) {
        New-Item -ItemType Directory -Path $reportDir | Out-Null
    }

    $commit = (git -C $repoRoot rev-parse --short HEAD).Trim()
    $lines = New-Object System.Collections.Generic.List[string]
    $lines.Add("# PBR Golden Verification Report")
    $lines.Add("")
    $lines.Add("生成时间：$(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')")
    $lines.Add("仓库：``$repoRoot``")
    $lines.Add("提交：``$commit``")
    $lines.Add("配置：``$Configuration|$Platform``")
    $lines.Add("Baseline：``$BaselinePath``")
    $lines.Add("UpdateBaseline：``$BaselineUpdated``")
    $lines.Add("StrictHash：``$($StrictHash.IsPresent)``")
    $lines.Add("")
    $lines.Add("## 结果")
    $lines.Add("")
    $lines.Add("| Mode | Size | Non-black | Mean RGB | SHA256 | Result | Capture | Log |")
    $lines.Add("| --- | --- | ---: | --- | --- | --- | --- | --- |")
    foreach ($result in $Results) {
        $failureText = if ($FailuresByMode.ContainsKey($result.Name) -and $FailuresByMode[$result.Name].Count -gt 0) { "failed" } else { "passed" }
        $shaPrefix = $result.Stats.Sha256.Substring(0, 12)
        $meanRgb = "$($result.Stats.MeanRed) / $($result.Stats.MeanGreen) / $($result.Stats.MeanBlue)"
        $lines.Add("| $($result.Name) | $($result.Stats.Width)x$($result.Stats.Height) | $($result.Stats.NonBlackPercent)% | $meanRgb | ``$shaPrefix`` | $failureText | ``$($result.Capture)`` | ``$($result.Log)`` |")
    }

    $lines.Add("")
    $lines.Add("## Baseline 覆盖")
    $lines.Add("")
    $lines.Add("- ``import``：真实 Assimp PBR asset probe，检查 imported mesh 与 forward PBR draw path。")
    $lines.Add("- ``texture-set``：真实 PBR texture set probe，检查 albedo / metallic / roughness / AO / normal 贴图链路。")
    $lines.Add("- ``deferred-texture-set``：同一 texture set 进入 deferred G-buffer + deferred lighting。")
    $lines.Add("")
    $lines.Add("## 失败项")
    $lines.Add("")
    $anyFailure = $false
    foreach ($result in $Results) {
        if (!$FailuresByMode.ContainsKey($result.Name) -or $FailuresByMode[$result.Name].Count -eq 0) {
            continue
        }

        $anyFailure = $true
        foreach ($failure in $FailuresByMode[$result.Name]) {
            $lines.Add("- $($result.Name): $failure")
        }
    }
    if (!$anyFailure) {
        $lines.Add("- 无")
    }

    $lines | Set-Content -Path $reportFullPath -Encoding UTF8
    Write-Host "PBR golden verification report written to $reportFullPath"
}

if (!(Test-Path $verifyScript)) {
    throw "Verification script was not found: $verifyScript"
}

Invoke-PbrGoldenModes

$results = @(
    foreach ($mode in $goldenModes) {
        Get-ModeResult -Mode $mode
    }
)

$baselineFullPath = Resolve-RepoPath -Path $BaselinePath
$baselineDir = Split-Path -Parent $baselineFullPath
if (!(Test-Path $baselineDir)) {
    New-Item -ItemType Directory -Path $baselineDir | Out-Null
}

$failuresByMode = @{}
if ($UpdateBaseline) {
    $baseline = New-BaselineDocument -Results $results
    $baseline | ConvertTo-Json -Depth 8 | Set-Content -Path $baselineFullPath -Encoding UTF8
}
else {
    if (!(Test-Path $baselineFullPath)) {
        throw "Missing baseline file: $baselineFullPath. Run with -UpdateBaseline first."
    }

    $baseline = Get-Content -Path $baselineFullPath -Raw | ConvertFrom-Json
    foreach ($result in $results) {
        $baselineMode = $baseline.modes | Where-Object { $_.name -eq $result.Name } | Select-Object -First 1
        if (!$baselineMode) {
            $failuresByMode[$result.Name] = @("missing baseline mode")
            continue
        }

        $failuresByMode[$result.Name] = @(Compare-ResultToBaseline -Result $result -BaselineMode $baselineMode -Tolerances $baseline.tolerances)
    }
}

Write-GoldenReport -Results $results -FailuresByMode $failuresByMode -BaselineUpdated $UpdateBaseline.IsPresent

if ($DiscardCaptures) {
    foreach ($result in $results) {
        if (Test-Path $result.CapturePath) {
            Remove-Item -Path $result.CapturePath -Force
        }
    }
}

$failureCount = 0
foreach ($key in $failuresByMode.Keys) {
    $failureCount += $failuresByMode[$key].Count
}
if ($failureCount -gt 0) {
    exit 1
}

exit 0
