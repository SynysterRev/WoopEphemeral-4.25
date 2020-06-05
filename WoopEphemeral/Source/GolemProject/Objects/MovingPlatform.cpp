// Fill out your copyright notice in the Description page of Project Settings.


#include "MovingPlatform.h"
#include "Helpers/HelperLibrary.h"
#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "Player/FistProjectile.h"
#include "TimerManager.h"
#include "Components/MeshComponent.h"

// Sets default values
AMovingPlatform::AMovingPlatform(const FObjectInitializer& OI)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	pathParent = CreateDefaultSubobject<USceneComponent>(TEXT("pathParen"));
	path1 = CreateDefaultSubobject<USceneComponent>(TEXT("Path1"));
	if (Root != nullptr && pathParent != nullptr)
	{
		pathParent->SetupAttachment(Root);
	}
	if (pathParent != nullptr && path1 != nullptr)
	{
		path1->SetupAttachment(pathParent);
		pathInfos.Add({ 100.f, 0.f, EMovingType::Line });
	}

}

void AMovingPlatform::BeginPlay()
{
	Super::BeginPlay();
	Init();
}

void AMovingPlatform::Init()
{
	if (pathParent != nullptr)
	{
		pathParent->GetChildrenComponents(false, childrens);
	}
	if (childrens.Num() == 0)
	{
		HelperLibrary::Print("WARNING There is no path on " + GetName(), 15.f, FColor::Yellow);
	}
	for (auto& path : childrens)
	{
		if (path != nullptr)
		{
			worldCheckpoint.Add(path->GetComponentLocation());
		}
	}

	if (pathInfos.Num() < worldCheckpoint.Num())
	{
		HelperLibrary::Print("ERROR pathInfos is not initialized for all path points on " + GetName(), 15.f, FColor::Red);
		return;
	}
	if (pathInfos.Num() > 0)
	{
		waitTime = pathInfos[0].waitTime;
	}
	else
	{
		waitTime = 0.f;
	}
	if (startIndexCheckpoint >= 0 && startIndexCheckpoint < worldCheckpoint.Num())
	{
		currentIndex = startIndexCheckpoint;
	}
	else
	{
		currentIndex = 0;
	}
	if (direction == EMovingDirection::Backward)
	{
		nextIndex = currentIndex - 1;
	}
	else
	{
		nextIndex = currentIndex + 1;
	}
	if (alwaysActive)
	{
		isActivate = true;
		if (isStair)
		{
			platformType = EMovingPlatformType::PingPong;
		}
	}
	if (nextIndex < 0)
	{
		nextIndex = worldCheckpoint.Num() - 1;
	}
	else if (nextIndex >= worldCheckpoint.Num())
	{
		nextIndex = 0;
	}
	velocity = FVector::ZeroVector;
	if (worldCheckpoint.IsValidIndex(currentIndex))
	{
		SetActorLocation(worldCheckpoint[currentIndex]);
	}
	isPause = false;
	HasBeenActivated = true;
	meshComponent = FindComponentByClass<UMeshComponent>();
	if (meshComponent && activatedByHand)
	{
		HasBeenActivated = false;
		meshComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &AMovingPlatform::OverlapActivation);
		meshComponent->OnComponentHit.AddUniqueDynamic(this, &AMovingPlatform::HitActivation);
		SetResponseToPawn(false);
		meshComponent->SetScalarParameterValueOnMaterials("Opacity", opacityMin);
		actualOpacity = opacityMin;
		isCollidingWithPlayer = false;
	}
	else
	{
		isCollidingWithPlayer = true;
	}

}

void AMovingPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (meshComponent && activatedByHand)
	{
		if (isCollidingWithPlayer)
		{
			actualOpacity = FMath::Lerp(actualOpacity, 1.f, 0.1f);
		}
		else
		{
			actualOpacity = FMath::Lerp(actualOpacity, opacityMin, 0.1f);
		}
		meshComponent->SetScalarParameterValueOnMaterials("Opacity", actualOpacity);
	}
	if (worldCheckpoint.Num() <= 1 || !isActivate || direction == EMovingDirection::None || isPause)
	{
		return;
	}
	if (waitTime > 0)
	{
		waitTime -= DeltaTime;
		return;
	}

	if (pathInfos.Num() < worldCheckpoint.Num())
	{
		return;
	}

	int refIndex = direction == EMovingDirection::Forward ? currentIndex : nextIndex;

	if (pathInfos.IsValidIndex(refIndex))
	{
		if (pathInfos[refIndex].movingType == EMovingType::Line)
		{
			MoveLine(DeltaTime);
		}
		else if (pathInfos[refIndex].movingType == EMovingType::Curve)
		{
			MoveCurve(DeltaTime, refIndex);
		}
	}
	else
	{
		MoveLine(DeltaTime);
	}
}

