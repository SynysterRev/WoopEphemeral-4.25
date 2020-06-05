// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Activable.h"
#include "PressurePlate.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPresurePlate);

UCLASS()
class GOLEMPROJECT_API APressurePlate : public AActor
{
	GENERATED_BODY()


protected:

	UPROPERTY(EditInstanceOnly)
		TArray<AActor*> objectsToActivate;
	UPROPERTY(EditInstanceOnly)
		EActivationType activationTypeOnPress = EActivationType::Activate;
	UPROPERTY(EditInstanceOnly)
		EActivationType activationTypeOnRelease = EActivationType::Desactivate;

	UPROPERTY(VisibleAnywhere)
		class UBoxComponent* boxCollider = nullptr;
	UPROPERTY(VisibleAnywhere)
		class UStaticMeshComponent* mesh = nullptr;


	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
		bool isPressed = false;
	short countObjectOn = 0;

	UPROPERTY(EditAnywhere)
		float offsetWhenPresed = -10.f;
	UPROPERTY(EditAnywhere)
		float timeToLerp = 0.5f;

	float timerLerp = 0.f;

	FVector startPos;
	FVector pressedPos;
	bool isMoving = false;
public:
	APressurePlate();

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FPresurePlate OnPressedPlate;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FPresurePlate OnReleasedPlate;

protected:
	UFUNCTION()
		void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
		void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void ActivateObjects(EActivationType activationType);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

};
