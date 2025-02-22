// Golem Project - Créajeux 2020


#include "Bumper.h"
#include "Player/CharacterControllerFPS.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SceneComponent.h"
#include "Helpers/HelperLibrary.h"

// Sets default values
ABumper::ABumper()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>("Root");
	RootComponent = Root;
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	MeshComponent->SetupAttachment(Root);
	MeshBeam = CreateDefaultSubobject<UStaticMeshComponent>("Beam");
	MeshBeam->SetupAttachment(MeshComponent);
	TriggerBumper = CreateDefaultSubobject<UStaticMeshComponent>("TriggerBumper");
	TriggerBumper->SetupAttachment(MeshComponent);
}

// Called when the game starts or when spawned
void ABumper::BeginPlay()
{
	Super::BeginPlay();
	TriggerBumper->OnComponentBeginOverlap.AddDynamic(this, &ABumper::OnBeginOverlap);
}

void ABumper::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OverlappedComponent != nullptr && OtherActor != nullptr && OtherComp != nullptr)
	{
		if (ACharacterControllerFPS* character = Cast<ACharacterControllerFPS>(OtherActor))
		{
			FVector dirCharacter = character->GetVelocity();
			dirCharacter = dirCharacter.GetSafeNormal();
			FVector directionLaunch = MeshComponent->GetUpVector().GetSafeNormal();
			character->LaunchCharacter(directionLaunch * ForceLaunch, !KeepLastVelocity, !KeepZVelocity);
			//FVector 
			if (character->GetDashComponent())
			{
				character->IsDashingDown = false;
				character->GetDashComponent()->CancelDashDown();
				character->GetDashComponent()->ResetDashInAir();
			}
		}
	}
}

// Called every frame
void ABumper::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

