// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SpaceshipPawn.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UStaticMeshComponent;
class UCockpitGlassComponent;
class UNiagaraComponent;
class UAudioComponent;

UENUM(BlueprintType)
enum class EFlightCameraView : uint8
{
	CockpitPilot UMETA(DisplayName = "Cockpit Pilot View"),
	ObservationCabin UMETA(DisplayName = "Side Observation Window"),
	CupolaDome UMETA(DisplayName = "Top Cupola Dome Window"),
	ExternalCinematic UMETA(DisplayName = "External Cinematic View")
};

UCLASS()
class SPACEODYSSEY_API ASpaceshipPawn : public APawn
{
	GENERATED_BODY()

public:
	ASpaceshipPawn();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* SpaceshipMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* CockpitInteriorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCockpitGlassComponent* CockpitGlassComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* MainCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* ExternalSpringArm;

	// Camera View Sockets / Locations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight|Camera")
	FVector PilotSeatOffset = FVector(50.f, 0.f, 40.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight|Camera")
	FVector ObservationSeatOffset = FVector(-30.f, 70.f, 30.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight|Camera")
	FVector CupolaOffset = FVector(0.f, 0.f, 90.f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight|Camera")
	EFlightCameraView CurrentCameraView = EFlightCameraView::CockpitPilot;

	// Flight Physics & Dynamics
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight|Movement")
	float NormalSpeed = 8000.f; // cm/s (80 m/s ~ Mach 0.25 on earth)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight|Movement")
	float BoostSpeed = 45000.f; // cm/s (450 m/s ~ Supercruise / Re-entry)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight|Movement")
	float WarpCruiseSpeed = 300000.f; // cm/s (Interplanetary Cruise)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight|Movement")
	float AccelerationRate = 3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight|Movement")
	float PitchSensitivity = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight|Movement")
	float RollSensitivity = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight|Movement")
	float YawSensitivity = 30.f;

	// States
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight|Status")
	float CurrentSpeed = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight|Status")
	float TargetSpeed = 3000.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight|Status")
	float CurrentAltitude = 0.f; // Meters above planet surface

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight|Status")
	float AtmosphericDensity = 1.0f; // 1.0 at ground, 0.0 in deep space

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight|Status")
	bool bIsBoosting = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight|Status")
	bool bIsCruiseAutopilot = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight|Status")
	bool bIsFreeLooking = false;

	// Inputs
	void MoveForwardInput(float Value);
	void MoveRightInput(float Value);
	void MoveUpInput(float Value);
	void TurnInput(float Value);
	void LookUpInput(float Value);
	void RollInput(float Value);

	void StartBoost();
	void StopBoost();
	void ToggleCruiseMode();
	void SwitchCameraView();
	void ToggleFreeLook();
	void WarpJumpToMoon();

	UFUNCTION(BlueprintCallable, Category = "Flight")
	void SetCameraView(EFlightCameraView NewView);

protected:
	virtual void BeginPlay() override;

	FRotator FreeLookRotation = FRotator::ZeroRotator;
	FRotator TargetMeshTilt = FRotator::ZeroRotator;
	float CurrentForwardThrust = 0.f;
	float CurrentRightStrafe = 0.f;
	float CurrentUpStrafe = 0.f;
	float CurrentPitchInput = 0.f;
	float CurrentYawInput = 0.f;
	float CurrentRollInput = 0.f;

	void UpdateAtmosphereAndAltitude(float DeltaSeconds);
	void UpdateFlightDynamics(float DeltaSeconds);
	void UpdateCameraTransform(float DeltaSeconds);
};
