// Fill out your copyright notice in the Description page of Project Settings.


#include "GrappleComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GolemProjectCharacter.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Helpers/HelperLibrary.h"
#include "Camera/CameraComponent.h"
#include "Classes/Components/SkeletalMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Player/ProjectileHand.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Interfaces/Targetable.h"
#include "GolemProjectGameMode.h"
#include "SwingPhysic.h"
#include "DashComponent.h"
#include "DrawDebugHelpers.h"
#include "Player/Rope.h"
#include "Components/CapsuleComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

//#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UGrappleComponent::UGrappleComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...
}


// Called when the game starts
void UGrappleComponent::BeginPlay()
{
	Super::BeginPlay();
	SetTickGroup(ETickingGroup::TG_PrePhysics);
	AActor* owner = GetOwner();
	mCharacter = Cast<AGolemProjectCharacter>(owner);
	if (mCharacter)
	{
		mSkeletalMesh = mCharacter->GetMesh();
		if (mSkeletalMesh)
		{
			mIdBone = mSkeletalMesh->GetBoneIndex("hand_r");
		}
	}
	if (UChildActorComponent* child = HelperLibrary::GetComponentByName<UChildActorComponent>(mCharacter, "ShoulderCamera"))
	{
		mCamera = HelperLibrary::GetComponentByName<UCameraComponent>(child->GetChildActor(), "Camera");
	}
	IsFiring = false;
	world = GetWorld();

	if (world)
	{
		GameMode = Cast<AGolemProjectGameMode>(world->GetAuthGameMode());
	}
	if (mCharacter)
	{
		if (APlayerController* ctrl = Cast<APlayerController>(mCharacter->GetController()))
		{
			PlayerCameraManager = ctrl->PlayerCameraManager;
		}

		mCharacter->GetCustomCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &UGrappleComponent::OnBeginOverlap);
	}
	isColorRed = true;
	HasCreatedTarget = false;
}

void UGrappleComponent::CheckElementTargetable()
{
	if (mCharacter == nullptr) return;
	if (GameMode == nullptr) return;
	TArray<AActor*> allActors = GameMode->GetActorsTargetable();
	if (allActors.Num() <= 0) return;
	TArray<AActor*> actorCloseEnough;

	if (UCameraComponent* followingCam = mCharacter->GetFollowCamera())
	{
		if (world)
		{
			for (AActor* actor : allActors)
			{
				if (actor == nullptr || !actor->Implements<UTargetable>()) continue;
				//get all the actors that are close to the player
				if (FVector::DistSquared(actor->GetActorLocation(), mCharacter->GetActorLocation()) < maxDistanceSwinging * maxDistanceSwinging &&
					FVector::DistSquared(actor->GetActorLocation(), mCharacter->GetActorLocation()) > minDistance* minDistance)
				{
					actorCloseEnough.Add(actor);
				}
			}
			HelperLibrary::SortActorsByDistanceTo(actorCloseEnough, mCharacter);
			float bestDot = -1.0f;
			float haveFoundActor = false;
			for (AActor* actor : actorCloseEnough)
			{
				if (actor == nullptr) continue;
				// > 0 object seen
				FVector FromSoftware = (actor->GetActorLocation() - mCharacter->GetActorLocation());
				FromSoftware.Normalize();
				float dot = FVector::DotProduct(followingCam->GetForwardVector(), FromSoftware);
				//to change and finish
				if (dot > minDot&& dot > bestDot)
				{
					bestDot = dot;
					ClosestGrapplingHook = actor;
					haveFoundActor = true;
					if (bestDot == 1.0f) break;
					//FHitResult hitResult;
					//if (world->LineTraceSingleByChannel(hitResult, GetHandPosition(), actor->GetActorLocation(), ECollisionChannel::ECC_Visibility))
					//{
					//	if (ClosestGrapplingHook != nullptr && ClosestGrapplingHook == hitResult.GetActor()) return;
					//	if (ITargetable* target = Cast<ITargetable>(hitResult.GetActor()))
					//	{
					//		if (ITargetable* Lasttarget = Cast<ITargetable>(ClosestGrapplingHook))
					//		{
					//			Lasttarget->Execute_DestroyHUD(ClosestGrapplingHook);
					//		}
					//		target->Execute_CreateHUD(hitResult.GetActor());
					//		ClosestGrapplingHook = actor;
					//		
					//		HelperLibrary::Print("sds");
					//		if (bestDot == 1.0f) return;
					//		//return;
					//	}
					//}
				}
			}
			if (ClosestGrapplingHook != nullptr)
			{
				FHitResult hitResult;
				if (actorCloseEnough.Num() == 0 || !haveFoundActor)
				{
					if (!IsFiring)
					{
						if (ITargetable* Lasttarget = Cast<ITargetable>(ClosestGrapplingHook))
						{
							Lasttarget->Execute_DestroyHUD(ClosestGrapplingHook);
							if (mCharacter)
								mCharacter->DeactivateTargetGrapple();
						}
						ClosestGrapplingHook = nullptr;
						LastClosestGrapplingHook = nullptr;
						return;
					}
				}
				if (world->LineTraceSingleByChannel(hitResult, GetHandPosition(), ClosestGrapplingHook->GetActorLocation(), ECollisionChannel::ECC_Visibility))
				{
					if (LastClosestGrapplingHook == hitResult.GetActor()) return;
					ITargetable* Lasttarget = Cast<ITargetable>(LastClosestGrapplingHook);
					if (Lasttarget)
					{
						Lasttarget->Execute_DestroyHUD(LastClosestGrapplingHook);
					}
					ITargetable* target = Cast<ITargetable>(hitResult.GetActor());
					if (target)
					{
						target->Execute_CreateHUD(hitResult.GetActor());
						LastClosestGrapplingHook = hitResult.GetActor();
						if (mCharacter)
							mCharacter->ActivateTargetGrapple(LastClosestGrapplingHook);
					}
					return;
				}
			}
		}
	}
}

