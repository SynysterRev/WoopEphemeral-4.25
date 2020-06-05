// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "GolemProjectCharacter.h"
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
		Player = Cast<AGolemProjectCharacter>(owner);
		if (Player)
		{
			PlayerController = Cast<APlayerController>(Player->GetController());
		}
	}
	CanTakeDamage = true;
	Life = MaxLife;
	if (Player != nullptr)
	{
		LastPositionGrounded = Player->GetActorLocation();
		PositionCheckPoint = Player->GetActorLocation();
	}
	IsFallingDown = false;
	bIsDead = false;
}


void UHealthComponent::InflictDamage(int _damage)
{
	if (CanTakeDamage)
	{
		Life -= _damage;
		CanTakeDamage = false;
		HelperLibrary::Print(FString::FromInt(Life));
		if (Life <= 0)
		{
			OnCharacterDie.Broadcast();
			bIsDead = true;
			Life = 0;
			if (UWorld* world = GetWorld())
			{
				world->GetTimerManager().SetTimer(TimerHandlerRespawn, this, &UHealthComponent::Respawn, TimerRespawn, false);
			}
			if (PlayerController != nullptr && Player != nullptr)
			{
				Player->ActivateDeath(true);
				Player->DisableInput(PlayerController);
				Player->Event_Death();
			}
		}
		else
		{
			if (UWorld* world = GetWorld())
			{
				world->GetTimerManager().SetTimer(TimerHandlerInvul, this, &UHealthComponent::ResetInvulnerability, TimerInvulnerability, false);
			}
		}
	}
}

void UHealthComponent::ResetInvulnerability()
{
	CanTakeDamage = true;
}

void UHealthComponent::Respawn()
{
	Life = MaxLife;
	CanTakeDamage = true;
	if (PlayerController != nullptr && Player != nullptr)
	{
		Player->EnableInput(PlayerController);
		Player->Destroy();
		//Player->SetActorLocation(PositionCheckPoint);
		////Player->ActivateDeath(false);
		//IsFallingDown = false;
		//bIsDead = false;
		////Player->ResetMeshOnRightPlace();
	}
}

void UHealthComponent::RespawnFromFalling()
{
	CanTakeDamage = true;
	if (PlayerController != nullptr && Player != nullptr)
	{
		Player->EnableInput(PlayerController);
		IsFallingDown = false;
		Player->SetActorLocation(PositionCheckPoint);
	}
}

void UHealthComponent::KillCharacterFromFalling()
{
	IsFallingDown = true;
	InflictDamage(FallDamage);
	if (bIsDead) return;

	RespawnFromFalling();
	/*if (UWorld* world = GetWorld())
	{
		world->GetTimerManager().SetTimer(TimerHandlerRespawn, this, &UHealthComponent::RespawnFromFalling, 1.f, false);
	}*/
}

// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Player != nullptr && !IsFallingDown)
	{
		if (Player->GetActorLocation().Z <= -1000.0f)
		{
			KillCharacterFromFalling();
		}
	}
}

void UHealthComponent::SetLastPositionGrounded(FVector _lastPositionGrounded)
{
	LastPositionGrounded = _lastPositionGrounded;
}

void UHealthComponent::SetPositionCheckPoint(FVector _positionCheckPoint)
{
	PositionCheckPoint = _positionCheckPoint;
}
void UHealthComponent::SetLife(int _Life)
{
	Life = _Life;
}
