@echo off
setlocal

if not exist "S:\SpaceOdyssey.uproject" (
    subst S: "%~dp0."
)

if not exist "S:\SpaceOdyssey.uproject" (
    echo S: drive is already assigned to another location.
    echo Remove it with: subst S: /d
    pause
    exit /b 1
)

start "" "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "S:\SpaceOdyssey.uproject"
endlocal
