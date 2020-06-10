// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "CharacterControllerFPS.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Helpers/HelperLibrary.h"
#include "GameFramework/PlayerController.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	if (AActor* owner = GetOwner())
	{
		Player = Cast<ACharacterControllerFPS>(owner);
		if (Player)
		{
			PlayerController = Cast<APlayerController>(Player->GetController());
		}
	}
	Life = MaxLife;
	CanTakeDamage = true;
	if (Player != nullptr)
	{
		PositionCheckPoint = Player->GetActorLocation();
	}
	bIsDead = false;
}


void UHealthComponent::InflictDamage(int _damage)
{
	if (CanTakeDamage)
	{
		Life -= _damage;
		CanTakeDamage = false;
		if (Life <= 0)
		{
			bIsDead = true;
			Life = 0;
			if (UWorld* world = GetWorld())
			{
				world->GetTimerManager().SetTimer(TimerHandlerRespawn, this, &UHealthComponent::Respawn, TimerRespawn, false);
			}
			if (PlayerController != nullptr && Player != nullptr)
			{
				Player->ActivateDeath();
				Player->DisableInput(PlayerController);
				Player->Event_Death();
			}
		}
	}
}

void UHealthComponent::Respawn()
{
	OnCharacterDie.Broadcast();
	Life = MaxLife;
	CanTakeDamage = true;
	if (PlayerController != nullptr && Player != nullptr)
	{
		Player->EnableInput(PlayerController);
		Player->Destroy();
	}
}

// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Player != nullptr)
	{
		//if player fall kill him
		if (Player->GetActorLocation().Z <= -1000.0f)
		{
			InflictDamage(5);
		}
	}
}

void UHealthComponent::SetPositionCheckPoint(FVector _positionCheckPoint)
{
	PositionCheckPoint = _positionCheckPoint;
}

void UHealthComponent::SetLife(int _Life)
{
	Life = _Life;
}
