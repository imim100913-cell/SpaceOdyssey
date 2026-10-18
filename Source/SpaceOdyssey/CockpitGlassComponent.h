// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CockpitGlassComponent.generated.h"

class UMaterialInstanceDynamic;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPACEODYSSEY_API UCockpitGlassComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCockpitGlassComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Glass FX")
	void InitializeGlassMaterial(UMaterialInstanceDynamic* InGlassMID);

	// Update glass properties based on atmosphere and speed
	UFUNCTION(BlueprintCallable, Category = "Glass FX")
	void UpdateAtmosphericGlassEffects(float AltitudeKm, float SpeedKmh, float AtmosphericDensity, bool bIsReentering);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glass FX")
	float CondensationAmount = 0.0f; // Low altitude humidity/fogging

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glass FX")
	float FrostAmount = 0.0f; // Stratosphere freezing effect (10km - 50km)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glass FX")
	float DustGlintAmount = 0.0f; // Space dust / micro-scratch glint in direct sunlight

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glass FX")
	float HeatPlasmaGlow = 0.0f; // Re-entry fiery plasma glow

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	UMaterialInstanceDynamic* GlassMaterialInstance = nullptr;
};
