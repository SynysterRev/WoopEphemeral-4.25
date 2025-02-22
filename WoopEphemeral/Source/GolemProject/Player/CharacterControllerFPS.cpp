// Golem Project - Créajeux 2020


#include "CharacterControllerFPS.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Player/GrappleComponent.h"
#include "Player/PushingComponent.h"
#include "Player/FistComponent.h"
#include "Player/HealthComponent.h"
#include "Player/SwingPhysic.h"
#include "Player/WallMechanicalComponent.h"

#include "Helpers/HelperLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AMyProjectFPSWoopCharacter

ACharacterControllerFPS::ACharacterControllerFPS()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Configure character movement
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->JumpZVelocity = 800.f;
		GetCharacterMovement()->AirControl = 0.6f;
	}

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	//create weapon and location to spawn skills
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon"));
	Weapon->bCastDynamicShadow = false;
	Weapon->CastShadow = false;
	Weapon->SetupAttachment(RootComponent);

	MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	MuzzleLocation->SetupAttachment(Weapon);
	MuzzleLocation->SetRelativeLocation(FVector(0.0f, 60.0f, 11.0f));

	MuzzleGrappleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleGrappleLocation"));
	MuzzleGrappleLocation->SetupAttachment(Weapon);
	MuzzleGrappleLocation->SetRelativeLocation(FVector(0.0f, 50.0f, 3.0f));

	//custom capsule use when swinging
	customCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("capsule"));
	customCapsule->InitCapsuleSize(43.0f, 97.0f);
	customCapsule->SetupAttachment(RootComponent);
}

void ACharacterControllerFPS::BeginPlay()
{
	//get all component
	dashComponent = FindComponentByClass<UDashComponent>();
	mGrapple = FindComponentByClass<UGrappleComponent>();
	PushingComponent = FindComponentByClass<UPushingComponent>();
	FistComp = FindComponentByClass<UFistComponent>();
	HealthComponent = FindComponentByClass<UHealthComponent>();
	WallMechanicalComponent = FindComponentByClass<UWallMechanicalComponent>();

	//hide mesh
	Mesh1P->SetHiddenInGame(false, true);

	Weapon->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	//allow to start timer on speedrun level
	HasAlreadyMove = false;

	IsDashingDown = false;

	isFistSkillEnabled = true;
	isGrappleSkillEnabled = true;

	if (mGrapple)
	{
		InputComponent->BindAction("AssistedGrapple", IE_Released, mGrapple, &UGrappleComponent::StopClimb);
	}

	if (PushingComponent)
	{
		pushedObjectIsCollidingForward = false;
		pushedObjectIsCollidingBackward = false;
		PushingComponent->OnStartPushingObject.AddDynamic(this, &ACharacterControllerFPS::SetUpBlockOffsetPositon);
	}

	//get friction for dash
	if (GetCharacterMovement())
	{
		initialGroundFriction = GetCharacterMovement()->GroundFriction;
	}

	//we don't need collision when walking
	customCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Super::BeginPlay();
}

void ACharacterControllerFPS::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (mGrapple && !mGrapple->GetSwingPhysics())
	{
		FRotator rotFinal = FRotator::ZeroRotator;
		rotFinal.Yaw = GetActorRotation().Yaw;

		FRotator rot = FMath::Lerp(GetActorRotation(), rotFinal, 0.09f);
		SetActorRotation(rot);
	}

	//when multiples launch at the same frame, dispatch the call 1 / frame
	if (launchInfos.Num() > 0)
	{
		Super::LaunchCharacter(launchInfos[0].launchVelocity, launchInfos[0].bXYOverride, launchInfos[0].bZOverride);
		launchInfos.RemoveAt(0);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ACharacterControllerFPS::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ACharacterControllerFPS::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACharacterControllerFPS::MoveRight);


	//skills
	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &ACharacterControllerFPS::Dash);
	PlayerInputComponent->BindAction("DashDown", IE_Pressed, this, &ACharacterControllerFPS::DashDown);

	PlayerInputComponent->BindAction("FireFist", IE_Released, this, &ACharacterControllerFPS::Fire);

	PlayerInputComponent->BindAction("FireGrapple", IE_Released, this, &ACharacterControllerFPS::FireGrapple);
	PlayerInputComponent->BindAction("AssistedGrapple", IE_Pressed, this, &ACharacterControllerFPS::UseAssistedGrapple);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ACharacterControllerFPS::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ACharacterControllerFPS::LookUpAtRate);
}

