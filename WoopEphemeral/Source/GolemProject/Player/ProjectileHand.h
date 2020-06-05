// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ProjectileHand.generated.h"

UCLASS()
class GOLEMPROJECT_API AProjectileHand : public AActor
{
	GENERATED_BODY()
private:
	FVector direction;
	class UStaticMeshComponent* meshComponent;
	class UGrappleComponent* grappleComponent;
	
	bool bIsColliding;
	bool bIsComingBack;
	bool bIsGrapplingPossible;
	bool bIsSwingingPossible;
	bool bIsAssisted;

	UPROPERTY(EditAnywhere)
	float MaxSpeed;

	UPROPERTY(EditAnywhere, Category = "physics", meta = (AllowPrivateAccess = "true"))
	float velocity = 3000.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Fist, meta = (AllowPrivateAccess = "true"))
	class UProjectileMovementComponent* ProjectileComponent;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);
public:
	// Sets default values for this actor's properties
	AProjectileHand();

	FORCEINLINE	void SetColliding(const bool& _isColliding) { bIsGrapplingPossible = _isColliding; };
	
	FORCEINLINE const bool& IsCollidingGrappling() { return bIsGrapplingPossible; };

	FORCEINLINE const bool& IsCollidingSwinging() { return bIsSwingingPossible; };

	FORCEINLINE void SetCollidingSwinging(const bool& _enable) { bIsSwingingPossible = _enable; };

	FORCEINLINE const FVector GetLocation() { return GetMeshComponent()->GetComponentLocation(); };

	void SetComingBack(const bool& _isComingBack);

	FORCEINLINE const bool& IsComingBack() { return bIsComingBack; };

	FORCEINLINE void SetAssisted(const bool& _isAssisted) { bIsAssisted = _isAssisted; }

	UFUNCTION()
	void LaunchProjectile(const FVector& _direction, UGrappleComponent* _grapple);

	UFUNCTION()
	void DestroyProjectile();

	UFUNCTION()
	FORCEINLINE class UStaticMeshComponent* GetMeshComponent() { return meshComponent; };

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Impact Action Event"), Category = Events)
	void ImpactEvent();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
