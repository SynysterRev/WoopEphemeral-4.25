// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "GolemProjectCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include <Engine/Engine.h>
#include "Player/GrappleComponent.h"
#include "Blueprint/UserWidget.h"
#include "Helpers/HelperLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Interfaces/Targetable.h"
#include "Camera/PlayerCameraManager.h"
#include "GolemProjectGameMode.h"
#include "Player/HealthComponent.h"
#include "Player/PushingComponent.h"
#include "Interfaces/Interactable.h"
#include "Player/FistComponent.h"
#include "Player/SwingPhysic.h"
#include "Player/RaycastingComponent.h"
#include "Objects/PushableBloc.h"
#include "Player/WallMechanicalComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Containers/Array.h"
//#include "Player/SlowMoComponent.h"

//////////////////////////////////////////////////////////////////////////
// AGolemProjectCharacter


AGolemProjectCharacter::AGolemProjectCharacter()
{
	// Set size for collision capsule
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	}

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input..
		GetCharacterMovement()->RotationRate = FRotator(0.0f, 1620.0f, 0.0f); // ...at this rotation rate
		GetCharacterMovement()->JumpZVelocity = 600.f;
		GetCharacterMovement()->AirControl = 0.2f;
	}

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	if (CameraBoom)
	{
		CameraBoom->SetupAttachment(RootComponent);
		CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character
		CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	}

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	if (FollowCamera)
	{
		FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
		FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	}

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character)
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
	customCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("capsule"));
	customCapsule->InitCapsuleSize(43.0f, 97.0f);
	customCapsule->SetupAttachment(RootComponent);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AGolemProjectCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{

	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AGolemProjectCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	//Input left Mouse Click
	PlayerInputComponent->BindAction("Fire1", IE_Released, this, &AGolemProjectCharacter::Fire);

	PlayerInputComponent->BindAction("Fire2", IE_Pressed, this, &AGolemProjectCharacter::ChangeCameraPressed);
	PlayerInputComponent->BindAction("Fire2", IE_Released, this, &AGolemProjectCharacter::ChangeCameraReleased);
	PlayerInputComponent->BindAction("ChangeToGrapple", IE_Pressed, this, &AGolemProjectCharacter::ChangeToGrapple);
	PlayerInputComponent->BindAction("ChangeToFist", IE_Pressed, this, &AGolemProjectCharacter::ChangeToFist);

	PlayerInputComponent->BindAction("AssistedGrapple", IE_Pressed, this, &AGolemProjectCharacter::UseAssistedGrapple);

	PlayerInputComponent->BindAction("SwitchArm", IE_Pressed, this, &AGolemProjectCharacter::SwitchArm);

	PlayerInputComponent->BindAction("DashDown", IE_Pressed, this, &AGolemProjectCharacter::DashDown);

	PlayerInputComponent->BindAction("Pause", IE_Pressed, this, &AGolemProjectCharacter::PauseEvent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGolemProjectCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGolemProjectCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AGolemProjectCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AGolemProjectCharacter::LookUpAtRate);

	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &AGolemProjectCharacter::Dash);

	//PlayerInputComponent->BindAction("Interacte", IE_Pressed, this, &AGolemProjectCharacter::Interact);
}

