// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "GolemProjectGameMode.h"
#include "GolemProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

AGolemProjectGameMode::AGolemProjectGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/Player/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

}
