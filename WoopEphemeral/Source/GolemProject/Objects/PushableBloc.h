// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interactable.h"
#include "PushableBloc.generated.h"

class AGolemProjectCharacter;

UCLASS()
class GOLEMPROJECT_API APushableBloc : public AActor, public IInteractable
{
	GENERATED_BODY()


protected:
	virtual void BeginPlay() override;
	virtual void Tick(float deltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		class UStaticMeshComponent* mesh;

	AGolemProjectCharacter* playerActor = nullptr;
	bool useGravity = true;
	UPROPERTY(BlueprintReadWrite)
		bool isUsed = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector sizeBlock = FVector::OneVector;
	UPROPERTY(BlueprintReadWrite)
		FVector pushingDirection = FVector::ZeroVector;
	UPROPERTY(BlueprintReadWrite)
		FVector pushingPosition = FVector::ZeroVector;
	UPROPERTY(BlueprintReadWrite)
		FRotator pushingRotation = FRotator::ZeroRotator;
	UPROPERTY(BlueprintReadWrite)
		FVector pushingRightHandPosition = FVector::ZeroVector;
	UPROPERTY(BlueprintReadWrite)
		FVector pushingLeftHandPosition = FVector::ZeroVector;
public:
	APushableBloc();

	UFUNCTION(BlueprintCallable)
		FVector GetSizeBlock() { return sizeBlock; }

	UFUNCTION(BlueprintCallable)
		FVector GetPushingDirection() { return pushingDirection; }
	UFUNCTION(BlueprintCallable)
		FVector GetPushingPosition() { return pushingPosition; }
	UFUNCTION(BlueprintCallable)
		FRotator GetPushingRotation() { return pushingRotation; }


	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable")
		const bool Interact(AActor* caller);
	virtual const bool Interact_Implementation(AActor* caller) override;

};
