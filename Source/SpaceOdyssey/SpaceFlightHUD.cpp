// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpaceFlightHUD.h"
#include "SpaceshipPawn.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "UObject/ConstructorHelpers.h"

ASpaceFlightHUD::ASpaceFlightHUD()
{
	static ConstructorHelpers::FObjectFinder<UFont> FontObj(TEXT("/Engine/EngineFonts/Roboto"));
	if (FontObj.Succeeded())
	{
		MainHUDFont = FontObj.Object;
	}
}

void ASpaceFlightHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas) return;

	ASpaceshipPawn* Spaceship = Cast<ASpaceshipPawn>(GetOwningPawn());
	if (!Spaceship) return;

	float CanvasW = Canvas->SizeX;
	float CanvasH = Canvas->SizeY;

	// Semi-transparent Sci-Fi HUD color (Cyan / Ice Blue)
	FLinearColor PrimaryColor = FLinearColor(0.2f, 0.85f, 1.0f, 0.85f);
	FLinearColor AccentColor = FLinearColor(1.0f, 0.65f, 0.1f, 0.9f);
	FLinearColor WhiteColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.7f);

	// 1. Top Bar: View Mode & Mission Info
	FString ViewName = TEXT("COCKPIT PILOT");
	switch (Spaceship->CurrentCameraView)
	{
	case EFlightCameraView::ObservationCabin: ViewName = TEXT("SIDE OBSERVATION WINDOW"); break;
	case EFlightCameraView::CupolaDome: ViewName = TEXT("TOP CUPOLA DOME VIEW"); break;
	case EFlightCameraView::ExternalCinematic: ViewName = TEXT("EXTERNAL CINEMATIC VIEW"); break;
	default: break;
	}

	FString StatusText = FString::Printf(TEXT("VIEW: [ %s ]   |   AUTOPILOT CRUISE: %s"), 
		*ViewName, 
		Spaceship->bIsCruiseAutopilot ? TEXT("[ ACTIVE - ENJOY SCENERY ]") : TEXT("[ OFF ]"));

	DrawText(StatusText, Spaceship->bIsCruiseAutopilot ? AccentColor : PrimaryColor, 40.f, 30.f, MainHUDFont, 1.1f);

	if (Spaceship->bIsCruiseAutopilot)
	{
		const float TourProgress = Spaceship->GetScenicTourProgress();
		const FString TourText = FString::Printf(TEXT("SCENIC TOUR: %s   |   PROGRESS: %3.0f%%   |   C: TAKE MANUAL CONTROL"),
			*Spaceship->GetScenicTourPhaseText(), TourProgress * 100.f);
		DrawText(TourText, WhiteColor, 40.f, 56.f, MainHUDFont, 0.95f);
		DrawRect(FLinearColor(0.03f, 0.08f, 0.12f, 0.8f), 40.f, 82.f, CanvasW - 80.f, 5.f);
		DrawRect(AccentColor, 40.f, 82.f, (CanvasW - 80.f) * TourProgress, 5.f);
	}

	// 2. Flight Telemetry (Left Bottom)
	float AltitudeKm = Spaceship->CurrentAltitude / 1000.0f;
	float SpeedKmh = (Spaceship->CurrentSpeed / 100.0f) * 3.6f;
	float MachNum = SpeedKmh / 1225.0f;

	FString TelemetryHeader = TEXT("/// FLIGHT TELEMETRY ///");
	FString AltString = FString::Printf(TEXT("ALTITUDE : %8.1f km"), AltitudeKm);
	FString SpdString = FString::Printf(TEXT("SPEED    : %8.0f km/h  (Mach %.2f)"), SpeedKmh, MachNum);
	
	FString LayerString = TEXT("ATMOSPHERE : TROPOSPHERE (1.00)");
	if (AltitudeKm > 80.0f)
	{
		LayerString = TEXT("ATMOSPHERE : DEEP SPACE / VACUUM (0.00)");
	}
	else if (AltitudeKm > 50.0f)
	{
		LayerString = FString::Printf(TEXT("ATMOSPHERE : MESOSPHERE (%.2f)"), Spaceship->AtmosphericDensity);
	}
	else if (AltitudeKm > 12.0f)
	{
		LayerString = FString::Printf(TEXT("ATMOSPHERE : STRATOSPHERE (%.2f)"), Spaceship->AtmosphericDensity);
	}

	float BaseY = CanvasH - 180.f;
	DrawText(TelemetryHeader, AccentColor, 40.f, BaseY, MainHUDFont, 1.0f);
	DrawText(AltString, PrimaryColor, 40.f, BaseY + 25.f, MainHUDFont, 1.1f);
	DrawText(SpdString, PrimaryColor, 40.f, BaseY + 50.f, MainHUDFont, 1.1f);
	DrawText(LayerString, WhiteColor, 40.f, BaseY + 75.f, MainHUDFont, 1.0f);

	// 3. Control Help (Right Bottom)
	float HelpX = CanvasW - 380.f;
	DrawText(TEXT("/// CONTROLS ///"), AccentColor, HelpX, BaseY - 25.f, MainHUDFont, 1.0f);
	DrawText(TEXT("W / S / A / D : Pitch / Roll / Throttle"), WhiteColor, HelpX, BaseY, MainHUDFont, 0.9f);
	DrawText(TEXT("Left Shift    : Boost (Super Cruise)"), WhiteColor, HelpX, BaseY + 20.f, MainHUDFont, 0.9f);
	DrawText(TEXT("C             : Start / Stop Full Scenic Tour"), PrimaryColor, HelpX, BaseY + 40.f, MainHUDFont, 0.9f);
	DrawText(TEXT("V             : Switch Window / Viewpoint"), PrimaryColor, HelpX, BaseY + 60.f, MainHUDFont, 0.9f);
	DrawText(TEXT("Middle Click  : Free-Look Head Turn"), WhiteColor, HelpX, BaseY + 80.f, MainHUDFont, 0.9f);
	DrawText(TEXT("J             : Warp to Moon Orbit"), AccentColor, HelpX, BaseY + 100.f, MainHUDFont, 0.9f);
}
