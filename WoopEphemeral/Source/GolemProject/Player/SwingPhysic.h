// Golem Project - Créajeux 2020

#pragma once

class GOLEMPROJECT_API USwingPhysic 
{
private:
	class ACharacterControllerFPS* character;
	class UCharacterMovementComponent* characterMovement;
	class AActor* target;
	class UGrappleComponent* grapple;
	class UCableComponent* cable;
	class UWorld* world;

	FVector velocity;
	FVector lastLocation;
	FVector lastVelocity;
	FVector acceleration;

	FVector newLocation;
	FVector segment;
	FVector direction;
	FVector movement;
	FVector cameraDirection;
	
	float length;
	float minLength = 200.0f;
	float maxLength = 1000.0f;
	float dist;
	float diff;
	float percent;

	float scaleGravity = 3.0f;
	float friction = 0.9998f;
	float forceMovement = 5.0f;
	float speedRotation = 0.01f;
	float releaseForce = 1.0f;
	float reduceRopeSpeed = 300.0f;
	float frameRateMin = 30.0f;

	bool bIsFistActive = false;
	bool bWasStopped = false;

public:
	USwingPhysic(class UGrappleComponent* _grappleHook);
	USwingPhysic();
	~USwingPhysic();

	void SetTarget(AActor*& _target) { target = _target; };
	AActor*& GetTarget() { return target; };
	FVector GetVelocity() { return velocity; };
	FVector GetDirection() { return direction; };
	void AddForceMovement(FVector _direction);
	void SetCameraDirection(FVector _direction);
	void InvertVelocity(FVector _hitNormal);
	void Tick(float _deltaTime);

	void SetScaleGravity(float _scaleGravity) { scaleGravity = _scaleGravity; };
	void SetFriction(float _friction) { friction = _friction; };
	void SetReduceRopeSpeed(float _reduceRopeSpeed) { reduceRopeSpeed = _reduceRopeSpeed; };
	void SetForceMovement(float _forceMovement) { forceMovement = _forceMovement; };
	void SetSpeedRotation(float _speedRotation) { speedRotation = _speedRotation; };
	void SetMinLength(float _minLength) { minLength = _minLength; };
	void SetMaxLength(float _maxLength) { maxLength = _maxLength; };
	void SetReleaseForce(float _releaseForce) { releaseForce = _releaseForce; };
	void SetFrameRateMin(float _frameRateMin) { frameRateMin = _frameRateMin; };
	void ReduceRope();
	bool WasStopped()
	{
		return bWasStopped;
	}
};
