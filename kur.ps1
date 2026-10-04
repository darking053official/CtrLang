# CtrLang - Windows PowerShell kurulum

$ErrorActionPreference = "Stop"

Write-Host "=== CtrLang Kurulum ===" -ForegroundColor Cyan
Write-Host ""

$cc = $null
if (Get-Command gcc -ErrorAction SilentlyContinue) {
    $cc = "gcc"
} elseif (Get-Command clang -ErrorAction SilentlyContinue) {
    $cc = "clang"
} else {
    Write-Host "HATA: gcc veya clang bulunamadi" -ForegroundColor Red
    exit 1
}

Write-Host "Derleyici: $cc"
Write-Host ""

Write-Host "Derleniyor..." -ForegroundColor Yellow
if (!(Test-Path build)) { New-Item -ItemType Directory -Path build | Out-Null }

& $cc -std=c11 -O2 -w -Isrc -Ikutuphane -Ivendor\mongoose -Ivendor\sqlite -Ivendor\cjson -Ivendor\sds -Ivendor\uthash src\*.c -o ctrc.exe -lws2_32 -lpthread

if (!(Test-Path ctrc.exe)) {
    Write-Host "HATA: Derleme basarisiz" -ForegroundColor Red
    exit 1
}

Write-Host "Derleme basarili" -ForegroundColor Green
Write-Host ""

$binDir = "$env:USERPROFILE\CtrLang"
if (!(Test-Path $binDir)) { New-Item -ItemType Directory -Path $binDir | Out-Null }

Copy-Item ctrc.exe "$binDir\ctrc.exe" -Force
Copy-Item ctrc.exe "$binDir\ctr.exe" -Force

Write-Host "Kuruldu: $binDir" -ForegroundColor Green
Write-Host ""

$userPath = [Environment]::GetEnvironmentVariable("PATH", "User")
if ($userPath -notlike "*$binDir*") {
    [Environment]::SetEnvironmentVariable("PATH", "$userPath;$binDir", "User")
    Write-Host "PATH guncellendi" -ForegroundColor Green
}

Write-Host ""
Write-Host "Test:" -ForegroundColor Yellow
& "$binDir\ctrc.exe" --platform

Write-Host ""
Write-Host "Kurulum tamam!" -ForegroundColor Green