void AGolemProjectCharacter::BeginPlay()
{
	//currentSightWidget = CreateWidget(GetWorld(), sightHudClass);
	dashComponent = FindComponentByClass<UDashComponent>();
	mGrapple = FindComponentByClass<UGrappleComponent>();
	FistComp = FindComponentByClass<UFistComponent>();
	HealthComponent = FindComponentByClass<UHealthComponent>();
	PushingComponent = FindComponentByClass<UPushingComponent>();
	sightCamera = HelperLibrary::GetComponentByName<UChildActorComponent>(this, "ShoulderCamera");
	sightCameraL = HelperLibrary::GetComponentByName<UChildActorComponent>(this, "ShoulderCameraL");
	RaycastingComponent = FindComponentByClass<URaycastingComponent>();
	WallMechanicalComponent = FindComponentByClass<UWallMechanicalComponent>();
	//SlowMoComponent = FindComponentByClass<USlowMoComponent>();
	if (GetCharacterMovement())
	{
		initialGroundFriction = GetCharacterMovement()->GroundFriction;
	}
	pc = Cast<APlayerController>(GetController());
	if (HealthComponent)
	{
		HealthComponent->SetLastPositionGrounded(GetActorLocation());
	}
	if (pc)
	{
		pc->bShowMouseCursor = showCursor;
	}
	if (PushingComponent)
	{
		PushingComponent->OnStartPushingObject.AddDynamic(this, &AGolemProjectCharacter::SetUpBlockOffsetPositon);
	}

	if (mGrapple)
	{
		//mGrapple->IsTargetingGrapple = true;
		//ChangeToGrapple();
		InputComponent->BindAction("AssistedGrapple", IE_Released, mGrapple, &UGrappleComponent::StopClimb);
	}

	if (FistComp)
		FistComp->IsTargetingFist = false;

	IsInteracting = false;
	IsAiming = false;
	WantToAim = false;

	if (WallMechanicalComponent)
		WallMechanicalComponent->EndJump.AddDynamic(this, &AGolemProjectCharacter::AimAtEndOfWallJump);

	HasAlreadyMove = false;
	IsDashingDown = false;

	customCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Super::BeginPlay();
}

void AGolemProjectCharacter::Tick(float _deltaTime)
{
	Super::Tick(_deltaTime);
	if (mGrapple && !mGrapple->GetSwingPhysics())
	{
		FRotator rotFinal = FRotator::ZeroRotator;
		rotFinal.Yaw = GetActorRotation().Yaw;

		FRotator rot = FMath::Lerp(GetActorRotation(), rotFinal, 0.09f);
		SetActorRotation(rot);
	}

	if (PushingComponent && PushingComponent->GetIsPushingObject(false) && actorToInteract)
	{
		if (fabs(startPushingZ - GetActorLocation().Z) > 2.f)
		{
			StopPushBloc();
		}
		else
		{
			actorToInteract->SetActorLocation(GetActorLocation() + PushingComponent->GetBlockOffsetPosition());
			rightHandPosition = GetActorLocation() + offsetRightHand;
			leftHandPosition = GetActorLocation() + offsetLeftHand;
		}
	}

	if (NeedToReachLocation)
	{
		GoToLocation();
	}
	else if (HasToRotate)
	{
		RotateCharacter();
	}
	if (launchInfos.Num() > 0)
	{
		Super::LaunchCharacter(launchInfos[0].launchVelocity, launchInfos[0].bXYOverride, launchInfos[0].bZOverride);
		launchInfos.RemoveAt(0);
	}
}

void AGolemProjectCharacter::LaunchCharacter(FVector LaunchVelocity, bool bXYOverride, bool bZOverride)
{
	FLaunchInfo launchInfo;
	launchInfo.bZOverride = bZOverride;
	launchInfo.bXYOverride = bXYOverride;
	launchInfo.launchVelocity = LaunchVelocity;
	launchInfos.Add(launchInfo);
}

void AGolemProjectCharacter::Jump()
{
	if (IsDashingDown) return;
	if (GetCharacterMovement() != nullptr && GetCharacterMovement()->IsFalling() && WallMechanicalComponent != nullptr && WallMechanicalComponent->CanWallJump)
	{
		if (WallMechanicalComponent->WallJump())
		{
			if (isSightCameraEnabled)
			{
				ChangeCameraReleased();
			}
			if (dashComponent && IsDashing())
				dashComponent->CancelDash();

			WallJumpEvent();
		}
	}
	else if (PushingComponent == nullptr || !PushingComponent->GetIsPushingObject())
	{
		if (!HasAlreadyMove)
		{
			HasAlreadyMove = true;
			OnStartMoving.Broadcast();
		}
		Super::Jump();
	}

	if (mGrapple && mGrapple->GetSwingPhysics())
	{
		mGrapple->StopSwingPhysics();
	}
}