//add all the launchCharacter to an array to dispatch them, 1 / frame
void ACharacterControllerFPS::LaunchCharacter(FVector LaunchVelocity, bool bXYOverride, bool bZOverride)
{
	FLaunchInfos launchInfo;
	launchInfo.bZOverride = bZOverride;
	launchInfo.bXYOverride = bXYOverride;
	launchInfo.launchVelocity = LaunchVelocity;
	launchInfos.Add(launchInfo);
}

//////////////////////////////////////////////////////////////////////////
// Movement
void ACharacterControllerFPS::MoveForward(float Value)
{
	if (IsDashingDown) return;
	if (PushingComponent && PushingComponent->GetIsStartingPushingObject() || (mGrapple && mGrapple->GetIsFiring() && !mGrapple->GetSwingPhysics()))
	{
		return;
	}
	m_valueForward = Value;
	if (Value != 0.0f)
	{
		FVector Direction = GetActorForwardVector();
		if (PushingComponent == nullptr || !PushingComponent->GetIsPushingObject())
		{
			if (mGrapple && mGrapple->GetSwingPhysics())
			{
				FVector temp = GetActorForwardVector();
				temp.Z = 0.f;
				mGrapple->GetSwingPhysics()->AddForceMovement(temp.GetSafeNormal() * m_valueForward);
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
		//start timer
		if (!HasAlreadyMove)
		{
			HasAlreadyMove = true;
			OnStartMoving.Broadcast();
		}
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void ACharacterControllerFPS::MoveRight(float Value)
{
	if (IsDashingDown) return;
	if (PushingComponent && PushingComponent->GetIsStartingPushingObject() || (mGrapple && mGrapple->GetIsFiring() && !mGrapple->GetSwingPhysics()))
	{
		return;
	}
	m_valueRight = Value;
	if (Value != 0.0f)
	{
		if (mGrapple && mGrapple->GetSwingPhysics())
		{
			FVector temp = GetActorRightVector();
			temp.Z = 0.f;
			mGrapple->GetSwingPhysics()->AddForceMovement(temp.GetSafeNormal() * m_valueRight);
		}
		else
		{

			//start timer
			if (!HasAlreadyMove)
			{
				HasAlreadyMove = true;
				OnStartMoving.Broadcast();
			}
			// add movement in that direction
			AddMovementInput(GetActorRightVector(), Value);
		}
	}
}

void ACharacterControllerFPS::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information

	if (mGrapple && !mGrapple->CanMove()) return;
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ACharacterControllerFPS::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information

	if (mGrapple && !mGrapple->CanMove()) { return;}
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ACharacterControllerFPS::AddControllerPitchInput(float Val)
{
	if (mGrapple && !mGrapple->CanMove()) { return; }
	Super::AddControllerPitchInput(Val);
}

void ACharacterControllerFPS::AddControllerYawInput(float Val)
{
	if (mGrapple && !mGrapple->CanMove()) { return; }
	Super::AddControllerYawInput(Val);
}

void ACharacterControllerFPS::Jump()
{
	if (IsDashingDown) return;
	if (GetCharacterMovement() != nullptr && GetCharacterMovement()->IsFalling() && WallMechanicalComponent != nullptr && WallMechanicalComponent->CanWallJump)
	{
		if (WallMechanicalComponent->WallJump())
		{
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

//////////////////////////////////////////////////////////////////////////
// ??
void ACharacterControllerFPS::InflictDamage(int _damage)
{
	if (HealthComponent != nullptr)
		HealthComponent->InflictDamage(_damage);
}

bool ACharacterControllerFPS::IsCharacterDead()
{
	if (HealthComponent != nullptr)
	{
		return HealthComponent->IsDead();
	}
	return false;
}

void ACharacterControllerFPS::ActivateDeath()
{
	UCapsuleComponent* capsule = GetCapsuleComponent();
	if (mGrapple)
	{
		mGrapple->StopSwingPhysicsOnDeath();
	}
	/*if (capsule)
	{
		capsule->SetCollisionProfileName("NoCollision");
		capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}*/
	//FollowCamera->DetachFromParent(true);
	//GetCharacterMovement()->StopMovementImmediately();
	/*GetMesh1P()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh1P()->SetSimulatePhysics(true);*/
}

void ACharacterControllerFPS::ResetFriction()
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->GroundFriction = initialGroundFriction;
	}
}

//////////////////////////////////////////////////////////////////////////
// Skills
void ACharacterControllerFPS::Dash()
{
	if (IsDashingDown) return;
	if (mGrapple && !mGrapple->GetIsFiring() && dashComponent && !PushingComponent->GetIsPushingObject())
	{
		if (Controller != NULL)
		{
			FVector direction = GetLastMovementInputVector();

			//dash in direction he looks if not moving
			if (m_valueForward == 0.0f && m_valueRight == 0.0f)
			{
				direction = GetActorForwardVector();
			}
			direction.Normalize();
			//start timer
			if (!HasAlreadyMove)
			{
				HasAlreadyMove = true;
				OnStartMoving.Broadcast();
			}
			dashComponent->Dash(direction);
		}
	}
}

void ACharacterControllerFPS::DashDown()
{
	if (IsDashingDown) return;
	if (mGrapple && !mGrapple->GetIsFiring() && dashComponent && !PushingComponent->GetIsPushingObject())
	{
		if (dashComponent->DashDown())
		{
			IsDashingDown = true;
			//start timer
			if (!HasAlreadyMove)
			{
				HasAlreadyMove = true;
				OnStartMoving.Broadcast();
			}
		}
	}
}

//fire with fist
void ACharacterControllerFPS::Fire()
{
	if (PushingComponent && PushingComponent->GetIsPushingObject())
	{
		return;
	}
	if (isFistSkillEnabled && FistComp)
	{
		FistComp->GoToDestination(MuzzleLocation->GetComponentTransform());
		if (!HasAlreadyMove)
		{
			HasAlreadyMove = true;
			OnStartMoving.Broadcast();
		}
	}
}

void ACharacterControllerFPS::FireGrapple()
{
	if (PushingComponent && PushingComponent->GetIsPushingObject())
	{
		return;
	}
	if (isGrappleSkillEnabled && mGrapple)
	{
		//make grapple comeback if already launch
		mGrapple->Cancel();
		mGrapple->GoToDestination(false, MuzzleGrappleLocation->GetComponentTransform());
		if (!HasAlreadyMove)
		{
			HasAlreadyMove = true;
			OnStartMoving.Broadcast();
		}
	}
}

//fire autograpple to swing
void ACharacterControllerFPS::UseAssistedGrapple()
{
	if (IsDashingDown) return;
	if (isGrappleSkillEnabled && mGrapple)
	{
		//shrink the rope
		if (mGrapple->GetSwingPhysics() && !mGrapple->GetSwingPhysics()->WasStopped())
		{
			mGrapple->SetClimb(true);
		}
		else
			mGrapple->GoToDestination(true, MuzzleGrappleLocation->GetComponentTransform());
		if (!HasAlreadyMove)
		{
			HasAlreadyMove = true;
			OnStartMoving.Broadcast();
		}
	}
}

bool ACharacterControllerFPS::IsCharacterSwinging()
{
	return mGrapple != nullptr && mGrapple->IsSwinging;
}

void ACharacterControllerFPS::SetUpBlockOffsetPositon()
{
	if (actorToInteract && PushingComponent)
	{
		PushingComponent->SetBlockOffsetPosition(actorToInteract->GetActorLocation() - GetActorLocation());
		startPushingZ = GetActorLocation().Z;
		offsetLeftHand = leftHandPosition - GetActorLocation();
		offsetRightHand = rightHandPosition - GetActorLocation();
	}
}

/*bool ACharacterControllerFPS::PushBloc(FVector pushingDirection, FVector pushingPosition, FRotator pushingRotation)
{
	FVector tempPos = pushingPosition;
	if (GetWorld() && !GetWorld()->FindTeleportSpot(this, tempPos, pushingRotation) || tempPos != pushingPosition)
	{
		return false;
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
}*/

/*void ACharacterControllerFPS::StopPushBloc()
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
}*/

