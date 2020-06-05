// Fill out your copyright notice in the Description page of Project Settings.


#include "PushableBloc.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "GolemProjectCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "GolemProjectCharacter.h"
#include "Helpers/HelperLibrary.h"
#include "GameFramework/WorldSettings.h"


APushableBloc::APushableBloc()
{
	mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(mesh);

	if (mesh != nullptr)
	{
		mesh->SetSimulatePhysics(false);
	}

}

void APushableBloc::BeginPlay()
{
	Super::BeginPlay();
	CanBeActivatedByFist = false;
}

void APushableBloc::Tick(float _deltaTime)
{
	Super::Tick(_deltaTime);
	if (useGravity)
	{
		if (AWorldSettings* worldSeting = GetWorldSettings())
		{
			FVector newPosition = GetActorLocation() + FVector::UpVector * worldSeting->GetGravityZ() * _deltaTime * 0.5f;
			FHitResult hit;
			float lastZ = GetActorLocation().Z;
			if (SetActorLocation(newPosition, true, &hit) && lastZ - GetActorLocation().Z > 1.f)
			{
				if (isUsed && playerActor)
				{
					SetActorLocation(newPosition + pushingDirection, true);
					playerActor->StopPushBloc();
					isUsed = false;
				}
			}
			else if (!isUsed)
			{
				if (hit.GetActor() == playerActor)
				{
					SetActorLocation(newPosition);
				}
				else
				{
					useGravity = false;
				}
			}
		}
	}
}

const bool APushableBloc::Interact_Implementation(AActor* caller)
{
	if (pushingDirection.IsNearlyZero(0.01f) && pushingPosition.IsNearlyZero(0.01f) && pushingRotation.IsNearlyZero(0.01f) && !isUsed)
	{
		return false;
	}
	playerActor = Cast<AGolemProjectCharacter>(caller);
	if (playerActor == nullptr)
	{
		return false;
	}
	else
	{
		isUsed = !isUsed;
		if (isUsed)
		{
			if (playerActor->PushBloc(pushingDirection, pushingPosition, pushingRotation))
			{
				playerActor->SetRightHandPosition(pushingRightHandPosition);
				playerActor->SetLeftHandPosition(pushingLeftHandPosition);
				useGravity = true;
			}
			else
			{
				isUsed = false;
			}
		}
		else
		{
			playerActor->StopPushBloc();
			playerActor = nullptr;
		}
		return true;
	}

	return false;
}