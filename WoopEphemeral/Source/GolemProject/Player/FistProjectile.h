// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FistProjectile.generated.h"

UCLASS()
class GOLEMPROJECT_API AFistProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFistProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY()
	float Speed;

	UPROPERTY(EditAnywhere, Category = Destroy, meta = (AllowPrivateAccess = "true"))
	float TimerDisappear = 2.0f;

	UPROPERTY(EditAnywhere)
	FName BoucingTag;

	UPROPERTY(BlueprintReadOnly)
	class AGolemProjectCharacter* character;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(Blueprintcallable)
	FORCEINLINE class UProjectileMovementComponent* GetProjectileComponent() { return ProjectileComponent; };
	
	UFUNCTION(Blueprintcallable)
	FORCEINLINE class UStaticMeshComponent* GetMeshComponent() { return MeshComponent; };

	UFUNCTION(Blueprintcallable)
	void LaunchFist(const FVector& _direction, bool _shouldBounce, float _maxDistance, float _maxSpeed, class AGolemProjectCharacter* _character);

	UFUNCTION(BlueprintImplementableEvent)
	void Event_DestructionFistFX_BP();

	UFUNCTION()
	void ActivateFX();

	UFUNCTION(Blueprintcallable)
	const float GetRemainingTimeBeforeDestroy();

	UFUNCTION()
	void DestroyFist();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Fist, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* MeshComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Fist, meta = (AllowPrivateAccess = "true"))
	class UProjectileMovementComponent* ProjectileComponent;
	FVector Direction;
	FVector lastPosition;
	float maxDistance;
	float distanceTravelled;
	void BounceMovement(FVector _normal);

	FTimerHandle TimerHandleDisappear;
	FTimerHandle TimerHandleFXDisappear;
	bool bHasStopped;
};
