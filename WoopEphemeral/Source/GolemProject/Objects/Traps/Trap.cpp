// Fill out your copyright notice in the Description page of Project Settings.


#include "Trap.h"

// Sets default values
ATrap::ATrap()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ATrap::BeginPlay()
{
	Super::BeginPlay();
	if (alwaysActive)
	{
		Activate(nullptr);
	}
}

const bool ATrap::Activate_Implementation(AActor* caller)
{
	if (isActivate)
	{
		return false;
	}
	isActivate = true;
	return true;
}

const bool ATrap::Desactivate_Implementation(AActor* caller)
{
	if (!isActivate)
	{
		return false;
	}
	isActivate = false;
	return true;
}

const bool ATrap::Switch_Implementation(AActor* caller)
{
	isActivate = !isActivate;
	return true;
}

