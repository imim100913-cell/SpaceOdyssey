// Copyright Epic Games, Inc. All Rights Reserved.

#include "CockpitGlassComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

UCockpitGlassComponent::UCockpitGlassComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCockpitGlassComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCockpitGlassComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GlassMaterialInstance)
	{
		GlassMaterialInstance->SetScalarParameterValue(TEXT("CondensationAmount"), CondensationAmount);
		GlassMaterialInstance->SetScalarParameterValue(TEXT("FrostAmount"), FrostAmount);
		GlassMaterialInstance->SetScalarParameterValue(TEXT("DustGlintAmount"), DustGlintAmount);
		GlassMaterialInstance->SetScalarParameterValue(TEXT("HeatPlasmaGlow"), HeatPlasmaGlow);
	}
}

void UCockpitGlassComponent::InitializeGlassMaterial(UMaterialInstanceDynamic* InGlassMID)
{
	GlassMaterialInstance = InGlassMID;
}

void UCockpitGlassComponent::UpdateAtmosphericGlassEffects(float AltitudeKm, float SpeedKmh, float AtmosphericDensity, bool bIsReentering)
{
	// 1. Condensation / Humidity: High at low altitude (< 3km)
	if (AltitudeKm < 3.0f)
	{
		CondensationAmount = FMath::FInterpTo(CondensationAmount, FMath::Clamp(1.0f - (AltitudeKm / 3.0f), 0.0f, 0.8f), GetWorld()->GetDeltaSeconds(), 0.5f);
	}
	else
	{
		CondensationAmount = FMath::FInterpTo(CondensationAmount, 0.0f, GetWorld()->GetDeltaSeconds(), 1.0f);
	}

	// 2. Frosting / Ice Crystals in Upper Atmosphere (10km - 60km)
	if (AltitudeKm >= 8.0f && AltitudeKm <= 70.0f)
	{
		float TargetFrost = FMath::Sin(FMath::Clamp((AltitudeKm - 8.0f) / (70.0f - 8.0f) * PI, 0.0f, PI));
		FrostAmount = FMath::FInterpTo(FrostAmount, TargetFrost, GetWorld()->GetDeltaSeconds(), 0.8f);
	}
	else
	{
		FrostAmount = FMath::FInterpTo(FrostAmount, 0.0f, GetWorld()->GetDeltaSeconds(), 0.5f);
	}

	// 3. Space Micro-dust Glint: High in Space (> 80km)
	if (AltitudeKm > 80.0f)
	{
		DustGlintAmount = FMath::FInterpTo(DustGlintAmount, 1.0f, GetWorld()->GetDeltaSeconds(), 0.5f);
	}
	else
	{
		DustGlintAmount = FMath::FInterpTo(DustGlintAmount, 0.2f, GetWorld()->GetDeltaSeconds(), 0.5f);
	}

	// 4. Re-entry Heat Plasma: High speed inside atmosphere
	if (bIsReentering || (AtmosphericDensity > 0.05f && SpeedKmh > 3000.0f))
	{
		float HeatIntensity = FMath::Clamp((SpeedKmh - 3000.0f) / 10000.0f * AtmosphericDensity, 0.0f, 1.0f);
		HeatPlasmaGlow = FMath::FInterpTo(HeatPlasmaGlow, HeatIntensity, GetWorld()->GetDeltaSeconds(), 2.0f);
	}
	else
	{
		HeatPlasmaGlow = FMath::FInterpTo(HeatPlasmaGlow, 0.0f, GetWorld()->GetDeltaSeconds(), 1.5f);
	}
}
