// Golem Project - Créajeux 2020


#include "SlowMoComponent.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
// Sets default values for this component's properties
USlowMoComponent::USlowMoComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

// Called when the game starts
void USlowMoComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

}

void USlowMoComponent::SetEnableSlowMo()
{
	if (!bEnable)
	{
		bEnable = true;

		if (UWorld* world = GetWorld())
		{
			world->GetWorldSettings()->SetTimeDilation(slowTime);
			world->GetTimerManager().SetTimer(handleTimer, this, &USlowMoComponent::SetDisableSlowMo, coolDown * slowTime, false);
		}
	}
}


void USlowMoComponent::SetDisableSlowMo()
{
	bEnable = false;
	if (UWorld* world = GetWorld())
	{
		world->GetWorldSettings()->SetTimeDilation(1.0f);
	}
}
