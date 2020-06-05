// Fill out your copyright notice in the Description page of Project Settings.


#include "FistComponent.h"
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
#include "Player/FistProjectile.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "DrawDebugHelpers.h"
#include "GrappleComponent.h"
#include "SwingPhysic.h"
#include "Objects/MovingPlatform.h"

// Sets default values for this component's properties
UFistComponent::UFistComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UFistComponent::BeginPlay()
{
	Super::BeginPlay();
	SetTickGroup(ETickingGroup::TG_PostPhysics);

	AActor* owner = GetOwner();
	mCharacter = Cast<AGolemProjectCharacter>(owner);
	mSkeletalMesh = mCharacter->GetMesh();
	mIdBone = mSkeletalMesh->GetBoneIndex("hand_l");
	UChildActorComponent* child = HelperLibrary::GetComponentByName<UChildActorComponent>(mCharacter, "ShoulderCamera");
	mCamera = HelperLibrary::GetComponentByName<UCameraComponent>(child->GetChildActor(), "Camera");
	world = GetWorld();
	CanFire = true;
	isColorRed = true;
}

void UFistComponent::UpdateIKArm()
{
	if (world && mCamera && mCharacter)
	{
		FVector offset = mCamera->GetForwardVector() * accuracy;
		mDirection = offset - mCharacter->GetActorLocation();
		IKposition = offset;
		mDirection.Z = 0.0f;
		
		if (UGrappleComponent* grapple = mCharacter->FindComponentByClass<UGrappleComponent>())
		{
			if (USwingPhysic* phys = grapple->GetSwingPhysics())
			{
				phys->SetCameraDirection(mDirection);
				return;
			}
		}

		mCharacter->SetActorRotation(mDirection.Rotation());

		//I don't know how anim works in cpp
		//UAnimInstance* animBp = mSkeletalMesh->GetAnimInstance();

	}
}

FVector UFistComponent::GetHandPosition()
{
	FVector pos = FVector::ZeroVector;
	if (mSkeletalMesh)
	{
		pos = mSkeletalMesh->GetBoneTransform(mIdBone).GetLocation();
	}
	return pos;
}

void UFistComponent::SetIKArm(FVector& _lookAt, bool& _isBlend)
{
	if (!currentProjectile)
		_lookAt = IKposition;
	if (mCharacter)
	{
		_isBlend = (mCharacter->GetSightCameraEnabled());
	}
}

void UFistComponent::GoToDestination()
{
	if (!currentProjectile && CanFire)
	{
		if (world && mCamera && mSkeletalMesh)
		{
			mSkeletalMesh->HideBone(mIdBone, EPhysBodyOp::PBO_None);

			currentProjectile = world->SpawnActor<AFistProjectile>(fistProjectileClass, mSkeletalMesh->GetBoneTransform(mIdBone));
			if (currentProjectile)
			{
				FVector offset = GetHandPosition() + mCamera->GetForwardVector() * accuracy;
				FVector direction = (offset - currentProjectile->GetActorLocation());
				direction /= direction.Size();

				//currentProjectile->Instigator = mCharacter->GetInstigator();
				currentProjectile->SetOwner(mCharacter);
				currentProjectile->LaunchFist(direction, true, maxDistance, speedMax, mCharacter);
				currentProjectile = nullptr;
				world->GetTimerManager().SetTimer(TimerHandleFire, this, &UFistComponent::ResetFire, (maxDistance / speedMax) + 0.60f, false);
				CanFire = false;
				mCharacter->FireEvent();
				mCharacter->OnFireProjectile.Broadcast();

				if (UGrappleComponent* grapple = mCharacter->FindComponentByClass<UGrappleComponent>())
				{
					if (USwingPhysic* phys = grapple->GetSwingPhysics())
					{
						phys->AddForceMovement(-direction * pushingForce);
					}
				}
			}
		}
	}
}

void UFistComponent::DisplayTrajectory()
{

}

void UFistComponent::ResetFire()
{
	CanFire = true;
	mCharacter->OnResetProjectile.Broadcast();
	if (mSkeletalMesh)
	{
		mSkeletalMesh->UnHideBone(mIdBone);
		mSkeletalMesh->bRequiredBonesUpToDate = false;
	}
}

void UFistComponent::DeleteHelpingAim()
{
	if (HelperAiming.Num() != 0)
	{
		for (int i = 0; i < HelperAiming.Num(); ++i)
		{
			if (HelperAiming[i] != nullptr)
			{
				HelperAiming[i]->Destroy();
			}
		}
		HelperAimingMesh.Empty();
		HelperAiming.Empty();
		isColorRed = true;
	}
}

