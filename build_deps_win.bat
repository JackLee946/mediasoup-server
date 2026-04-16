@echo off
rem ============================================================
rem  build_deps_win.bat
rem  Build all Windows dependencies for mediasoup-server (VS2019)
rem  Run from the repo root with a VS2019 x64 Developer Prompt.
rem ============================================================

set CURRENT_DIR=%~dp0
set DEPS_DIR=%CURRENT_DIR%deps
set OUT_DIR=%CURRENT_DIR%lib\win\x64

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

rem -------------------------------------------------------
rem  libuv
rem -------------------------------------------------------
echo.
echo === Building libuv ===
set LIBUV_DIR=%DEPS_DIR%\libuv
set LIBUV_BUILD=%LIBUV_DIR%\build_win

if not exist "%LIBUV_BUILD%" mkdir "%LIBUV_BUILD%"
cd /d "%LIBUV_BUILD%"
cmake .. -G "Visual Studio 16 2019" -A x64 -DLIBUV_BUILD_TESTS=OFF
cmake --build . --config Release
copy /y Release\uv_a.lib "%OUT_DIR%\"

rem -------------------------------------------------------
rem  OpenSSL (requires perl + nasm in PATH)
rem  Alternatively point OPENSSL_ROOT_DIR to a pre-built install.
rem  Skipping build here — install Win64 OpenSSL from
rem  https://slproweb.com/products/Win32OpenSSL.html
rem  and adjust the path in genvs2019.bat.
rem -------------------------------------------------------
echo.
echo === OpenSSL: please install Win64 OpenSSL 3.x from ===
echo     https://slproweb.com/products/Win32OpenSSL.html
echo     Then copy libssl.lib + libcrypto.lib to %OUT_DIR%
echo.

rem -------------------------------------------------------
rem  libsrtp2
rem -------------------------------------------------------
echo.
echo === Building libsrtp2 ===
set SRTP_DIR=%DEPS_DIR%\libsrtp
set SRTP_BUILD=%SRTP_DIR%\build_win

if not exist "%SRTP_BUILD%" mkdir "%SRTP_BUILD%"
cd /d "%SRTP_BUILD%"
cmake .. -G "Visual Studio 16 2019" -A x64 ^
    -DENABLE_OPENSSL=ON ^
    -DTEST_APPS=OFF
cmake --build . --config Release
copy /y Release\srtp2.lib "%OUT_DIR%\"

rem -------------------------------------------------------
rem  usrsctp
rem -------------------------------------------------------
echo.
echo === Building usrsctp ===
set SCTP_DIR=%DEPS_DIR%\usrsctp
set SCTP_BUILD=%SCTP_DIR%\build_win

if not exist "%SCTP_BUILD%" mkdir "%SCTP_BUILD%"
cd /d "%SCTP_BUILD%"
cmake .. -G "Visual Studio 16 2019" -A x64 ^
    -Dsctp_werror=OFF ^
    -Dsctp_build_programs=OFF ^
    -Dsctp_build_fuzzer=OFF
cmake --build . --config Release
copy /y Release\usrsctp.lib "%OUT_DIR%\" 2>nul
copy /y usrsctplib\Release\usrsctp.lib "%OUT_DIR%\" 2>nul

rem -------------------------------------------------------
rem  oatpp
rem -------------------------------------------------------
echo.
echo === Building oatpp ===
set OATPP_DIR=%DEPS_DIR%\oatpp
set OATPP_BUILD=%OATPP_DIR%\build_win

if not exist "%OATPP_BUILD%" mkdir "%OATPP_BUILD%"
cd /d "%OATPP_BUILD%"
cmake .. -G "Visual Studio 16 2019" -A x64 ^
    -DOATPP_BUILD_TESTS=OFF ^
    -DOATPP_INSTALL=OFF
cmake --build . --config Release
copy /y src\Release\oatpp.lib "%OUT_DIR%\"

rem -------------------------------------------------------
rem  oatpp-openssl
rem -------------------------------------------------------
echo.
echo === Building oatpp-openssl ===
set OATPP_SSL_DIR=%DEPS_DIR%\oatpp-openssl
set OATPP_SSL_BUILD=%OATPP_SSL_DIR%\build_win

if not exist "%OATPP_SSL_BUILD%" mkdir "%OATPP_SSL_BUILD%"
cd /d "%OATPP_SSL_BUILD%"
cmake .. -G "Visual Studio 16 2019" -A x64 ^
    -Doatpp_DIR="%OATPP_BUILD%" ^
    -DOATPP_BUILD_TESTS=OFF
cmake --build . --config Release
copy /y src\Release\oatpp-openssl.lib "%OUT_DIR%\"

rem -------------------------------------------------------
rem  oatpp-websocket
rem -------------------------------------------------------
echo.
echo === Building oatpp-websocket ===
set OATPP_WS_DIR=%DEPS_DIR%\oatpp-websocket
set OATPP_WS_BUILD=%OATPP_WS_DIR%\build_win

if not exist "%OATPP_WS_BUILD%" mkdir "%OATPP_WS_BUILD%"
cd /d "%OATPP_WS_BUILD%"
cmake .. -G "Visual Studio 16 2019" -A x64 ^
    -Doatpp_DIR="%OATPP_BUILD%" ^
    -DOATPP_BUILD_TESTS=OFF
cmake --build . --config Release
copy /y src\Release\oatpp-websocket.lib "%OUT_DIR%\"

rem -------------------------------------------------------
rem  abseil-cpp
rem -------------------------------------------------------
echo.
echo === Building abseil-cpp ===
set ABSL_DIR=%DEPS_DIR%\abseil-cpp
set ABSL_BUILD=%ABSL_DIR%\build_win

if not exist "%ABSL_BUILD%" mkdir "%ABSL_BUILD%"
cd /d "%ABSL_BUILD%"
cmake .. -G "Visual Studio 16 2019" -A x64 ^
    -DABSL_BUILD_TESTING=OFF ^
    -DABSL_ENABLE_INSTALL=OFF
cmake --build . --config Release
rem Copy all generated .lib files
for /r . %%f in (Release\absl_*.lib) do copy /y "%%f" "%OUT_DIR%\"

echo.
echo === Done building Windows dependencies ===
echo     Libraries placed in: %OUT_DIR%
echo.
echo Next steps:
echo   1. Copy OpenSSL libssl.lib + libcrypto.lib to %OUT_DIR%
echo   2. Run genvs2019.bat to generate the VS solution
echo   3. Open vc2019\mediasoup-server.sln in VS2019 and build

cd /d "%CURRENT_DIR%"
pause