//launch projectile
void UGrappleComponent::GoToDestination(bool _isAssisted)
{
	if (_isAssisted && ClosestGrapplingHook == nullptr || mSkeletalMesh == nullptr) return;
	if (!currentProjectile)
	{
		if (world && mCamera)
		{
			currentProjectile = world->SpawnActor<AProjectileHand>(handProjectileClass, mSkeletalMesh->GetBoneTransform(mIdBone));
			if (currentProjectile)
			{
				mCharacter->GrapplingFireEvent();
				mCharacter->OnFireGrapple.Broadcast();
				mSkeletalMesh->HideBone(mIdBone, EPhysBodyOp::PBO_None);
				FVector offset = _isAssisted ? ClosestGrapplingHook->GetActorLocation() : (mCharacter->GetVirtualRightHandPosition() + mCamera->GetForwardVector() * maxDistanceGrappling);
				FVector direction = (offset - mCharacter->GetVirtualRightHandPosition());
				direction.Normalize();
				if (currentProjectile->GetMeshComponent())
				{
					mLastLocation = currentProjectile->GetLocation();
				}
				mDistance = 0.0f;
				//DrawDebugLine(world, mCamera->GetComponentLocation(), offset, FColor::Red, true);

				//currentProjectile->Instigator = mCharacter->GetInstigator();
				currentProjectile->SetOwner(mCharacter);
				currentProjectile->LaunchProjectile(direction, this);
				IsFiring = true;
				bIsAssisted = _isAssisted;
				currentProjectile->SetAssisted(_isAssisted);

				//Create the rope visual
				rope = world->SpawnActor<ARope>(ropeClass);
				rope->SetGrappleComponent(this);


			}
		}
	}
}

//cancel projectile
void UGrappleComponent::Cancel()
{
	if (currentProjectile && !currentProjectile->IsCollidingSwinging())
	{
		currentProjectile->SetComingBack(true);
	}
}

void UGrappleComponent::SetIKArm(FVector& _lookAt, bool& _isBlend)
{
	if (!currentProjectile)
		_lookAt = IKposition;
	else if (currentProjectile->GetMeshComponent())
		_lookAt = currentProjectile->GetLocation();
	else
		_lookAt = currentProjectile->GetActorLocation();

	if (mCharacter)
	{
		_isBlend = (!IsTargetingGrapple || mCharacter->GetSightCameraEnabled() || currentProjectile);
	}
}

