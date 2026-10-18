// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpaceshipPawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CockpitGlassComponent.h"
#include "Kismet/KismetMathLibrary.h"

ASpaceshipPawn::ASpaceshipPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// Root Spaceship Body
	SpaceshipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpaceshipMesh"));
	RootComponent = SpaceshipMesh;
	SpaceshipMesh->SetSimulatePhysics(false);
	SpaceshipMesh->SetEnableGravity(false);
	SpaceshipMesh->SetCollisionProfileName(TEXT("Pawn"));

	// Interior Cockpit Mesh
	CockpitInteriorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CockpitInteriorMesh"));
	CockpitInteriorMesh->SetupAttachment(RootComponent);

	// Cockpit Glass Component
	CockpitGlassComp = CreateDefaultSubobject<UCockpitGlassComponent>(TEXT("CockpitGlassComp"));

	// External Spring Arm for 3rd Person View
	ExternalSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("ExternalSpringArm"));
	ExternalSpringArm->SetupAttachment(RootComponent);
	ExternalSpringArm->TargetArmLength = 1200.0f;
	ExternalSpringArm->SocketOffset = FVector(0.f, 0.f, 250.f);
	ExternalSpringArm->bEnableCameraLag = true;
	ExternalSpringArm->bEnableCameraRotationLag = true;
	ExternalSpringArm->CameraLagSpeed = 10.0f;
	ExternalSpringArm->CameraRotationLagSpeed = 10.0f;

	// Main Camera
	MainCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("MainCamera"));
	MainCamera->SetupAttachment(RootComponent);
	MainCamera->SetRelativeLocation(PilotSeatOffset);
	MainCamera->bUsePawnControlRotation = false;

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void ASpaceshipPawn::BeginPlay()
{
	Super::BeginPlay();
	SetCameraView(EFlightCameraView::CockpitPilot);
}

void ASpaceshipPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateAtmosphereAndAltitude(DeltaSeconds);
	UpdateFlightDynamics(DeltaSeconds);
	UpdateCameraTransform(DeltaSeconds);
}

void ASpaceshipPawn::UpdateAtmosphereAndAltitude(float DeltaSeconds)
{
	// Assuming planet center is at (0,0,0) and radius is 6371km (637,100,000 cm)
	// For game scale: Level (0,0,0) to Z is altitude
	FVector WorldPos = GetActorLocation();
	CurrentAltitude = FMath::Max(WorldPos.Z / 100.0f, 0.0f); // Altitude in meters

	float AltitudeKm = CurrentAltitude / 1000.0f;

	// Atmospheric Density curve (Approximation of barometric formula)
	// At 0km = 1.0, 10km = 0.25, 50km = 0.01, >80km = 0.0 (Space)
	AtmosphericDensity = FMath::Clamp(FMath::Exp(-AltitudeKm / 8.5f), 0.0f, 1.0f);

	// Update glass component
	if (CockpitGlassComp)
	{
		float SpeedKmh = (CurrentSpeed / 100.0f) * 3.6f;
		CockpitGlassComp->UpdateAtmosphericGlassEffects(AltitudeKm, SpeedKmh, AtmosphericDensity, false);
	}
}

void ASpaceshipPawn::UpdateFlightDynamics(float DeltaSeconds)
{
	// Determine Target Speed
	float DesiredMaxSpeed = bIsBoosting ? BoostSpeed : NormalSpeed;
	if (bIsCruiseAutopilot)
	{
		DesiredMaxSpeed = WarpCruiseSpeed;
	}

	TargetSpeed = FMath::FInterpTo(TargetSpeed, CurrentForwardThrust * DesiredMaxSpeed, DeltaSeconds, 2.0f);
	CurrentSpeed = FMath::FInterpTo(CurrentSpeed, TargetSpeed, DeltaSeconds, AccelerationRate * 0.001f);

	// Apply Rotations (Pitch, Roll, Yaw)
	if (!bIsFreeLooking && !bIsCruiseAutopilot)
	{
		FRotator DeltaRotation = FRotator(
			CurrentPitchInput * PitchSensitivity * DeltaSeconds,
			CurrentYawInput * YawSensitivity * DeltaSeconds,
			CurrentRollInput * RollSensitivity * DeltaSeconds
		);
		AddActorLocalRotation(DeltaRotation);
	}
	else if (bIsCruiseAutopilot)
	{
		// Gentle orbit stabilization
		FRotator CurrentRot = GetActorRotation();
		FRotator TargetRot = FRotator(0.f, CurrentRot.Yaw + (DeltaSeconds * 2.0f), 0.f);
		SetActorRotation(FMath::RInterpTo(CurrentRot, TargetRot, DeltaSeconds, 0.5f));
	}

	// Translation Movement (Forward + Strafe)
	FVector ForwardVector = GetActorForwardVector() * CurrentSpeed * DeltaSeconds;
	FVector RightVector = GetActorRightVector() * (CurrentRightStrafe * NormalSpeed * 0.5f) * DeltaSeconds;
	FVector UpVector = GetActorUpVector() * (CurrentUpStrafe * NormalSpeed * 0.5f) * DeltaSeconds;

	FVector NewLocation = GetActorLocation() + ForwardVector + RightVector + UpVector;
	SetActorLocation(NewLocation, true);
}

