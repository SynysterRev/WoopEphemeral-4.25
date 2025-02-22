// Golem Project - Créajeux 2020


#include "PortalLevel.h"
#include "Player/CharacterControllerFPS.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Helpers/HelperLibrary.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
APortalLevel::APortalLevel()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>("Root");
	RootComponent = Root;
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	MeshComponent->SetupAttachment(Root);
	BoxDetectionCloseEnough = CreateDefaultSubobject<UBoxComponent>("TriggerDetection");
	BoxDetectionCloseEnough->SetupAttachment(MeshComponent);
	ColliderPortal = CreateDefaultSubobject<UStaticMeshComponent>("ColliderPortal");
	ColliderPortal->SetupAttachment(MeshComponent);
	CenterToReach = CreateDefaultSubobject<USceneComponent>("PointToReach");
	CenterToReach->SetupAttachment(MeshComponent);
}

// Called when the game starts or when spawned
void APortalLevel::BeginPlay()
{
	Super::BeginPlay();
	BoxDetectionCloseEnough->OnComponentBeginOverlap.AddDynamic(this, &APortalLevel::OnBeginOverlapDetection);
	BoxDetectionCloseEnough->OnComponentEndOverlap.AddDynamic(this, &APortalLevel::OnEndOverlapDetection);
}

void APortalLevel::OnBeginOverlapDetection(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OverlappedComponent != nullptr && OtherActor != nullptr && OtherComp != nullptr)
	{
		if (ACharacterControllerFPS* character = Cast<ACharacterControllerFPS>(OtherActor))
		{
			IsPlayerCloseEnough = true;
			characterTarget = character;
			IsCharacterCloseEnoughEvent(true);
		}
	}
}

void APortalLevel::OnEndOverlapDetection(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OverlappedComp != nullptr && OtherActor != nullptr && OtherComp != nullptr)
	{
		if (ACharacterControllerFPS* character = Cast<ACharacterControllerFPS>(OtherActor))
		{
			IsPlayerCloseEnough = false;
			characterTarget = nullptr;
			IsCharacterCloseEnoughEvent(false);
		}
	}
}

// Called every frame
void APortalLevel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (IsPlayerCloseEnough)
	{
		float distance = FVector::Dist(characterTarget->GetActorLocation(), CenterToReach->GetComponentLocation());
		if (distance < 85.0f && IsLevelUnlocked())
		{
			CharacterTriggeredPortalEvent();
		}
	}
}

const bool APortalLevel::Interact_Implementation(AActor* caller)
{
	/*if (ACharacterControllerFPS* character = Cast<ACharacterControllerFPS>(caller))
	{
		if (IsLevelUnlocked())
		{
			CharacterTriggeredPortalEvent();
			return true;
		}
	}*/
	return false;
}

