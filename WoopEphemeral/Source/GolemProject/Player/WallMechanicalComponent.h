// Golem Project - Créajeux 2020

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WallMechanicalComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEndJump);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GOLEMPROJECT_API UWallMechanicalComponent : public UActorComponent
{
	GENERATED_BODY()
private:
	class UWorld* world;
	class ACharacterControllerFPS* mCharacter;
	TArray<class AActor*> ActorsToIgnore;
	FVector DirectionJump;
public:	
	// Sets default values for this component's properties
	UWallMechanicalComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	float DistanceFromWallToJump = 50.0f;

	UPROPERTY(EditAnywhere)
	float HeightJump = 1.0f;

	UPROPERTY(EditAnywhere)
	float ForceJump = 700.0f;

	UPROPERTY(EditAnywhere)
	float DistanceBehind = 299.0f;

	UPROPERTY(EditAnywhere)
	float CDWallJump = 0.3f;

	float timerWallJump;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	bool WallJump();

	UFUNCTION(BlueprintCallable)
	void ClearWallToIgnore();

	UPROPERTY(BlueprintReadOnly)
	bool IsWallJumping;

	UPROPERTY(BlueprintReadOnly)
	bool CanWallJump;

	UPROPERTY()
	FEndJump EndJump;
};