FVector UGrappleComponent::GetHandPosition()
{
	FVector pos = FVector::ZeroVector;
	if (mSkeletalMesh)
	{
		pos = mSkeletalMesh->GetBoneTransform(mIdBone).GetLocation();
	}
	return pos;
}

void UGrappleComponent::DisplayHelping()
{
	FVector end = mCharacter->GetVirtualRightHandPosition() + mCamera->GetForwardVector() * maxDistanceGrappling;
	FVector location = mCharacter->GetVirtualRightHandPosition();
	FVector direction = end - location;
	FVector scale;
	FRotator rotation = direction.Rotation();
	FHitResult hitResult;
	if (HelperAiming == nullptr)
	{
		HelperAiming = world->SpawnActor<AActor>(HelperAimingClass);
		if (HelperAiming != nullptr)
			HelperAimingMesh = HelperAiming->FindComponentByClass<UStaticMeshComponent>();
	}
	else if (HelperAiming != nullptr)
	{
		HelperAiming->SetActorLocation(location);
		HelperAiming->SetActorRotation(rotation);
		scale = HelperAiming->GetActorScale3D();
		FVector distance = direction.GetSafeNormal() * maxDistanceGrappling;
		scale.Z = distance.Size() / 100.0f;
		HelperAiming->SetActorScale3D(scale);
		if (UKismetSystemLibrary::SphereTraceSingle(world, location, end, 6.0f, TraceTypeQuery1, false, ActorToIgnore, EDrawDebugTrace::None, hitResult, true))
		{
			UPhysicalMaterial* physMat;
			if (hitResult.GetComponent() != nullptr && hitResult.GetComponent()->GetMaterial(0))
			{
				physMat = hitResult.GetComponent()->GetMaterial(0)->GetPhysicalMaterial();
				if (physMat != nullptr && physMat->SurfaceType == EPhysicalSurface::SurfaceType1)
				{
					if (isColorRed)
					{
						HelperAimingMesh->SetVectorParameterValueOnMaterials("Color", FVector4(0.0f, 50.0f, 0.0f, 0.0f));
						isColorRed = false;
					}
				}
				else
				{
					if (!isColorRed)
					{
						HelperAimingMesh->SetVectorParameterValueOnMaterials("Color", FVector4(50.0f, 0.0f, 0.0f, 0.0f));
						isColorRed = true;
					}
				}
				distance = hitResult.ImpactPoint - location;
				scale.Z = distance.Size() / 100.0f;
				HelperAiming->SetActorScale3D(scale);
			}
		}
		else
		{
			if (!isColorRed)
			{
				HelperAimingMesh->SetVectorParameterValueOnMaterials("Color", FVector4(50.0f, 0.0f, 0.0f, 0.0f));
				isColorRed = true;
			}
		}
	}
}

void UGrappleComponent::UpdateIKArm()
{
	if (world && mCamera && mCharacter)
	{
		FVector offset = mCamera->GetForwardVector() * accuracy;
		mDirection = offset - mCharacter->GetActorLocation();
		IKposition = offset;
		mDirection.Z = 0.0f;

		//Rotate character when he is aiming something
		mCharacter->SetActorRotation(mDirection.Rotation());

		//I don't know how anim works in cpp
		//UAnimInstance* animBp = mSkeletalMesh->GetAnimInstance();
	}
}

