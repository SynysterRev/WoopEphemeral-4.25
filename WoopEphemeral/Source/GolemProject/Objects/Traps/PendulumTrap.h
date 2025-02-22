// Golem Project - Créajeux 2020

#pragma once

#include "CoreMinimal.h"
#include "Objects/Traps/Trap.h"
#include "PendulumTrap.generated.h"

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPendulum);

UCLASS()
class GOLEMPROJECT_API APendulumTrap : public ATrap
{

	GENERATED_BODY()

		APendulumTrap();

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PendulumTrap")
		float timePerCycle = 1.f;
	//in degree
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PendulumTrap")
		float limitAngle = 90.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PendulumTrap")
		float ejectionMultiplier = 10.f;
	//-1 for infinite cycle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PendulumTrap")
		int nbCycleBeforeStop = -1;

	int actualCycle;
	float balanceFactor = 1.f;
	float lastTime;
	float time = 0.f;
	UPROPERTY(VisibleAnywhere, blueprintReadOnly)
		USceneComponent* root;
	UPROPERTY(VisibleAnywhere)
		USceneComponent* pivot;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		class UStaticMeshComponent* handle;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		class UStaticMeshComponent* blade;

	FVector bladeVelocity;
	FVector lastBladePosition;


	virtual void BeginPlay() override;
	virtual void Tick(float dt) override;
	UFUNCTION()
		void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual const bool Activate_Implementation(AActor* caller) override;
	virtual const bool Switch_Implementation(AActor* caller) override;

public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FPendulum OnStartCycle;
};