void AMovingPlatform::MoveLine(float dt)
{
	float distanceToGo = (direction == EMovingDirection::Forward ? pathInfos[currentIndex].speed : pathInfos[nextIndex].speed) * dt;/* *
		(direction == EMovingDirection::Forward ? speedCurve[currentIndex].Evaluate(percentageTraveledDistance) : speedCurve[nextIndex].Evaluate(percentageTraveledDistance));*/
	while (distanceToGo > 0 && waitTime <= 0.f)
	{
		FVector directionToNextCheckpoint = worldCheckpoint[nextIndex] - GetActorLocation();
		float dist = distanceToGo;
		if (directionToNextCheckpoint.SizeSquared() < dist * dist)
		{
			dist = directionToNextCheckpoint.Size();
			SetActorLocation(worldCheckpoint[nextIndex]);
			SetNextIndex();
			if (!isActivate)
			{
				distanceToGo = 0.f;
			}
		}
		else
		{
			velocity = directionToNextCheckpoint.GetSafeNormal() * dist;
			SetActorLocation(GetActorLocation() + velocity);
		}
		distanceToGo -= dist;
	}
}

void AMovingPlatform::MoveCurve(float dt, int refIndex)
{
	float distanceToGo = (direction == EMovingDirection::Forward ? pathInfos[currentIndex].speed : pathInfos[nextIndex].speed) * dt; /**
		(direction == EMovingDirection::Forward ? speedCurve[currentIndex].Evaluate(timerLerp) : speedCurve[nextIndex].Evaluate(1.f - timerLerp));*/
	while (distanceToGo > 0.f && waitTime <= 0.f)
	{
		if (USplineComponent* spline = Cast<USplineComponent>(childrens[refIndex]))
		{
			FVector nextPos = worldCheckpoint[refIndex] + spline->GetLocationAtTime(direction == EMovingDirection::Forward ? timerLerp : 1.f - timerLerp, ESplineCoordinateSpace::Local, true);

			FVector directionNextPos = nextPos - GetActorLocation();
			float dist = distanceToGo;
			if (directionNextPos.SizeSquared() < dist * dist)
			{
				dist = directionNextPos.Size();
				if (timerLerp >= 1.f)
				{
					SetNextIndex();
					timerLerp = 0.f;
				}
				else
				{
					timerLerp += 0.01f;
					if (timerLerp > 1.f)
					{
						timerLerp = 1.f;
					}
				}
			}
			velocity = directionNextPos.GetSafeNormal() * dist;
			SetActorLocation(GetActorLocation() + velocity);
			distanceToGo -= dist;
		}
		else
		{
			HelperLibrary::Print(FString::Printf(TEXT("WARNING path%d is not a spline on "), refIndex) + GetName(), 15.f, FColor::Red);
		}
	}
}