// Called every frame
void UGrappleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	CheckElementTargetable();

	//Update Ik Arm animation
	if (mCharacter && mCamera)
	{
		if (IsTargetingGrapple && mCharacter->GetSightCameraEnabled() && !currentProjectile)
		{
			UpdateIKArm();
			DisplayHelping();
			isAiming = true;
		}
		else
		{
			if (HelperAiming != nullptr)
			{
				HelperAiming->Destroy();
				HelperAiming = nullptr;
				isAiming = false;
				isColorRed = true;
			}
		}
	}
	if (currentProjectile && currentProjectile->GetMeshComponent() && mSkeletalMesh)
	{
		mDirection = currentProjectile->GetLocation() - mCharacter->GetVirtualRightHandPosition();
		mDistance += FVector::Dist(mLastLocation, currentProjectile->GetLocation());
		float distanceWithCharacter = mDirection.Size();
		mLastLocation = currentProjectile->GetLocation();
		if (currentProjectile->IsComingBack())
		{
			if (distanceWithCharacter < offsetStop)
			{
				PlayerIsNear();
				return;
			}
		}
		else if (currentProjectile->IsCollidingGrappling())
		{
			if (mCharacter)
			{
				if (!bIsAssisted)
				{
					if (distanceWithCharacter > offsetStop)
					{
						AttractCharacter();
					}
					else
					{
						PlayerIsNear();
						return;
					}
				}
			}
		}
		else if (currentProjectile->IsCollidingSwinging())
		{
			if (mCharacter && bIsAssisted)
			{
				//Create the swing physics for the player
				if (!swingPhysic && ClosestGrapplingHook)
				{
					mCharacter->StartSwingEvent();
					mCharacter->ChangeToFist();

					swingPhysic = new USwingPhysic(this);
					
					swingPhysic->SetScaleGravity(scaleGravity);
					swingPhysic->SetFriction(friction);
					swingPhysic->SetForceMovement(forceMovement);
					swingPhysic->SetSpeedRotation(speedRotation);
					swingPhysic->SetMinLength(minDistanceSwinging);
					swingPhysic->SetMaxLength(maxDistanceSwinging);
					swingPhysic->SetReleaseForce(releaseForce);
					swingPhysic->SetReduceRopeSpeed(reduceRopeSpeed);
					swingPhysic->SetFrameRateMin(frameRateMin);

					IsSwinging = true;
					if (mCharacter->GetCustomCapsuleComponent())
					{
						mCharacter->GetCustomCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					}
					//Reset dash when the player grappled something
					if (UDashComponent* dashComp = mCharacter->FindComponentByClass<UDashComponent>())
						dashComp->ResetDashInAir();
				}
			}
		}
		else
		{
			if (mDistance > maxDistanceGrappling)
			{
				currentProjectile->SetComingBack(true);
			}
		}

		//if swing Physics exists we have to tick it
		if (swingPhysic)
		{
			if (bIsClimbing)
				swingPhysic->ReduceRope();

			swingPhysic->Tick(DeltaTime);

			if (bDestroyCustomPhy)
			{
				delete swingPhysic;
				swingPhysic = nullptr;
				bDestroyCustomPhy = false;
			}
		}

	}
}