// Called every frame
void UFistComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (mCharacter)
	{
		if (IsTargetingFist && mCharacter->GetSightCameraEnabled())
		{
			UpdateIKArm();
			if (CanFire && mCamera && world)
			{
				FVector end = GetHandPosition() + mCamera->GetForwardVector() * maxDistance;
				FVector direction = (end - GetHandPosition()).GetSafeNormal();
				FVector location = GetHandPosition();
				FVector scale;
				FRotator rotation = direction.Rotation();
				CanInteract = false;
				ActorToIgnore.Empty();
				for (int i = 0; i < NumberBounce; ++i)
				{
					//if helping is not spawn, spawn it
					if (HelperAiming.Num() <= i)
					{
						AActor* actorSpawned = world->SpawnActor<AActor>(HelperAimingClass);
						HelperAiming.Add(actorSpawned);
						if (HelperAiming[i] != nullptr)
							HelperAimingMesh.Add(actorSpawned->FindComponentByClass<UStaticMeshComponent>());
					}
					if (HelperAiming[i] != nullptr)
					{
						HelperAiming[i]->SetActorLocation(location);
						FHitResult hitResult;
						HelperAiming[i]->SetActorRotation(rotation);
						scale = HelperAiming[i]->GetActorScale3D();
						FVector distance = direction * maxDistance;
						scale.Z = distance.Size() / 100.0f;
						HelperAiming[i]->SetActorScale3D(scale);
						//raycast to see if there is any obstacle in front of player
						if (UKismetSystemLibrary::SphereTraceSingle(world, location, end, 11.0f, TraceTypeQuery1, false, ActorToIgnore, EDrawDebugTrace::None, hitResult, true))
						{
							ActorToIgnore.Empty();
							ActorToIgnore.Add(hitResult.GetActor());
							UPhysicalMaterial* physMat;
							distance = hitResult.ImpactPoint - location;
							scale.Z = distance.Size() / 100.0f;
							HelperAiming[i]->SetActorScale3D(scale);
							if (hitResult.GetActor() != nullptr)
							{
								if (IInteractable* interactable = Cast<IInteractable>(hitResult.GetActor()))
								{
									if (interactable->CanBeActivatedByFist)
										CanInteract = true;
								}
								if (AMovingPlatform* platform = Cast<AMovingPlatform>(hitResult.GetActor()))
								{
									if (platform->activatedByHand && !platform->HasBeenActivated)
										CanInteract = true;
								}
							}
							//scale the helping actor to avoid it to going through wall
							if (hitResult.GetComponent() != nullptr && hitResult.GetComponent()->GetMaterial(0) != nullptr)
							{
								physMat = hitResult.GetComponent()->GetMaterial(0)->GetPhysicalMaterial();
								//if it's a bouncing surface the calculate the direction of bounce 
								if (physMat != nullptr && physMat->SurfaceType == EPhysicalSurface::SurfaceType2)
								{
									direction = direction.MirrorByVector(hitResult.ImpactNormal);
									end = direction * maxDistance;
									location = hitResult.ImpactPoint;
									rotation = direction.Rotation();
								}
								else
								{
									//stop loop
									if (HelperAiming.Num() != 0)
									{
										for (int j = i + 1; j < HelperAiming.Num(); ++j)
										{
											UStaticMeshComponent* staticMeshToDelete = HelperAiming[j]->FindComponentByClass<UStaticMeshComponent>();
											if (HelperAimingMesh.Contains(staticMeshToDelete))
												HelperAimingMesh.Remove(staticMeshToDelete);
											HelperAiming[j]->Destroy();
											HelperAiming.RemoveAt(j);
										}
									}
									i = NumberBounce;
								}
							}
						}
					}
				}
				if (CanInteract)
				{
					if (isColorRed)
					{
						for (int i = 0; i < HelperAimingMesh.Num(); ++i)
						{
							HelperAimingMesh[i]->SetVectorParameterValueOnMaterials("Color", FVector4(0.0f, 50.0f, 0.0f, 0.0f));
						}
						isColorRed = false;
					}
				}
				else
				{
					if (!isColorRed)
					{
						for (int i = 0; i < HelperAimingMesh.Num(); ++i)
						{
							HelperAimingMesh[i]->SetVectorParameterValueOnMaterials("Color", FVector4(50.0f, 0.0f, 0.0f, 0.0f));
						}
						isColorRed = true;
					}
				}
			}
			else
			{
				if (HelperAiming.Num() != 0)
				{
					for (int i = 0; i < HelperAiming.Num(); ++i)
					{
						if (HelperAiming[i] != nullptr)
						{
							HelperAiming[i]->Destroy();
						}
					}
					HelperAimingMesh.Empty();
					HelperAiming.Empty();
					isColorRed = true;
				}
			}
		}
		else
		{
			if (HelperAiming.Num() != 0)
			{
				for (int i = 0; i < HelperAiming.Num(); ++i)
				{
					if (HelperAiming[i] != nullptr)
					{
						HelperAiming[i]->Destroy();
					}
				}
				HelperAimingMesh.Empty();
				HelperAiming.Empty();
				isColorRed = true;
			}
		}
	}
}

