// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GolemProjectGameMode.generated.h"

UCLASS(minimalapi)
class AGolemProjectGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGolemProjectGameMode();

	inline TArray<AActor*> GetActorsTargetable() const
	{
		return ActorsTargetable;
	}

protected:
	UPROPERTY(BlueprintReadWrite)
		TArray<AActor*> ActorsTargetable;
};