void UGrappleComponent::StopSwingPhysics(const bool& _comingBack)
{
	if (swingPhysic && currentProjectile)
	{
		HelperLibrary::Print("destroyswing");
		bIsAssisted = false;
		bDestroyCustomPhy = true;
		IsSwinging = false;
		currentProjectile->SetComingBack(_comingBack);
		mCharacter->GetCustomCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void UGrappleComponent::StopSwingPhysicsOnDeath()
{
	if (rope)
		rope->HideMesh();
	//if (swingPhysic && currentProjectile)
	//{
	//	bIsAssisted = false;
	//	delete swingPhysic;
	//	swingPhysic = nullptr;
	//	IsSwinging = false;
	//	currentProjectile->DestroyProjectile();
	//	currentProjectile = nullptr;
	//	//currentProjectile->SetComingBack(_comingBack);
	//	mCharacter->GetCustomCapsuleComponent()->OnComponentBeginOverlap.RemoveAll(this);
	//}
}

bool UGrappleComponent::CheckGround(FVector _impactNormal)
{
	bool bStopSwingPhysics = false;
	float angle = 0.5f;
	if (world)
	{
		if (rope && _impactNormal.Z > angle)
		{
				FRotator rotFinal = FRotator::ZeroRotator;
				rotFinal.Yaw = mCharacter->GetActorRotation().Yaw;
				mCharacter->SetActorRotation(rotFinal);
				bStopSwingPhysics = true;
		}
	}
	return bStopSwingPhysics;
}

void UGrappleComponent::DeleteHelpingAim()
{
	if (HelperAiming != nullptr)
	{
		HelperAiming->Destroy();
		HelperAiming = nullptr;
		isAiming = false;
		isColorRed = true;
	}
}

void UGrappleComponent::PlayerIsNear()
{
	if (mCharacter)
	{
		//Find destination stop player

		if (mCharacter->GetCharacterMovement() && currentProjectile->IsCollidingGrappling())
		{
			mCharacter->GetCharacterMovement()->Velocity *= stopScaleVelocity;
		}
		mCharacter->ResetFriction();

		if (mSkeletalMesh)
		{
			mSkeletalMesh->UnHideBone(mIdBone);
			mSkeletalMesh->bRequiredBonesUpToDate = false;
		}

		if (rope)
			rope->Destroy();

		if (currentProjectile)
		{
			mCharacter->EndGrapplingEvent();
			currentProjectile->Destroy();
		}

		bIsAttracting = false;
		rope = nullptr;
		currentProjectile = nullptr;
		IsFiring = false;
		mCharacter->OnResetGrapple.Broadcast();
	}
}

void UGrappleComponent::AttractCharacter()
{
	FVector tempDir;
	mDirection.Normalize();
	tempDir = mDirection;
	if (mCharacter && mCharacter->GetCharacterMovement())
	{
		if (!bIsAttracting)
			mCharacter->StartingAttractionEvent();

		bIsAttracting = true;
		mCharacter->GetCharacterMovement()->GroundFriction = 0.0f;
		mCharacter->LaunchCharacter(mDirection * velocity, true, true);

		//change rotation player when the grapple isn't assisted
		FRotator finalRotation = tempDir.Rotation();
		finalRotation.Add(-90.0f, 0.0f, 0.0f);

		FRotator rotation = FMath::Lerp(mCharacter->GetActorRotation(), finalRotation, 0.1f);
		mCharacter->SetActorRotation(rotation);

		if (world)
		{
			//check if the player doesn't collide with anything(in front of him)
			FHitResult hit;
			if (world->LineTraceSingleByChannel(hit, mCharacter->GetActorLocation(), mCharacter->GetActorLocation() + mDirection * offsetBlockingObject, ECollisionChannel::ECC_Visibility))
			{
				if (mCharacter)
					mCharacter->GetCharacterMovement()->Velocity *= stopScaleVelocity;

				currentProjectile->SetComingBack(true);
			}
		}
	}
}

void UGrappleComponent::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	TArray<FHitResult> AllResults;

	// Get the location of this actor
	auto Start = mCharacter->GetActorLocation();
	// Get the location of the other component
	auto End = OtherComp->GetComponentLocation();

	// Now do a spherical sweep to find the overlap
	GetWorld()->SweepMultiByObjectType(
		AllResults,
		Start,
		End,
		mCharacter->GetActorQuat(),
		FCollisionObjectQueryParams::AllObjects,
		FCollisionShape::MakeCapsule(
			mCharacter->GetCustomCapsuleComponent()->GetScaledCapsuleRadius(),
			mCharacter->GetCustomCapsuleComponent()->GetScaledCapsuleHalfHeight()),
		FCollisionQueryParams::FCollisionQueryParams(false)
	);

	// Finally check which hit result is the one from this event
	for (auto HitResult : AllResults)
	{
		if (OtherComp->GetUniqueID() == HitResult.GetComponent()->GetUniqueID()) {

			// A component with the same UniqueID means we found our overlap!

			// Do your stuff here, using info from 'HitResult'
			if (CheckGround(HitResult.Normal))
			{
				if (swingPhysic)
				{
					HelperLibrary::Print(HitResult.GetComponent()->GetPathName());
					StopSwingPhysics();
				}
				return;
			}
			else
			{
				if (swingPhysic)
					swingPhysic->InvertVelocity(HitResult.Normal);
			}
		}
	}
}

void UGrappleComponent::SetClimb(bool _isClimbing) {
	if (!bIsClimbing)
		mCharacter->StartReducingRopeEvent();

	bIsClimbing = _isClimbing;
};

void UGrappleComponent::StopClimb() {
	mCharacter->EndReducingRopeEvent();
	bIsClimbing = false;
};
