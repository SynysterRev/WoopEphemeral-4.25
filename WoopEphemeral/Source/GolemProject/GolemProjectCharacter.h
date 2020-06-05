// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Player/DashComponent.h"
#include "Interfaces/Interactable.h"
#include "GolemProjectCharacter.generated.h"


USTRUCT()
struct FLaunchInfo
{
	GENERATED_BODY()
		UPROPERTY()
		FVector launchVelocity;
	bool bXYOverride;
	bool bZOverride;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStartMoving);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGetGrapple);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGetFist);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFireProjectile);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FResetProjectile);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFireGrapple);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FResetGrapple);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGrappleEquiped);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFistEquiped);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHasReachPosition);
UCLASS(config = Game)
class AGolemProjectCharacter : public ACharacter
{
	GENERATED_BODY()

private:

	TArray<FLaunchInfo> launchInfos;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Skills", meta = (AllowPrivateAccess = "true"))
		bool isGrappleSkillEnabled = false;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Skills", meta = (AllowPrivateAccess = "true"))
		bool isFistSkillEnabled = false;

	UPROPERTY()
		bool isSightCameraEnabled = false;

	float m_valueForward;

	float m_valueRight;

	bool WantToAim;

	UPROPERTY()
		class UUserWidget* currentSightWidget = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grapple Hook", meta = (AllowPrivateAccess = "true"))
		class UGrappleComponent* mGrapple;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = FistComp, meta = (AllowPrivateAccess = "true"))
		class UFistComponent* FistComp;

	UPROPERTY(EditAnywhere)
		class UCapsuleComponent* customCapsule;

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Health, meta = (AllowPrivateAccess = "true"))
		class UHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pushing, meta = (AllowPrivateAccess = "true"))
		class UPushingComponent* PushingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pushing, meta = (AllowPrivateAccess = "true"))
		class URaycastingComponent* RaycastingComponent;

	class APlayerController* pc;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pushing, meta = (AllowPrivateAccess = "true"))
		class UWallMechanicalComponent* WallMechanicalComponent;

	UPROPERTY()
		class UChildActorComponent* sightCamera;

	UPROPERTY()
		class UChildActorComponent* sightCameraL;

	/*UPROPERTY()
	class USlowMoComponent* SlowMoComponent;*/

	float initialGroundFriction;

	bool HasPressedAiming;

	bool HasAlreadyMove;

	TArray<FVector> PathToFollow;

	bool KeepControllerDisable;

	bool NeedToReachLocation;

	bool HasToRotate;

	FRotator RotationToReach;

protected:

	UPROPERTY(BlueprintReadWrite)
		AActor* actorToInteract;
	UPROPERTY(BlueprintReadWrite)
		bool pushedObjectIsCollidingForward;
	UPROPERTY(BlueprintReadWrite)
		bool pushedObjectIsCollidingBackward;

	float startPushingZ;

	UPROPERTY(BlueprintReadWrite)
		FVector rightHandPosition;
	FVector offsetRightHand;
	UPROPERTY(BlueprintReadWrite)
		FVector leftHandPosition;
	FVector offsetLeftHand;


	virtual void BeginPlay() override;
	virtual void Tick(float _deltaTime) override;

	//virtual void Tick(float _deltaTime) override;

	UFUNCTION(BlueprintCallable)
	void Fire();

	/** Called for forwards/backward input */
	UFUNCTION(BlueprintCallable)
	void MoveForward(float Value);

	UFUNCTION(BlueprintCallable)
	/** Called for side to side input */
	void MoveRight(float Value);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	UFUNCTION(BlueprintCallable)
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	UFUNCTION(BlueprintCallable)
	void LookUpAtRate(float Rate);

	UFUNCTION(BlueprintCallable)
	void ChangeCameraPressed();

	UFUNCTION(BlueprintCallable)
	void ChangeCameraReleased();

	UPROPERTY(EditAnywhere)
		UDashComponent* dashComponent;

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
		bool CanDash()
	{
		if (dashComponent == nullptr)
		{
			return false;
		}
		return dashComponent->CanDash();
	}

	void Jump() override;

	UFUNCTION(BlueprintCallable)
		void Dash();

	UFUNCTION(BlueprintCallable)
	void DashDown();

	UFUNCTION(BlueprintCallable)
		void UseAssistedGrapple();

	UFUNCTION()
		void SetUpBlockOffsetPositon();

	UFUNCTION()
		void AimAtEndOfWallJump();

	UFUNCTION(BlueprintCallable)
	void SwitchArm();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "PauseMenu"), Category = Events)
		void PauseEvent();

