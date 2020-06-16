// Fill out your copyright notice in the Description page of Project Settings.


#include "FistComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "CharacterControllerFPS.h"
#include "Engine/Engine.h"
#include "Helpers/HelperLibrary.h"
#include "Camera/CameraComponent.h"
#include "Player/ProjectileHand.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Interfaces/Targetable.h"
#include "Player/FistProjectile.h"
#include "DrawDebugHelpers.h"
#include "GrappleComponent.h"
#include "SwingPhysic.h"
#include "Interfaces/Interactable.h"
#include "Interfaces/Activable.h"

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
	mCharacter = Cast<ACharacterControllerFPS>(owner);

	if (mCharacter)
		mSkeletalMesh = mCharacter->GetMesh1P();
	if (mSkeletalMesh)
		mIdBone = mSkeletalMesh->GetBoneIndex("hand_l");

	world = GetWorld();
	CanFire = true;
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

void UFistComponent::GoToDestination()
{
	if (!currentProjectile && CanFire)
	{
		if (world && mSkeletalMesh)
		{
			mSkeletalMesh->HideBone(mIdBone, EPhysBodyOp::PBO_None);

			currentProjectile = world->SpawnActor<AFistProjectile>(fistProjectileClass, mSkeletalMesh->GetBoneTransform(mIdBone));
			if (currentProjectile)
			{
				FHitResult hitResult;
				FCollisionQueryParams collisionQueryParems;
				//check if there's a wall to adjust the accuracy
				FVector end = mCharacter->GetFirstPersonCameraComponent()->GetComponentLocation() + mCharacter->GetFirstPersonCameraComponent()->GetForwardVector() * maxDistance;
				bool hit = world->LineTraceSingleByChannel(hitResult, mCharacter->GetFirstPersonCameraComponent()->GetComponentLocation(), end,
					ECC_Visibility, collisionQueryParems);

				FVector direction;
				direction = hit ? (hitResult.ImpactPoint - GetHandPosition()) : (end - GetHandPosition());
				direction = direction.GetSafeNormal();
				currentProjectile->SetOwner(mCharacter);
				currentProjectile->LaunchFist(direction, true, maxDistance, speedMax, mCharacter);
				currentProjectile = nullptr;

				//launch timer to reset fire
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

void UFistComponent::ResetFire()
{
	//allow player to shoot again and update HUD
	CanFire = true;
	mCharacter->OnResetProjectile.Broadcast();
	//unhide bones
	if (mSkeletalMesh)
	{
		mSkeletalMesh->UnHideBone(mIdBone);
		mSkeletalMesh->bRequiredBonesUpToDate = false;
	}
}

void UFistComponent::CheckElementInteractableHitable()
{
	FHitResult hitResult;
	FCollisionQueryParams collisionQueryParems;
	//check if there's a wall to adjust the accuracy
	FVector end = mCharacter->GetFirstPersonCameraComponent()->GetComponentLocation() + mCharacter->GetFirstPersonCameraComponent()->GetForwardVector() * maxDistance;
	bool hit = world->LineTraceSingleByChannel(hitResult, mCharacter->GetFirstPersonCameraComponent()->GetComponentLocation(), end,
		ECC_Visibility, collisionQueryParems);
	if (hit)
	{
		if (IInteractable* interactable = Cast<IInteractable>(hitResult.GetActor()))
		{
			if (interactable->CanBeActivatedByFist)
				OnInteractableCouldBeHit.Broadcast();
			else
				OnNothingInteractableCouldBeHit.Broadcast();
		}
		else if (IActivable* activable = Cast<IActivable>(hitResult.GetActor()))
		{
			if (activable->CanBeActivatedByFist)
				OnInteractableCouldBeHit.Broadcast();
			else
				OnNothingInteractableCouldBeHit.Broadcast();
		}
		else
		{
			OnNothingInteractableCouldBeHit.Broadcast();
		}
	}
	else
	{
		OnNothingInteractableCouldBeHit.Broadcast();
	}
}

// Called every frame
void UFistComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	//change hud color.. to do..
	CheckElementInteractableHitable();
}