void AMovingPlatform::SetNextIndex()
{
	currentIndex = nextIndex;
	waitTime = pathInfos[currentIndex].waitTime;
	if (direction == EMovingDirection::Forward)
	{
		nextIndex += 1;
		if (nextIndex >= worldCheckpoint.Num())
		{ //we reach the end

			switch (platformType)
			{
			case EMovingPlatformType::PingPong:
				nextIndex = worldCheckpoint.Num() - 2;
				direction = EMovingDirection::Backward;
				break;
			case EMovingPlatformType::Loop:
				nextIndex = 0;
				break;
			case EMovingPlatformType::OnceLoop:
				nextIndex = 0;
				break;
			case EMovingPlatformType::Once:
				nextIndex = worldCheckpoint.Num() - 2;
				direction = EMovingDirection::Backward;
				if (addCycleOnActivate && remainingCycle > 0)
				{
					remainingCycle--;
				}
				else
				{
					isActivate = false;
				}
				break;
			}
		}
		else if (nextIndex == 1 && platformType == EMovingPlatformType::OnceLoop)
		{
			if (addCycleOnActivate && remainingCycle > 0)
			{
				remainingCycle--;
			}
			else
			{
				isActivate = false;
			}
		}
	}
	else if (direction == EMovingDirection::Backward)
	{
		nextIndex -= 1;
		if (nextIndex < 0)
		{ //reached the beginning

			switch (platformType)
			{
			case EMovingPlatformType::PingPong:
				nextIndex = 1;
				direction = EMovingDirection::Forward;
				break;
			case EMovingPlatformType::Loop:
				nextIndex = worldCheckpoint.Num() - 1;
				break;
			case EMovingPlatformType::OnceLoop:
				nextIndex = worldCheckpoint.Num() - 1;
				if (addCycleOnActivate && remainingCycle > 0)
				{
					remainingCycle--;
				}
				else
				{
					isActivate = false;
				}
				break;
			case EMovingPlatformType::Once:
				nextIndex = 1;
				direction = EMovingDirection::Forward;
				if (addCycleOnActivate && remainingCycle > 0)
				{
					remainingCycle--;
				}
				else
				{
					isActivate = false;
				}
				break;
			}
		}
	}
}

void AMovingPlatform::SetResponseToPawn(bool collideWith)
{
	if (meshComponent)
	{
		if (collideWith)
		{
			ActivateEvent();
			HasBeenActivated = true;
			isCollidingWithPlayer = true;
			meshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
		}
		else
		{
			isCollidingWithPlayer = false;
			meshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		}
	}
}

void AMovingPlatform::OverlapActivation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (activatedByHand && OtherActor)
	{
		if (AFistProjectile* fist = Cast<AFistProjectile>(OtherActor))
		{
			SetResponseToPawn(true);
			FTimerHandle ExecuteTimerHandle;
			GetWorldTimerManager().SetTimer(ExecuteTimerHandle, FTimerDelegate::CreateUObject(this, &AMovingPlatform::SetResponseToPawn, false), activateTime, false);
			OtherActor->Destroy();
		}
	}
}

void AMovingPlatform::HitActivation(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (activatedByHand && OtherActor)
	{
		if (AFistProjectile* fist = Cast<AFistProjectile>(OtherActor))
		{
			SetResponseToPawn(true);
			FTimerHandle ExecuteTimerHandle;
			GetWorldTimerManager().SetTimer(ExecuteTimerHandle, FTimerDelegate::CreateUObject(this, &AMovingPlatform::SetResponseToPawn, false), activateTime, false);
			OtherActor->Destroy();
		}
	}
}

const bool AMovingPlatform::Activate_Implementation(AActor* caller)
{
	if (isStair && direction != EMovingDirection::Forward)
	{
		direction = EMovingDirection::Forward;
		isActivate = true;
		SetNextIndex();
		return true;
	}
	if (isActivate)
	{
		if (addCycleOnActivate)
		{
			remainingCycle++;
		}
		return false;
	}
	isActivate = true;
	return true;
}

const bool AMovingPlatform::Desactivate_Implementation(AActor* caller)
{
	if (alwaysActive)
	{
		return false;
	}
	if (isStair && direction != EMovingDirection::Backward)
	{
		direction = EMovingDirection::Backward;
		SetNextIndex();
		isActivate = true;
		return true;
	}
	if (!isActivate)
	{
		return false;
	}
	else
	{
		isActivate = false;
	}
	return true;
}

const bool AMovingPlatform::Switch_Implementation(AActor* caller)
{
	if (isStair)
	{
		if (direction == EMovingDirection::Forward)
		{
			direction = EMovingDirection::Backward;
		}
		else if (direction == EMovingDirection::Backward)
		{
			direction = EMovingDirection::Forward;
		}
		SetNextIndex();
		isActivate = true;
	}
	else
	{
		isActivate = !isActivate;
	}
	if (alwaysActive)
	{
		isActivate = true;
	}
	return true;
}

void AMovingPlatform::SetSpawner_Implementation(AActor* _spawner)
{
	spawner = _spawner;
}
