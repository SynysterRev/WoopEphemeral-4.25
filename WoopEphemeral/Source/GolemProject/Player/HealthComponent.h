// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Helpers/HelperLibrary.h"
#include "HealthComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCharacterDie);
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GOLEMPROJECT_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Health", meta = (AllowPrivateAccess = "true"))
		int MaxLife = 1;
	UPROPERTY(EditAnywhere, Category = "Health", meta = (AllowPrivateAccess = "true"))
	int Life = 1;
	UPROPERTY(EditAnywhere, Category = "Health", meta = (AllowPrivateAccess = "true"))
	float TimerRespawn = 1.5f;

	bool CanTakeDamage;
	FTimerHandle TimerHandlerInvul;
	FTimerHandle TimerHandlerRespawn;
	class ACharacterControllerFPS* Player;
	class APlayerController* PlayerController;
	FVector PositionCheckPoint;

private:
	void Respawn();
	bool bIsDead;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		inline FVector GetPositionCheckPoint() const
	{
		return PositionCheckPoint;
	};

	/** register position of the last checkpoint enter (maybe register transform?)**/
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void SetPositionCheckPoint(FVector _positionCheckPoint);

	UFUNCTION(BlueprintCallable, Category = "Health")
		void SetLife(int _Life);

	UFUNCTION(BlueprintCallable, Category = "Health")
		inline int GetLife() const
	{
		return Life;
	};

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void InflictDamage(int _damage);
		
	UFUNCTION(BlueprintCallable, Category = "Health")
		inline bool IsDead() { return bIsDead; }

	//delegate call on death
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FCharacterDie OnCharacterDie;
};
