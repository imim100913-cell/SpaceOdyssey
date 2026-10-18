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

	if (bAutoStartScenicTour)
	{
		StartScenicTour();
	}
}

void ASpaceshipPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateFlightDynamics(DeltaSeconds);
	UpdateAtmosphereAndAltitude(DeltaSeconds);
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
	if (bIsCruiseAutopilot)
	{
		UpdateScenicTour(DeltaSeconds);
		return;
	}

	// Determine Target Speed
	float DesiredMaxSpeed = bIsBoosting ? BoostSpeed : NormalSpeed;

	TargetSpeed = FMath::FInterpTo(TargetSpeed, CurrentForwardThrust * DesiredMaxSpeed, DeltaSeconds, 2.0f);
	CurrentSpeed = FMath::FInterpTo(CurrentSpeed, TargetSpeed, DeltaSeconds, AccelerationRate * 0.001f);

	// Apply Rotations (Pitch, Roll, Yaw)
	if (!bIsFreeLooking)
	{
		FRotator DeltaRotation = FRotator(
			CurrentPitchInput * PitchSensitivity * DeltaSeconds,
			CurrentYawInput * YawSensitivity * DeltaSeconds,
			CurrentRollInput * RollSensitivity * DeltaSeconds
		);
		AddActorLocalRotation(DeltaRotation);
	}
	// Translation Movement (Forward + Strafe)
	FVector ForwardVector = GetActorForwardVector() * CurrentSpeed * DeltaSeconds;
	FVector RightVector = GetActorRightVector() * (CurrentRightStrafe * NormalSpeed * 0.5f) * DeltaSeconds;
	FVector UpVector = GetActorUpVector() * (CurrentUpStrafe * NormalSpeed * 0.5f) * DeltaSeconds;

	FVector NewLocation = GetActorLocation() + ForwardVector + RightVector + UpVector;
	SetActorLocation(NewLocation, true);
}

void ASpaceshipPawn::BuildScenicTourRoute()
{
	ScenicTourWaypoints.Reset();
	ScenicTourSegmentDurations.Reset();

	ScenicTourOrigin = GetActorLocation();
	ScenicTourStartRotation = GetActorRotation();

	const TArray<FVector> LocalWaypoints = {
		FVector(0.f, 0.f, 0.f),
		FVector(60000.f, 0.f, 80000.f),
		FVector(180000.f, 30000.f, 350000.f),
		FVector(500000.f, 250000.f, 900000.f),
		FVector(1300000.f, 700000.f, 1400000.f),
		FVector(2700000.f, 850000.f, 1700000.f),
		FVector(3500000.f, 200000.f, 900000.f),
		FVector(3200000.f, -900000.f, 700000.f),
		FVector(1800000.f, -700000.f, 1200000.f),
		FVector(500000.f, -150000.f, 400000.f),
		FVector(50000.f, 0.f, 60000.f)
	};

	for (const FVector& LocalPoint : LocalWaypoints)
	{
		ScenicTourWaypoints.Add(ScenicTourOrigin + ScenicTourStartRotation.RotateVector(LocalPoint));
	}

	ScenicTourSegmentDurations = { 12.f, 15.f, 18.f, 22.f, 35.f, 28.f, 24.f, 32.f, 24.f, 18.f };
	ScenicTourTotalDuration = 0.f;
	for (const float Duration : ScenicTourSegmentDurations)
	{
		ScenicTourTotalDuration += Duration;
	}
}

FVector ASpaceshipPawn::EvaluateScenicTourSegment(int32 SegmentIndex, float Alpha) const
{
	const int32 LastPointIndex = ScenicTourWaypoints.Num() - 1;
	const FVector& P0 = ScenicTourWaypoints[FMath::Max(SegmentIndex - 1, 0)];
	const FVector& P1 = ScenicTourWaypoints[SegmentIndex];
	const FVector& P2 = ScenicTourWaypoints[FMath::Min(SegmentIndex + 1, LastPointIndex)];
	const FVector& P3 = ScenicTourWaypoints[FMath::Min(SegmentIndex + 2, LastPointIndex)];
	const float A2 = Alpha * Alpha;
	const float A3 = A2 * Alpha;

	return 0.5f * ((2.f * P1) + ((-P0 + P2) * Alpha) +
		((2.f * P0 - 5.f * P1 + 4.f * P2 - P3) * A2) +
		((-P0 + 3.f * P1 - 3.f * P2 + P3) * A3));
}

