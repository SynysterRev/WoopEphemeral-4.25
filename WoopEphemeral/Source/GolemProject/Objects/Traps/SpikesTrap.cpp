// Golem Project - Créajeux 2020


#include "SpikesTrap.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Player/CharacterControllerFPS.h"
#include "Helpers/HelperLibrary.h"

ASpikesTrap::ASpikesTrap()
{
	RootComp = CreateDefaultSubobject<USceneComponent>("RootComp");
	RootComponent = RootComp;
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	if (StaticMesh)
	{
		StaticMesh->SetupAttachment(RootComp);
	}
	StaticMeshSpikes = CreateDefaultSubobject<UStaticMeshComponent>("Spike");
	if (StaticMeshSpikes)
	{
		StaticMeshSpikes->SetupAttachment(RootComp);
	}
	BoxTrigger = CreateDefaultSubobject<UBoxComponent>("TriggerActivation");
	if (BoxTrigger)
	{
		BoxTrigger->SetupAttachment(RootComp);
	}
}

void ASpikesTrap::BeginPlay()
{
	Super::BeginPlay();
	
	if (StaticMeshSpikes)
	{
		StaticMeshSpikes->OnComponentBeginOverlap.AddUniqueDynamic(this, &ASpikesTrap::OverlapDamage);
		EndLocation = StaticMeshSpikes->GetComponentLocation() + GetActorUpVector() * offset;
		StartingLocation = StaticMeshSpikes->GetComponentLocation();
	}
	if (BoxTrigger)
	{
		BoxTrigger->OnComponentBeginOverlap.AddUniqueDynamic(this, &ASpikesTrap::OverlapActivation);
		BoxTrigger->OnComponentEndOverlap.AddUniqueDynamic(this, &ASpikesTrap::EndOverlapActivation);
		BoxTrigger->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	}
	if (alwaysActive)
	{
		Activate(nullptr);
	}
	if (AlwaysOut)
	{
		StaticMeshSpikes->SetWorldLocation(EndLocation);
	}

	TimeBetweenActivation += Interval;
}

void ASpikesTrap::Tick(float _deltaTime)
{
	Super::Tick(_deltaTime);
	if (AlwaysOut)
		return;
	if (ActivationAuto)
	{
		if (!RetractSpike)
		{
			TimerBeforeTrigger += _deltaTime;

			if (TimerBeforeTrigger >= TimeBetweenActivation)
			{
				TimerSpikes += _deltaTime * Speed;
				if (StaticMeshSpikes)
				{
					StaticMeshSpikes->SetWorldLocation(FMath::Lerp(StartingLocation, EndLocation, TimerSpikes));
				}
				if (TimerSpikes >= 1.0f)
				{
					TimerBeforeTrigger = 0.0f;
					TimerSpikes = 0.0f;
					RetractSpike = true;
				}
			}
		}
		else if (RetractSpike)
		{
			TimerBeforeTrigger += _deltaTime;

			if (TimerBeforeTrigger >= TimeSpikesOut)
			{
				TimerSpikes += _deltaTime * Speed;
				if (StaticMeshSpikes)
				{
					StaticMeshSpikes->SetWorldLocation(FMath::Lerp(EndLocation, StartingLocation, TimerSpikes));
				}
				if (TimerSpikes >= 1.0f)
				{
					TimerBeforeTrigger = 0.0f;
					TimerSpikes = 0.0f;
					RetractSpike = false;
				}
			}
		}
	}
	else
	{

		if (IsTriggered && !RetractSpike)
		{
			TimerBeforeTrigger += _deltaTime;

			if (TimerBeforeTrigger >= TimerTrigger)
			{
				TimerSpikes += _deltaTime * Speed;
				if (StaticMeshSpikes)
				{
					StaticMeshSpikes->SetWorldLocation(FMath::Lerp(StartingLocation, EndLocation, TimerSpikes));
				}
				if (TimerSpikes >= 1.0f)
				{
					IsTriggered = false;
					TimerBeforeTrigger = 0.0f;
					TimerSpikes = 0.0f;
					RetractSpike = true;
				}
			}
		}
		else if (RetractSpike)
		{
			TimerBeforeTrigger += _deltaTime;

			if (TimerBeforeTrigger >= TimerTrigger)
			{
				TimerSpikes += _deltaTime * Speed;
				if (StaticMeshSpikes)
				{
					StaticMeshSpikes->SetWorldLocation(FMath::Lerp(EndLocation, StartingLocation, TimerSpikes));
				}
				if (TimerSpikes >= 1.0f)
				{
					TimerBeforeTrigger = 0.0f;
					TimerSpikes = 0.0f;
					RetractSpike = false;
					if (IsPlayerStillHere)
						IsTriggered = true;
				}
			}
		}
	}
}

void ASpikesTrap::OverlapDamage(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OverlappedComponent != nullptr && OtherActor != nullptr && OtherComp != nullptr && (isActivate || ActivationAuto))
	{
		if (ACharacterControllerFPS* character = Cast<ACharacterControllerFPS>(OtherActor))
		{
			character->InflictDamage(damage);
		}
	}
}

void ASpikesTrap::OverlapActivation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OverlappedComponent != nullptr && OtherActor != nullptr && OtherComp != nullptr && isActivate)
	{
		if (ACharacterControllerFPS* character = Cast<ACharacterControllerFPS>(OtherActor))
		{
			IsTriggered = true;
			IsPlayerStillHere = true;
		}
	}
}

void ASpikesTrap::EndOverlapActivation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OverlappedComponent != nullptr && OtherActor != nullptr && OtherComp != nullptr && isActivate)
	{
		if (ACharacterControllerFPS* character = Cast<ACharacterControllerFPS>(OtherActor))
		{
			IsPlayerStillHere = false;
		}
	}
}