void AGolemProjectCharacter::Dash()
{
	if (IsDashingDown) return;
	if (mGrapple && !mGrapple->GetIsFiring() && dashComponent && !PushingComponent->GetIsPushingObject())
	{
		if (Controller != NULL)
		{
			FVector direction = GetLastMovementInputVector();

			if (m_valueForward == 0.0f && m_valueRight == 0.0f)
			{
				direction = GetActorForwardVector();
			}
			direction.Normalize();
			if (!HasAlreadyMove)
			{
				HasAlreadyMove = true;
				OnStartMoving.Broadcast();
			}
			dashComponent->Dash(direction);
		}
	}
}

void AGolemProjectCharacter::DashDown()
{
	if (IsDashingDown) return;
	if (mGrapple && !mGrapple->GetIsFiring() && dashComponent && !PushingComponent->GetIsPushingObject())
	{
		if (dashComponent->DashDown())
			IsDashingDown = true;
	}
}

void AGolemProjectCharacter::AimAtEndOfWallJump()
{
	if (WantToAim)
	{
		WantToAim = false;
		ChangeCameraPressed();
	}
}

void AGolemProjectCharacter::SwitchArm()
{
	if (!FistComp->IsTargetingFist)
	{
		ChangeToFist();
	}
	else if (!mGrapple->IsTargetingGrapple)
	{
		ChangeToGrapple();
	}
}

void AGolemProjectCharacter::UseAssistedGrapple()
{
	if (IsDashingDown) return;
	if (isGrappleSkillEnabled && mGrapple)
	{
		ChangeToGrapple();

		if (mGrapple->GetSwingPhysics() && !mGrapple->GetSwingPhysics()->WasStopped())
		{
			mGrapple->SetClimb(true);
		}
		else
			mGrapple->GoToDestination(true);
	}
}

void AGolemProjectCharacter::ChangeToGrapple()
{
	if (!isGrappleSkillEnabled || FistComp == nullptr || mGrapple == nullptr || mGrapple->IsTargetingGrapple) return;
	if (mGrapple != nullptr && mGrapple->GetSwingPhysics()) return;
	mGrapple->IsTargetingGrapple = true;
	FistComp->IsTargetingFist = false;
	OnGrappleEquiped.Broadcast();
	if (isSightCameraEnabled && pc && sightCamera)
	{
		pc->SetViewTargetWithBlend(sightCamera->GetChildActor(), 0.25f);
	}
}

void AGolemProjectCharacter::ChangeToFist()
{
	if (!isFistSkillEnabled || FistComp == nullptr || mGrapple == nullptr || FistComp && FistComp->IsTargetingFist) return;
	if (mGrapple != nullptr && mGrapple->GetSwingPhysics()) return;

	OnFistEquiped.Broadcast();
	FistComp->IsTargetingFist = true;
	mGrapple->IsTargetingGrapple = false;
	if (isSightCameraEnabled && pc && sightCameraL)
	{
		pc->SetViewTargetWithBlend(sightCameraL->GetChildActor(), 0.25f);
	}
}

void AGolemProjectCharacter::Fire()
{
	if (PushingComponent && PushingComponent->GetIsPushingObject())
	{
		return;
	}
	if (isGrappleSkillEnabled && mGrapple && mGrapple->IsTargetingGrapple)
	{
		mGrapple->Cancel();

		if (isSightCameraEnabled)
		{
			mGrapple->GoToDestination(false);
			if (!HasAlreadyMove)
			{
				HasAlreadyMove = true;
				OnStartMoving.Broadcast();
			}
		}
	}
	else if (isFistSkillEnabled && FistComp && FistComp->IsTargetingFist)
	{
		if (isSightCameraEnabled)
		{
			FistComp->GoToDestination();
			if (!HasAlreadyMove)
			{
				HasAlreadyMove = true;
				OnStartMoving.Broadcast();
			}
		}
	}
}

