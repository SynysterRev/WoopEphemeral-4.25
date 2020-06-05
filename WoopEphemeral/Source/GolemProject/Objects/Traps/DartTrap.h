// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/Traps/Trap.h"
#include "DartTrap.generated.h"

/**
 *
 */
UCLASS()
class GOLEMPROJECT_API ADartTrap : public ATrap
{
	GENERATED_BODY()

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "DartTrap")
		float cooldown = 1.f;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "DartTrap")
		float speedProjectile = 100.f;

	//à changer quand les projectiles auront été créés
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DartTrap")
		TSubclassOf<class AProjectile> projectilePrefab = nullptr;
	float timerCooldown = 0.f;
public:
	virtual void Tick(float DeltaTime) override;
};
