// Fill out your copyright notice in the Description page of Project Settings.


#include "DartTrap.h"
#include "Engine/World.h"
#include "Objects/Projectile.h"
#include "Helpers/HelperLibrary.h"

void ADartTrap::BeginPlay()
{
	Super::BeginPlay();
	timerCooldown = cooldown;
}

void ADartTrap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!isActivate)
	{
		return;
	}
	timerCooldown -= DeltaTime;
	if (timerCooldown <= 0.f)
	{
		timerCooldown = cooldown;
		if (UWorld* world = GetWorld())
		{
			if (projectilePrefab == nullptr)
			{
				HelperLibrary::Print("You forgot to set the projectile on " + GetName(), 5.f, FColor::Red);
				return;
			}
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			if (AProjectile* projectile = world->SpawnActorDeferred<AProjectile>(projectilePrefab, GetTransform()))
			{
				projectile->LaunchProjectile(this, GetActorForwardVector() * speedProjectile, damage);
				projectile->FinishSpawning(GetTransform());
			}
		}
	}
}