void AGolemProjectCharacter::TurnAtRate(float Rate)
{
	if (GetWorld())
	{
		// calculate delta for this frame from the rate information
		AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	}
}

void AGolemProjectCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	if (GetWorld())
	{
		AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
	}
}

void AGolemProjectCharacter::ChangeCameraPressed()
{
	if (WallMechanicalComponent != nullptr && !WallMechanicalComponent->CanAim)
	{
		WantToAim = true;
		return;
	}
	if (PushingComponent && PushingComponent->GetIsPushingObject())
	{
		return;
	}
	if (sightCamera && pc)
	{
		if (!isSightCameraEnabled)
		{
			if (isGrappleSkillEnabled && mGrapple && mGrapple->IsTargetingGrapple)
			{
				pc->SetViewTargetWithBlend(sightCamera->GetChildActor(), 0.25f);
				isSightCameraEnabled = true;
				if (GetCharacterMovement())
				{
					GetCharacterMovement()->bOrientRotationToMovement = false;
				}
				IsAiming = true;
				WantToAim = false;
			}
			else if (isFistSkillEnabled && FistComp && FistComp->IsTargetingFist)
			{
				pc->SetViewTargetWithBlend(sightCameraL->GetChildActor(), 0.25f);
				isSightCameraEnabled = true;

				/*if (mGrapple->GetSwingPhysics())
					SlowMoComponent->SetEnableSlowMo();*/

				if (GetCharacterMovement())
				{
					GetCharacterMovement()->bOrientRotationToMovement = false;
				}
				IsAiming = true;
				WantToAim = false;
			}

		}
	}
}

void AGolemProjectCharacter::ChangeCameraReleased()
{
	if (WallMechanicalComponent != nullptr && !WallMechanicalComponent->CanAim && WantToAim)
	{
		WantToAim = false;
		return;
	}
	if (PushingComponent && PushingComponent->GetIsPushingObject())
	{
		return;
	}
	if (sightCamera && pc)
	{
		if (isSightCameraEnabled)
		{
			IsAiming = false;
			isSightCameraEnabled = false;
			if (GetCharacterMovement())
			{
				GetCharacterMovement()->bOrientRotationToMovement = true;
			}
			pc->SetViewTargetWithBlend(this, 0.25f);
		}
	}
}

void AGolemProjectCharacter::MoveForward(float Value)
{
	if (IsDashingDown) return;
	if (PushingComponent && PushingComponent->GetIsStartingPushingObject() || (mGrapple && mGrapple->GetFiring() && !mGrapple->GetSwingPhysics()))
	{
		return;
	}
	m_valueForward = Value;
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		if (PushingComponent == nullptr || !PushingComponent->GetIsPushingObject())
		{
			if (mGrapple && FollowCamera && mGrapple->GetSwingPhysics())
			{
				FVector temp = FollowCamera->GetForwardVector();
				temp.Z = 0.f;
				mGrapple->GetSwingPhysics()->AddForceMovement(temp.GetSafeNormal() * m_valueForward);
			}
			else
			{
				if (mGrapple && mGrapple->IsTargetingGrapple && (isSightCameraEnabled || mGrapple->GetProjectile()) && GetCharacterMovement() && !GetCharacterMovement()->IsFalling())
				{
					Direction = mGrapple->GetDirection();
				}
				else if (FistComp && FistComp->IsTargetingFist && isSightCameraEnabled)
				{
					Direction = FistComp->GetDirection();
				}
			}
		}
		else
		{
			if (PushingComponent == nullptr || (PushingComponent->GetIsPushingObject(false) &&
				(pushedObjectIsCollidingForward && Value > 0 || pushedObjectIsCollidingBackward && Value < 0)))
			{
				Direction = FVector::ZeroVector;
			}
			else
			{
				Direction = PushingComponent->GetPushingDirection();
			}
		}
		Direction = Direction.GetSafeNormal();
		if (!HasAlreadyMove)
		{
			HasAlreadyMove = true;
			OnStartMoving.Broadcast();
		}
		AddMovementInput(Direction, Value);
	}
}

