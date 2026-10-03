@echo off
setlocal

for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VS_PATH=%%i"
)

if defined VS_PATH (
    call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat"
) else (
    echo Error: C++ build tools not found on this system.
    exit /b 1
)

:: Ensure output directories exist
if not exist "engine-lib\lib\debug" mkdir "engine-lib\lib\debug"
if not exist "engine-lib\lib\release" mkdir "engine-lib\lib\release"
if not exist "engine-lib\include" mkdir "engine-lib\include"

echo ========================================
echo Building Debug Distribution...
echo ========================================
cmake -B engine-lib/build/debug -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Debug
if %errorlevel% neq 0 exit /b %errorlevel%

cmake --build engine-lib/build/debug --target Engine
if %errorlevel% neq 0 exit /b %errorlevel%

echo.
echo ========================================
echo Building Release Distribution...
echo ========================================
cmake -B engine-lib/build/release -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release
if %errorlevel% neq 0 exit /b %errorlevel%

cmake --build engine-lib/build/release --target Engine
if %errorlevel% neq 0 exit /b %errorlevel%

echo.
echo ========================================
echo Staging Libs and Headers...
echo ========================================

:: Copy compiled static libs recursively using /S
xcopy /S /Y /Q "engine-lib\build\debug\*.lib" "engine-lib\lib\debug\"
xcopy /S /Y /Q "engine-lib\build\release\*.lib" "engine-lib\lib\release\"

:: Copy public headers from Engine\include
if exist "Engine\include" (
    xcopy /E /Y /Q /I "Engine\include\*" "engine-lib\include\"
) else if exist "Engine\src" (
    xcopy /E /Y /Q /I "Engine\src\*.hpp" "engine-lib\include\"
)

echo.
echo Build successful.