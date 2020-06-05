// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class GOLEMPROJECT_API AProjectile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProjectile(const FObjectInitializer& OI);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
		int damage;
	UPROPERTY(EditAnywhere)
		class UProjectileMovementComponent* projectileMovement;

	UPROPERTY(BlueprintReadOnly, Category = "Projectile")
		AActor* launcher;
	UFUNCTION()
		void OverlapDamage(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:

	UFUNCTION(BlueprintNativeEvent, Category = "Projectile")
		void LaunchProjectile(AActor* _launcher, FVector velocity, int _damage, float gravityScale = 0.f);

	virtual void LaunchProjectile_Implementation(AActor* _launcher, FVector velocity, int _damage, float gravityScale = 0.f);


};
