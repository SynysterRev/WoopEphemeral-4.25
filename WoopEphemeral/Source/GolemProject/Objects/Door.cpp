// Fill out your copyright notice in the Description page of Project Settings.


#include "Door.h"
#include "Math/UnrealMathVectorCommon.h"
#include "Classes/Components/SceneComponent.h"
#include "Helpers/HelperLibrary.h"
#include "UObject/UObjectGlobals.h"

// Sets default values
ADoor::ADoor(const FObjectInitializer& OI)
	: Super(OI)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	USceneComponent* Root = OI.CreateDefaultSubobject<USceneComponent>(this, TEXT("Root"));
	SetRootComponent(Root);
	destination = CreateDefaultSubobject<USceneComponent>(TEXT("DestinationDoor"));
	if (Root != nullptr && destination != nullptr)
	{
		destination->SetupAttachment(Root);
	}
}

// Called when the game starts or when spawned
void ADoor::BeginPlay()
{
	Super::BeginPlay();
	startPos = GetActorLocation();
	if (destination != nullptr)
	{
		destinationPos = destination->GetComponentLocation();
	}
}

// Called every frame
void ADoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (openBy == EOpenDoorBy::Movement)
	{
		if (!isDissolving)
		{
			float lastTimer = timerLerp;
			if (isActivate)
			{
				timerLerp += DeltaTime / timeToMove;
				if (timerLerp > 1.f)
				{
					timerLerp = 1.f;
				}
			}
			else
			{
				timerLerp -= DeltaTime / timeToMove;
				if (timerLerp < 0.f)
				{
					timerLerp = 0.f;
				}
			}
			if (!SetActorLocation(FMath::Lerp(startPos, destinationPos, timerLerp)))
			{
				HelperLibrary::Print(TEXT("cant move"), 5.f, FColor::Red);
				timerLerp = lastTimer;
			}
		}
	}
}

const bool ADoor::Activate_Implementation(AActor* caller)
{
	if (isActivate)
	{
		return false;
	}
	isActivate = true;
	return true;
}

const bool ADoor::Desactivate_Implementation(AActor* caller)
{
	if (!isActivate)
	{
		return false;
	}
	isActivate = false;
	return true;
}

const bool ADoor::Switch_Implementation(AActor* caller)
{
	isActivate = !isActivate;
	return true;
}