void ASpaceshipPawn::UpdateCameraTransform(float DeltaSeconds)
{
	if (CurrentCameraView == EFlightCameraView::ExternalCinematic)
	{
		return;
	}

	// Calculate base seat offset
	FVector BaseOffset = PilotSeatOffset;
	switch (CurrentCameraView)
	{
	case EFlightCameraView::CockpitPilot:
		BaseOffset = PilotSeatOffset;
		break;
	case EFlightCameraView::ObservationCabin:
		BaseOffset = ObservationSeatOffset;
		break;
	case EFlightCameraView::CupolaDome:
		BaseOffset = CupolaOffset;
		break;
	default:
		break;
	}

	// Smoothly interpolate camera relative position
	MainCamera->SetRelativeLocation(FMath::VInterpTo(MainCamera->GetRelativeLocation(), BaseOffset, DeltaSeconds, 5.0f));

	// Apply FreeLook head rotation
	FRotator BaseViewRotation = FRotator::ZeroRotator;
	if (CurrentCameraView == EFlightCameraView::ObservationCabin)
	{
		BaseViewRotation = FRotator(0.f, 90.f, 0.f); // Look out the side window by default
	}
	else if (CurrentCameraView == EFlightCameraView::CupolaDome)
	{
		BaseViewRotation = FRotator(80.f, 0.f, 0.f); // Look up through dome by default
	}

	FRotator FinalCameraRot = BaseViewRotation + FreeLookRotation;
	MainCamera->SetRelativeRotation(FMath::RInterpTo(MainCamera->GetRelativeRotation(), FinalCameraRot, DeltaSeconds, 10.0f));
}

void ASpaceshipPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Axis mappings
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ASpaceshipPawn::MoveForwardInput);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ASpaceshipPawn::MoveRightInput);
	PlayerInputComponent->BindAxis(TEXT("MoveUp"), this, &ASpaceshipPawn::MoveUpInput);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ASpaceshipPawn::TurnInput);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ASpaceshipPawn::LookUpInput);
	PlayerInputComponent->BindAxis(TEXT("Roll"), this, &ASpaceshipPawn::RollInput);

	// Action mappings
	PlayerInputComponent->BindAction(TEXT("Boost"), IE_Pressed, this, &ASpaceshipPawn::StartBoost);
	PlayerInputComponent->BindAction(TEXT("Boost"), IE_Released, this, &ASpaceshipPawn::StopBoost);
	PlayerInputComponent->BindAction(TEXT("ToggleCruiseMode"), IE_Pressed, this, &ASpaceshipPawn::ToggleCruiseMode);
	PlayerInputComponent->BindAction(TEXT("SwitchCameraView"), IE_Pressed, this, &ASpaceshipPawn::SwitchCameraView);
	PlayerInputComponent->BindAction(TEXT("ToggleFreeLook"), IE_Pressed, this, &ASpaceshipPawn::ToggleFreeLook);
	PlayerInputComponent->BindAction(TEXT("WarpJumpToMoon"), IE_Pressed, this, &ASpaceshipPawn::WarpJumpToMoon);
}

void ASpaceshipPawn::MoveForwardInput(float Value)
{
	CurrentForwardThrust = FMath::Clamp(Value, -0.3f, 1.0f);
	if (bIsCruiseAutopilot)
	{
		CurrentForwardThrust = 1.0f;
	}
}

void ASpaceshipPawn::MoveRightInput(float Value)
{
	CurrentRightStrafe = Value;
}

void ASpaceshipPawn::MoveUpInput(float Value)
{
	CurrentUpStrafe = Value;
}

void ASpaceshipPawn::TurnInput(float Value)
{
	if (bIsFreeLooking)
	{
		FreeLookRotation.Yaw = FMath::Clamp(FreeLookRotation.Yaw + (Value * 2.0f), -120.f, 120.f);
	}
	else
	{
		CurrentYawInput = Value;
	}
}

void ASpaceshipPawn::LookUpInput(float Value)
{
	if (bIsFreeLooking)
	{
		FreeLookRotation.Pitch = FMath::Clamp(FreeLookRotation.Pitch + (Value * 2.0f), -80.f, 80.f);
	}
	else
	{
		CurrentPitchInput = Value;
	}
}

void ASpaceshipPawn::RollInput(float Value)
{
	CurrentRollInput = Value;
}

void ASpaceshipPawn::StartBoost()
{
	bIsBoosting = true;
}

void ASpaceshipPawn::StopBoost()
{
	bIsBoosting = false;
}

void ASpaceshipPawn::ToggleCruiseMode()
{
	bIsCruiseAutopilot = !bIsCruiseAutopilot;
}

void ASpaceshipPawn::SwitchCameraView()
{
	uint8 NextIndex = (static_cast<uint8>(CurrentCameraView) + 1) % 4;
	SetCameraView(static_cast<EFlightCameraView>(NextIndex));
}

void ASpaceshipPawn::ToggleFreeLook()
{
	bIsFreeLooking = !bIsFreeLooking;
	if (!bIsFreeLooking)
	{
		FreeLookRotation = FRotator::ZeroRotator;
	}
}

void ASpaceshipPawn::WarpJumpToMoon()
{
	// Accelerate quickly to high altitude / Moon orbit
	FVector CurrentLoc = GetActorLocation();
	FVector MoonVector = FVector(CurrentLoc.X + 2000000.f, CurrentLoc.Y + 1500000.f, CurrentLoc.Z + 3500000.f);
	SetActorLocation(MoonVector);
}

void ASpaceshipPawn::SetCameraView(EFlightCameraView NewView)
{
	CurrentCameraView = NewView;

	if (CurrentCameraView == EFlightCameraView::ExternalCinematic)
	{
		MainCamera->AttachToComponent(ExternalSpringArm, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		MainCamera->SetRelativeLocation(FVector::ZeroVector);
		MainCamera->SetRelativeRotation(FRotator::ZeroRotator);
	}
	else
	{
		MainCamera->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		FreeLookRotation = FRotator::ZeroRotator;
	}
}
