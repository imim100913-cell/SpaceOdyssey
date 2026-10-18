@echo off
chcp 65001 > nul
echo ========================================================
echo   Space Odyssey: Earth to Moon (UE5.8 Space Flight)
echo   DirectX 11 安定モードで Unreal Editor を起動しています...
echo ========================================================
start "" "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "C:\SpaceOdyssey\SpaceOdyssey.uproject" -d3d11
exit
