// Golem Project - Créajeux 2020

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RaycastingComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GOLEMPROJECT_API URaycastingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URaycastingComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY()
	class USkeletalMeshComponent* MeshComponent;

	class AGolemProjectCharacter* Character;

	class UHealthComponent* HealthComponent;

	//IK foot
	float CalculateOffsetIK(FName _boneName, FName _boneName2, FVector& _floorNormal);
	FRotator CalculateRotationOffsetFoot(FVector _floorNormal);

	UPROPERTY(EditAnywhere)
	FName LeftFootName = FName(TEXT("foot_l"));

	UPROPERTY(EditAnywhere)
	FName LeftFoot2Name = FName(TEXT("ball_l"));

	UPROPERTY(EditAnywhere)
	FName RightFootName = FName(TEXT("foot_r"));

	UPROPERTY(EditAnywhere)
	FName RightFoot2Name = FName(TEXT("ball_r"));

	UPROPERTY(EditAnywhere)
	float MaxDistanceBelowCapsule = 50.0f;

	UPROPERTY(EditAnywhere)
	float InterpSpeed = 15.0f;

	FVector BoneLocation;

	UPROPERTY(BlueprintReadOnly)
	float LeftFootOffset;
	UPROPERTY(BlueprintReadOnly)
	float RightFootOffset;
	UPROPERTY(BlueprintReadOnly)
	float PelvisOffset;
	UPROPERTY(BlueprintReadOnly)
	FRotator LeftFootRotationOffset;
	UPROPERTY(BlueprintReadOnly)
	FRotator RightFootRotationOffset;

	UPROPERTY(BlueprintReadOnly)
	bool IsGroundFlat;

	FVector FloorNormalLeft;
	FVector FloorNormalRight;

	//calculate respawn position
	int32 indexFootR;
	int32 indexFootL;
	bool FootRGrounded;
	bool FootLGrounded;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
