// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpaceOdysseyGameModeBase.h"
#include "SpaceshipPawn.h"
#include "SpaceFlightHUD.h"

ASpaceOdysseyGameModeBase::ASpaceOdysseyGameModeBase()
{
	DefaultPawnClass = ASpaceshipPawn::StaticClass();
	HUDClass = ASpaceFlightHUD::StaticClass();
}
