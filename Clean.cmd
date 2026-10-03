@echo off
setlocal

echo ========================================
echo Cleaning Engine Build Artifacts...
echo ========================================

:: Clean engine-lib build and staging folders
if exist "engine-lib\build" (
    echo Removing engine-lib\build...
    rmdir /s /q "engine-lib\build"
)

if exist "engine-lib\lib" (
    echo Removing engine-lib\lib...
    rmdir /s /q "engine-lib\lib"
)

if exist "engine-lib\include" (
    echo Removing engine-lib\include...
    rmdir /s /q "engine-lib\include"
)

:: Clean root binary and build directories
if exist "bin" (
    echo Removing root bin\...
    rmdir /s /q "bin"
)

if exist "build" (
    echo Removing root build\...
    rmdir /s /q "build"
)

echo.
echo Cleaned successfully!