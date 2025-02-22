// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FistComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCanHitSomethingInteractable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNothingInteractableHit);
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GOLEMPROJECT_API UFistComponent : public UActorComponent
{
	GENERATED_BODY()

	class UWorld* world;
	class AFistProjectile* currentProjectile;
	class UCameraComponent* mCamera;
	class ACharacterControllerFPS* mCharacter;
	FTimerHandle TimerHandleFire;
	FVector mDirection;
	bool CanFire;

private :
	void ResetFire();

	void CheckElementInteractableHitable();

public:	
	// Sets default values for this component's properties
	UFistComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = Projectile, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AFistProjectile> fistProjectileClass;

	UPROPERTY(EditAnywhere, Category = Swing)
	float pushingForce = 0.0f;

	UPROPERTY(EditAnywhere)
	float TimerFire = 1.0f;

	UPROPERTY(EditAnywhere)
	int32 NumberBounce;

	UPROPERTY(EditAnywhere, Category = "physics", meta = (AllowPrivateAccess = "true"))
	float maxDistance = 3000.0f;

	UPROPERTY(EditAnywhere, Category = "physics", meta = (AllowPrivateAccess = "true"))
	float speedMax = 5000.0f;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	/** Fire the projectile**/
	void GoToDestination(FTransform _spawningTransform);

	FORCEINLINE const FVector& GetDirection() { return mDirection; };

	/** returns the instance of the current projectile**/
	FORCEINLINE class AFistProjectile* GetProjectile() { return currentProjectile; };

	UPROPERTY(BlueprintAssignable)
	FCanHitSomethingInteractable OnInteractableCouldBeHit;

	UPROPERTY(BlueprintAssignable)
	FNothingInteractableHit OnNothingInteractableCouldBeHit;

};