void AGolemProjectCharacter::MoveRight(float Value)
{
	if (IsDashingDown) return;
	if (PushingComponent && PushingComponent->GetIsPushingObject() || (mGrapple && mGrapple->GetFiring() && !mGrapple->GetSwingPhysics()))
	{
		return;
	}
	m_valueRight = Value;
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector
		FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		if (mGrapple && mGrapple->GetSwingPhysics() && FollowCamera)
		{
			FVector temp = FollowCamera->GetRightVector();
			temp.Z = 0.f;
			mGrapple->GetSwingPhysics()->AddForceMovement(temp.GetSafeNormal() * m_valueRight);
		}
		else
		{
			// add movement in that direction
			if (isSightCameraEnabled && mGrapple &&
				(mGrapple->IsTargetingGrapple || mGrapple->GetProjectile()) && GetCharacterMovement() && !GetCharacterMovement()->IsFalling())
			{
				Direction = mGrapple->GetDirection();

				float X = Direction.X;
				float Y = Direction.Y;
				float Z = Direction.Z;

				Direction.X = -Y;
				Direction.Y = X;
			}
			else if (isSightCameraEnabled && FistComp && FistComp->IsTargetingFist)
			{
				Direction = FistComp->GetDirection();

				float X = Direction.X;
				float Y = Direction.Y;
				float Z = Direction.Z;

				Direction.X = -Y;
				Direction.Y = X;
			}
			Direction = Direction.GetSafeNormal();
			if (!HasAlreadyMove)
			{
				HasAlreadyMove = true;
				OnStartMoving.Broadcast();
			}
			AddMovementInput(Direction, Value);
		}
	}
}

void AGolemProjectCharacter::ResetFriction()
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->GroundFriction = initialGroundFriction;
	}
}

void AGolemProjectCharacter::SetUpBlockOffsetPositon()
{
	if (actorToInteract && PushingComponent)
	{
		PushingComponent->SetBlockOffsetPosition(actorToInteract->GetActorLocation() - GetActorLocation());
		startPushingZ = GetActorLocation().Z;
		offsetLeftHand = leftHandPosition - GetActorLocation();
		offsetRightHand = rightHandPosition - GetActorLocation();
	}
}

bool AGolemProjectCharacter::PushBloc(FVector pushingDirection, FVector pushingPosition, FRotator pushingRotation)
{
	FVector tempPos = pushingPosition;
	if (GetWorld() && !GetWorld()->FindTeleportSpot(this, tempPos, pushingRotation) || tempPos != pushingPosition)
	{
		return false;
	}
	if (isSightCameraEnabled)
	{
		ChangeCameraReleased();
	}
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	if (PushingComponent)
	{
		PushingComponent->PushBloc(pushingDirection, pushingPosition, pushingRotation);
		PushingComponent->SetBlock(Cast<APushableBloc>(actorToInteract));
	}
	IsInteracting = true;
	return true;
}

void AGolemProjectCharacter::StopPushBloc()
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}
	if (PushingComponent)
	{
		PushingComponent->StopPushBloc();
	}
	pushedObjectIsCollidingForward = false;
	pushedObjectIsCollidingBackward = false;
	IsInteracting = false;
}

void AGolemProjectCharacter::InflictDamage(int _damage)
{
	if (HealthComponent != nullptr)
		HealthComponent->InflictDamage(_damage);
}

bool AGolemProjectCharacter::IsCharacterDead()
{
	if (HealthComponent) return HealthComponent->IsDead();
	return false;
}

