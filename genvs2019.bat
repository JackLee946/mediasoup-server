@echo off
set CURRENT_DIR=%~dp0
set PROJECT_NAME=vc2019

echo %CURRENT_DIR%%PROJECT_NAME%
if not exist %CURRENT_DIR%%PROJECT_NAME% (
    mkdir %CURRENT_DIR%%PROJECT_NAME%
)
del /f/s/q "%CURRENT_DIR%%PROJECT_NAME%\CMakeCache.txt" 2>nul
cd /d %CURRENT_DIR%%PROJECT_NAME%

rem Set OpenSSL path (adjust if OpenSSL is installed elsewhere)
set OPENSSL_ROOT_DIR=%CURRENT_DIR%deps\openssl

rem Generate VS2019 project (x64 platform).
cmake ../ -G "Visual Studio 16 2019" -A x64 ^
    -DOPENSSL_ROOT_DIR="%OPENSSL_ROOT_DIR%" ^
    -DCMAKE_BUILD_TYPE=Debug

pause
