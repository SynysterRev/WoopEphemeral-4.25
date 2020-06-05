// Fill out your copyright notice in the Description page of Project Settings.


#include "PressurePlate.h"

#include "Helpers/HelperLibrary.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Math/UnrealMathVectorCommon.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

APressurePlate::APressurePlate()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	if (mesh)
	{
		SetRootComponent(mesh);
	}

	boxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	if (boxCollider && mesh)
	{
		boxCollider->SetupAttachment(mesh);
	}

}

void APressurePlate::BeginPlay()
{
	Super::BeginPlay();
	if (boxCollider)
	{
		boxCollider->OnComponentBeginOverlap.AddUniqueDynamic(this, &APressurePlate::OnBeginOverlap);
		boxCollider->OnComponentEndOverlap.AddUniqueDynamic(this, &APressurePlate::OnEndOverlap);
	}
	if (mesh)
	{
		startPos = mesh->GetComponentLocation();
		pressedPos = startPos;
		pressedPos.Z += offsetWhenPresed;
	}

}

void APressurePlate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (isMoving)
	{
		if (isPressed)
		{
			timerLerp += DeltaTime / timeToLerp;
			if (timerLerp > 1.f)
			{
				timerLerp = 1.f;
				isMoving = false;
			}
		}
		else
		{
			timerLerp -= DeltaTime / timeToLerp;
			if (timerLerp < 0.f)
			{
				timerLerp = 0.f;
				isMoving = false;
			}
		}
		if (mesh)
		{
			mesh->SetWorldLocation(FMath::Lerp(startPos, pressedPos, timerLerp));
		}
	}

}

void APressurePlate::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	countObjectOn++;
	if (countObjectOn == 1)
	{
		HelperLibrary::Print(TEXT("Press"), 2.0f, FColor::Blue);
		isPressed = true;
		isMoving = true;
		ActivateObjects(activationTypeOnPress);
		OnPressedPlate.Broadcast();
	}
}

void APressurePlate::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

	countObjectOn--;
	if (countObjectOn == 0)
	{
		HelperLibrary::Print(TEXT("Releasse"), 2.0f, FColor::Blue);
		isPressed = false;
		isMoving = true;
		ActivateObjects(activationTypeOnRelease);
		OnReleasedPlate.Broadcast();
	}
}

void APressurePlate::ActivateObjects(EActivationType activationType)
{
	for (auto& objectToActivate : objectsToActivate)
	{
		if (objectToActivate != nullptr)
		{
			if (IActivable* activable = Cast<IActivable>(objectToActivate))
			{
				if (activationType == EActivationType::Activate)
				{
					activable->Execute_Activate(objectToActivate, this);
				}
				else if (activationType == EActivationType::Desactivate)
				{
					activable->Execute_Desactivate(objectToActivate, this);
				}
				else if (activationType == EActivationType::Switch)
				{
					activable->Execute_Switch(objectToActivate, this);
				}
			}
			else
			{
				HelperLibrary::Print(FString::Printf(TEXT("%s don\'t implement Activable interface"), *(objectToActivate->GetName())), 5.f, FColor::Yellow);
			}

		}
	}
}