// Golem Project - Créajeux 2020


#include "PendulumTrap.h"
#include "Components/StaticMeshComponent.h"
#include "Helpers/HelperLibrary.h"
#include "Player/CharacterControllerFPS.h"



APendulumTrap::APendulumTrap()
{
	root = CreateDefaultSubobject<USceneComponent>("Root");
	SetRootComponent(root);
	pivot = CreateDefaultSubobject<USceneComponent>("Pivot");
	if (pivot && root)
	{
		pivot->SetupAttachment(root);
		handle = CreateDefaultSubobject<UStaticMeshComponent>("Handle");
		if (handle)
		{
			handle->SetupAttachment(pivot);
		}
		blade = CreateDefaultSubobject<UStaticMeshComponent>("Blade");
		if (blade)
		{
			blade->SetupAttachment(pivot);
		}
	}
	PrimaryActorTick.bCanEverTick = true;
}

void APendulumTrap::BeginPlay()
{
	Super::BeginPlay();
	if (blade)
	{
		blade->OnComponentBeginOverlap.AddUniqueDynamic(this, &APendulumTrap::BeginOverlap);
		lastBladePosition = blade->GetComponentLocation();
	}
	if (isActivate && nbCycleBeforeStop > 0)
	{
		actualCycle = nbCycleBeforeStop;
	}
}

void APendulumTrap::Tick(float dt)
{
	Super::Tick(dt);
	if (isActivate && blade)
	{
		if (time == 0.f || time == PI)
		{
			OnStartCycle.Broadcast();
		}
		bladeVelocity = (blade->GetComponentLocation() - lastBladePosition) / dt;
		lastBladePosition = blade->GetComponentLocation();
		lastTime = time;
		time += dt * (2.f - balanceFactor) / timePerCycle * 2.f;

		if (time >= PI && lastTime < PI)
		{
			actualCycle--;
			time = PI;
			if (actualCycle == 0)
			{
				Desactivate(nullptr);
			}
		}
		if (time > 2.f * PI)
		{
			actualCycle--;
			time = 0.f;
			if (actualCycle == 0)
			{
				Desactivate(nullptr);
			}
		}

		float speedSin = FMath::Sin(time - PI / 2.f);
		//value between 1 and 1.5 depend of where is the pendulum	
		balanceFactor = 1.f + (1.f - fabs(speedSin)) / 2.f;
		if (pivot)
		{
			pivot->SetRelativeRotation(FRotator::MakeFromEuler(FVector::ForwardVector * speedSin * limitAngle * balanceFactor));
		}

	}
}

void APendulumTrap::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OverlappedComponent != nullptr && OtherActor != nullptr && OtherComp != nullptr && isActivate)
	{
		if (ACharacterControllerFPS* character = Cast<ACharacterControllerFPS>(OtherActor))
		{
			FVector temp = bladeVelocity;
			if (temp.Z < 0.f)
			{
				//temp.Z = 0.f;
			}
			character->InflictDamage(damage);
			//character->LaunchCharacter(temp * ejectionMultiplier, false, false);
		}
	}
}

const bool APendulumTrap::Activate_Implementation(AActor* caller)
{
	bool temp = Super::Activate_Implementation(caller);
	if (temp && isActivate && nbCycleBeforeStop > 0)
	{
		actualCycle = nbCycleBeforeStop;
	}
	return temp;
}

const bool APendulumTrap::Switch_Implementation(AActor* caller)
{
	bool temp = Super::Switch_Implementation(caller);
	if (temp && isActivate && nbCycleBeforeStop > 0)
	{
		actualCycle = nbCycleBeforeStop;
	}
	return temp;
}