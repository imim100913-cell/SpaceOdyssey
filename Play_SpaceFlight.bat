@echo off
chcp 65001 > nul
title 🚀 Space Odyssey — Earth to Moon
echo.
echo  ╔══════════════════════════════════════════════════════════════╗
echo  ║                                                              ║
echo  ║   🚀  SPACE ODYSSEY: Earth to Moon                          ║
echo  ║       Unreal Engine 5.8 Space Flight Experience              ║
echo  ║                                                              ║
echo  ╠══════════════════════════════════════════════════════════════╣
echo  ║                                                              ║
echo  ║   Controls:                                                  ║
echo  ║     Space     = Ascend (break through atmosphere!)           ║
echo  ║     W / S     = Forward / Backward                          ║
echo  ║     A / D     = Turn Left / Right                            ║
echo  ║     Mouse     = Look Around                                  ║
echo  ║     F         = Auto-Cruise Toggle                           ║
echo  ║     Ctrl      = Descend                                      ║
echo  ║     Esc       = Exit                                         ║
echo  ║                                                              ║
echo  ╚══════════════════════════════════════════════════════════════╝
echo.
echo  Starting game...
echo.

start "" "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "C:\SpaceOdyssey\SpaceOdyssey.uproject" /Game/Maps/SpaceFlight_Cinematic -game -WINDOWED -ResX=1920 -ResY=1080 -d3d11

exit
