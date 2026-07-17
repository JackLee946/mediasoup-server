@echo off
setlocal

rem ============================================================
rem  Build OpenSSL for VS2019 x64.
rem  Prerequisites: Perl in PATH. NASM is optional because no-asm is used.
rem ============================================================

set CURRENT_DIR=%~dp0
set OPENSSL_SRC=%CURRENT_DIR%deps\openssl
set OPENSSL_BUILD=%OPENSSL_SRC%\build_win_x64
set OPENSSL_INSTALL=%OPENSSL_BUILD%\install

if not exist "%OPENSSL_SRC%\Configure" (
    echo OpenSSL source directory was not found: %OPENSSL_SRC%
    exit /b 1
)

where perl >nul 2>nul
if errorlevel 1 (
    echo Perl was not found in PATH. Install Strawberry Perl or ActivePerl first.
    exit /b 1
)

where nmake >nul 2>nul
if errorlevel 1 (
    echo nmake was not found in PATH. Run this script from a VS2019 x64 Developer Command Prompt.
    exit /b 1
)

if not exist "%OPENSSL_BUILD%" mkdir "%OPENSSL_BUILD%"
cd /d "%OPENSSL_SRC%"

perl Configure VC-WIN64A no-asm no-shared --prefix="%OPENSSL_INSTALL%" --openssldir="%OPENSSL_INSTALL%\ssl"
if errorlevel 1 exit /b %ERRORLEVEL%

nmake
if errorlevel 1 exit /b %ERRORLEVEL%

nmake install_sw
if errorlevel 1 exit /b %ERRORLEVEL%

echo.
echo OpenSSL was installed to:
echo   %OPENSSL_INSTALL%
echo.
echo Use this path as OPENSSL_ROOT_DIR when running genvs2019.bat.

endlocal
