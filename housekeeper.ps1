# Wrapper for the housekeeper.ps1 script from git@github.com:dimatcimbal/Housekeeper.git
# $scriptPath = Join-Path (Get-Item -Path $PSScriptRoot).FullName "..\Housekeeper\housekeeper.ps1"
$scriptPath = Join-Path (Get-Item -Path $PSScriptRoot).FullName "Housekeeper\housekeeper.ps1"
& $scriptPath @Args