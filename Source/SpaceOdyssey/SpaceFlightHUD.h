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

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD Font")
	UFont* MainHUDFont;
};
