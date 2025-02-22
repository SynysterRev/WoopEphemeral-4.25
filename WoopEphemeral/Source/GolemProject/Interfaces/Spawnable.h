// Golem Project - Créajeux 2020

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Spawnable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USpawnable : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class GOLEMPROJECT_API ISpawnable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Spawnable")
		void SetSpawner(AActor* _spawner);
};
