// Golem Project - Créajeux 2020


#include "WallMechanicalComponent.h"
#include "Engine/World.h"
#include "Player/CharacterControllerFPS.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Helpers/HelperLibrary.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UWallMechanicalComponent::UWallMechanicalComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UWallMechanicalComponent::BeginPlay()
{
	Super::BeginPlay();
	AActor* owner = GetOwner();
	mCharacter = Cast<ACharacterControllerFPS>(owner);
	world = GetWorld();
	IsWallJumping = false;
	CanWallJump = true;
	timerWallJump = CDWallJump;
}


// Called every frame
void UWallMechanicalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (timerWallJump > 0.0f)
		timerWallJump -= DeltaTime;
}

bool UWallMechanicalComponent::WallJump()
{
	if (world && mCharacter && timerWallJump <= 0.f)
	{
		FHitResult hitResult;
		FHitResult hitResult2;
		FVector forward = mCharacter->GetActorForwardVector();
		//ray all around player to see if there's a wall close enough
		for (float i = 0.0f; i <= 360.0f; i += 10.0f)
		{
			FVector dir = UKismetMathLibrary::RotateAngleAxis(forward, i, FVector::UpVector);
			FVector start(mCharacter->GetActorLocation() - mCharacter->GetActorUpVector() * 96.0f);
			FVector end(start + dir * DistanceFromWallToJump);
			if (world->LineTraceSingleByChannel(hitResult, start, end, ECC_Visibility))
			{
				//avoid to walljump on the same wall
				if (ActorsToIgnore.Contains(hitResult.GetActor())) return false;
				FVector endBehind(hitResult.ImpactPoint + hitResult.Normal * DistanceBehind);
				//check if there's enough place to walljump
				if (!world->LineTraceSingleByChannel(hitResult2, hitResult.ImpactPoint, endBehind, ECC_Visibility))
				{
					ActorsToIgnore.Empty();
					ActorsToIgnore.Add(hitResult.GetActor());
					DirectionJump = hitResult.Normal;
					DirectionJump.Z = HeightJump;
					IsWallJumping = true;
					//maybe reduce or cap velocity to a maximum ?
					mCharacter->LaunchCharacter(DirectionJump * ForceJump, false, true);
					timerWallJump = CDWallJump;
					return true;
				}
			}
		}
	}
	return false;
}

void UWallMechanicalComponent::ClearWallToIgnore()
{
	ActorsToIgnore.Empty();
}

