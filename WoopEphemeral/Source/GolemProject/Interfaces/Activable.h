// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Activable.generated.h"

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class EActivationType : uint8
{
	Activate		UMETA(DisplayName = "Activate"),
	Desactivate 	UMETA(DisplayName = "Desactivate"),
	Switch			UMETA(DisplayName = "Switch"),
	None			UMETA(DisplayName = "None")
};

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UActivable : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class GOLEMPROJECT_API IActivable
{
	GENERATED_BODY()

		// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Activable")
		const bool Activate(AActor* caller);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Activable")
		const bool Desactivate(AActor* caller);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Activable")
		const bool Switch(AActor* caller);
};
