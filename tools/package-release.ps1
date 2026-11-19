param(
    [string]$WorkspaceRoot = (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent),
    [string]$Version = (Get-Content (Join-Path (Split-Path $PSScriptRoot -Parent) 'VERSION') -Raw).Trim(),
    [switch]$OpenGLOnly
)
$ErrorActionPreference='Stop'
if ($Version -notmatch '^\d+\.\d+(\.\d+)?$') { throw 'Invalid release version' }
$sourceRoot=Split-Path $PSScriptRoot -Parent
$primary=Join-Path $WorkspaceRoot "builds/v$Version/opengl"
$fallback=Join-Path $WorkspaceRoot "builds/v$Version/d3d9"
$runtime=Join-Path $WorkspaceRoot 'dependencies/runtime/x64'
$publishRelease=Join-Path $WorkspaceRoot "releases/v$Version"
$zipPath=Join-Path $WorkspaceRoot "releases/re3-enhanced-v$Version.zip"
if ((Test-Path -LiteralPath $publishRelease) -or (Test-Path -LiteralPath $zipPath)) {
    throw 'Version already packaged. Increment VERSION; published packages are not overwritten.'
}
$packagingCommit=(git -C $sourceRoot rev-parse HEAD).Trim()
$commit=(Get-Content (Join-Path $primary 'BUILD-COMMIT.txt') -Raw).Trim()
if (git -C $sourceRoot status --porcelain) { throw 'Source tree must be clean before packaging' }
$changes=@(git -C $sourceRoot diff --name-only $commit $packagingCommit)
$codeChanges=@($changes | Where-Object { $_ -notmatch '^(tools/|docs/|ENHANCED\.md$|ROADMAP\.md$|RELEASE-NOTES\.txt$)' })
if ($codeChanges.Count) { throw 'Build source differs from current compiler inputs; rebuild first' }
$staging=Join-Path $WorkspaceRoot ("builds/v$Version/package-"+[Guid]::NewGuid().ToString('N'))
$release=Join-Path $staging "v$Version"
$zipStage=Join-Path $staging "re3-enhanced-v$Version.zip"
$builds=if ($OpenGLOnly) { @($primary) } else { @($primary,$fallback) }
foreach ($build in $builds) {
    if ((Get-Content (Join-Path $build 'BUILD-COMMIT.txt') -Raw).Trim() -ne $commit) {
        throw "Build/source commit mismatch: $build"
    }
    & (Join-Path $WorkspaceRoot 'tools/verify-pdb.ps1') -Directory $build
    if ($LASTEXITCODE -and $LASTEXITCODE -ne 0) { throw 'EXE/PDB verification failed' }
}
$game=Join-Path $release 'game'
$alternative=Join-Path $release 'alternatives/Direct3D9'
$docs=Join-Path $release 'docs'
foreach($folder in @($game,$docs)) { $null=New-Item -ItemType Directory -Path $folder -Force }
Get-ChildItem -LiteralPath $primary -Force | Copy-Item -Destination $game -Recurse -Force
Get-ChildItem -LiteralPath $runtime -Force | Copy-Item -Destination $game -Recurse -Force
if (!$OpenGLOnly) {
    $null=New-Item -ItemType Directory -Path $alternative -Force
    foreach($name in @('re3.exe','re3.pdb','BUILD-COMMIT.txt','BUILD-SHA256.txt')) {
        Copy-Item -LiteralPath (Join-Path $fallback $name) -Destination $alternative
    }
}
foreach($name in @('ENHANCED.md','ROADMAP.md')) {
    Copy-Item -LiteralPath (Join-Path $sourceRoot $name) -Destination $release
}
Get-ChildItem -LiteralPath (Join-Path $sourceRoot 'docs') -Force | Copy-Item -Destination $docs -Recurse -Force
Copy-Item -LiteralPath (Join-Path $sourceRoot 'VERSION') -Destination $release
Copy-Item -LiteralPath (Join-Path $sourceRoot 'RELEASE-NOTES.txt') -Destination (Join-Path $release 'KURULUM.txt')
@('scripts','plugins','CLEO/CLEO_TEXT') | ForEach-Object {
    $folder=Join-Path $game $_
    $null=New-Item -ItemType Directory -Path $folder -Force
    'Place compatible x64 ASI mods or CLEO scripts/text files in the appropriate folder.' |
        Set-Content -LiteralPath (Join-Path $folder 'README.txt') -Encoding UTF8
}
# Reject accidental x86 ASIs/plugins and mismatched executable backends.
foreach($file in Get-ChildItem -LiteralPath $release -Recurse -File | Where-Object Extension -in '.exe','.dll','.asi','.cleo') {
    $bytes=[IO.File]::ReadAllBytes($file.FullName)
    if ($bytes.Length -lt 64 -or [BitConverter]::ToUInt16($bytes,0) -ne 0x5A4D) { throw "Not a PE: $($file.Name)" }
    $pe=[BitConverter]::ToInt32($bytes,0x3C)
    if($pe -lt 0 -or $pe+24 -gt $bytes.Length -or [BitConverter]::ToUInt32($bytes,$pe) -ne 0x4550 -or
        [BitConverter]::ToUInt16($bytes,$pe+4) -ne 0x8664) { throw "Not x64: $($file.FullName)" }
}
$mainBytes=[Text.Encoding]::ASCII.GetString([IO.File]::ReadAllBytes((Join-Path $game 're3.exe')))
if (!$mainBytes.ToLowerInvariant().Contains('opengl32.dll') -or !$mainBytes.ToLowerInvariant().Contains('winmm.dll')) {
    throw 'Main EXE must contain the OpenGL platform path and import WINMM for ASI loading'
}
$files=@(Get-ChildItem -LiteralPath $release -Recurse -File | Sort-Object FullName | ForEach-Object {
    [ordered]@{path=$_.FullName.Substring($release.Length+1).Replace('\','/');bytes=$_.Length;sha256=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash}
})
[ordered]@{version=$Version;sourceCommit=$commit;packagingCommit=$packagingCommit;mainBackend='OpenGL';fallbackBackend=$(if ($OpenGLOnly) { $null } else { 'Direct3D9' });files=$files} |
    ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $release 'manifest.json') -Encoding UTF8
Compress-Archive -LiteralPath $release -DestinationPath $zipStage
# Publish only after all checks and compression succeed. All moves stay within
# the explicitly selected workspace; existing published versions stay immutable.
$workspacePrefix=[IO.Path]::GetFullPath($WorkspaceRoot).TrimEnd('\')+'\'
foreach($path in @($release,$publishRelease,$zipStage,$zipPath)) {
    if (![IO.Path]::GetFullPath($path).StartsWith($workspacePrefix,[StringComparison]::OrdinalIgnoreCase)) {
        throw 'Publish path escapes workspace'
    }
}
Move-Item -LiteralPath $release -Destination $publishRelease
Move-Item -LiteralPath $zipStage -Destination $zipPath
Get-FileHash -LiteralPath $zipPath -Algorithm SHA256 | Format-List | Out-String |
    Set-Content -LiteralPath "$zipPath.sha256.txt" -Encoding UTF8
Write-Output "PACKAGED v$Version source=$commit files=$($files.Count)"
Get-Item -LiteralPath $zipPath | Select-Object FullName,Length
