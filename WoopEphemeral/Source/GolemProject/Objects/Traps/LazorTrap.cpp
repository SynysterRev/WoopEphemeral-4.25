// Golem Project - Créajeux 2020


#include "LazorTrap.h"
#include "Components/StaticMeshComponent.h"
#include "Player/CharacterControllerFPS.h"
#include "Helpers/HelperLibrary.h"

ALazorTrap::ALazorTrap()
{
	RootComp = CreateDefaultSubobject<USceneComponent>("RootComp");
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	RootComponent = RootComp;
	StaticMesh->SetupAttachment(RootComponent);
	LazerMesh = CreateDefaultSubobject<UStaticMeshComponent>("LazerMesh");
	LazerMesh->SetupAttachment(RootComponent);
}

void ALazorTrap::BeginPlay()
{
	if (UWorld* world = GetWorld())
	{
		FHitResult hitResult;
		FVector locationStart = GetActorLocation();
		FVector locationEnd = GetActorForwardVector() * MaxDistance;
		if (world->LineTraceSingleByChannel(hitResult, locationStart, locationEnd, ECollisionChannel::ECC_Visibility))
		{
			FVector scale(LazerMesh->GetComponentScale());
			scale.Z = FVector::Dist(hitResult.ImpactPoint, locationStart) / 100.0f;
			LazerMesh->SetWorldScale3D(scale);
		}
	}
	LazerMesh->OnComponentBeginOverlap.AddDynamic(this, &ALazorTrap::OverlapDamage);
}

void ALazorTrap::Tick(float _deltaTime)
{

}

void ALazorTrap::OverlapDamage(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OverlappedComponent != nullptr && OtherActor != nullptr && OtherComp != nullptr)
	{
		if (ACharacterControllerFPS* character = Cast<ACharacterControllerFPS>(OtherActor))
		{
			character->InflictDamage(1);
		}
	}
}
