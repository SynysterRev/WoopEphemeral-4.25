// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileHand.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Player/GrappleComponent.h"
#include "Helpers/HelperLibrary.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "GolemProject/GolemProjectCharacter.h"

// Sets default values
AProjectileHand::AProjectileHand()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AProjectileHand::BeginPlay()
{
	Super::BeginPlay();

	meshComponent = FindComponentByClass<UStaticMeshComponent>();
	if (meshComponent)
	{
		meshComponent->OnComponentHit.AddDynamic(this, &AProjectileHand::OnHit);
	}
	ProjectileComponent = FindComponentByClass<UProjectileMovementComponent>();
	if (ProjectileComponent)
	{
		ProjectileComponent->MaxSpeed = MaxSpeed;
		ProjectileComponent->InitialSpeed = 0.0f;
		ProjectileComponent->UpdatedComponent = meshComponent;
	}

	bIsColliding = false;
	bIsComingBack = false;
}

void AProjectileHand::SetComingBack(const bool& _isComingBack)
{
	if (_isComingBack)
	{
		bIsColliding = false;
		if (meshComponent != nullptr)
		{
			meshComponent->SetCollisionProfileName(TEXT("NoCollision"));
		}

		if (AGolemProjectCharacter* character = Cast<AGolemProjectCharacter>(GetOwner()))
		{
			character->ComingBackEvent();
		}
	}

	bIsComingBack = _isComingBack;
}

void AProjectileHand::LaunchProjectile(const FVector& _direction, UGrappleComponent* _grapple)
{
	direction = _direction;
		grappleComponent = _grapple;
		if (ProjectileComponent)
		{
			ProjectileComponent->Velocity = direction * velocity;
		}
}

void AProjectileHand::DestroyProjectile()
{
	bIsColliding = false;
	bIsComingBack = false;
	Destroy();
}

// Called every frame
void AProjectileHand::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsComingBack)
	{
		if (ProjectileComponent->HasStoppedSimulation())
		{
			ProjectileComponent->SetUpdatedComponent(RootComponent);
		}

		if (grappleComponent && !grappleComponent->GetSwingPhysics() && meshComponent && ProjectileComponent)
		{
			FVector dir = grappleComponent->GetHandPosition() - meshComponent->GetComponentLocation();
			dir.Normalize();
			ProjectileComponent->Velocity = dir * velocity;
		}
	}
	else if (ProjectileComponent)
	{
		if (bIsColliding)
			ProjectileComponent->Velocity = FVector::ZeroVector;
	}
}

void AProjectileHand::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	//change direction projectile, maybe add timer when the projectile is reallly blocked
	/*if (bIsComingBack && OtherActor && OtherActor != this && ProjectileComponent)
	{
		FVector changeDir = OtherActor->GetActorLocation() - GetActorLocation();
		changeDir.Normalize();
		ProjectileComponent->Velocity = changeDir * velocity;
	}*/
	if (!bIsComingBack && HitComponent && OtherActor != this)
	{
		bIsColliding = true;
		UPhysicalMaterial* physMat;
		if (Hit.GetComponent() && Hit.GetComponent()->GetMaterial(0))
		{
			physMat = Hit.GetComponent()->GetMaterial(0)->GetPhysicalMaterial();
			if (physMat != nullptr)
			{
				if (!bIsAssisted && physMat->SurfaceType == SurfaceType1)
				{
					bIsGrapplingPossible = true;
					ImpactEvent();
					return;
				}
				else if (bIsAssisted && physMat->SurfaceType == SurfaceType3)
				{
					bIsSwingingPossible = true;
					ImpactEvent();
					return;
				}
			}
		}

		SetComingBack(true);
	}
}



