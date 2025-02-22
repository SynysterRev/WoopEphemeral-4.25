// Golem Project - Créajeux 2020


#include "BumperAir.h"
#include "Player/CharacterControllerFPS.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SceneComponent.h"

// Sets default values
ABumperAir::ABumperAir()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>("Root");
	RootComponent = Root;
	MeshBeam = CreateDefaultSubobject<UStaticMeshComponent>("Beam");
	MeshBeam->SetupAttachment(Root);
	SphereTrigger = CreateDefaultSubobject<UStaticMeshComponent>("TriggerActivation");
	SphereTrigger->SetupAttachment(MeshBeam);
}

// Called when the game starts or when spawned
void ABumperAir::BeginPlay()
{
	Super::BeginPlay();
	SphereTrigger->OnComponentBeginOverlap.AddDynamic(this, &ABumperAir::OnBeginOverlap);
}

// Called every frame
void ABumperAir::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABumperAir::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OverlappedComponent != nullptr && OtherActor != nullptr && OtherComp != nullptr)
	{
		if (ACharacterControllerFPS* character = Cast<ACharacterControllerFPS>(OtherActor))
		{
			FVector directionCharacter = character->GetVelocity().GetSafeNormal();
			float dot = FVector::DotProduct(directionCharacter, GetActorUpVector());
			FVector directionLaunch = dot >= 0.0f ? GetActorUpVector() : -GetActorUpVector();
			character->LaunchCharacter(directionLaunch * ForceLaunch, false, true);
		}
	}
}

