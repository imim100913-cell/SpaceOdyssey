// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SpaceFlightHUD.generated.h"

UCLASS()
class SPACEODYSSEY_API ASpaceFlightHUD : public AHUD
{
	GENERATED_BODY()

public:
	ASpaceFlightHUD();

	virtual void DrawHUD() override;

	// Toggle help overlay visibility
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	bool bShowHelpOverlay = true;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD Font")
	UFont* MainHUDFont;

	void DrawKeyBadge(float X, float Y, const FString& KeyName, const FString& ActionDesc, const FLinearColor& BadgeColor);
	void DrawGlassStatusBox(float X, float Y, float Width, float Height, class ASpaceshipPawn* Spaceship);
};
