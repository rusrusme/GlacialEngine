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

echo Building debug distribution...
cmake -B build/debugdist -DCMAKE_BUILD_TYPE=Debug -DDEBUG_DIST=ON
cmake --build build/debugdist --target Engine --clean-first
echo.

echo Building release distribution...
cmake -B build/releasedist -DCMAKE_BUILD_TYPE=Release -DRELEASE_DIST=ON
cmake --build build/releasedist --target Engine --clean-first
echo.

echo Built successful