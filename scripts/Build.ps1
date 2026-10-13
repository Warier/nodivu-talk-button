$ErrorActionPreference='Stop'
$projectRoot=Split-Path $PSScriptRoot -Parent
if(-not(Get-Command cl.exe -ErrorAction SilentlyContinue) -or $env:VSCMD_ARG_TGT_ARCH -notin @('x64','amd64')){
 $vswhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
 $vs=& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
 if(-not $vs){throw 'Instale Visual Studio Build Tools com C++ x64 e Windows SDK.'}
 & (Join-Path $vs 'Common7/Tools/Launch-VsDevShell.ps1') -Arch amd64 -HostArch amd64 -SkipAutomaticLocation | Out-Null
}
$build=Join-Path $projectRoot 'build';$package=Join-Path $projectRoot 'dist/package'
New-Item -ItemType Directory -Force -Path $build,$package | Out-Null
Push-Location $build
try{
 & cl.exe /nologo /utf-8 /std:c++17 /EHsc /O2 /W4 /WX /MT /LD "/I$projectRoot/third_party/clap/include" "$projectRoot/src/clap_plugin.cpp" /link "/OUT:$package/talk-button.clap"
 if($LASTEXITCODE){throw 'Build CLAP falhou'}
 & cl.exe /nologo /utf-8 /std:c++17 /EHsc /O2 /W4 /WX /MT "$projectRoot/tests/dsp_test.cpp" /Fe:dsp_test.exe
 if($LASTEXITCODE){throw 'Build de teste falhou'}
 & ./dsp_test.exe
 if($LASTEXITCODE){throw 'Teste DSP falhou'}
 foreach($file in @('plugin.json','nodivu-package.json','README.md','logo.png','LICENSE','THIRD-PARTY-NOTICES.md')){Copy-Item -LiteralPath "$projectRoot/$file" -Destination $package}
 Copy-Item -LiteralPath "$projectRoot/third_party/clap/LICENSE" -Destination "$package/CLAP-LICENSE"
 $meta=Get-Content "$projectRoot/nodivu-package.json" -Raw | ConvertFrom-Json
 $zip=Join-Path $projectRoot "dist/nodivu-talk-button-$($meta.version)-windows-x64.zip"
 Compress-Archive -Path "$package/*" -DestinationPath $zip -Force
 ((Get-FileHash -LiteralPath $zip -Algorithm SHA256).Hash.ToLower()+'  '+(Split-Path $zip -Leaf)) | Set-Content "$projectRoot/dist/SHA256SUMS.txt"
 Write-Output "PASS: $zip"
}finally{Pop-Location}