public:
	AGolemProjectCharacter();

	virtual void LaunchCharacter(FVector LaunchVelocity, bool bXYOverride, bool bZOverride) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hud")
		TSubclassOf<class UUserWidget>  sightHudClass;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;

	UFUNCTION(BlueprintCallable, Category = "Hud")
		FORCEINLINE bool& GetSightCameraEnabled() { return isSightCameraEnabled; };

	FORCEINLINE bool& IsGrappleSkillEnabled() { return isGrappleSkillEnabled; };

	FORCEINLINE bool& IsFistSkillEnabled() { return isFistSkillEnabled; };

	UFUNCTION(BlueprintCallable, Category = "Skill")
		void SetGrappleSkillEnabled(bool _enable) { isGrappleSkillEnabled = _enable; if (_enable)OnGetGrapple.Broadcast(); };

	UFUNCTION(BlueprintCallable, Category = "Skill")
		void SetFistSkillEnabled(bool _enable) { isFistSkillEnabled = _enable; if (_enable)OnGetFist.Broadcast(); };

	void ResetFriction();
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE class UHealthComponent* GetHealthComponent() const { return HealthComponent; }

	FORCEINLINE class URaycastingComponent* GetRaycastingComponent() const { return RaycastingComponent; }

	FORCEINLINE class UCapsuleComponent* GetCustomCapsuleComponent() const { return customCapsule; }

	FORCEINLINE class UDashComponent* GetDashComponent() const { return dashComponent; }

	UFUNCTION(BlueprintCallable)
		FVector GetVirtualRightHandPosition();

	UFUNCTION(BlueprintCallable)
		FVector GetVirtualLeftHandPosition();

	UFUNCTION(BlueprintCallable, Category = "Dash")
		FORCEINLINE bool IsDashing() { return dashComponent->IsDashing(); };

	UFUNCTION(BlueprintCallable, Category = "IK")
		void SetRightHandPosition(FVector newPos) { rightHandPosition = newPos; }
	UFUNCTION(BlueprintCallable, Category = "IK")
		void SetLeftHandPosition(FVector newPos) { leftHandPosition = newPos; }


	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Dash Start Action Event"), Category = Events)
		void DashStartEvent();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Dash End Action Event"), Category = Events)
		void DashEndEvent();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Fire Action Event"), Category = Events)
		void FireEvent();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Grappling Action Event"), Category = Events)
		void GrapplingFireEvent();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "End Grappling Action Event"), Category = Events)
		void EndGrapplingEvent();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Start Reducing Rope Action Event"), Category = Events)
		void StartReducingRopeEvent();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "End Reducing Rope Action Event"), Category = Events)
		void EndReducingRopeEvent();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Stop Reducing Rope Action Event"), Category = Events)
		void StopReducingRopeEvent();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = " Start Swing Action Event"), Category = Events)
		void StartSwingEvent();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Starting Attraction Action Event"), Category = Events)
		void StartingAttractionEvent();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Coming Back Action Event"), Category = Events)
		void ComingBackEvent();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Wall Jump Action Event"), Category = Events)
		void WallJumpEvent();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Ground Impact Action Event"), Category = Events)
		void GroundImpactEvent(const FVector& impactPoint);

	bool PushBloc(FVector pushingDirection, FVector pushingPosition, FRotator pushingRotation);

	void StopPushBloc();

	UFUNCTION(BlueprintCallable)
		void ChangeToGrapple();

	UFUNCTION(BlueprintCallable)
		void ChangeToFist();

	void InflictDamage(int _damage);

	bool IsCharacterDead();

	void ActivateDeath(bool _activate);

	UPROPERTY(BlueprintReadOnly)
		bool IsInteracting;

	UPROPERTY(BlueprintReadOnly)
		bool IsAiming;

	UFUNCTION(BlueprintImplementableEvent)
		void ActivateTargetGrapple(class AActor* _target);

	UFUNCTION(BlueprintImplementableEvent)
		void DeactivateTargetGrapple();

	UFUNCTION(BlueprintImplementableEvent)
		void Event_Death();

	UFUNCTION(BlueprintCallable)
		bool IsCharacterSwinging();

	UPROPERTY(BlueprintReadWrite)
		bool IsDashingDown;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Debug")
		bool showCursor = false;

	UFUNCTION(BlueprintCallable)
		void ShowMouseCursor(bool _showCursor) { showCursor = _showCursor; pc->bShowMouseCursor = _showCursor; }

	UPROPERTY(BlueprintAssignable)
		FStartMoving OnStartMoving;

	UPROPERTY(BlueprintAssignable)
		FGetGrapple OnGetGrapple;

	UPROPERTY(BlueprintAssignable)
		FGetFist OnGetFist;

	UPROPERTY(BlueprintAssignable)
		FFireProjectile OnFireProjectile;

	UPROPERTY(BlueprintAssignable)
		FResetProjectile OnResetProjectile;

	UPROPERTY(BlueprintAssignable)
		FFireGrapple OnFireGrapple;

	UPROPERTY(BlueprintAssignable)
		FResetGrapple OnResetGrapple;

	UPROPERTY(BlueprintAssignable)
		FGrappleEquiped OnGrappleEquiped;

	UPROPERTY(BlueprintAssignable)
		FFistEquiped OnFistEquiped;

	UPROPERTY()
		FHasReachPosition OnLocationReach;

	UFUNCTION(BlueprintCallable)
		bool CanGoToLocation(FVector _location, bool _shoulKeepControllerDisable, bool _walk, FVector _directionToWatch, bool _hasToRotate);

	UFUNCTION()
		bool GoToLocation();

	void RotateCharacter();

	UPROPERTY(BlueprintReadOnly)
		bool IsWalking;
};