FVector ASpaceshipPawn::EvaluateScenicTourTangent(int32 SegmentIndex, float Alpha) const
{
	const int32 LastPointIndex = ScenicTourWaypoints.Num() - 1;
	const FVector& P0 = ScenicTourWaypoints[FMath::Max(SegmentIndex - 1, 0)];
	const FVector& P1 = ScenicTourWaypoints[SegmentIndex];
	const FVector& P2 = ScenicTourWaypoints[FMath::Min(SegmentIndex + 1, LastPointIndex)];
	const FVector& P3 = ScenicTourWaypoints[FMath::Min(SegmentIndex + 2, LastPointIndex)];
	const float A2 = Alpha * Alpha;

	return 0.5f * ((-P0 + P2) +
		(2.f * (2.f * P0 - 5.f * P1 + 4.f * P2 - P3) * Alpha) +
		(3.f * (-P0 + 3.f * P1 - 3.f * P2 + P3) * A2));
}

void ASpaceshipPawn::UpdateScenicTour(float DeltaSeconds)
{
	if (ScenicTourWaypoints.Num() < 2 || ScenicTourSegmentDurations.Num() != ScenicTourWaypoints.Num() - 1)
	{
		BuildScenicTourRoute();
	}

	if (bIsScenicTourComplete)
	{
		CurrentSpeed = 0.f;
		TargetSpeed = 0.f;
		return;
	}

	ScenicTourElapsedTime += DeltaSeconds * FMath::Max(ScenicTourTimeScale, 0.1f);
	if (ScenicTourElapsedTime >= ScenicTourTotalDuration)
	{
		if (bLoopScenicTour)
		{
			ScenicTourElapsedTime = FMath::Fmod(ScenicTourElapsedTime, ScenicTourTotalDuration);
			CurrentScenicTourSegment = INDEX_NONE;
		}
		else
		{
			ScenicTourElapsedTime = ScenicTourTotalDuration;
			bIsScenicTourComplete = true;
			CurrentScenicTourPhase = EScenicTourPhase::MissionComplete;
			CurrentSpeed = 0.f;
			TargetSpeed = 0.f;
			SetActorLocation(ScenicTourWaypoints.Last(), false, nullptr, ETeleportType::TeleportPhysics);
			SetCameraView(EFlightCameraView::CockpitPilot);
			return;
		}
	}

	float RemainingTime = ScenicTourElapsedTime;
	int32 SegmentIndex = 0;
	for (; SegmentIndex < ScenicTourSegmentDurations.Num() - 1; ++SegmentIndex)
	{
		if (RemainingTime <= ScenicTourSegmentDurations[SegmentIndex])
		{
			break;
		}
		RemainingTime -= ScenicTourSegmentDurations[SegmentIndex];
	}

	const float SegmentDuration = ScenicTourSegmentDurations[SegmentIndex];
	const float Alpha = FMath::Clamp(RemainingTime / SegmentDuration, 0.f, 1.f);
	const FVector PreviousLocation = GetActorLocation();
	const FVector NewLocation = EvaluateScenicTourSegment(SegmentIndex, Alpha);
	const FVector Tangent = EvaluateScenicTourTangent(SegmentIndex, Alpha).GetSafeNormal();

	if (SegmentIndex != CurrentScenicTourSegment)
	{
		CurrentScenicTourSegment = SegmentIndex;
		SetScenicTourPhaseForSegment(SegmentIndex);
		ApplyTourCameraForSegment(SegmentIndex);
	}

	if (!Tangent.IsNearlyZero())
	{
		FRotator DesiredRotation = Tangent.Rotation();
		const float BankDirection = (SegmentIndex % 2 == 0) ? 1.f : -1.f;
		DesiredRotation.Roll = FMath::Sin(Alpha * PI) * 12.f * BankDirection;
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), DesiredRotation, DeltaSeconds, 2.5f));
	}

	SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
	CurrentSpeed = DeltaSeconds > KINDA_SMALL_NUMBER ? FVector::Distance(PreviousLocation, NewLocation) / DeltaSeconds : 0.f;
	TargetSpeed = CurrentSpeed;
}

