// Golem Project - Créajeux 2020

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Activable.h"
#include "ObjectActivator.generated.h"

USTRUCT()
struct FActivatorInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		AActor* actor;
	UPROPERTY(EditAnywhere)
		bool baseState;
	UPROPERTY(EditAnywhere)
		bool neededState;
};

UCLASS()
class GOLEMPROJECT_API AObjectActivator : public AActor, public IActivable
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AObjectActivator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditInstanceOnly)
		TArray<FActivatorInfo> activators;
	UPROPERTY(EditInstanceOnly)
		TArray<AActor*> objectsToActivate;

	TMap<AActor*, bool> statesObjects;

	const bool HaveErrorOnCaller(AActor* caller);
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool IsActivated();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Activable")
		const bool Activate(AActor* caller);
	virtual const bool Activate_Implementation(AActor* caller) override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Activable")
		bool Desactivate(AActor* caller);
	virtual const bool Desactivate_Implementation(AActor* caller) override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Activable")
		bool Switch(AActor* caller);
	virtual const bool Switch_Implementation(AActor* caller) override;
};
