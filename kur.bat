@echo off
REM CtrLang - Windows kurulum

setlocal enabledelayedexpansion

echo === CtrLang Kurulum ===
echo.

REM Platform algila
ver >nul 2>&1
if errorlevel 1 (
    echo Bu script Windows icindir
    exit /b 1
)

REM Derleyici kontrol
where gcc >nul 2>&1
if errorlevel 1 (
    where clang >nul 2>&1
    if errorlevel 1 (
        echo HATA: gcc veya clang bulunamadi
        echo MinGW kurun: https://www.mingw-w64.org/
        exit /b 1
    )
    set CC=clang
) else (
    set CC=gcc
)

echo Derleyici: %CC%
echo.

REM Derle
echo Derleniyor...
if not exist build mkdir build

%CC% -std=c11 -O2 -w -Isrc -Ikutuphane -Ivendor\mongoose -Ivendor\sqlite -Ivendor\cjson -Ivendor\sds -Ivendor\uthash src\*.c -o ctrc.exe -lws2_32 -lpthread

if not exist ctrc.exe (
    echo HATA: Derleme basarisiz
    exit /b 1
)

echo Derleme basarili
echo.

REM Kur
set BIN_DIR=%USERPROFILE%\CtrLang
if not exist "%BIN_DIR%" mkdir "%BIN_DIR%"

copy ctrc.exe "%BIN_DIR%\ctrc.exe" >nul
copy ctrc.exe "%BIN_DIR%\ctr.exe" >nul

echo Kuruldu: %BIN_DIR%
echo.

REM PATHe ekle
echo PATHe ekleniyor...
setx PATH "%PATH%;%BIN_DIR%" >nul 2>&1

if errorlevel 1 (
    echo UYARI: PATHe eklenemedi. Manuel ekleyin:
    echo   %BIN_DIR%
) else (
    echo PATH guncellendi
)

echo.
echo Test:
"%BIN_DIR%\ctrc.exe" --platform

echo.
echo Kurulum tamam!
echo.
echo Kullanim:
echo   ctr --calistir merhaba.ctr
echo   ctrc --yardim
echo.
echo NOT: Yeni bir terminal acin (PATH icin).

endlocal
