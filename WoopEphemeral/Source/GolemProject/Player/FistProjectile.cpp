// Fill out your copyright notice in the Description page of Project Settings.


#include "FistProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "Helpers/HelperLibrary.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Interfaces/Interactable.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
AFistProjectile::AFistProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AFistProjectile::BeginPlay()
{
	Super::BeginPlay();
	MeshComponent = FindComponentByClass <UStaticMeshComponent>();
	if (MeshComponent)
	{
		MeshComponent->OnComponentHit.AddDynamic(this, &AFistProjectile::OnHit);
	}
	ProjectileComponent = FindComponentByClass<UProjectileMovementComponent>();
	if (ProjectileComponent)
	{
		ProjectileComponent->MaxSpeed = Speed;
		ProjectileComponent->InitialSpeed = 0.0f;
		ProjectileComponent->UpdatedComponent = MeshComponent;
	}
}

void AFistProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	ProjectileComponent->bShouldBounce = false;
	if (HitComponent != nullptr && OtherActor != nullptr && OtherComponent != nullptr)
	{
		if (IInteractable* interactable = Cast<IInteractable>(OtherActor))
		{
			interactable->Execute_Interact(OtherActor, this);
		}
		else
		{
			UPhysicalMaterial* physMat;
			if (Hit.GetComponent() && Hit.GetComponent()->GetMaterial(0) != nullptr)
			{
				physMat = Hit.GetComponent()->GetMaterial(0)->GetPhysicalMaterial();
				if (physMat && physMat->SurfaceType == SurfaceType2)
				{
					if (ProjectileComponent)
						ProjectileComponent->bShouldBounce = true;
					BounceMovement(Hit.ImpactNormal);
				}
				else
				{
					if (ProjectileComponent)
						ProjectileComponent->bShouldBounce = false;
				}
			}
		}

	}
	bHasStopped = true;
	//ActivateFX();
	//world->GetTimerManager().SetTimer(TimerHandleFXDisappear, this, &AFistProjectile::ActivateFX, TimerDisappear - 1.0f, false);
}

// Called every frame
void AFistProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	/*if (distanceTravelled < maxDistance)
	{
		distanceTravelled += FVector::Dist(lastPosition, MeshComponent->GetComponentLocation());
	}
	else
	{
		DestroyFist();
	}
	lastPosition = MeshComponent->GetComponentLocation();*/
}

void AFistProjectile::LaunchFist(const FVector& _direction, bool _shouldBounce, float _maxDistance, float _maxSpeed, AGolemProjectCharacter* _character)
{
	Direction = _direction;
	if (ProjectileComponent)
	{
		ProjectileComponent->Velocity = Direction * Speed;
		lastPosition = MeshComponent->GetComponentLocation();
		maxDistance = _maxDistance;
		character = _character;
		Speed = _maxSpeed;
		SetLifeSpan(_maxDistance / Speed);
	}
}

void AFistProjectile::ActivateFX()
{
	Event_DestructionFistFX_BP();
}

const float AFistProjectile::GetRemainingTimeBeforeDestroy()
{
	float time = (bHasStopped) ? UKismetSystemLibrary::K2_GetTimerRemainingTimeHandle(this, TimerHandleDisappear) : 1.0f;
	return time;
}

void AFistProjectile::DestroyFist()
{
	Destroy();
}

void AFistProjectile::BounceMovement(FVector _normal)
{
	if (ProjectileComponent != nullptr)
	{
		FVector direction = ProjectileComponent->Velocity.GetSafeNormal();
		FVector newDirection = direction.MirrorByVector(_normal).GetSafeNormal();
		ProjectileComponent->Velocity = newDirection * Speed;
	}
}
