@echo off
set CURRENT_DIR=%~dp0
set PROJECT_NAME=vc2019

echo %CURRENT_DIR%%PROJECT_NAME%
if not exist %CURRENT_DIR%%PROJECT_NAME% (
    mkdir %CURRENT_DIR%%PROJECT_NAME%
)
del /f/s/q "%CURRENT_DIR%%PROJECT_NAME%\CMakeCache.txt" 2>nul
cd /d %CURRENT_DIR%%PROJECT_NAME%

rem Set OpenSSL path. Override OPENSSL_ROOT_DIR if OpenSSL is installed elsewhere.
if "%OPENSSL_ROOT_DIR%"=="" (
    if exist "%CURRENT_DIR%release\lib\libcrypto.a" (
        set OPENSSL_ROOT_DIR=%CURRENT_DIR%release
    ) else if exist "%CURRENT_DIR%deps\openssl\build_win_x64\install\lib\libssl.lib" (
        set OPENSSL_ROOT_DIR=%CURRENT_DIR%deps\openssl\build_win_x64\install
    ) else (
        set OPENSSL_ROOT_DIR=C:\Program Files\OpenSSL-Win64
    )
)

echo OPENSSL_ROOT_DIR=%OPENSSL_ROOT_DIR%

rem NOTE: -DCMAKE_GENERATOR_INSTANCE explicitly points CMake at the VS install,
rem bypassing vswhere.exe (whose instance registration is currently corrupted
rem on this machine). CMAKE_BUILD_TYPE is intentionally NOT set here: VS is a
rem multi-config generator and CMAKE_BUILD_TYPE has no effect on it, so it
rem must not be baked into any output-path variables (see CMakeLists.txt).
cmake ../ -G "Visual Studio 16 2019" -A x64 ^
    -DOPENSSL_ROOT_DIR="%OPENSSL_ROOT_DIR%" ^
    -DCMAKE_GENERATOR_INSTANCE="C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional"

pause
