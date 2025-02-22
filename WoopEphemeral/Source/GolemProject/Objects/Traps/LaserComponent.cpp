// Golem Project - Créajeux 2020


#include "LaserComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Player/CharacterControllerFPS.h"
#include "Helpers/HelperLibrary.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
ULaserComponent::ULaserComponent()
{
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	LazerMesh = CreateDefaultSubobject<UStaticMeshComponent>("LazerMesh");
	PrimaryComponentTick.bCanEverTick = true;
	// ...
}


// Called when the game starts
void ULaserComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UWorld* world = GetWorld())
	{
		FHitResult hitResult;
		FVector locationStart = LazerMesh->GetComponentLocation();
		FVector locationEnd = LazerMesh->GetUpVector() * MaxDistance;
		DrawDebugLine(GetWorld(), locationStart, locationEnd, FColor::Red, true);
		
		if (world->LineTraceSingleByChannel(hitResult, locationStart, locationEnd, ECollisionChannel::ECC_Visibility))
		{
			FVector scale(LazerMesh->GetComponentScale());
			scale.Z = FVector::Dist(hitResult.ImpactPoint, locationStart) / 100.0f;
			LazerMesh->SetWorldScale3D(scale);
		}

		LazerMesh->OnComponentBeginOverlap.AddDynamic(this, &ULaserComponent::OverlapDamage);
	}

}


// Called every frame
void ULaserComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void ULaserComponent::OverlapDamage(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OverlappedComponent != nullptr && OtherActor != nullptr && OtherComp != nullptr)
	{
		if (ACharacterControllerFPS* character = Cast<ACharacterControllerFPS>(OtherActor))
		{
			character->InflictDamage(1);
		}
	}
}