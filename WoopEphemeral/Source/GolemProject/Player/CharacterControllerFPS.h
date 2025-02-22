// Golem Project - Créajeux 2020

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Player/DashComponent.h"
#include "CharacterControllerFPS.generated.h"

class UInputComponent;

//struct to call launch character on different frame
USTRUCT()
struct FLaunchInfos
{
	GENERATED_BODY()
		UPROPERTY()
		FVector launchVelocity;
	bool bXYOverride;
	bool bZOverride;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStartMove);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGetFistSkill);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFireProjectile);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FResetProjectile);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGetGrappleSkill);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFireGrapple);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FResetGrapple);
UCLASS()
class GOLEMPROJECT_API ACharacterControllerFPS : public ACharacter
{
	GENERATED_BODY()

	TArray<FLaunchInfos> launchInfos;

#pragma region Components
	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* Mesh1P;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* Weapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* MuzzleLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* MuzzleGrappleLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grapple Hook", meta = (AllowPrivateAccess = "true"))
		class UGrappleComponent* mGrapple;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pushing, meta = (AllowPrivateAccess = "true"))
		class UPushingComponent* PushingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Dash, meta = (AllowPrivateAccess = "true"))
		class UDashComponent* dashComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = FistComp, meta = (AllowPrivateAccess = "true"))
		class UFistComponent* FistComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Health, meta = (AllowPrivateAccess = "true"))
		class UHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Collision, meta = (AllowPrivateAccess = "true"))
		class UCapsuleComponent* customCapsule;	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = WallMechanical, meta = (AllowPrivateAccess = "true"))
		class UWallMechanicalComponent* WallMechanicalComponent;

#pragma endregion Components

#pragma region Variables
	float initialGroundFriction;

	float m_valueForward;

	float m_valueRight;

	bool HasAlreadyMove;

	bool isFistSkillEnabled;

	bool isGrappleSkillEnabled;

#pragma endregion Variables

public:
	ACharacterControllerFPS();

protected:

	//use to interact with interruptor or moving bloc
	UPROPERTY(BlueprintReadWrite)
		AActor* actorToInteract;
	UPROPERTY(BlueprintReadWrite)
		bool pushedObjectIsCollidingForward;
	UPROPERTY(BlueprintReadWrite)
		bool pushedObjectIsCollidingBackward;

	UPROPERTY(BlueprintReadOnly)
		bool IsInteracting;

	float startPushingZ;

	UPROPERTY(BlueprintReadWrite)
		FVector rightHandPosition;
	FVector offsetRightHand;
	UPROPERTY(BlueprintReadWrite)
		FVector leftHandPosition;
	FVector offsetLeftHand;

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	void AddControllerPitchInput(float Val) override;

	void AddControllerYawInput(float Val) override;

	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	// End of APawn interface
	virtual void BeginPlay();

	virtual void Tick(float DeltaTime);

	void Jump() override;
public:

	//variables
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;

	UPROPERTY(BlueprintReadWrite)
		bool IsDashingDown;

	//functions
	virtual void LaunchCharacter(FVector LaunchVelocity, bool bXYOverride, bool bZOverride) override;

	FTransform GetSpawningGrappleTransform() { return MuzzleGrappleLocation->GetComponentTransform(); }

