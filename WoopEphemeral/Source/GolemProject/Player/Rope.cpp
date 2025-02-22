// Golem Project - Créajeux 2020


#include "Rope.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Player/GrappleComponent.h"
#include "Player/ProjectileHand.h"
#include "GolemProject/GolemProjectCharacter.h"
// Sets default values
ARope::ARope()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	sceneComp = CreateDefaultSubobject<USceneComponent>("SceneRoot");
	sceneComp->SetupAttachment(RootComponent);
	staticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>("Rope");
	staticMeshComp->SetupAttachment(sceneComp);
}

void ARope::HideMesh()
{
	staticMeshComp->SetHiddenInGame(true);
}

// Called when the game starts or when spawned
void ARope::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ARope::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (grappleComp && grappleComp->GetProjectile())
	{
		FVector rdhand = grappleComp->GetSpawningLocation();
		direction = grappleComp->GetProjectile()->GetLocation() - rdhand;
		SetActorLocation(rdhand);
		SetActorRotation(direction.Rotation());

		FVector scale = GetActorScale3D();
		scale.Z = direction.Size() / 100.0f;
		SetActorScale3D(scale);
	}
}

