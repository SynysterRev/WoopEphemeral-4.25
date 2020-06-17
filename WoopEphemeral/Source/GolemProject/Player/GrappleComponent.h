// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GrappleComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCanHitSomethingGrappable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNothingGrapplableHit);
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GOLEMPROJECT_API UGrappleComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	class UWorld* world;
	class ACharacterControllerFPS* mCharacter;
	class AProjectileHand* currentProjectile;
	class AActor* ClosestGrapplingHook;
	class AActor* LastClosestGrapplingHook;
	class AGolemProjectGameMode* GameMode;
	class APlayerCameraManager* PlayerCameraManager;
	class USwingPhysic* swingPhysic = nullptr;
	class ARope* rope = nullptr;
	bool HasCreatedTarget;

	FVector mDestination;
	FVector mDirection;
	FVector mLastLocation;
	FTransform mSpawningTransform;

	int32 mIdBone;
	bool bIsAssisted = false;
	bool bIsClimbing = false;
	bool bIsAttracting = false;
	bool bDestroyCustomPhy = false;
	bool bCanMove = true;
	bool IsFiring = false;

	//values to edit
	UPROPERTY(EditAnywhere, Category = "Swing Physics", meta = (AllowPrivateAccess = "true"))
	float minDistanceSwinging = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Swing physics", meta = (AllowPrivateAccess = "true"))
	float maxDistanceSwinging = 1650.0f;

	UPROPERTY(EditAnywhere, Category = "Swing Physics", meta = (AllowPrivateAccess = "true"))
	float scaleGravity = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Swing Physics", meta = (AllowPrivateAccess = "true"))
	float friction = 0.9998f;

	UPROPERTY(EditAnywhere, Category = "Swing Physics", meta = (AllowPrivateAccess = "true"))
	float forceMovement = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Swing Physics", meta = (AllowPrivateAccess = "true"))
	float speedRotation = 0.06f;

	UPROPERTY(EditAnywhere, Category = "Swing Physics", meta = (AllowPrivateAccess = "true"))
	float releaseForce = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Swing Physics", meta = (AllowPrivateAccess = "true"))
	float reduceRopeSpeed = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Swing Physics", meta = (AllowPrivateAccess = "true"))
	float frameRateMin = 30.0f;
	
	UPROPERTY(EditAnywhere, Category = "Swing Physics", meta = (AllowPrivateAccess = "true"))
	float radiusOnGround = 17.0f;
	
	UPROPERTY(EditAnywhere, Category = "Swing Render", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ARope> ropeClass;

	float mDistance;
	
protected:
	UPROPERTY(EditAnywhere, Category = "projectile", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AProjectileHand> handProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grapple physics", meta = (AllowPrivateAccess = "true"))
	float maxDistanceGrappling = 3000.0f;

	UPROPERTY(EditAnywhere, Category = "Grapple physics", meta = (AllowPrivateAccess = "true"))
	float velocity = 5000.0f;

	UPROPERTY(EditAnywhere, Category = "Grapple physics", meta = (AllowPrivateAccess = "true"))
	float offsetStop = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Grapple physics", meta = (AllowPrivateAccess = "true"))
	float offsetBlockingObject = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Grapple physics", meta = (AllowPrivateAccess = "true"))
	float stopScaleVelocity = 0.0f;

	UPROPERTY(EditAnywhere, Category = "AutoGrapple", meta = (AllowPrivateAccess = "true"))
	float maxDot = 1.0f;

	UPROPERTY(EditAnywhere, Category = "AutoGrapple", meta = (AllowPrivateAccess = "true"))
	float minDot = 0.0f;

	UPROPERTY(EditAnywhere, Category = "physics", meta = (AllowPrivateAccess = "true"))
	float minDistance = 300.0f;

	// Called when the game starts
	virtual void BeginPlay() override;

	void CheckElementTargetable();

	UFUNCTION()
	void AttractCharacter();

	UFUNCTION()
	void PlayerIsNear();

	UFUNCTION()
	void CheckElementGrappable();

public:
	UPROPERTY()
	FVector IKposition;
	// Sets default values for this component's properties
	UGrappleComponent();

	UFUNCTION(BlueprintCallable)
	/** Fire the grapple if true for swinging**/
	void GoToDestination(bool _isAssisted, FTransform _spawningTransform);

	UFUNCTION()
	void Cancel();

	UFUNCTION()
	/** Return is player using his grapple**/
	FORCEINLINE bool& GetIsFiring() { return IsFiring; }

	FORCEINLINE const FVector& GetDirection() { return mDirection; };

	UFUNCTION(BlueprintCallable)
	/** Return the projectile if there'is one**/
	FORCEINLINE class AProjectileHand* GetProjectile() { return currentProjectile; };

	const FVector GetSpawningLocation();

	UFUNCTION(BlueprintCallable)
	/** Return the closest target where player can use his grapple**/
	FORCEINLINE class AActor* GetClosestGrapplingHook() { return ClosestGrapplingHook; };

	FORCEINLINE class USwingPhysic* GetSwingPhysics() { return swingPhysic; };

	UFUNCTION()
	/** Return the character**/
	FORCEINLINE class ACharacterControllerFPS* GetCharacter() { return mCharacter; };

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	//Implicit Delete SwingPhysics var 
	void StopSwingPhysics(const bool& _comingBack = true);

	void StopSwingPhysicsOnDeath();
		
	bool CheckGround(FVector _impactNormal);

	bool IsSwinging;

	void SetClimb(bool _isClimbing);

	void StopClimb();

	void SetCanMove(bool _canMove) { bCanMove = _canMove; }

	bool CanMove() { return bCanMove; }

	UPROPERTY(BlueprintAssignable)
	FCanHitSomethingGrappable OnGrappableCouldBeHit;

	UPROPERTY(BlueprintAssignable)
	FNothingGrapplableHit OnNothingGrappableCouldBeHit;
};