void AGolemProjectCharacter::ActivateDeath(bool _activate)
{
	UCapsuleComponent* capsule = GetCapsuleComponent();
	if (FistComp)
		FistComp->DeleteHelpingAim();
	if (mGrapple)
	{
		mGrapple->DeleteHelpingAim();
		mGrapple->StopSwingPhysicsOnDeath();
	}
	ChangeCameraReleased();
	if (_activate)
	{
		if (capsule)
		{
			capsule->SetCollisionProfileName("NoCollision");
			capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		FollowCamera->DetachFromParent(true);
		GetCharacterMovement()->StopMovementImmediately();
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetMesh()->SetSimulatePhysics(true);
	}
}

bool AGolemProjectCharacter::IsCharacterSwinging()
{
	return mGrapple != nullptr && mGrapple->IsSwinging;
}

bool AGolemProjectCharacter::CanGoToLocation(FVector _location, bool _shoulKeepControllerDisable, bool _walk, FVector _directionToWatch, bool _hasToRotate)
{
	UNavigationPath* path = UNavigationSystemV1::FindPathToLocationSynchronously(GetWorld(), GetActorLocation(), _location, GetController());
	if (path != nullptr && path->PathPoints.Num() > 0 && !GetCharacterMovement()->IsFalling())
	{
		KeepControllerDisable = _shoulKeepControllerDisable;
		NeedToReachLocation = true;
		PathToFollow = path->PathPoints;
		HasToRotate = _hasToRotate;
		RotationToReach = UKismetMathLibrary::FindLookAtRotation(GetActorForwardVector(), _directionToWatch);
		RotationToReach.Pitch = 0.0f;
		RotationToReach.Roll = 0.0f;
		if (pc != nullptr)
			DisableInput(pc);
		if (GetCharacterMovement() != nullptr)
		{
			GetCharacterMovement()->MaxWalkSpeed = 200.0f;
			GetCharacterMovement()->StopMovementImmediately();
		}
		IsWalking = _walk;
		return true;
	}
	return false;
}

bool AGolemProjectCharacter::GoToLocation()
{
	if (PathToFollow.Num() > 0)
	{
		FVector nextLocation = PathToFollow[0];
		nextLocation.Z = GetActorLocation().Z;
		if (FVector::Dist(nextLocation, GetActorLocation()) < 2.0f)
		{
			PathToFollow.RemoveAt(0);
			if (PathToFollow.Num() == 0)
			{
				NeedToReachLocation = false;
				if (!KeepControllerDisable && pc != nullptr)
				{
					EnableInput(pc);
				}
				if (GetCharacterMovement() != nullptr)
					GetCharacterMovement()->MaxWalkSpeed = 600.0f;
				IsWalking = false;
				return true;
			}
		}
		else
		{
			FVector direction = PathToFollow[0] - GetActorLocation();
			AddMovementInput(direction);
			return false;
		}
	}
	return true;
}

void AGolemProjectCharacter::RotateCharacter()
{
	FRotator rot = FMath::Lerp(GetActorRotation(), RotationToReach, 0.09f);
	SetActorRotation(rot);
	if (rot.Yaw >= RotationToReach.Yaw - 0.25f && rot.Yaw <= RotationToReach.Yaw + 0.25f)
	{
		OnLocationReach.Broadcast();
		HasToRotate = false;
	}
}

FVector AGolemProjectCharacter::GetVirtualRightHandPosition()
{
	FVector pos = FVector::ZeroVector;
	if (GetMesh())
	{
		int id = GetMesh()->GetBoneIndex("VB hand_r");
		pos = GetMesh()->GetBoneTransform(id).GetLocation();
	}
	return pos;
}

FVector AGolemProjectCharacter::GetVirtualLeftHandPosition()
{
	FVector pos = FVector::ZeroVector;
	if (GetMesh())
	{
		int id = GetMesh()->GetBoneIndex("VB hand_l");
		pos = GetMesh()->GetBoneTransform(id).GetLocation();
	}
	return pos;
}