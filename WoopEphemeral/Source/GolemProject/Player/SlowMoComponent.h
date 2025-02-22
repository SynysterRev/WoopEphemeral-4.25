// Golem Project - Créajeux 2020

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SlowMoComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GOLEMPROJECT_API USlowMoComponent : public UActorComponent
{
	GENERATED_BODY()

private:	
	bool bEnable = false;
	float currentTime = 0.0f;
	FTimerHandle handleTimer;
protected:
	UPROPERTY(EditAnywhere)
	float coolDown = 0.5f;

	UPROPERTY(EditAnywhere)
	float slowTime = 0.2f;

	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	USlowMoComponent();

	void SetEnableSlowMo();
	void SetDisableSlowMo();
};
