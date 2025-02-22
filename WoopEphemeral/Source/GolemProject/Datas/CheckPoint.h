// Golem Project - Créajeux 2020

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CheckPoint.generated.h"

UCLASS()
class GOLEMPROJECT_API ACheckPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACheckPoint();

	UFUNCTION()
	void BeginOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Save", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* TriggerSave;

private:
	bool HasAlreadyBeenUsed;


};