#pragma region SkillsFunctions
	//dash functions
	UFUNCTION(BlueprintCallable)
	void Dash();

	UFUNCTION(BlueprintCallable)
	void DashDown();

	UFUNCTION(BlueprintCallable)
	bool CanDash()
	{
		if (dashComponent == nullptr)
		{
			return false;
		}
		return dashComponent->CanDash();
	}

	UFUNCTION(BlueprintCallable, Category = "Dash")
	/** Returns is character dashing**/
	FORCEINLINE bool IsDashing() { return dashComponent->IsDashing(); };

	UFUNCTION(BlueprintCallable, Category = "Fist")
		void Fire();

	UFUNCTION(BlueprintCallable, Category = "Grapple")
		void FireGrapple();

	UFUNCTION(BlueprintCallable, Category = "Skill")
		void SetFistSkillEnabled(bool _enable) { isFistSkillEnabled = _enable; if (_enable)OnGetFist.Broadcast(); };

	UFUNCTION(BlueprintCallable, Category = "Skill")
		void SetGrappleSkillEnabled(bool _enable) { isGrappleSkillEnabled = _enable; if (_enable)OnGetGrapple.Broadcast(); };

	UFUNCTION(BlueprintCallable, Category = "Grapple")
		void UseAssistedGrapple();

	UFUNCTION(BlueprintCallable, Category = "Grapple")
	/** Returns is character using his grapple to swing**/
		bool IsCharacterSwinging();

	UFUNCTION()
		void SetUpBlockOffsetPositon();

	//bool PushBloc(FVector pushingDirection, FVector pushingPosition, FRotator pushingRotation);

	//void StopPushBloc();

#pragma endregion SkillsFunctions

	void InflictDamage(int _damage);

	/** Returns is character dead**/
	bool IsCharacterDead();

	/** Called when the player died **/
	void ActivateDeath();

	/** reset ground friction on the player **/
	void ResetFriction();

#pragma region Delegate
	UPROPERTY(BlueprintAssignable)
	FStartMove OnStartMoving;

	UPROPERTY(BlueprintAssignable)
	FGetFistSkill OnGetFist;

	UPROPERTY(BlueprintAssignable)
	FFireProjectile OnFireProjectile;

	UPROPERTY(BlueprintAssignable)
	FResetProjectile OnResetProjectile;

	UPROPERTY(BlueprintAssignable)
		FGetGrappleSkill OnGetGrapple;

	UPROPERTY(BlueprintAssignable)
		FFireGrapple OnFireGrapple;

	UPROPERTY(BlueprintAssignable)
		FResetGrapple OnResetGrapple;
#pragma endregion Delegate

#pragma region BP_Event
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Dash Start Action Event"), Category = Events)
		void DashStartEvent();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Dash End Action Event"), Category = Events)
		void DashEndEvent();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Ground Impact Action Event"), Category = Events)
		void GroundImpactEvent(const FVector& impactPoint);

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

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Death Event"), Category = Events)
		void Event_Death();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Activate Targeting Event"), Category = Events)
		void ActivateTargetGrapple(class AActor* _target);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Deactivate Targeting Event"), Category = Events)
		void DeactivateTargetGrapple();
#pragma endregion BP_Event

#pragma region Accessors_Components_Vars
	UFUNCTION(BlueprintCallable, Category = "Mesh")
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }

	UFUNCTION(BlueprintCallable, Category = "Camera")
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	UFUNCTION(BlueprintCallable, Category = "Dash")
	/** Returns DashComponent **/
	FORCEINLINE class UDashComponent* GetDashComponent() const { return dashComponent; }

	UFUNCTION(BlueprintCallable, Category = "Fist")
	/** Returns FistComponent **/
	FORCEINLINE class UFistComponent* GetFistComponent() const { return FistComp; }

	UFUNCTION(BlueprintCallable, Category = "Health", meta = (AllowPrivateAccess = "true"))
	/** Returns HealthComponent **/
	FORCEINLINE class UHealthComponent* GetHealthComponent() const { return HealthComponent; }

	UFUNCTION(BlueprintCallable, Category = "Capsule")
	/** Returns CustomCapsule subobject **/
		FORCEINLINE class UCapsuleComponent* GetCustomCapsuleComponent() const { return customCapsule; }

	UFUNCTION(BlueprintCallable, Category = "Fist")
	/** Has player unlocked fist **/
	FORCEINLINE bool& IsFistSkillEnabled() { return isFistSkillEnabled; };

	UFUNCTION(BlueprintCallable, Category = "Grapple")
	/** Has player unlocked grapple **/
	FORCEINLINE bool& IsGrappleSkillEnabled() { return isGrappleSkillEnabled; };
#pragma endregion Accessors_Components_Vars
};