void ASpaceshipPawn::SetScenicTourPhaseForSegment(int32 SegmentIndex)
{
	switch (SegmentIndex)
	{
	case 0: CurrentScenicTourPhase = EScenicTourPhase::Launch; break;
	case 1: CurrentScenicTourPhase = EScenicTourPhase::AtmosphericAscent; break;
	case 2: CurrentScenicTourPhase = EScenicTourPhase::Stratosphere; break;
	case 3: CurrentScenicTourPhase = EScenicTourPhase::EarthOrbit; break;
	case 4: CurrentScenicTourPhase = EScenicTourPhase::TranslunarCruise; break;
	case 5: CurrentScenicTourPhase = EScenicTourPhase::LunarApproach; break;
	case 6: CurrentScenicTourPhase = EScenicTourPhase::LunarFlyby; break;
	default: CurrentScenicTourPhase = EScenicTourPhase::EarthReturn; break;
	}
}

void ASpaceshipPawn::ApplyTourCameraForSegment(int32 SegmentIndex)
{
	if (!bAutoCycleTourViews)
	{
		return;
	}

	static const EFlightCameraView TourViews[] = {
		EFlightCameraView::CockpitPilot,
		EFlightCameraView::CockpitPilot,
		EFlightCameraView::CupolaDome,
		EFlightCameraView::ExternalCinematic,
		EFlightCameraView::ObservationCabin,
		EFlightCameraView::CockpitPilot,
		EFlightCameraView::ExternalCinematic,
		EFlightCameraView::ObservationCabin,
		EFlightCameraView::CupolaDome,
		EFlightCameraView::CockpitPilot
	};

	SetCameraView(TourViews[FMath::Clamp(SegmentIndex, 0, UE_ARRAY_COUNT(TourViews) - 1)]);
}

void ASpaceshipPawn::StartScenicTour()
{
	bIsCruiseAutopilot = true;
	bIsScenicTourComplete = false;
	bIsFreeLooking = false;
	ScenicTourElapsedTime = 0.f;
	CurrentScenicTourSegment = INDEX_NONE;
	CurrentForwardThrust = 1.f;
	BuildScenicTourRoute();
	SetScenicTourPhaseForSegment(0);
	ApplyTourCameraForSegment(0);
}

void ASpaceshipPawn::StopScenicTour()
{
	bIsCruiseAutopilot = false;
	bIsScenicTourComplete = false;
	CurrentForwardThrust = 0.f;
	TargetSpeed = 0.f;
	CurrentScenicTourSegment = INDEX_NONE;
}

float ASpaceshipPawn::GetScenicTourProgress() const
{
	return ScenicTourTotalDuration > 0.f ? FMath::Clamp(ScenicTourElapsedTime / ScenicTourTotalDuration, 0.f, 1.f) : 0.f;
}

FString ASpaceshipPawn::GetScenicTourPhaseText() const
{
	switch (CurrentScenicTourPhase)
	{
	case EScenicTourPhase::Launch: return TEXT("LAUNCH");
	case EScenicTourPhase::AtmosphericAscent: return TEXT("ATMOSPHERIC ASCENT");
	case EScenicTourPhase::Stratosphere: return TEXT("STRATOSPHERE / EARTH REVEAL");
	case EScenicTourPhase::EarthOrbit: return TEXT("EARTH ORBIT");
	case EScenicTourPhase::TranslunarCruise: return TEXT("TRANSLUNAR CRUISE");
	case EScenicTourPhase::LunarApproach: return TEXT("LUNAR APPROACH");
	case EScenicTourPhase::LunarFlyby: return TEXT("LUNAR FLYBY");
	case EScenicTourPhase::EarthReturn: return TEXT("EARTH RETURN");
	case EScenicTourPhase::MissionComplete: return TEXT("MISSION COMPLETE");
	default: return TEXT("SCENIC TOUR");
	}
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
	if (bIsCruiseAutopilot)
	{
		StopScenicTour();
	}
	else
	{
		StartScenicTour();
	}
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
	StopScenicTour();
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
