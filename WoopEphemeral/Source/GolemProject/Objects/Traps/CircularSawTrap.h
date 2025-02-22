// Golem Project - Créajeux 2020

#pragma once

#include "CoreMinimal.h"
#include "Objects/Traps/Trap.h"
#include "CircularSawTrap.generated.h"

/**
 *
 */
UCLASS()
class GOLEMPROJECT_API ACircularSawTrap : public ATrap
{
	GENERATED_BODY()
public:
	ACircularSawTrap();
protected:
	UPROPERTY(VisibleAnywhere)
		USceneComponent* root;
	UPROPERTY(VisibleAnywhere)
		USceneComponent* destination;
	UPROPERTY(VisibleAnywhere)
		class UStaticMeshComponent* mesh;

	UPROPERTY(EditAnywhere)
		float speed = 100.f;
	//in degree
	UPROPERTY(EditAnywhere)
		float rotationSpeed = 180.f;
	UPROPERTY(EditAnywhere)
		bool changeRotationOnChangeDirection = false;


	short dir = 1;


	FVector startPos;
	FVector destinationPos;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
		void OverlapActivation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
