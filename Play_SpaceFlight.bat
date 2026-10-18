@echo off
chcp 65001 > nul
title 🚀 Space Odyssey — Earth to Moon (Low-Spec PC Mode)
echo.
echo  ╔══════════════════════════════════════════════════════════════╗
echo  ║                                                              ║
echo  ║   🚀  SPACE ODYSSEY: Earth to Moon (軽量モード)              ║
echo  ║       Optimized for Low-Spec PCs (DX11 / 720p / Fast LUT)   ║
echo  ║                                                              ║
echo  ╠══════════════════════════════════════════════════════════════╣
echo  ║                                                              ║
echo  ║   Controls:                                                  ║
echo  ║     Space     = Ascend (上昇・大気圏突破)                    ║
echo  ║     W / S     = Forward / Backward (前進・後退)              ║
echo  ║     A / D     = Turn Left / Right (左右旋回)                 ║
echo  ║     Mouse     = Look Around (360°視点)                       ║
echo  ║     C / F     = Auto-Cruise Toggle (自動遊覧モード)          ║
echo  ║     V         = Switch Camera View (視点切替)                ║
echo  ║     J         = Warp to Moon (月軌道ワープ)                  ║
echo  ║     Ctrl      = Descend (下降)                               ║
echo  ║     Esc       = Exit (終了)                                  ║
echo  ║                                                              ║
echo  ╚══════════════════════════════════════════════════════════════╝
echo.
echo  Starting lightweight game mode (1280x720, DX11, Low Scalability)...
echo.

start "" "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0SpaceOdyssey.uproject" /Game/Maps/SpaceFlight_Cinematic -game -WINDOWED -ResX=1280 -ResY=720 -d3d11 -NOSPLASH -NoVerifyGC -ExecCmds="sg.ResolutionQuality 65,sg.ShadowQuality 0,sg.PostProcessQuality 1,sg.EffectsQuality 0,r.VolumetricCloud 0,r.DynamicGlobalIlluminationMethod 0"

exit
